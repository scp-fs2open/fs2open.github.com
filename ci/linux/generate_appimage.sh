#!/usr/bin/env bash

INSTALL_FOLDER=$1

# safety check
if [ ! -d bin -o ! -f cmake_install.cmake ]; then
	echo "ERROR! This script must be run from within the build root!"
	exit 1
fi

if [ ! -x "$(which wget)" ]; then
	echo "ERROR! Required utility is not available: wget"
	exit 1
fi

# install newest appimagetool if it's not already available
if [ ! -x ./bin/appimagetool ]; then
	APPIMAGE_TOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-$(uname -m).AppImage"

	wget -O ./bin/appimagetool "$APPIMAGE_TOOL_URL" || { echo "ERROR! Failed to get appimagetool!" && exit 1; }
	chmod +x ./bin/appimagetool
fi

# This shouldn't be needed with newer runtimes, but they still generate an
# error if fusermount is missing even though it works. So to skip error msg
# and have max compatibility with runtimes and containers we'll use it.
export APPIMAGE_EXTRACT_AND_RUN=1

# Install Freespace2 targets
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER/Freespace2 -DCOMPONENT=Unspecified -P cmake_install.cmake
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER/Freespace2 -DCOMPONENT=Freespace2 -P cmake_install.cmake

# We need to be a bit creative for determining the AppImage name since we don't want to hard-code the name
FILENAME="$(find $INSTALL_FOLDER/Freespace2/bin -name 'fs2_open_*' -type f -printf "%f\n").AppImage"
./bin/appimagetool -n "$INSTALL_FOLDER/Freespace2" "$INSTALL_FOLDER/$FILENAME"
chmod +x "$INSTALL_FOLDER/$FILENAME"

# Maybe install qtFRED targets
if [ -f qtfred/cmake_install.cmake ]; then
	cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER/qtFRED -DCOMPONENT=Unspecified -P cmake_install.cmake
	cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_FOLDER/qtFRED -DCOMPONENT=qtFRED -P cmake_install.cmake

	# We need to be a bit creative for determining the AppImage name since we don't want to hard-code the name
	FILENAME="$(find $INSTALL_FOLDER/qtFRED/bin -iname 'qtfred_*' -type f -printf "%f\n").AppImage"
	./bin/appimagetool -n "$INSTALL_FOLDER/qtFRED" "$INSTALL_FOLDER/$FILENAME"
	chmod +x "$INSTALL_FOLDER/$FILENAME"
fi

ls -al $INSTALL_FOLDER
