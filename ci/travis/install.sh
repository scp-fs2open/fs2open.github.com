#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo apt-get install libsdl2-dev
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty
fi
