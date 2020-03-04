//
// Created by xmly on 2020/2/6.
//

#ifndef FFMPEG_LEARN_YUV_H
#define FFMPEG_LEARN_YUV_H
typedef struct H264FrameDef {
    unsigned int length;
    unsigned char *dataBuffer;
} H264Frame;
typedef struct H264YUVDef {
    unsigned int width;
    unsigned int height;
    H264Frame luma;
    H264Frame chromaB;
    H264Frame chromaR;
} H264YUV_Frame;
#endif //FFMPEG_LEARN_YUV_H
