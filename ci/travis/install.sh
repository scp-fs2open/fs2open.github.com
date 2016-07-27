#!/usr/bin/env sh

set -ex

FILENAME=
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    if [ ! -d "$HOME/cmake/bin" ]; then
        # If the cache does not currently contain CMake then download and install it
        FILENAME=cmake-3.4.3-Linux-x86_64

    	mkdir -p $HOME/cmake/

    	wget -O /tmp/cmake.tar.gz --no-check-certificate https://www.cmake.org/files/v3.4/$FILENAME.tar.gz
    	tar -xzf /tmp/cmake.tar.gz -C $HOME/cmake/ --strip-components=1
    fi
	export PATH=$HOME/cmake/bin:$PATH
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty xcpretty-travis-formatter thefox-pastebin
    
    brew update
    brew outdated cmake || brew upgrade cmake
fi

cmake --version
