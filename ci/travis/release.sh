#!/usr/bin/env sh

set -ex
mkdir -p /tmp/builds

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
	cd build
    for config in $BUILD_CONFIGS
    do
        cd "$config"
        ninja install appimage
        cd ..
    done

    ls -al /tmp/release
    (cd /tmp/release && tar -cvzf /tmp/builds/$PACKAGE_NAME-builds-Linux.tar.gz *.AppImage)
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd build
    
    for config in $BUILD_CONFIGS
    do
        cmake --build . --config "$config" --target install | tee build.log \
            | xcpretty -f `xcpretty-travis-formatter`
        XCODE_RET=${PIPESTATUS[0]}
        if [ "$XCODE_RET" -ne "0" ]; then
            pastebin -e 1d -f build.log
            exit $XCODE_RET
        fi
    done

    ls -al /tmp/release
    
    (cd /tmp/release && tar -cvzf /tmp/builds/$PACKAGE_NAME-builds-MacOSX.tar.gz *)
fi
