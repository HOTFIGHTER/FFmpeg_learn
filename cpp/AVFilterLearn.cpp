//
// Created by xmly on 2020/2/12.
//


#include "AVFilterLearn.h"

int AVFilterLearn::avFilter(char *inputFile, char *oFile) {
    int ret;
    FILE *inFile = NULL;
    const char *inFileName = inputFile;
    inFile = fopen(inFileName, "rb+");
    if (!inFile) {
        printf("Fail to open file\n");
        return -1;
    }
    int in_width = 480;
    int in_height = 272;

    FILE *outFile = NULL;
    const char *outFileName = oFile;
    outFile = fopen(outFileName, "wb");
    if (!outFile) {
        printf("Fail to open file\n");
        return -1;
    }

    AVFilterGraph *filter_graph = avfilter_graph_alloc();
    if (!filter_graph) {
        printf("Fail to create filter graph!\n");
        return -1;
    }
    char args[512];
    AVFilter *bufferSrc = const_cast<AVFilter *>(avfilter_get_by_name("buffer"));
    AVFilterContext *bufferSrc_ctx;
    ret = avfilter_graph_create_filter(&bufferSrc_ctx, bufferSrc, "in", args, NULL, filter_graph);
    if (ret < 0) {
        printf("Fail to create filter bufferSrc\n");
        return -1;
    }

    AVBufferSinkParams *bufferSink_params;
    AVFilterContext *bufferSink_ctx;
    AVFilter *bufferSink = const_cast<AVFilter *>(avfilter_get_by_name("buffersink"));
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    bufferSink_params = av_buffersink_params_alloc();
    bufferSink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&bufferSink_ctx, bufferSink, "out", NULL, bufferSink_params, filter_graph);
    if (ret < 0) {
        printf("Fail to create filter sink filter\n");
        return -1;
    }

    AVFilter *splitFilter = const_cast<AVFilter *>(avfilter_get_by_name("split"));
    AVFilterContext *splitFilter_ctx;
    ret = avfilter_graph_create_filter(&splitFilter_ctx, splitFilter, "split", "outputs=2", NULL, filter_graph);
    if (ret < 0) {
        printf("Fail to create split filter\n");
        return -1;
    }

    AVFilter *cropFilter = const_cast<AVFilter *>(avfilter_get_by_name("crop"));
    AVFilterContext *cropFilter_ctx;
    ret = avfilter_graph_create_filter(&cropFilter_ctx, cropFilter, "crop", "out_w=iw:out_h=ih/2:x=0:y=0", NULL,
                                       filter_graph);
    if (ret < 0) {
        printf("Fail to create crop filter\n");
        return -1;
    }

    AVFilter *vflipFilter = const_cast<AVFilter *>(avfilter_get_by_name("vflip"));
    AVFilterContext *vflipFilter_ctx;
    ret = avfilter_graph_create_filter(&vflipFilter_ctx, vflipFilter, "vflip", NULL, NULL, filter_graph);
    if (ret < 0) {
        printf("Fail to create vflip filter\n");
        return -1;
    }

    AVFilter *overlayFilter = const_cast<AVFilter *>(avfilter_get_by_name("overlay"));
    AVFilterContext *overlayFilter_ctx;
    ret = avfilter_graph_create_filter(&overlayFilter_ctx, overlayFilter, "overlay", "y=0:H/2", NULL, filter_graph);
    if (ret < 0) {
        printf("Fail to create overlay filter\n");
        return -1;
    }

    ret = avfilter_link(bufferSrc_ctx, 0, splitFilter_ctx, 0);
    if (ret != 0) {
        printf("Fail to link src filter and split filter\n");
        return -1;
    }
    ret = avfilter_link(splitFilter_ctx, 0, overlayFilter_ctx, 0);
    if (ret != 0) {
        printf("Fail to link split filter and overlay filter main pad\n");
        return -1;
    }
    ret = avfilter_link(splitFilter_ctx, 1, cropFilter_ctx, 0);
    if (ret != 0) {
        printf("Fail to link split filter's second pad and crop filter\n");
        return -1;
    }
    ret = avfilter_link(cropFilter_ctx, 0, vflipFilter_ctx, 0);
    if (ret != 0) {
        printf("Fail to link crop filter and vflip filter\n");
        return -1;
    }
    ret = avfilter_link(vflipFilter_ctx, 0, overlayFilter_ctx, 1);
    if (ret != 0) {
        printf("Fail to link vflip filter and overlay filter's second pad\n");
        return -1;
    }
    ret = avfilter_link(overlayFilter_ctx, 0, bufferSink_ctx, 0);
    if (ret != 0) {
        printf("Fail to link overlay filter and sink filter\n");
        return -1;
    }
    ret = avfilter_graph_config(filter_graph, NULL);
    if (ret < 0) {
        printf("Fail in filter graph\n");
        return -1;
    }


    AVFrame *frame_in = av_frame_alloc();
    unsigned char *frame_buffer_in = (unsigned char *) av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                                                          in_width, in_height, 1));
    av_image_fill_arrays(frame_in->data, frame_in->linesize, frame_buffer_in,
                         AV_PIX_FMT_YUV420P, in_width, in_height, 1);
    AVFrame *frame_out = av_frame_alloc();
    unsigned char *frame_buffer_out = (unsigned char *) av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                                                           in_width, in_height, 1));
    av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_buffer_out,
                         AV_PIX_FMT_YUV420P, in_width, in_height, 1);

    frame_in->width = in_width;
    frame_in->height = in_height;
    frame_in->format = AV_PIX_FMT_YUV420P;

    while (true) {
        if (fread(frame_buffer_in, 1, in_width * in_height * 3 / 2, inFile) != in_width * in_height * 3 / 2) {
            break;
        }
        frame_in->data[0] = frame_buffer_in;
        frame_in->data[1] = frame_buffer_in + in_width * in_height;
        frame_in->data[2] = frame_buffer_in + in_width * in_height * 5 / 4;

        if (av_buffersrc_add_frame(bufferSrc_ctx, frame_in) < 0) {
            printf("Error while add frame.\n");
            break;
        }
        ret = av_buffersink_get_frame(bufferSink_ctx, frame_out);
        if (ret < 0) {
            break;
        }
        if (frame_out->format == AV_PIX_FMT_YUV420P) {
            for (int i = 0; i < frame_out->height; i++) {
                fwrite(frame_out->data[0] + frame_out->linesize[0] * i, 1, frame_out->width, outFile);
            }
            for (int i = 0; i < frame_out->height / 2; i++) {
                fwrite(frame_out->data[1] + frame_out->linesize[1] * i, 1, frame_out->width / 2, outFile);
            }
            for (int i = 0; i < frame_out->height / 2; i++) {
                fwrite(frame_out->data[2] + frame_out->linesize[2] * i, 1, frame_out->width / 2, outFile);
            }
        }
        av_frame_unref(frame_out);
    }
    fclose(inFile);
    fclose(outFile);
    av_frame_free(&frame_in);
    av_frame_free(&frame_out);
    avfilter_graph_free(&filter_graph);
    return 0;
}

//初始化滤波器
int AVFilterLearn::init_filters(const char *filters_descr) {
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = const_cast<AVFilter *>(avfilter_get_by_name("buffer"));
    AVFilter *buffersink = const_cast<AVFilter *>(avfilter_get_by_name("buffersink"));
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational time_base = pFormatCtx->streams[video_stream_index]->time_base;
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
             time_base.num, time_base.den,
             pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        printf("Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        printf("Cannot create buffer sink\n");
        goto end;
    }
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

    end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

//初始化视频解码器与播放器
int AVFilterLearn::open_input(const char *file_name) {
    //注册所有组件
    av_register_all();
    //分配上下文
    pFormatCtx = avformat_alloc_context();
    //打开视频文件
    if (avformat_open_input(&pFormatCtx, file_name, NULL, NULL) != 0) {
        printf("Couldn't open file:%s\n", file_name);
        return -1;
    }
    //检索多媒体流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Couldn't find stream information.");
        return -1;
    }
    //寻找视频流的第一帧
    int i;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && video_stream_index < 0) {
            video_stream_index = i;
        }
    }
    if (video_stream_index == -1) {
        printf("couldn't find a video stream.");
        return -1;
    }

    //获取codec上下文指针
    pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;
    //寻找视频流的解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        printf("couldn't find Codec.");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Couldn't open codec.");
        return -1;
    }

    return 0;
}

int main() {

}