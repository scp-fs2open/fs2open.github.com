#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Due to a bug in gcc the array bounds check isn't working correctly
    # This can be removed when gcc is updated
    AUTOGEN_CONFIG="--enable-generic-architecture --prefix=/tmp/release"

    # First compile the release build
    ./autogen.sh $AUTOGEN_CONFIG

    make -j4

    make distclean

    # Now compile debug
    ./autogen.sh $AUTOGEN_CONFIG --enable-debug

    make -j4

    ls -al /tmp/release/bin
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    :
fi
