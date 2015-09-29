#!/usr/bin/env sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Nothing to do here
    :
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty
fi
