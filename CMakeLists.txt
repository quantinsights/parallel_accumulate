cmake_minimum_required(VERSION 3.30.3)
project(parallel_accumulate)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    src/main.cpp
)

include_directories(include)

add_executable(parallel_accumulate ${SOURCES})	