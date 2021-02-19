#!/usr/bin/env bash

INSTALL_FOLDER=$1

# Install Freespace2 targets
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER -DCOMPONENT=Unspecified -P cmake_install.cmake
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER -DCOMPONENT=Freespace2 -P cmake_install.cmake

# We need to be a bit creative for determining the AppImage name since we don't want to hard-code the name
FILENAME="$(find $INSTALL_FOLDER/bin -name 'fs2_open_*' -type f -printf "%f\n").AppImage"
appimagetool -n $INSTALL_FOLDER "$INSTALL_FOLDER/$FILENAME"
chmod +x "$INSTALL_FOLDER/$FILENAME"

ls -al $INSTALL_FOLDER
