#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <pthread.h>
#include <unistd.h>
#include "transcoder_headers.h"
#include <time.h>

void* print_hash_line(void* arg) {
    while (*(int*)arg) {
        printf("#");
        fflush(stdout);
        sleep(1); // Sleep for 1 second
    }
    return NULL;
}

int mov(const char *input_file) {
    if (input_file == NULL) {
        fprintf(stderr, "No input file provided\n");
        return 1;
    }
    AVFormatContext *formatContext = NULL;

    // Open the video file
    if (avformat_open_input(&formatContext, input_file, NULL, NULL) != 0) {
        fprintf(stderr, "Error opening the file\n");
        return 1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        fprintf(stderr, "Error finding stream information\n");
        return 1;
    }

    // Print information about the video
    //av_dump_format(formatContext, 0, input_file, 0);

    // Change the video format to mov
    const AVOutputFormat *outputFormat = av_guess_format("mov", NULL, NULL);
    if (!outputFormat) {
        fprintf(stderr, "Error guessing format\n");
        return 1;
    }

    // Create a new video file
    AVFormatContext *outputContext = NULL;
    if (avformat_alloc_output_context2(&outputContext, outputFormat, NULL, "output.mov") < 0) {
        fprintf(stderr, "Error creating output context\n");
        return 1;
    }

    // Add streams to the output file
    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *inStream = formatContext->streams[i];
        AVStream *outStream = avformat_new_stream(outputContext, NULL);
        if (!outStream) {
            fprintf(stderr, "Error creating new stream\n");
            return 1;
        }

        // Copy codec parameters from input to output stream
        if (avcodec_parameters_copy(outStream->codecpar, inStream->codecpar) < 0) {
            fprintf(stderr, "Error copying codec parameters\n");
            return 1;
        }

        // Find the encoder for the stream
        const AVCodec *codec = avcodec_find_encoder(outStream->codecpar->codec_id);
        if (!codec) {
            fprintf(stderr, "Error finding encoder\n");
            return 1;
        }

        // Allocate a codec context for the encoder
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            fprintf(stderr, "Error allocating codec context\n");
            return 1;
        }

        // Copy the codec parameters from the input stream to the codec context
        if (avcodec_parameters_to_context(codecContext, inStream->codecpar) < 0) {
            fprintf(stderr, "Error copying codec parameters to context\n");
            return 1;
        }

        // Set the timebase for the codec context
        codecContext->time_base = inStream->time_base;

        // Open the codec
        if (avcodec_open2(codecContext, codec, NULL) < 0) {
            fprintf(stderr, "Error opening codec\n");
            return 1;
        }

        // Copy the codec parameters from the codec context to the output stream
        if (avcodec_parameters_from_context(outStream->codecpar, codecContext) < 0) {
            fprintf(stderr, "Error copying codec parameters from context\n");
            return 1;
        }

        // Set the timebase for the output stream
        outStream->time_base = codecContext->time_base;

        avcodec_free_context(&codecContext);
    }

    // Open the output file
    if (avio_open(&outputContext->pb, "output.mov", AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "Error opening the output file\n");
        return 1;
    }

    // Write the header to the output file
    if (avformat_write_header(outputContext, NULL) < 0) {
        fprintf(stderr, "Error writing header\n");
        return 1;
    }

    // Create a flag and a thread for printing hashes
    int print_flag = 1;
    pthread_t print_thread;
    pthread_create(&print_thread, NULL, print_hash_line, &print_flag);

    // Read packets from the input file and write them to the output file
    AVPacket packet;
    while (av_read_frame(formatContext, &packet) >= 0) {
        AVStream *inStream = formatContext->streams[packet.stream_index];
        AVStream *outStream = outputContext->streams[packet.stream_index];

        // Convert PTS/DTS
        packet.pts = av_rescale_q_rnd(packet.pts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        packet.dts = av_rescale_q_rnd(packet.dts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;
        packet.stream_index = outStream->index;

        if (av_interleaved_write_frame(outputContext, &packet) < 0) {
            fprintf(stderr, "Error writing frame\n");
            return 1;
        }
        av_packet_unref(&packet);
    }

    // Stop the hash printing thread
    print_flag = 0;
    pthread_join(print_thread, NULL);
    printf("\n");

    // Write the trailer to the output file
    if (av_write_trailer(outputContext) < 0) {
        fprintf(stderr, "Error writing trailer\n");
        return 1;
    }

    // Close the output file
    avio_close(outputContext->pb);

    // Cleanup and close the file
    avformat_close_input(&formatContext);
    avformat_free_context(outputContext);

    return 0;
}