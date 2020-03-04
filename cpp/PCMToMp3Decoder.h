//
// Created by xmly on 2020/2/10.
//

#ifndef FFMPEG_LEARN_PCMTOMP3DECODER_H
#define FFMPEG_LEARN_PCMTOMP3DECODER_H
extern "C" {
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"
}
// PCM的原始参数
#define PCM_IN_FORMAT AV_SAMPLE_FMT_S16
#define PCM_IN_CHANNELS 1
#define PCM_IN_SAMPLE_RATE 44100

/** 申请智能指针变量*/
#define NEW_PTR(T, P, V, Fn) T *P = V; std::shared_ptr<T> P##P(P, [&P](T *){if(P != nullptr){Fn;}})

using namespace std;
class PCMToMp3Decoder {
public:
void decodePCMToMp3(char* input,char* output);
};


#endif //FFMPEG_LEARN_PCMTOMP3DECODER_H
