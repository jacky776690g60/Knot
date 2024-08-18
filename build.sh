#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

BUILD_DIR="build"
SOURCE_DIR=$(pwd)
FORCE=false

# Function to clean up and return to the original directory
cleanup() {
    echo "Returning to original directory: $SOURCE_DIR"
    cd "$SOURCE_DIR"
}

# Set up trap to ensure cleanup happens even if the script fails
trap cleanup EXIT

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
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory does not exist. Run with --force to create it."
    exit 1
fi

echo "Changing to $BUILD_DIR directory ..."
cd "$BUILD_DIR"

if $FORCE; then
    echo "Running CMake ..."
    if ! cmake "$SOURCE_DIR"; then
        echo "CMake configuration failed. Exiting."
        exit 1
    fi
fi

echo "Building project ..."
if ! make; then
    echo "Build failed. Exiting."
    exit 1
fi

echo "Build complete."
echo "Executables can be found in $BUILD_DIR/dist/"