#!/usr/bin/env bash

set -e
set -u

if [[ ! "$(curl -V)" == *"sftp"* ]]; then
    # Before
    echo "Before:"
    curl -V

    if [ "$TRAVIS_OS_NAME" = "linux" ]; then

        # Since the Travis CI people are not capable of enabling SSH/SCP support for their libcurl we need to build that manually...
        # These commands are based on the tutorial from here: http://zeroset.mnim.org/2013/03/14/sftp-support-for-curl-in-ubuntu-12-10-quantal-quetzal-and-later/
        mkdir /tmp/curl
        cd /tmp/curl
        sudo apt-get update
        sudo apt-get install build-essential debhelper libssh2-1-dev libgnutls-dev libidn11-dev libkrb5-dev libldap2-dev libnss3-dev librtmp-dev libtool openssh-server quilt
        apt-get source curl
        sudo apt-get build-dep curl

        cd curl-*
        export DEB_BUILD_OPTIONS="nocheck"
        sudo dpkg-buildpackage -uc -us -j"$(grep -c ^processor /proc/cpuinfo)"

        cd ..
        dpkg -l | grep curl
        ls -al
        sudo dpkg -i libcurl3*.deb
        sudo dpkg -i curl*.deb
    elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
        brew install curl --with-libssh2
        export PATH="/usr/local/opt/curl/bin:$PATH"
    fi

    # After
    echo "After:"
    curl -V
fi

cd /tmp/builds

for file in *; do
    if [ "$TEST_BUILD" = true ]; then
        # Upload to fs2downloads
        curl -k "ftp://swc.fs2downloads.com/swc.fs2downloads.com/builds/test/$VERSION_NAME/" --user "$FS2DOWNLOADS_USER:$FS2DOWNLOADS_PASSWORD" -T "$file" --ftp-create-dirs
    else
        # Upload to indiegames
        curl -k "sftp://scp.indiegames.us/~/public_html/builds/nightly/$VERSION_NAME/" --user "$INDIEGAMES_USER:$INDIEGAMES_PASSWORD" -T "$file" --ftp-create-dirs

        # Upload to fs2downloads
        curl -k "ftp://swc.fs2downloads.com/swc.fs2downloads.com/builds/nightly/$VERSION_NAME/" --user "$FS2DOWNLOADS_USER:$FS2DOWNLOADS_PASSWORD" -T "$file" --ftp-create-dirs
    fi
done
