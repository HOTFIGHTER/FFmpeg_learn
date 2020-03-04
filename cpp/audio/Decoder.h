//
// Created by xmly on 2019/7/3.
//

#ifndef FFMPEG_LEARN_DECODER_H
#define FFMPEG_LEARN_DECODER_H


#include "MediaPlayer.h"

class Decoder {
public:
    int init_condec_context(MediaPlayer* player);
    int init_input_format_context(MediaPlayer* player, const char* file_name);
    int64_t get_play_time(MediaPlayer* player);
    void player_wait_for_frame(MediaPlayer *player, int64_t stream_time);
    int decode_video(MediaPlayer* player, AVPacket* packet);
    int decode_audio(MediaPlayer* player, AVPacket* packet);
private:
    MediaPlayer *player;
    int stream_index;
};


#endif //FFMPEG_LEARN_DECODER_H
