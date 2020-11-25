cmake_minimum_required(VERSION 3.5)

project(dict_prefix LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dict_prefix main.c timing.cpp)
