//
// Created by xmly on 2020/3/1.
//

#ifndef FFMPEG_LEARN_YUVTOH264_H
#define FFMPEG_LEARN_YUVTOH264_H

extern "C" { //这步必须加，不然会报ld: symbol(s) not found for architecture x86_64错误
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class YUVToH264 {
private:
    AVCodec *pAvCodec = NULL;
    AVCodecContext *pAvCodecContext = NULL;
    AVFormatContext *pAvFormatContext = NULL;
    AVStream *pAVStream=NULL;
    AVFrame *pAvFrame = NULL;
    AVPacket *pAvPacket=NULL;
    SwsContext *pSws_context = NULL;
public:
    YUVToH264();
    ~YUVToH264();
    int yuvToH264(const char *input, const char *output);
    int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);
};


#endif //FFMPEG_LEARN_YUVTOH264_H
