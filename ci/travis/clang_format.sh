#!/usr/bin/env bash

set -e

if ! git rev-list $TRAVIS_COMMIT_RANGE 2>1 >/dev/null; then
    echo "\$TRAVIS_COMMIT_RANGE ($TRAVIS_COMMIT_RANGE) is not valid for this build (probably caused by a rebase)."
    exit 0
fi

cd "$TRAVIS_BUILD_DIR"

git diff -U0 --no-color $TRAVIS_COMMIT_RANGE | clang-format-diff-4.0 -p1 -i -sort-includes

if [[ -n $(git diff) ]]; then
    echo "Code style is not consistent with clang-format configuration! The following files were formatted incorrectly:"
    git diff --name-only
    exit 1
else
    echo -e "\033[1;32m\xE2\x9C\x93 passed:\033[0m Formatting test succeeded";
fi
