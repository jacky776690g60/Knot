cmake_minimum_required(VERSION 3.10)
project(Knot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_executable(encrypter encrypter.cpp)
add_executable(decrypter decrypter.cpp)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist/knot")

set_target_properties(encrypter decrypter
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)

# Copy config.json to the output directory
configure_file(${CMAKE_SOURCE_DIR}/config.json ${OUTPUT_DIR}/config.json COPYONLY)