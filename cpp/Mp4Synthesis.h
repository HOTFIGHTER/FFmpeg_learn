//
// Created by xmly on 2020/2/11.
//

#ifndef FFMPEG_LEARN_MP4SYNTHESIS_H
#define FFMPEG_LEARN_MP4SYNTHESIS_H
extern "C" { //这步必须加，不然会报ld: symbol(s) not found for architecture x86_64错误
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
}

#define USE_H264BSF 1
#define USE_AACBSF 1
class Mp4Synthesis {
public:
    int muxer_main(char *inputH264FileName,char *inputAacFileName,char *outMP4FileName);
    int open_input_file(const char *filename);
};


#endif //FFMPEG_LEARN_MP4SYNTHESIS_H
