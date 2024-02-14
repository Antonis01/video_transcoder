#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "Usage: %s <output file>\n", argv[0]);
        return 1;

    }

    const char *input_file = argv[1];

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
    av_dump_format(formatContext, 0, input_file, 0);

    // Cleanup and close the file
    avformat_close_input(&formatContext);

    return 0;
}