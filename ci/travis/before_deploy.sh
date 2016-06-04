#!/usr/bin/env sh

set -ex

if [[ "$NIGHTLY_BUILD" == true ]]; then
    echo "Fixing bintray config..."
    sed -i "s/\$VERSION_NAME/$VERSION_NAME/g" ci/bintray.json
fi