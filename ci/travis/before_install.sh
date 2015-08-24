#!/usr/bin/env sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo add-apt-repository --yes ppa:zoogie/sdl2-snapshots
    sudo apt-get update -qq
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    # Nothing to do here
    :
fi
