#!/bin/bash
# This script will package release artifacts for macOS and 64-bit Linux in a single tar.gz.
# It’s assumed to be executed on macOS.

set -eax

SCRIPT_DIR=$(dirname $0)
cd $SCRIPT_DIR

VERSION=$(git show-ref -s --abbrev=8 HEAD)

# Clean up previous releases
rm -rf $SCRIPT_DIR/build_cmake/{macos,unix}

# Building macOS
build_cmake/scripts/build_macos.sh

# Building Linux release on mac
docker build --tag cblite:latest - < Dockerfile
docker run -i -t -v $(pwd):/source cblite:latest build_cmake/scripts/build_unix.sh

# Packaging
(cd build_cmake; find . -name "*.so" -o -name "*.dylib") | xargs tar -C build_cmake -czvf $SCRIPT_DIR/cblite-$VERSION.tar.gz
