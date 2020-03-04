//
// Created by xmly on 2020/2/9.
//

#include "AACToPCMDecoder.h"

void PCMToAACDecoder::musicPlayer(const char *input, const char *output) {
    av_register_all();
    avformat_network_init();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //打开音频文件
    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0) {
        cout << ("%s", "无法打开音频文件") << endl;
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        cout << ("%s", "无法获取输入文件信息") << endl;
        return;
    }
    //获取音频流索引位置
    int i = 0;
    AVCodecParameters *pAudioCodecParmeters = NULL;
    AVRational videoAVRational = {1,50};
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            pAudioCodecParmeters=pFormatCtx->streams[i]->codecpar;
            videoAVRational = pFormatCtx->streams[i]->time_base;
            break;
        }
    }
    //获取解码器
    AVCodec *codec = avcodec_find_decoder(pAudioCodecParmeters->codec_id);
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    codecCtx->pkt_timebase=videoAVRational;
    //这步一定要
    avcodec_parameters_to_context(codecCtx, pAudioCodecParmeters);
    //打开解码器
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        cout << ("%s", "无法打开解码器") << endl;
        return;
    }
    //压缩数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrContext = swr_alloc();
    swr_alloc_set_opts(swrContext, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, codecCtx->sample_rate,
                       av_get_default_channel_layout(codecCtx->channels), codecCtx->sample_fmt,
                       codecCtx->sample_rate, 0, NULL);
    swr_init(swrContext);
    //音频格式  重采样设置参数
//    AVSampleFormat in_sample = codecCtx->sample_fmt;//原音频的采样位数
//    //输出采样格式
//    AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;//16位
//    int in_sample_rate = codecCtx->sample_rate;// 输入采样率
//    int out_sample_rate = 44100;//输出采样
//    //输入声道布局
//    uint64_t in_ch_layout = codecCtx->channel_layout;
//    //输出声道布局
//    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;//2通道 立体声
//    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample, out_sample_rate, in_ch_layout, in_sample,
//                       in_sample_rate, 0, NULL);
//    swr_init(swrContext);
//    int out_channerl_nb = av_get_channel_layout_nb_channels(out_ch_layout);
//    cout << ("声道数量%d ", out_channerl_nb) << endl;
    //设置音频缓冲区间 16bit   44100  PCM数据
//    uint8_t *out_buffer = (uint8_t *) av_malloc(2 * 44100);
    int out_buffer_size = av_samples_get_buffer_size(NULL,av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO) , 1024,
                                                     AV_SAMPLE_FMT_S16, 1);
    uint8_t* out_buffer = (uint8_t*)av_malloc(out_buffer_size);
    FILE *fp_pcm = fopen(output, "wb");//输出到文件
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == AVMEDIA_TYPE_AUDIO) {
            if (!avcodec_send_packet(codecCtx, packet)) {
                while (!avcodec_receive_frame(codecCtx, frame)) {

                    swr_convert(swrContext, &out_buffer, out_buffer_size, (const uint8_t**)frame->data, frame->nb_samples);
                    fwrite(out_buffer, 1, out_buffer_size, fp_pcm);//输出到文件
                }
            }
        }
    }
    fclose(fp_pcm);
    av_frame_free(&frame);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);
}

int main() {
    //const char *input = "http://mytianh5.oss-cn-beijing.aliyuncs.com/website/video/zaojiaoji.mp4";
    const char *input ="../res/begin.mp4";
    const char *output = "../res/audio.pcm";
    PCMToAACDecoder *pPCMToAACDecoder = new PCMToAACDecoder();
    pPCMToAACDecoder->musicPlayer(input, output);

}