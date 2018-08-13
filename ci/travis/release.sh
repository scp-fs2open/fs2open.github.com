#!/usr/bin/env sh

set -ex
mkdir -p /tmp/builds

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
	cd build
    for config in $BUILD_CONFIGS
    do
        cd "$config"

        # Compile all targets
        ninja all

        # Install Freespace2 targets
        cmake -DCMAKE_INSTALL_PREFIX=/tmp/release/$config/fso -DCOMPONENT=Unspecified -P cmake_install.cmake
        cmake -DCMAKE_INSTALL_PREFIX=/tmp/release/$config/fso -DCOMPONENT=Freespace2 -P cmake_install.cmake

        # We need to be a bit creative for determining the AppImage name since we don't want to hard-code the name
        FILENAME="$(find /tmp/release/$config/fso/bin -name 'fs2_open_*' -type f -printf "%f\n").AppImage"
        $HOME/appimagetool -n /tmp/release/$config/fso "/tmp/release/$config/fso/$FILENAME"

        # Install qtFRED targets
        cmake -DCMAKE_INSTALL_PREFIX=/tmp/release/$config/qtfred/usr -DCOMPONENT=Unspecified -P cmake_install.cmake
        cmake -DCMAKE_INSTALL_PREFIX=/tmp/release/$config/qtfred/usr -DCOMPONENT=qtFRED -P cmake_install.cmake

        PREV="$(pwd)"
        cd /tmp/release/$config/qtfred

        unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH
        $HOME/linuxdeployqt usr/qtfred.desktop -bundle-non-qt-libs
        # Move our AppRun script to the right location
        mv usr/AppRun AppRun

        echo "Embed newer libstdc++ for distros that don't come with it (ubuntu 14.04)";
        mkdir -p usr/optional/; mkdir -p usr/optional/libstdc++/;
        cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 ./usr/optional/libstdc++/;

        $HOME/appimagetool -n . "$(find /tmp/release/$config/qtfred/usr/bin -name 'qtfred_*' -type f -printf "%f\n").AppImage"

        cd "$PREV"

        cd ..
    done

    cp `find /tmp/release -name '*.AppImage' -print` /tmp/release
    (cd /tmp/release && tar -cvzf /tmp/builds/$PACKAGE_NAME-builds-Linux.tar.gz *.AppImage)
    curl --upload-file /tmp/builds/$PACKAGE_NAME-builds-Linux.tar.gz "https://transfer.sh/$PACKAGE_NAME-builds-Linux.tar.gz"
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
