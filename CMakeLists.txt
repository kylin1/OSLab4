cmake_minimum_required(VERSION 3.5)
project(MultipleProcess)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp lib/klib.h lib/clock.h lib/i8259.h)
add_executable(MultipleProcess ${SOURCE_FILES})