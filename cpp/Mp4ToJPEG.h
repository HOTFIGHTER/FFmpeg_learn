//
// Created by xmly on 2020/3/1.
//

#ifndef FFMPEG_LEARN_MP4TOJPEG_H
#define FFMPEG_LEARN_MP4TOJPEG_H

extern "C" { //这步必须加，不然会报ld: symbol(s) not found for architecture x86_64错误
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class Mp4ToJPEG {
private:
    AVCodec *pAvCodec = NULL;
    AVCodecContext *pAvCodecContext = NULL;
    AVFormatContext *pAvFormatContext = NULL;
    AVFrame *pAvFrame = NULL;
    AVPacket *pAvPacket=NULL;
    SwsContext *pSws_context = NULL;

public:
    Mp4ToJPEG();
    ~Mp4ToJPEG();
    int mp4ToJPEG(const char *input, const char *output);

    int saveJPG(const char *output) const;
};


#endif //FFMPEG_LEARN_MP4TOJPEG_H
