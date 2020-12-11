#!/usr/bin/env bash

set -e

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

COMMIT_RANGE=$1..$2

if ! git rev-list $COMMIT_RANGE 2>1 >/dev/null; then
    echo "\$COMMIT_RANGE ($COMMIT_RANGE) is not valid for this build (probably caused by a rebase)."
    exit 0
fi

echo "Running clang-tidy on changed files"
git diff -U0 --no-color $COMMIT_RANGE | \
    $HERE/clang-tidy-diff.py -path "$(pwd)/build" -p1 \
    -regex '(code(?!\/graphics\/shaders\/compiled)|freespace2|qtfred|test|build|tools)\/.*\.(cpp|h)' \
    -clang-tidy-binary /usr/bin/clang-tidy-9 -j$(nproc) -quiet | \
    tee /tmp/clang-tidy-output.txt

if [[ -n $(grep "warning: " /tmp/clang-tidy-output.txt) ]] || [[ -n $(grep "error: " /tmp/clang-tidy-output.txt) ]]; then
    echo "Errors or warnings were detected in the clang-tidy output!"
    exit 1
else
    echo -e "\033[1;32m\xE2\x9C\x93 passed:\033[0m clang-tidy test succeeded";
fi
