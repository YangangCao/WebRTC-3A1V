cmake_minimum_required(VERSION 3.15)
project(NS C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(NS
        dr_mp3.h
        dr_wav.h
        main.c
        noise_suppression.c
        noise_suppression.h
        timing.h)
