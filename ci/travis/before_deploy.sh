#!/usr/bin/env sh

set -ex

if [ "$NIGHTLY_BUILD" = true ]; then
    echo "Fixing bintray config..."
    sed -e "s/\$VERSION_NAME/$VERSION_NAME/" ci/bintray.template.json > ci/bintray.json
fi