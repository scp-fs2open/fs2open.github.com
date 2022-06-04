#!/usr/bin/env bash

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

OS="$1"

source $HERE/dist_functions.sh

if [ "$OS" = "Linux" ]; then
    tar -cvzf "$(get_package_name)-builds-Linux.tar.gz" *

    echo "::set-output name=package_path::$(pwd)/$(get_package_name)-builds-Linux.tar.gz"
    echo "::set-output name=package_name::$(get_package_name)-builds-Linux.tar.gz"
    echo "::set-output name=package_mime::$(file -b --mime-type "$(pwd)/$(get_package_name)-builds-Linux.tar.gz")"
elif [ "$OS" = "Windows" ]; then
    7z a -xr'!*.pdb' "$(get_package_name)-builds-$ARCH-$SIMD.zip" "*"

    echo "::set-output name=package_path::$(pwd)/$(get_package_name)-builds-$ARCH-$SIMD.zip"
    echo "::set-output name=package_name::$(get_package_name)-builds-$ARCH-$SIMD.zip"
    echo "::set-output name=package_mime::$(file -b --mime-type "$(pwd)/$(get_package_name)-builds-$ARCH-$SIMD.zip")"

    7z a "$(get_package_name)-debug-$ARCH-$SIMD.7z" "*.pdb"

    echo "::set-output name=debug_path::$(pwd)/$(get_package_name)-debug-$ARCH-$SIMD.7z"
    echo "::set-output name=debug_name::$(get_package_name)-debug-$ARCH-$SIMD.7z"
    echo "::set-output name=debug_mime::$(file -b --mime-type "$(pwd)/$(get_package_name)-debug-$ARCH-$SIMD.7z")"
elif [ "$OS" = "Mac" ]; then
    tar -cvzf "$(get_package_name)-builds-Mac.tar.gz" *

    echo "::set-output name=package_path::$(pwd)/$(get_package_name)-builds-Mac.tar.gz"
    echo "::set-output name=package_name::$(get_package_name)-builds-Mac.tar.gz"
    echo "::set-output name=package_mime::$(file -b --mime-type "$(pwd)/$(get_package_name)-builds-Mac.tar.gz")"
else
    echo "Invalid OS: $OS"
fi
