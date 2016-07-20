#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Due to a bug in gcc the array bounds check isn't working correctly
    # This can be removed when gcc is updated
    AUTOGEN_CONFIG="--enable-fatal-warnings --enable-generic-architecture"
	CXXFLAGS="-Wno-array-bounds -Wno-unknown-pragmas"
    
    if [ "$CONFIGURATION" = "Debug" ]; then
        AUTOGEN_CONFIG="$AUTOGEN_CONFIG --enable-debug"
    fi
    
	export CXXFLAGS
    ./autogen.sh $AUTOGEN_CONFIG
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode
    tar -xvzf Frameworks.tgz
fi
