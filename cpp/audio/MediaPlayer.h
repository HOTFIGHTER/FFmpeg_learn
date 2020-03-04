//
// Created by xmly on 2019/7/3.
//

#ifndef FFMPEG_LEARN_MEDIAPLAYER_H
#define FFMPEG_LEARN_MEDIAPLAYER_H

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include <stdio.h>
#include <unistd.h>
#include <libavutil/imgutils.h>
#include <pthread.h>
#include <libavutil/time.h>

#define TAG "MediaPlayer"
#define MAX_AUDIO_FRAME_SIZE 48000 * 4
#define PACKET_SIZE 50
#define MIN_SLEEP_TIME_US 1000ll
#define AUDIO_TIME_ADJUST_US -200000ll

class MediaPlayer {
public:
    AVFormatContext *format_context;
    int video_stream_index;  //视频流index
    int audio_stream_index;  //音频流index
    AVCodecContext *video_codec_context;
    AVCodecContext *audio_codec_context;
    AVCodec *video_codec;
    AVCodec *audio_codec;
    uint8_t *buffer;
    AVFrame *yuv_frame;
    AVFrame *rgba_frame;
    int video_width;
    int video_height;
    SwrContext *swrContext;
    int out_channel_nb;
    int out_sample_rate;
    enum AVSampleFormat out_sample_fmt;
    uint8_t *audio_buffer;
    AVFrame *audio_frame;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int64_t start_time;
    int64_t audio_clock;
    pthread_t write_thread;
    pthread_t video_thread;
    pthread_t audio_thread;
};


#endif //FFMPEG_LEARN_MEDIAPLAYER_H
