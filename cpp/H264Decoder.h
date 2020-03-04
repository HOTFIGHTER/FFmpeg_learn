//
// Created by xmly on 2019/6/30.
//

#ifndef FFMPEG_LEARN_H264DECODER_H
#define FFMPEG_LEARN_H264DECODER_H

#include <cstdio>
#include "opengl/OpenGlWindow.h"

extern "C" { //这步必须加，不然会报ld: symbol(s) not found for architecture x86_64错误
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include "h264/YUV.h"
}

class H264Decoder {
private:
    AVCodec *pAvCodec = NULL;
    AVCodecContext *pAvCodecContext = NULL;
    AVFormatContext *pAvFormatContext = NULL;
    AVFrame *pAvFrame = NULL;
    AVPacket *pAvPacket=NULL;
    SwsContext *pSws_context = NULL;
    OpenGlWindow *pOpenGlWindow = NULL;
public:
    H264Decoder();

    ~H264Decoder();

    void init();

    int openFile(char path[]);

    int decodeH264Frame();

    void updateDecodedH264OnView(H264YUV_Frame *yuv_frame);
};


#endif //FFMPEG_LEARN_H264DECODER_H
