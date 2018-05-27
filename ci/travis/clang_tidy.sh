#!/usr/bin/env bash

set -e

if ! git rev-list $TRAVIS_COMMIT_RANGE 2>1 >/dev/null; then
    echo "\$TRAVIS_COMMIT_RANGE ($TRAVIS_COMMIT_RANGE) is not valid for this build (probably caused by a rebase)."
    exit 0
fi

cd "$TRAVIS_BUILD_DIR"

echo "Running clang-tidy on changed files"
git diff -U0 --no-color $TRAVIS_COMMIT_RANGE | \
    $TRAVIS_BUILD_DIR/ci/travis/clang-tidy-diff.py -path "$TRAVIS_BUILD_DIR/build" -p1 \
    -regex 'code/.*$|freespace2/.*$|qtfred/.*$|test/src/.*$|build/.*$|tools/.*' \
    -clang-tidy-binary /usr/bin/clang-tidy-4.0 2>/dev/null | \
    tee clang-tidy-output.txt

if [[ -n $(grep "warning: " clang-tidy-output.txt) ]] || [[ -n $(grep "error: " clang-tidy-output.txt) ]]; then
    echo "Errors or warnings were detected in the clang-tidy output!"
    exit 1
else
    echo -e "\033[1;32m\xE2\x9C\x93 passed:\033[0m clang-tidy test succeeded";
fi
