#!/usr/bin/env bash

set -e

UPLOAD_TYPE=$1

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

source $HERE/dist_functions.sh

if [ "$UPLOAD_TYPE" = "nightly" ]; then
    any_success=false
    if SSHPASS=$INDIEGAMES_SSHPASS upload_files_to_sftp "$INDIEGAMES_USER@scp.indiegames.us" "public_html/builds/nightly"; then
        any_success=true
    fi

    if SSHPASS=$DATACORDER_SSHPASS upload_files_to_sftp "$DATACORDER_USER@perses.feralhosting.com" "www/datacorder.perses.feralhosting.com/public_html/builds/nightly"; then
        any_success=true
    fi

    if [ "$any_success" = "false" ]; then
        echo "All uploads failed"
        exit 1
    fi
elif [ "$UPLOAD_TYPE" = "test" ]; then
    SSHPASS=$DATACORDER_SSHPASS upload_files_to_sftp "$DATACORDER_USER@perses.feralhosting.com" "www/datacorder.perses.feralhosting.com/public_html/builds/test"
else
    echo "Unknown upload type $UPLOAD_TYPE"
    exit 1
fi
