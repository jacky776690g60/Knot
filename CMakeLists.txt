# ================================================================
# CMakeLists.txt  --  Knot/CMakeLists.txt
#
# #Author Jack
# Created on 10/19, 2024
# Copyright © 2024 jacktogon. All rights reserved.
# ================================================================
cmake_minimum_required(VERSION 3.10)

project(Knot
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)


if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # Set static runtime library for MSVC
    add_compile_options(/W4)
    # add_compile_options(/wd4100 /wd4189) # Disable Specific warnings
    
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_win")
else()
    add_compile_options(-Wall -Wextra)
    
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_unix")
endif()
file(MAKE_DIRECTORY ${OUTPUT_DIR})


add_subdirectory(src)

configure_file(${CMAKE_SOURCE_DIR}/src/config.json ${OUTPUT_DIR}/config.json COPYONLY)