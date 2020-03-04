//
// Created by xmly on 2019/6/30.
//

#include "H264Decoder.h"
#include <iostream>
#include <sys/param.h>

using namespace std;

H264Decoder::H264Decoder() {
    av_register_all();//注册ffmpeg
    avcodec_register_all();//注册解码器
}

//析构函数
H264Decoder::~H264Decoder() {
    if (!pAvCodecContext) {
        avcodec_close(pAvCodecContext);
        pAvCodecContext = NULL;
    }
    if (!pAvFrame) {
        av_frame_free(&pAvFrame);
        pAvFrame = NULL;
    }
    if (!pAvFormatContext) {
        avformat_free_context(pAvFormatContext);
        pAvFormatContext = NULL;
    }
    pAvCodec = NULL;
}

int H264Decoder::decodeH264Frame() {
    pSws_context = sws_getContext(pAvCodecContext->width, pAvCodecContext->height, pAvCodecContext->pix_fmt,
                                  pAvCodecContext->width, pAvCodecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL,
                                  NULL, NULL);
    if (avcodec_open2(pAvCodecContext, pAvCodec, NULL)) {
        printf("open codec error\n");
        return -1;
    }
    pAvPacket = av_packet_alloc();
    pAvFrame = av_frame_alloc();
    uint8_t *pointers[4];
    int linesizes[4];
    /**根据AV_PIX_FMT_RGB24分配图片显示缓存大小*/
    av_image_alloc(pointers, linesizes, pAvCodecContext->width, pAvCodecContext->height, AV_PIX_FMT_RGB24, 1);
    while (!av_read_frame(pAvFormatContext, pAvPacket)) {
        if (pAvPacket->stream_index == AVMEDIA_TYPE_VIDEO) {
            if (!avcodec_send_packet(pAvCodecContext, pAvPacket)) {
                while (!avcodec_receive_frame(pAvCodecContext, pAvFrame)) {
                    //解码
                    sws_scale(pSws_context, (const uint8_t *const *) pAvFrame->data, pAvFrame->linesize, 0,
                              pAvFrame->height,
                              pointers, linesizes);
                    if (pOpenGlWindow != NULL) {
                        pOpenGlWindow->draw(pAvFrame->width, pAvFrame->height, pointers[0]);
                    } else {
                        pOpenGlWindow = new OpenGlWindow();
                    }
                }

            }
        }
    }
    av_free(pointers[0]);
    av_frame_free(&pAvFrame);
    av_packet_free(&pAvPacket);
    avcodec_close(pAvCodecContext);
    sws_freeContext(pSws_context);
    return 0;
}

int H264Decoder::openFile(char path[]) {
    int videoindex = -1;
    avformat_network_init();
    pAvFormatContext = avformat_alloc_context();
    if (!pAvFormatContext) {
        printf("Format not find\n");
        return -1;
    }
    int err_code;
    char *err_buf;
    AVCodecParameters *videoCodecParmeters = NULL;
    AVRational videoAVRational = {1,50};
    if (err_code = avformat_open_input(&pAvFormatContext, path, NULL, NULL)) {
        printf("Couldn't open file %s: %d(%s)", path, err_code, err_buf);
        return -1;
    }
    if (avformat_find_stream_info(pAvFormatContext, NULL) < 0) {
        cout << "dtream find failed\n" << endl;
        return -1;
    }
    for (int i = 0; i < pAvFormatContext->nb_streams; i++) {
        AVCodecParameters *codecParmeters = pAvFormatContext->streams[i]->codecpar;
        if (codecParmeters->codec_type == AVMEDIA_TYPE_VIDEO
            && NULL == videoCodecParmeters) {
            videoCodecParmeters = codecParmeters;
            videoAVRational = pAvFormatContext->streams[i]->time_base;
            videoindex = i;
            break;
        }
    }
    if (videoindex == -1) {
        cout << "not a video\n" << endl;
        return -1;
    }
    pAvCodec = avcodec_find_decoder(videoCodecParmeters->codec_id); //只用于解码h264格式
    if (!pAvCodec) {
        printf("Codec not find\n");
        return -1;
    }
    pAvCodecContext = avcodec_alloc_context3(pAvCodec);
    if (!pAvCodecContext) {
        printf("allocate codec context error\n");
        return -1;
    }
    pAvCodecContext->pkt_timebase=videoAVRational;
    avcodec_parameters_to_context(pAvCodecContext, videoCodecParmeters);
    return 0;
}

int main() {
    char path[] = "../res/begin.mp4";
    H264Decoder *pH264Decoder = new H264Decoder();
    int result = pH264Decoder->openFile(path);
    if (result != -1) {
        pH264Decoder->decodeH264Frame();
    }
    delete pH264Decoder;
}


