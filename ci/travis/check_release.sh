#!/usr/bin/env sh

RELEASE_BUILD=false
if [[ "$TRAVIS_TAG" =~ ^release.* ]]; then
    echo "This is a release tag!";
    RELEASE_BUILD=true;
fi

if ([[ "$RELEASE_BUILD" == true ]] && [[ "$TRAVIS_OS_NAME" == "linux" ]] && [[ "$CC" == "clang" ]]); then
    echo "Skipping non-release compiler";
    exit 0;
fi

if ([[ "$RELEASE_BUILD" == true ]] && [[ "$TRAVIS_OS_NAME" == "osx" ]] && [[ "$MACOSX_ARCH" == "i386" ]]); then
    echo "Skipping non-release architecture";
    exit 0;
fi

if ([[ "$RELEASE_BUILD" == true ]] && [[ "$CONFIGURATION" == "Debug" ]]); then
    echo "Skipping non-release configuration";
    exit 0;
fi
