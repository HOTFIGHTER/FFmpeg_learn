//
// Created by xmly on 2020/2/11.
//

#include "Mp4Synthesis.h"


int Mp4Synthesis::muxer_main(char *inputH264FileName, char *inputAacFileName, char *outMP4FileName) {
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex_v = -1, videoindex_out = -1;
    int audioindex_a = -1, audioindex_out = -1;
    int frame_index = 0;
    int64_t cur_pts_v = 0, cur_pts_a = 0;

    const char *in_filename_v = inputH264FileName;
    const char *in_filename_a = inputAacFileName;
    const char *out_filename = outMP4FileName;

    int acc_length = open_input_file(in_filename_a);
    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, NULL, NULL)) < 0) {
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        return ret;
    }
    if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, NULL, NULL)) < 0) {
        printf("Could not open input file:%d\n", ret);
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        return ret;
    }
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return ret;
    }
    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        if (ifmt_ctx_v->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVStream *in_stream = ifmt_ctx_v->streams[i];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
            videoindex_v = i;
            if (!out_stream) {
                printf("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                return ret;
            }
            videoindex_out = out_stream->index;
            //ret = av_dict_set(&out_stream->metadata, "rotate", "90", 0);
            if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                printf("Failed to copy context from input to output stream codec context\n");
                return -1;
            }
            //out_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }
    if (acc_length > 0) {
        for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
            if (ifmt_ctx_a->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                AVStream *in_stream = ifmt_ctx_a->streams[i];
                AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
                audioindex_a = i;
                if (!out_stream) {
                    printf("Failed allocating output stream\n");
                    ret = AVERROR_UNKNOWN;
                    return ret;
                }
                audioindex_out = out_stream->index;
                if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                    printf("Failed to copy context from input to output stream codec context\n");
                    return -1;
                }
                //out_stream->codec->codec_tag = 0;
                if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                break;
            }
        }
    }
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
            printf("Could not open output file '%s'", out_filename);
            return -1;
        }
    }
    int header_ret = avformat_write_header(ofmt_ctx, NULL);
    if (header_ret < 0) {
        printf("Error occurred when opening output file:%d\n", header_ret);
        return -1;
    }
#if USE_H264BSF
    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
    AVBitStreamFilterContext *aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif
    while (true) {
        AVFormatContext *ifmt_ctx;
        int stream_index = 0;
        AVStream *in_stream, *out_stream;
        int compare_tag = -1;
        if (acc_length > 0) {
            compare_tag = av_compare_ts(cur_pts_v, ifmt_ctx_v->streams[videoindex_v]->time_base,
                                        cur_pts_a, ifmt_ctx_a->streams[audioindex_a]->time_base);
        }
        if (compare_tag <= 0) {
            ifmt_ctx = ifmt_ctx_v;
            stream_index = videoindex_out;
            if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    in_stream = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if (pkt.stream_index == videoindex_v) {
                        if (pkt.pts == AV_NOPTS_VALUE) {
                            //Write PTS
                            AVRational time_base1 = in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration = (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts = (double) (frame_index * calc_duration) /
                                      (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt.dts = pkt.pts;
                            pkt.duration = (double) calc_duration / (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }

                        cur_pts_v = pkt.pts;
                        break;
                    }
                } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
            } else {
                break;
            }
        } else {
            ifmt_ctx = ifmt_ctx_a;
            stream_index = audioindex_out;
            if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    in_stream = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if (pkt.stream_index == audioindex_a) {
                        if (pkt.pts == AV_NOPTS_VALUE) {
                            //Write PTS
                            AVRational time_base1 = in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration = (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts = (double) (frame_index * calc_duration) /
                                      (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt.dts = pkt.pts;
                            pkt.duration = (double) calc_duration / (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_a = pkt.pts;

                        break;
                    }
                } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
            } else {
                break;
            }
        }
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));

        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = stream_index;

        printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
        //Write

        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            printf("Error muxing packet\n");
            break;
        }
        av_free_packet(&pkt);
    }
    av_write_trailer(ofmt_ctx);
#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
    av_bitstream_filter_close(aacbsfc);
#endif
    avformat_close_input(&ifmt_ctx_v);
    avformat_close_input(&ifmt_ctx_a);
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf("Error occurred.\n");
        return -1;
    }

    printf("======muxer mp4 success =====!\n");
    return 0;
}


int Mp4Synthesis::open_input_file(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "rb");// localfile文件名
    fseek(fp, 0L, SEEK_END); /* 定位到文件末尾 */
    int flen = ftell(fp); /* 得到文件大小 */
    return flen;
}

int main() {
    char *inputH264FileName = "../res/h264test.h264";
    char *inputAacFileName = "../res/free_aduio.aac";
    char *outMP4FileName = "../res/audio.mp4";
    Mp4Synthesis *mp4Synthesis = new Mp4Synthesis;
    mp4Synthesis->muxer_main(inputH264FileName, inputAacFileName, outMP4FileName);
    delete mp4Synthesis;
}
