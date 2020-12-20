#!/usr/bin/env bash

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

OS="$1"

source $HERE/dist_functions.sh

if [ "$OS" = "Linux" ]; then
    tar -cvzf "$(get_package_name)-builds-Linux.tar.gz" *
elif [ "$OS" = "Windows" ]; then
    7z a -xr'!*.pdb' "$(get_package_name)-builds-$ARCH-$SIMD.zip" "*"

    7z a "$(get_package_name)-debug-$ARCH-$SIMD.7z" "*.pdb"
else
    echo "Invalid OS: $OS"
fi
