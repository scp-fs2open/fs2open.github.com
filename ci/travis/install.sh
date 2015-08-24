#!/usr/bin/env sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo apt-get install libopenal-dev libogg-dev libvorbis-dev build-essential automake1.10 autoconf libsdl2-dev libtheora-dev libreadline6-dev libpng12-dev libjpeg62-dev liblua5.1-0-dev libjansson-dev libtool
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty
fi
