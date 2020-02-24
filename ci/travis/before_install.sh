#!/usr/bin/env sh

set -ex

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    export DEBIAN_FRONTEND=noninteractive

    sudo add-apt-repository -y "ppa:msulikowski/valgrind" # For fixing a bug with valgrind 3.11 which does not recognize the rdrand instruction

    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -
    sudo add-apt-repository -y 'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9 main'

    sudo apt-get -yq update
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    # Nothing to do here
    :
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    # Nothing to do here
    :
fi
