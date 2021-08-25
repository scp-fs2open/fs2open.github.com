#!/usr/bin/env sh

set -e

cd build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    ninja -k 20 all

    if [ "$CONFIGURATION" = "Debug" ]; then
        valgrind --leak-check=full --error-exitcode=1 --gen-suppressions=all \
            --suppressions="$TRAVIS_BUILD_DIR/ci/travis/valgrind.supp" ./bin/unittests --gtest_shuffle
    else
        ./bin/unittests --gtest_shuffle
    fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake --build . --config "$CONFIGURATION" | tee build.log | xcpretty
    XCODE_RET=${PIPESTATUS[0]}
    if [ "$XCODE_RET" -ne "0" ]; then
        tar -cvzf build.log.tar.gz build.log
        curl --upload-file build.log.tar.gz "https://keep.sh"
        exit $XCODE_RET
    fi
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    if [ "$CMAKE_GENERATOR" = "Ninja" ]; then
        cmake --build . --config "$CONFIGURATION"
    else
        cmake --build . --config "$CONFIGURATION" -- /verbosity:minimal
    fi
fi
