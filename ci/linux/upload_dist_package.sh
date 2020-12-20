#!/usr/bin/env bash

set -e

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

source $HERE/dist_functions.sh

SSHPASS=$INDIEGAMES_SSHPASS upload_files_to_sftp "$INDIEGAMES_USER@scp.indiegames.us" "public_html/builds/nightly"

SSHPASS=$DATACORDER_SSHPASS upload_files_to_sftp "$DATACORDER_USER@porphyrion.feralhosting.com" "www/datacorder.porphyrion.feralhosting.com/public_html/builds/nightly"
