cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 23)

project(ataparse CXX)

add_executable(ataparse)
target_sources(ataparse PRIVATE src/main.cpp)
target_compile_options(ataparse PRIVATE /W4 /WX)