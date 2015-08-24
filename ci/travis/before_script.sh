#!/usr/bin/env sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    AUTOGEN_CONFIG=
    
    if [ "$CONFIGURATION" = "Debug" ]; then
        AUTOGEN_CONFIG=--enable-debug
    fi
    
    ./autogen.sh $AUTOGEN_CONFIG
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode4
    tar -xvzf Frameworks.tgz
fi
