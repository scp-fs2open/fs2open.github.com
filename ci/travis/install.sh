#!/usr/bin/env sh

set -ex

FILENAME=
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    if [ ! -d "$HOME/cmake-3.8/bin" ]; then
        # If the cache does not currently contain CMake then download and install it
    	mkdir -p $HOME/cmake-3.8/

    	wget -O /tmp/cmake.tar.gz --no-check-certificate https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.tar.gz
    	tar -xzf /tmp/cmake.tar.gz -C $HOME/cmake-3.8/ --strip-components=1
    fi
	export PATH=$HOME/cmake-3.8/bin:$PATH
    
    cd $HOME
    
    if [ "${BUILD_DEPLOYMENT}" = true ]; then
        wget -c "https://github.com/probonopd/AppImageKit/releases/download/5/AppImageAssistant" # (64-bit)
        chmod a+x ./AppImageAssistant
    fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty
    
    brew update
    brew outdated cmake || brew upgrade cmake
fi

cmake --version
