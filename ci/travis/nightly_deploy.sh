#!/usr/bin/env sh

set -x
set -e
set -u

for file in /tmp/builds/*; do
	# Upload to indiegames
	curl -k "sftp://scp.indiegames.us:22/home/$INDIEGAMES_USER/public_html/builds/nightly/$VERSION_NAME/" --user "$INDIEGAMES_USER:$INDIEGAMES_PASSWORD" -T "$file" --ftp-create-dirs

	# Upload to fs2downloads
	curl -k "ftp://swc.fs2downloads.com:21/swc.fs2downloads.com/builds/nightly/$VERSION_NAME/" --user "$FS2DOWNLOADS_USER:$FS2DOWNLOADS_PASSWORD" -T "$file" --ftp-create-dirs
done