//
// Created by xmly on 2020/2/10.
//

#include <iostream>
#include "PCMToMp3Decoder.h"

void PCMToMp3Decoder::decodePCMToMp3(char *input, char *output) {
    AVFormatContext *pFormatCtx = nullptr;
    int result = avformat_alloc_output_context2(&pFormatCtx, nullptr, nullptr, output);
    if (result < 0) {
        return;
    }
    //获取编码器
    AVCodec *codec = avcodec_find_encoder(pFormatCtx->oformat->audio_codec);
    if (codec == nullptr) {
        return;
    }
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    codecCtx->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    codecCtx->bit_rate = 64000;
    codecCtx->sample_rate = 44100;
    codecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    codecCtx->channels = av_get_channel_layout_nb_channels(codecCtx->channel_layout);
    if (pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    // 打开编码器
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        cout << ("codec open failed.") << endl;
    }
    AVStream *audio_stream = avformat_new_stream(pFormatCtx, codec);
    audio_stream->id = pFormatCtx->nb_streams - 1;
    audio_stream->time_base = AVRational{1, codecCtx->sample_rate};
    //这步一定要
    if (avcodec_parameters_from_context(audio_stream->codecpar, codecCtx) < 0) {
        cout << ("copy params failed.") << endl;
    }
    if (avio_open(&pFormatCtx->pb, output, AVIO_FLAG_WRITE) < 0) {
        cout << ("open dst file failed.") << endl;
    }
    if (avformat_write_header(pFormatCtx, nullptr) < 0) {
        cout << ("write header failed.") << endl;
    }
    //解压缩数据
    AVFrame *audio_frame = av_frame_alloc();
    audio_frame->format = codecCtx->sample_fmt;
    audio_frame->channel_layout = codecCtx->channel_layout;
    audio_frame->nb_samples = codecCtx->frame_size;
    audio_frame->sample_rate = codecCtx->sample_rate;
    if (av_frame_get_buffer(audio_frame, 0) < 0) {
        cout << ("audio frame get buffer failed.") << endl;
    }
    AVFrame *buf_frame = av_frame_alloc();
    buf_frame->format = PCM_IN_FORMAT;
    buf_frame->nb_samples = audio_frame->nb_samples;
    buf_frame->channel_layout = (uint64_t) av_get_default_channel_layout(PCM_IN_CHANNELS);
    buf_frame->sample_rate = PCM_IN_SAMPLE_RATE;
    // 给frame的data和size分配空间
    if (av_frame_get_buffer(buf_frame, 0) < 0) {
        cout << ("allocate frame data failed.") << endl;
    }
    // 从pcm文件中读取适应音频帧的尺寸数据
    auto readSize = av_samples_get_buffer_size(nullptr, buf_frame->channels, buf_frame->nb_samples,
                                               (AVSampleFormat) buf_frame->format, 1);
    uint8_t *buf = (uint8_t *) av_malloc((size_t) readSize);
    SwrContext *swr_ctx = swr_alloc();
    swr_alloc_set_opts(swr_ctx,
                       audio_frame->channel_layout,
                       (AVSampleFormat) audio_frame->format,
                       audio_frame->sample_rate,
                       av_get_default_channel_layout(PCM_IN_CHANNELS),
                       PCM_IN_FORMAT,
                       PCM_IN_SAMPLE_RATE,
                       0, nullptr);
    swr_init(swr_ctx);
    // 申请一个packet，并初始化
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    FILE *inputData = nullptr;
    if ((inputData = fopen(input, "rb")) == nullptr) {
        cout << ("no readable file.") << endl;
    }
    // 循环读取frame数据
    int audio_pts = 0;
    while (true) {
        // 用来编码的帧
        AVFrame *encode_frame = nullptr;
        if (fread(buf, 1, (size_t) readSize, inputData) < 0) {
            cout << ("read input file failed.") << endl;
        } else if (!feof(inputData)) {
            // 文件没有到结尾，则获取编码帧
            av_samples_fill_arrays(buf_frame->data, buf_frame->linesize, (const uint8_t *) buf, buf_frame->channels,
                                   buf_frame->nb_samples, (AVSampleFormat) buf_frame->format, 1);
            swr_convert(swr_ctx, audio_frame->data, audio_frame->nb_samples, (const uint8_t **) buf_frame->data,
                        buf_frame->nb_samples);
            audio_frame->pts = audio_pts;
            audio_pts += av_rescale_q(audio_frame->nb_samples, AVRational{1, codecCtx->frame_size},
                                      codecCtx->time_base);
            encode_frame = audio_frame;
        } else {
            // 文件结束了，则发送一个空指针的frame，用来清空缓冲区
            encode_frame = nullptr;
        }
        // 发送一个frame
        if (avcodec_send_frame(codecCtx, encode_frame) < 0) {
            cout << ("send frame exception.") << endl;
        }
        while (true) {
            auto packet_ret = avcodec_receive_packet(codecCtx, &pkt);
            // 判断是否完全接受了packet
            if (packet_ret == AVERROR(EAGAIN) || packet_ret == AVERROR_EOF) {
                break;
            }
            // 检查是否接受异常
            if (packet_ret < 0) {
                cout << ("receive packet exception.") << endl;
            }
            av_packet_rescale_ts(&pkt, codecCtx->time_base, audio_stream->time_base);
            pkt.stream_index = audio_stream->index;
            av_interleaved_write_frame(pFormatCtx, &pkt);
            av_packet_unref(&pkt);
            // 编码帧为空，则表示已经处理完所有的编码，退出该循环
        }
        if (encode_frame == nullptr) {
            break;
        }
    }
    av_write_trailer(pFormatCtx);
}

int main() {
    char *input = "../res/audio.pcm";
    char *output = "../res/audio.mp3";
    PCMToMp3Decoder *pPCMToMp3Decoder = new PCMToMp3Decoder;
    pPCMToMp3Decoder->decodePCMToMp3(input, output);
    delete pPCMToMp3Decoder;
}