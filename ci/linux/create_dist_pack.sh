#!/usr/bin/env bash

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

OS="$1"

source $HERE/dist_functions.sh

if [ "$OS" = "Linux" ]; then
    tar -cvzf "$(get_package_name)-builds-Linux-$ARCH.tar.gz" *

    echo "package_path=$(pwd)/$(get_package_name)-builds-Linux-$ARCH.tar.gz" >> $GITHUB_OUTPUT
    echo "package_name=$(get_package_name)-builds-Linux-$ARCH.tar.gz" >> $GITHUB_OUTPUT
    echo "package_mime=$(file -b --mime-type "$(pwd)/$(get_package_name)-builds-Linux-$ARCH.tar.gz")" >> $GITHUB_OUTPUT
elif [ "$OS" = "Windows" ]; then
    7z a -xr'!*.pdb' "$(get_package_name)-builds-$ARCH-$SIMD.zip" "*"

    echo "package_path=$(pwd)/$(get_package_name)-builds-$ARCH-$SIMD.zip" >> $GITHUB_OUTPUT
    echo "package_name=$(get_package_name)-builds-$ARCH-$SIMD.zip" >> $GITHUB_OUTPUT
    echo "package_mime=$(file -b --mime-type "$(pwd)/$(get_package_name)-builds-$ARCH-$SIMD.zip")" >> $GITHUB_OUTPUT

    7z a "$(get_package_name)-debug-$ARCH-$SIMD.7z" "*.pdb"

    echo "debug_path=$(pwd)/$(get_package_name)-debug-$ARCH-$SIMD.7z" >> $GITHUB_OUTPUT
    echo "debug_name=$(get_package_name)-debug-$ARCH-$SIMD.7z" >> $GITHUB_OUTPUT
    echo "debug_mime=$(file -b --mime-type "$(pwd)/$(get_package_name)-debug-$ARCH-$SIMD.7z")" >> $GITHUB_OUTPUT
elif [ "$OS" = "Mac" ]; then
    tar -cvzf "$(get_package_name)-builds-Mac-$ARCH.tar.gz" *.app

    echo "package_path=$(pwd)/$(get_package_name)-builds-Mac-$ARCH.tar.gz" >> $GITHUB_OUTPUT
    echo "package_name=$(get_package_name)-builds-Mac-$ARCH.tar.gz" >> $GITHUB_OUTPUT
    echo "package_mime=$(file -b --mime-type "$(pwd)/$(get_package_name)-builds-Mac-$ARCH.tar.gz")" >> $GITHUB_OUTPUT
else
    echo "Invalid OS: $OS"
fi
