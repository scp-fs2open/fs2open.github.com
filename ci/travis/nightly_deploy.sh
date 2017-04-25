#!/usr/bin/env sh

set -e
set -u

cd /tmp/builds

for file in *; do
	# Upload to indiegames
	curl -k "ftp://scp.indiegames.us/public_html/builds/nightly/$VERSION_NAME/" --user "$INDIEGAMES_USER:$INDIEGAMES_PASSWORD" -T "$file" --ftp-create-dirs

	# Upload to fs2downloads
	curl -k "ftp://swc.fs2downloads.com/swc.fs2downloads.com/builds/nightly/$VERSION_NAME/" --user "$FS2DOWNLOADS_USER:$FS2DOWNLOADS_PASSWORD" -T "$file" --ftp-create-dirs
done