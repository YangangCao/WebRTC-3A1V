cmake_minimum_required(VERSION 3.9)
project(cng)
add_compile_options(-std=c++11)
add_executable(cng main.cpp
        cng.cpp
        )