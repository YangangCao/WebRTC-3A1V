cmake_minimum_required(VERSION 3.15)
project(AGC C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(AGC
        agc.c
        agc.h
        dr_wav.h
        main.c)
