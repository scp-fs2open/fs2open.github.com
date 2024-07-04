#!/usr/bin/env bash

set -e

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

COMMIT_RANGE=$1..$2

if ! git rev-list $COMMIT_RANGE 2>&1 >/dev/null; then
    echo "\$COMMIT_RANGE ($COMMIT_RANGE) is not valid for this build (probably caused by a rebase)."
    exit 0
fi

# In order to get a proper diff without irrelevant changes, we need to determine where we branched off from the base
# branch
BASE_COMMIT=$(git merge-base $1 $2)

echo "Running clang-tidy on changed files"
git diff -U0 --no-color "$BASE_COMMIT..$2" | \
    $HERE/clang-tidy-diff.py -path "$(pwd)/build" -p1 \
    -regex '(code(?!((\/graphics\/shaders\/compiled)|(\/globalincs\/windebug)))|freespace2|qtfred|test|build|tools)\/.*\.(cpp|h)' \
    -clang-tidy-binary /usr/bin/clang-tidy-9 -j$(nproc) -export-fixes "$(pwd)/clang-fixes.yaml"
