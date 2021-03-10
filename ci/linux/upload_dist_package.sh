#!/usr/bin/env bash

set -e

UPLOAD_TYPE=$1

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

source $HERE/dist_functions.sh

if [ "$UPLOAD_TYPE" = "nightly" ]; then
    SSHPASS=$INDIEGAMES_SSHPASS upload_files_to_sftp "$INDIEGAMES_USER@scp.indiegames.us" "public_html/builds/nightly"

    SSHPASS=$DATACORDER_SSHPASS upload_files_to_sftp "$DATACORDER_USER@porphyrion.feralhosting.com" "www/datacorder.porphyrion.feralhosting.com/public_html/builds/nightly"
elif [ "$UPLOAD_TYPE" = "test" ]; then
    SSHPASS=$DATACORDER_SSHPASS upload_files_to_sftp "$DATACORDER_USER@porphyrion.feralhosting.com" "www/datacorder.porphyrion.feralhosting.com/public_html/builds/test"
else
    echo "Unknown upload type $UPLOAD_TYPE"
    exit 1
fi
