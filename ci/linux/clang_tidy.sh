#!/usr/bin/env bash

set -e

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

git config --global --add safe.directory "$GITHUB_WORKSPACE"

COMMIT_RANGE=$1..$2

if ! git rev-list $COMMIT_RANGE 2>&1 >/dev/null; then
    echo "\$COMMIT_RANGE ($COMMIT_RANGE) is not valid for this build (probably caused by a rebase)."
    exit 0
fi

# In order to get a proper diff without irrelevant changes, we need to determine where we branched off from the base
# branch
BASE_COMMIT=$(git merge-base $1 $2)

# Allow the caller to override the clang-tidy binary and job count, since neither
# /usr/bin/clang-tidy-16 nor nproc are available outside the Linux CI container
# (e.g. macOS runners install clang-tidy via pip and use sysctl for core count).
CLANG_TIDY_BINARY="${CLANG_TIDY_BINARY:-/usr/bin/clang-tidy-16}"
NUM_JOBS="${NUM_JOBS:-$( (command -v nproc >/dev/null 2>&1 && nproc) || sysctl -n hw.ncpu )}"

# Note: Manually passing in the Vulkan flags that are normally provided by cmake (but are not so, here), to ensure
# that the source files are checked with the actual configuration used.
echo "Running clang-tidy on changed files"
git diff -U0 --no-color "$BASE_COMMIT..$2" | \
    $HERE/clang-tidy-diff.py -path "$(pwd)/build" -p1 \
    -extra-arg="-DWITH_VULKAN" \
    -extra-arg="-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1" \
    -extra-arg="-DVK_NO_PROTOTYPES" \
    -regex '(code(?!((\/graphics\/shaders\/compiled)|(\/globalincs\/windebug)|(\/def_files\/data)))|freespace2|qtfred|test\/src|build|tools)\/.*\.(cpp|h)' \
    -clang-tidy-binary "$CLANG_TIDY_BINARY" -j"$NUM_JOBS" -export-fixes "$(pwd)/clang-fixes.yaml"
