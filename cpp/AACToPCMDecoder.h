//
// Created by xmly on 2020/2/9.
//

#ifndef FFMPEG_LEARN_PCMTOAACDECODER_H
#define FFMPEG_LEARN_PCMTOAACDECODER_H

#include <iostream>

extern "C" {
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"
};

using namespace std;

class PCMToAACDecoder {
public:
    void musicPlayer(const char *input, const char *output);
};


#endif //FFMPEG_LEARN_PCMTOAACDECODER_H
