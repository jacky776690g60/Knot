#!/bin/bash

BUILD_DIR="build"
SOURCE_DIR=$(pwd)
FORCE=false

# Check for --force flag
if [[ "$1" == "--force" ]]; then
    FORCE=true
fi

if $FORCE; then
    if [ -d "$BUILD_DIR" ]; then
        echo "Removing $BUILD_DIR/ first ..."
        rm -rf "$BUILD_DIR"
    fi

    echo "Creating $BUILD_DIR directory ..."
    mkdir "$BUILD_DIR"

    echo "Changing to $BUILD_DIR directory ..."
    cd "$BUILD_DIR" || exit

    echo "Running CMake ..."
    cmake "$SOURCE_DIR"

    if [ $? -ne 0 ]; then
        echo "CMake configuration failed. Exiting."
        exit 1
    fi
else
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Build directory does not exist. Run with --force to create it."
        exit 1
    fi
    echo "Changing to $BUILD_DIR directory ..."
    cd "$BUILD_DIR" || exit
fi

echo "Building project ..."
make

if [ $? -ne 0 ]; then
    echo "Build failed. Exiting."
    exit 1
fi

echo "Build complete."

echo "Executables can be found in $BUILD_DIR/dist/"