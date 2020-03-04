//
// Created by xmly on 2020/3/1.
//

#include <iostream>
#include "Mp4ToJPEG.h"

using namespace std;

Mp4ToJPEG::Mp4ToJPEG() {

}

Mp4ToJPEG::~Mp4ToJPEG() {

}

int Mp4ToJPEG::mp4ToJPEG(const char *input, const char *output) {
    avformat_network_init();
    pAvFormatContext = avformat_alloc_context();
    //打开音频文件
    if (avformat_open_input(&pAvFormatContext, input, NULL, NULL) != 0) {
        cout << ("%s", "无法打开音频文件") << endl;
        return -1;
    }
    if (avformat_find_stream_info(pAvFormatContext, NULL) < 0) {
        cout << ("%s", "无法获取输入文件信息") << endl;
        return -1;
    }

    int videoindex = -1;
    AVCodecParameters *videoCodecParmeters = NULL;
    AVRational videoAVRational = {1, 50};
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
    pAvCodec = avcodec_find_decoder(videoCodecParmeters->codec_id);
    if (!pAvCodec) {
        printf("Codec not find\n");
        return -1;
    }
    pAvCodecContext = avcodec_alloc_context3(pAvCodec);
    if (!pAvCodecContext) {
        printf("allocate codec context error\n");
        return -1;
    }
    pAvCodecContext->pkt_timebase = videoAVRational;
    avcodec_parameters_to_context(pAvCodecContext, videoCodecParmeters);
    if (avcodec_open2(pAvCodecContext, pAvCodec, NULL)) {
        printf("open codec error\n");
        return -1;
    }
    pAvPacket = av_packet_alloc();
    pAvFrame = av_frame_alloc();
    int frameCout = 0;
    while (!av_read_frame(pAvFormatContext, pAvPacket)) {
        if (pAvPacket->stream_index == AVMEDIA_TYPE_VIDEO) {
            if (!avcodec_send_packet(pAvCodecContext, pAvPacket)) {
                while (!avcodec_receive_frame(pAvCodecContext, pAvFrame)) {
                    if (frameCout == 400) {
                        saveJPG(output);
                    }
                    frameCout++;
                }
            }
        }
    }
    av_frame_free(&pAvFrame);
    av_packet_free(&pAvPacket);
    avcodec_close(pAvCodecContext);
    sws_freeContext(pSws_context);
}

int Mp4ToJPEG::saveJPG(const char *output) const {
    int width = pAvFrame->width;
    int height = pAvFrame->height;
    AVCodecContext *pCodeCtx = NULL;
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
    if (avio_open(&pFormatCtx->pb, output, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file.");
        return -1;
    }
    AVStream *pAVStream = avformat_new_stream(pFormatCtx, 0);
    if (pAVStream == NULL) {
        return -1;
    }

    AVCodecParameters *parameters = pAVStream->codecpar;
    parameters->codec_id = pFormatCtx->oformat->video_codec;
    parameters->codec_type = AVMEDIA_TYPE_VIDEO;
    parameters->format = AV_PIX_FMT_YUVJ420P;
    parameters->width = pAvFrame->width;
    parameters->height = pAvFrame->height;

    AVCodec *pCodec = avcodec_find_encoder(pAVStream->codecpar->codec_id);

    if (!pCodec) {
        printf("Could not find encoder\n");
        return -1;
    }

    pCodeCtx = avcodec_alloc_context3(pCodec);
    if (!pCodeCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    if ((avcodec_parameters_to_context(pCodeCtx, pAVStream->codecpar)) < 0) {
        fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        return -1;
    }

    pCodeCtx->time_base = (AVRational) {1, 25};

    if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.");
        return -1;
    }

    int ret = avformat_write_header(pFormatCtx, NULL);
    if (ret < 0) {
        printf("write_header fail\n");
        return -1;
    }
    int y_size = width * height;

    //Encode
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    // 编码数据
    ret = avcodec_send_frame(pCodeCtx, pAvFrame);
    if (ret < 0) {
        printf("Could not avcodec_send_frame.");
        return -1;
    }
    // 得到编码后数据
    ret = avcodec_receive_packet(pCodeCtx, &pkt);
    if (ret < 0) {
        printf("Could not avcodec_receive_packet");
        return -1;
    }

    ret = av_write_frame(pFormatCtx, &pkt);

    if (ret < 0) {
        printf("Could not av_write_frame");
        return -1;
    }

    av_packet_unref(&pkt);
    //Write Trailer
    av_write_trailer(pFormatCtx);
    avcodec_close(pCodeCtx);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    return 0;
}

int main() {
    Mp4ToJPEG *pMp4ToJPEG = new Mp4ToJPEG();
    pMp4ToJPEG->mp4ToJPEG("../res/begin.mp4", "../res/begin.jpg");
    delete pMp4ToJPEG;
}