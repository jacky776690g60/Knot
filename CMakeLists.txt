cmake_minimum_required(VERSION 3.10)
project(Knot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


### Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    # add_compile_options(/wd4100 /wd4189) # Disable Specific warnings
else()
    add_compile_options(-Wall -Wextra)
endif()

### Set output directory
if(MSVC)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_win")
else()
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/_knot_unix")
endif()


file(MAKE_DIRECTORY ${OUTPUT_DIR})

### Add src dir
add_subdirectory(src)

### Copy config files
configure_file(${CMAKE_SOURCE_DIR}/src/config.json ${OUTPUT_DIR}/config.json COPYONLY)


