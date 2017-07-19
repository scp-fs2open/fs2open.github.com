#!/usr/bin/env sh

set -ex
mkdir -p /tmp/builds

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    PARENT_DIR="$(basename $PWD)"
    (cd .. && tar cvf /tmp/builds/$PACKAGE_NAME-source-Unix.tar.gz --exclude-vcs "$PARENT_DIR")

	cd build
    for config in $BUILD_CONFIGS
    do
        cd "$config"
        ninja appimage
        cd ..
    done

    cp `find /tmp/release -name '*.AppImage' -print` /tmp/release
    (cd /tmp/release && tar -cvzf /tmp/builds/$PACKAGE_NAME-builds-Linux.tar.gz *.AppImage)
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd build
    
    for config in $BUILD_CONFIGS
    do
        cmake --build . --config "$config" --target install | tee build.log \
            | xcpretty
        XCODE_RET=${PIPESTATUS[0]}
        if [ "$XCODE_RET" -ne "0" ]; then
            tar -cvzf build.log.tar.gz build.log
            curl --upload-file build.log.tar.gz "https://transfer.sh/build.log.tar.gz"
            exit $XCODE_RET
        fi
    done

    ls -al /tmp/release
    
    (cd /tmp/release && tar -cvzf /tmp/builds/$PACKAGE_NAME-builds-MacOSX.tar.gz *)
fi
