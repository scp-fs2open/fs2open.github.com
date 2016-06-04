#!/usr/bin/env sh

RELEASE_BUILD=false
NIGHTLY_BUILD=false
BUILD_DEPLOYMENT=false
VERSION_NAME=""

if [[ "$TRAVIS_TAG" =~ ^release.* ]]; then
    echo "This is a release tag!";
    RELEASE_BUILD=true;
    BUILD_DEPLOYMENT=true;
fi
if [[ "$TRAVIS_TAG" =~ ^nightly.* ]]; then
    echo "This is a nightly tag!";
    NIGHTLY_BUILD=true;
    BUILD_DEPLOYMENT=true;
    VERSION_NAME="$TRAVIS_COMMIT"
fi
VERSION_NAME="$TRAVIS_COMMIT"
NIGHTLY_BUILD=true
BUILD_DEPLOYMENT=true

if [[ "$BUILD_DEPLOYMENT" == true ]]; then
    if ([[ "$TRAVIS_OS_NAME" == "linux" ]] && [[ "$CC" == "clang" ]]); then
        echo "Skipping non-release compiler";
        exit 0;
    fi

    if ([[ "$TRAVIS_OS_NAME" == "osx" ]] && [[ "$MACOSX_ARCH" == "i386" ]]); then
        echo "Skipping non-release architecture";
        exit 0;
    fi

    if ([[ "$CONFIGURATION" == "Debug" ]]); then
        echo "Skipping non-release configuration";
        exit 0;
    fi
fi

export RELEASE_BUILD
export NIGHTLY_BUILD
export BUILD_DEPLOYMENT
export VERSION_NAME