cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE SRC_FILES src/*.cpp include/*.hpp *.h)

include_directories(include include/AntTweakBar src)

add_library(AntTweakBar ${SRC_FILES})
