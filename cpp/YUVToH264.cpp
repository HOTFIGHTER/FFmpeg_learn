//
// Created by xmly on 2020/3/1.
//

#include "YUVToH264.h"

YUVToH264::YUVToH264() {

}

YUVToH264::~YUVToH264() {

}

int YUVToH264::yuvToH264(const char *input, const char *output) {
    pAvFormatContext = avformat_alloc_context();
    pAvFormatContext->oformat = av_guess_format(NULL, output, NULL);
    if (avio_open(&pAvFormatContext->pb, output, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Failed to open output file! \n");
        return -1;
    }
    pAVStream = avformat_new_stream(pAvFormatContext, NULL);
    if (pAVStream == NULL) {
        return -1;
    }
    pAvCodecContext = pAVStream->codec;
    pAvCodecContext->codec_id = pAvFormatContext->oformat->video_codec;
    pAvCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    pAvCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    pAvCodecContext->width = 640;
    pAvCodecContext->height = 352;
    pAvCodecContext->bit_rate = 400000;
    //GOP:画面组，一组连续画面（一个完整的画面）
    pAvCodecContext->gop_size = 250;
    //设置帧率（重点）
    pAvCodecContext->time_base.num = 1;
    pAvCodecContext->time_base.den = 25;
    //设置量化参数（难点，我们一般设置默认值）
    pAvCodecContext->qmin = 10;
    pAvCodecContext->qmax = 51;
    pAvCodecContext->max_b_frames = 0;
    pAvCodec = avcodec_find_encoder(pAvCodecContext->codec_id);
    if (!pAvCodec) {
        printf("Could not find encoder\n");
        return -1;
    }
    //设置B帧最大值
    AVDictionary *param = 0;
    //H.264
    if (pAvCodecContext->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param, "preset", "slow", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
    }
    if (pAvCodecContext->codec_id == AV_CODEC_ID_H265) {
        av_dict_set(&param, "preset", "ultrafast", 0);
        av_dict_set(&param, "tune", "zero-latency", 0);
    }
    if (avcodec_open2(pAvCodecContext, pAvCodec, &param) < 0) {
        printf("Failed to open encoder! \n");
        return -1;
    }
    // 第七步：写入头文件信息
    avformat_write_header(pAvFormatContext, NULL);
    // 第八步：循环编码YUV文件为H264
    // 1、开辟缓冲区
    int buffer_size = av_image_get_buffer_size(pAvCodecContext->pix_fmt,
                                               pAvCodecContext->width,
                                               pAvCodecContext->height,
                                               1);
    int y_size = pAvCodecContext->width * pAvCodecContext->height;
    uint8_t *out_buffer = (uint8_t *)av_malloc(buffer_size);

    // 2、内存空间填充
    pAvFrame = av_frame_alloc();
    av_image_fill_arrays(pAvFrame->data,
                         pAvFrame->linesize,
                         out_buffer,
                         pAvCodecContext->pix_fmt,
                         pAvCodecContext->width,
                         pAvCodecContext->height,
                         1);

    // 3、开辟packet
    pAvPacket = (AVPacket *) av_malloc(buffer_size);
    int i = 0;
    int result = 0;
    int current_frame_index = 0;
    FILE *in_file = fopen(input, "rb");
    while (true) {
        // 从yuv文件里面读取缓冲区
        //读取大小：y_size * 3 / 2
        if (fread(out_buffer, 1, y_size * 3 / 2, in_file) <= 0) {
            break;
        } else if (feof(in_file)) {
            break;
        }
        // 将缓冲区数据转换成AVFrame类型
        //Y值
        pAvFrame->data[0] = out_buffer;
        //U值
        pAvFrame->data[1] = out_buffer + y_size;
        //V值
        pAvFrame->data[2] = out_buffer + y_size * 5 / 4;
        pAvFrame->pts = i;
        i++;
        // 第九步：视频编码处理
        // 1、发送一帧视频像素数据
        avcodec_send_frame(pAvCodecContext, pAvFrame);
        // 2、接收一帧视频压缩数据格式（像素数据编码而来）
        result = avcodec_receive_packet(pAvCodecContext, pAvPacket);
        if (result == 0) {
            // 编码成功
            // 第十步：将数据写入到输出文件
            pAvPacket->stream_index = pAVStream->index;
            result = av_write_frame(pAvFormatContext, pAvPacket);
            current_frame_index++;
            //是否输出成功
            if (result < 0) {
                return -1;
            }
        }
    }
    //第11步：写入剩余帧数据->可能没有
    flush_encoder(pAvFormatContext, 0);
    //第12步：写入文件尾部信息
    av_write_trailer(pAvFormatContext);
    //第13步：释放内存
    avcodec_close(pAvCodecContext);
    av_free(pAvFrame);
    av_free(out_buffer);
    av_packet_free(&pAvPacket);
    avio_close(pAvFormatContext->pb);
    avformat_free_context(pAvFormatContext);
    fclose(in_file);
}

int YUVToH264::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;
    while (true) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                    NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

int main() {
    YUVToH264 *pYUVToH264 = new YUVToH264;
    pYUVToH264->yuvToH264("../res/source.yuv", "../res/source.h264");
    delete pYUVToH264;
}