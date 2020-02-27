cmake_minimum_required(VERSION 3.13)
project(pnglib C)
set(CMAKE_C_STANDARD 11)

set(ZLIB_SRCS  ${ZLIB_SRCS}  contrib/masmx64/inffas8664.c )

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0 zlib)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFALGS_OTHER})

add_executable(exe main.c pnglib.c zlib.c zlib.h huffmanTree.c huffmanTree.h filteringAlgorithms.c UI.c UI.h)

target_link_libraries(exe ${GTK3_LIBRARIES} ${ZLIB_LIBRARIES})