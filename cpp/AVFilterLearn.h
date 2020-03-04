//
// Created by xmly on 2020/2/12.
//

#ifndef FFMPEG_LEARN_AVFILTERLEARN_H
#define FFMPEG_LEARN_AVFILTERLEARN_H
extern "C" { //这步必须加，不然会报ld: symbol(s) not found for architecture x86_64错误
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include <cstdio>

using namespace std;

class AVFilterLearn {
public:
    int avFilter(char *inputFile, char *outFile);

    int init_filters(const char *filters_descr);

    int open_input(const char *file_name);

private:
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
    int video_stream_index = -1;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVFrame *pFrame;
    AVFrame *pFrameRGBA;
    AVFrame *filter_frame;
    uint8_t *buffer;
    struct SwsContext *sws_ctx;
};


#endif //FFMPEG_LEARN_AVFILTERLEARN_H
