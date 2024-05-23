cmake_minimum_required(VERSION 3.28)
project(video_transcoder C)

set(CMAKE_C_STANDARD 23)

add_executable(video_transcoder main.c
        transcoder.c)
