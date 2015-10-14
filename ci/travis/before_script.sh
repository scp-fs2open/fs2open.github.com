#!/usr/bin/env sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Due to a bug in gcc the array bounds check isn't working correctly
    # This can be removed when gcc is updated
    AUTOGEN_CONFIG="CXXFLAGS=-Wno-array-bounds --enable-fatal-warnings"
    
    if [ "$CONFIGURATION" = "Debug" ]; then
        AUTOGEN_CONFIG="$AUTOGEN_CONFIG --enable-debug"
    fi
    
    ./autogen.sh $AUTOGEN_CONFIG
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode4
    tar -xvzf Frameworks.tgz
fi
