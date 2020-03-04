//
// Created by xmly on 2020/1/19.
//

#include "../ffmpeg/include/libavformat/avformat.h"
/**""*/
#include <iostream>
#define TAG "MediaPlayer"
#define MAX_AUDIO_FRAME_SIZE 48000 * 4
using namespace std;

void main() {
//    const char *input_cstr = "";
//    /**注册ffmpeg组件*/
//    av_register_all();
//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//    /**打开音频文件*/
//    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
//        cout << "%s", "无法打开音频文件";
//        return;
//    }
//    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
//        cout << "无法获取输入文件信息";
//        return;
//    }
//    /**获取音频流索引位置*/
//    int i = 0, audio_stream_idx = -1;
//    for (; i < pFormatCtx->nb_streams; i++) {
//        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
//            audio_stream_idx = i;
//            break;
//        }
//    }
//    /**获取音频解码器*/
//    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
//    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
//    if (codec == NULL) {
//        cout << "%s", "无法获取解码器";
//        return;
//    }
//    /**打开解码器*/
//    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
//        cout << "无法打开解码器";
//        return;
//    }

}