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
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>") # set dynamic lib
    add_compile_options(/W4)
    
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_win")
elseif(APPLE)
    add_compile_options(-Wall -Wextra)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_mac"   CACHE PATH "Output directory for macOS")
    ## OpenSSL root for MacOS
    set(OPENSSL_ROOT_DIR    "/usr/local/opt/openssl@3"    CACHE PATH "OpenSSL root directory")
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include" CACHE PATH "OpenSSL include directory")
    set(OPENSSL_LIBRARIES   "${OPENSSL_ROOT_DIR}/lib/libssl.dylib;${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib" CACHE STRING "OpenSSL libraries")
else() # Linux & Unix-like
    add_compile_options(-Wall -Wextra)
    # Remove static linking flags
    unset(CMAKE_FIND_LIBRARY_SUFFIXES)
    set(BUILD_SHARED_LIBS ON)
    unset(CMAKE_EXE_LINKER_FLAGS)

    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_unix")
endif()

file(MAKE_DIRECTORY ${OUTPUT_DIR})

add_subdirectory(src)

configure_file(${CMAKE_SOURCE_DIR}/src/config.json ${OUTPUT_DIR}/config.json COPYONLY)