#!/usr/bin/env bash

set -e
set -u

if [[ ! "$(curl -V)" == *"sftp"* ]]; then
    # Before
    echo "Before:"
    curl -V

    if [ "$TRAVIS_OS_NAME" = "linux" ]; then

        if [ -f ~/curl_cache/curl*.deb ]; then
            cd ~/curl_cache
            sudo dpkg -i libcurl3*.deb
            sudo dpkg -i curl*.deb
        else
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

            if [ ! -d ~/curl_cache ]; then
                mkdir ~/curl_cache
            fi

            cp libcurl3*.deb curl*.deb ~/curl_cache

            sudo dpkg -i libcurl3*.deb
            sudo dpkg -i curl*.deb
        fi
    elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
        brew install libssh2
        
        if [ -d ~/curl_cache/curl ]; then
            cd ~/curl_cache/curl
            sudo make install
        else
            mkdir /tmp/curl
            cd /tmp/curl

            curl -LO "https://curl.haxx.se/download/curl-7.64.0.tar.bz2"
            tar -xjf curl-7.64.0.tar.bz2

            cd curl-7.64.0
            ./configure --disable-debug --disable-dependency-tracking --disable-silent-rules --with-libssh2 --with-darwinssl --without-ca-bundle --without-ca-path
            make -j"$(sysctl hw.logicalcpu | cut -d " " -f 2)"

            if [ ! -d ~/curl_cache ]; then
                mkdir ~/curl_cache
            fi

            cp -a . ~/curl_cache/curl
            sudo make install
        fi
        export PATH=/usr/local/bin:$PATH
    fi

    # After
    echo "After:"
    curl -V
fi

cd /tmp/builds

for file in *; do
    if [ "$TEST_BUILD" = true ]; then
        # Upload to datacorder
        curl -k "sftp://porphyrion.feralhosting.com/www/datacorder.porphyrion.feralhosting.com/public_html/builds/test/$VERSION_NAME/" --user "$DATACORDER_USER:$DATACORDER_PASSWORD" -T "$file" --ftp-create-dirs
    else
        # Upload to indiegames
        curl -k "sftp://scp.indiegames.us/~/public_html/builds/nightly/$VERSION_NAME/" --user "$INDIEGAMES_USER:$INDIEGAMES_PASSWORD" -T "$file" --ftp-create-dirs

        # Upload to datacorder
        curl -k "sftp://porphyrion.feralhosting.com/www/datacorder.porphyrion.feralhosting.com/public_html/builds/nightly/$VERSION_NAME/" --user "$DATACORDER_USER:$DATACORDER_PASSWORD" -T "$file" --ftp-create-dirs
    fi
done
