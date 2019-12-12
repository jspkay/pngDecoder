cmake_minimum_required(VERSION 3.13)
project(pnglib C)

set(CMAKE_C_STANDARD 11)

add_executable(exe main.c pnglib.c zlib.c zlib.h huffmanTree.c huffmanTree.h)