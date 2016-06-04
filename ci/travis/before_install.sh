#!/usr/bin/env sh

set -ex

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Nothing to do here
    :
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    # Nothing to do here
    :
fi
