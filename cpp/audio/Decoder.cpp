//
// Created by xmly on 2019/7/3.
//

#include "Decoder.h"

//初始化输入格式上下文
int Decoder::init_input_format_context(MediaPlayer *player, const char *file_name) {
    //注册所有组件
    av_register_all();
    //分配上下文
    player->format_context = avformat_alloc_context();
    //打开视频文件
    if (avformat_open_input(&player->format_context, file_name, NULL, NULL) != 0) {
        ("Couldn't open file:%s\n", file_name);
        return -1;
    }
    //检索多媒体流信息
    if (avformat_find_stream_info(player->format_context, NULL) < 0) {
        printf("Couldn't find stream information.");
        return -1;
    }
    //寻找音视频流索引位置
    int i;
    player->video_stream_index = -1;
    player->audio_stream_index = -1;
    for (i = 0; i < player->format_context->nb_streams; i++) {
        if (player->format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && player->video_stream_index < 0) {
            player->video_stream_index = i;
        } else if (player->format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
                   && player->audio_stream_index < 0) {
            player->audio_stream_index = i;
        }
    }
    if (player->video_stream_index == -1) {
        printf("couldn't find a video stream.");
        return -1;
    }
    if (player->audio_stream_index == -1) {
        printf("couldn't find a audio stream.");
        return -1;
    }
    printf("video_stream_index=%d", player->video_stream_index);
    printf("audio_stream_index=%d", player->audio_stream_index);
    return 0;
}

//打开音视频解码器
int Decoder::init_condec_context(MediaPlayer *player) {
    //获取codec上下文指针
    player->video_codec_context = player->format_context->streams[player->video_stream_index]->codec;
    //寻找视频流的解码器
    player->video_codec = avcodec_find_decoder(player->video_codec_context->codec_id);
    if (player->video_codec == NULL) {
        printf("couldn't find video Codec.");
        return -1;
    }
    if (avcodec_open2(player->video_codec_context, player->video_codec, NULL) < 0) {
        printf("Couldn't open video codec.");
        return -1;
    }
    player->audio_codec_context = player->format_context->streams[player->audio_stream_index]->codec;
    player->audio_codec = avcodec_find_decoder(player->audio_codec_context->codec_id);
    if (player->audio_codec == NULL) {
        printf("couldn't find audio Codec.");
        return -1;
    }
    if (avcodec_open2(player->audio_codec_context, player->audio_codec, NULL) < 0) {
        printf("Couldn't open audio codec.");
        return -1;
    }
    // 获取视频宽高
    player->video_width = player->video_codec_context->width;
    player->video_height = player->video_codec_context->height;
    return 0;
}

//获取当前播放时间
int64_t Decoder::get_play_time(MediaPlayer *player) {
    return (int64_t) (av_gettime() - player->start_time);
}

/**
 * 延迟等待，音视频同步
 */
void Decoder::player_wait_for_frame(MediaPlayer *player, int64_t stream_time) {
    pthread_mutex_lock(&player->mutex);
    for (;;) {
        int64_t current_video_time = get_play_time(player);
        int64_t sleep_time = stream_time - current_video_time;
        if (sleep_time < -300000ll) {
            // 300 ms late
            int64_t new_value = player->start_time - sleep_time;
            player->start_time = new_value;
            pthread_cond_broadcast(&player->cond);
        }

        if (sleep_time <= MIN_SLEEP_TIME_US) {
            break;
        }

        if (sleep_time > 500000ll) {
            sleep_time = 500000ll;
        }
        //等待指定时长
//       pthread_cond_timeout_np(&player->cond, &player->mutex,
//                                (unsigned int) (sleep_time / 1000ll));
    }
    pthread_mutex_unlock(&player->mutex);
}

//视频解码
int Decoder::decode_video(MediaPlayer *player, AVPacket *packet) {
    //申请内存
    player->yuv_frame = av_frame_alloc();
    player->rgba_frame = av_frame_alloc();
    if(player->rgba_frame == NULL || player->yuv_frame == NULL) {
        printf("Couldn't allocate video frame.");
        return -1;
    }

    // buffer中数据用于渲染,且格式为RGBA
    int numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGBA, player->video_width, player->video_height, 1);

    player->buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(player->rgba_frame->data, player->rgba_frame->linesize, player->buffer, AV_PIX_FMT_RGBA,
                         player->video_width, player->video_height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(
            player->video_width,
            player->video_height,
            player->video_codec_context->pix_fmt,
            player->video_width,
            player->video_height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL);

    int frameFinished;
    //对该帧进行解码
    int ret = avcodec_decode_video2(player->video_codec_context, player->yuv_frame, &frameFinished, packet);
    if(ret < 0){
        printf("avcodec_decode_video2 error...");
        return -1;
    }
    if (frameFinished) {
        // lock native window
        //ANativeWindow_lock(player->native_window, &windowBuffer, 0);
        // 格式转换
        sws_scale(sws_ctx, (uint8_t const * const *)player->yuv_frame->data,
                  player->yuv_frame->linesize, 0, player->video_height,
                  player->rgba_frame->data, player->rgba_frame->linesize);
        // 获取stride
        //uint8_t * dst = windowBuffer.bits;
        //int dstStride = windowBuffer.stride * 4;
        uint8_t * src = player->rgba_frame->data[0];
        int srcStride = player->rgba_frame->linesize[0];
        // 由于window的stride和帧的stride不同,因此需要逐行复制
        int h;
//        for (h = 0; h < player->video_height; h++) {
//            memcpy(dst + h * dstStride, src + h * srcStride, (size_t) srcStride);
//        }

        //计算延迟
        int64_t pts = av_frame_get_best_effort_timestamp(player->yuv_frame);
        AVStream *stream = player->format_context->streams[player->video_stream_index];
        //转换（不同时间基时间转换）
        int64_t time = av_rescale_q(pts, stream->time_base, AV_TIME_BASE_Q);
        //音视频帧同步
        player_wait_for_frame(player, time);

        //ANativeWindow_unlockAndPost(player->native_window);
    }
//    //延迟等待
//    usleep(1000 * 16);
    return 0;
}

//音频解码初始化
void audio_decoder_prepare(MediaPlayer* player) {
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    player->swrContext = swr_alloc();

    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = player->audio_codec_context->sample_fmt;
    //输出采样格式16bit PCM
    player->out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = player->audio_codec_context->sample_rate;
    //输出采样率
    player->out_sample_rate = in_sample_rate;
    //声道布局（2个声道，默认立体声stereo）
    uint64_t in_ch_layout = player->audio_codec_context->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(player->swrContext,
                       out_ch_layout, player->out_sample_fmt, player->out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate,
                       0, NULL);
    swr_init(player->swrContext);
    //输出的声道个数
    player->out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
}

//音频解码
int Decoder::decode_audio(MediaPlayer *player, AVPacket *packet) {
    int got_frame = 0, ret;
    //解码
    ret = avcodec_decode_audio4(player->audio_codec_context, player->audio_frame, &got_frame, packet);
    if(ret < 0){
        printf("avcodec_decode_audio4 error...");
        return -1;
    }
    //解码一帧成功
    if(got_frame > 0){
        //音频格式转换
        swr_convert(player->swrContext, &player->audio_buffer,  MAX_AUDIO_FRAME_SIZE, (const uint8_t **)player->audio_frame->data, player->audio_frame->nb_samples);
        int out_buffer_size = av_samples_get_buffer_size(NULL, player->out_channel_nb,
                                                         player->audio_frame->nb_samples, player->out_sample_fmt, 1);
        //音视频帧同步
        int64_t pts = packet->pts;
        if (pts != AV_NOPTS_VALUE) {
            AVStream *stream = player->format_context->streams[player->audio_stream_index];
            player->audio_clock = av_rescale_q(pts, stream->time_base, AV_TIME_BASE_Q);
            player_wait_for_frame(player, player->audio_clock + AUDIO_TIME_ADJUST_US);
        }
        return 0;
    }
    return 0;
}



