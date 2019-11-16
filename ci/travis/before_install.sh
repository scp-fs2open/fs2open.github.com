#!/usr/bin/env sh

set -ex

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    export DEBIAN_FRONTEND=noninteractive

    sudo add-apt-repository -y "ppa:msulikowski/valgrind" # For fixing a bug with valgrind 3.11 which does not recognize the rdrand instruction
    sudo apt-get -yq update
    sudo apt-get -yq install valgrind
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    # Nothing to do here
    :
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    # Nothing to do here
    :
fi
