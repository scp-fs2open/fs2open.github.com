#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Due to a bug in gcc the array bounds check isn't working correctly
    # This can be removed when gcc is updated
    AUTOGEN_CONFIG="--enable-generic-architecture --prefix=/tmp/release"

    # First compile the release build
    ./autogen.sh $AUTOGEN_CONFIG

    make -j4
    make install

    make distclean

    # Now compile debug
    ./autogen.sh $AUTOGEN_CONFIG --enable-debug

    make -j4
    make install

    ls -al /tmp/release/bin
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode

    mkdir /tmp/release

    xcodebuild -configuration "Release" clean build | xcpretty -c
    (cd build/Release && cp *.app /tmp/release)

    xcodebuild -configuration "Debug" clean build | xcpretty -c
    (cd build/Debug && cp *.app /tmp/release)

    ls -al /tmp/release
fi
