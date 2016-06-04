#!/usr/bin/env sh

set -ex

mkdir -p /tmp/builds

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    touch /tmp/builds/builds-Linux.tar.gz
    exit 0
    
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

    (cd /tmp/release/bin && tar -cvzf /tmp/builds/builds-Linux.tar.gz *)
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    touch /tmp/builds/builds-MacOSX.tar.gz
    exit 0
    
    cd projects/Xcode

    mkdir /tmp/release

    xcodebuild -project FS2_Open.xcodeproj -configuration "Release" clean build | tee release.log | xcpretty
    XCODE_RET=${PIPESTATUS[0]}
    if [ "$XCODE_RET" -ne "0" ]; then
        pastebin -e 1D -f release.log
        exit $XCODE_RET
    fi

    ls build/Release
    (cd build/Release && cp -r *.app /tmp/release)

    xcodebuild -project FS2_Open.xcodeproj -configuration "Debug" clean build | tee debug.log | xcpretty
    XCODE_RET=${PIPESTATUS[0]}
    if [ "$XCODE_RET" -ne "0" ]; then
        pastebin -e 1D -f debug.log
        exit $XCODE_RET
    fi

    ls build/Debug
    (cd build/Debug && cp -r *.app /tmp/release)

    ls -al /tmp/release

    (cd /tmp/release && tar -cvzf /tmp/builds/builds-MacOSX.tar.gz *)
fi
