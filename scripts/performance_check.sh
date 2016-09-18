#!/usr/bin/env bash

# This script tests for performance changes between two branches in this repository.

set -ex

if [ "$#" -lt 1 ] || [ "$1" == "--help" ]; then
    echo "Runs FSO two times based off different branches to compare their performance"
    echo "Usage: ./performance_check.sh <test_branch> <fso_parameters>*"
    exit 1
fi

BRANCH="$1"
shift

echo "$*"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
FSO_REPO=$(readlink -f "$DIR/../")
FSO_DIR="$(printenv FS2PATH)"

cd "$FSO_REPO"

mkdir -p build/performance_test

function run_test {
    pushd build/performance_test

    NAME="$1"
    shift

    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DFSO_RUN_ARGUMENTS="$*" -DFSO_FREESPACE_PATH="$FSO_DIR" "${FSO_REPO}"
    ninja launch_fso
    cp "$FSO_DIR/profiling.csv" "/tmp/$NAME.csv"

    popd
}

git checkout master
run_test master "$@"

git checkout "${BRANCH}"
run_test changed "$@"

$DIR/performance_graph.py --base_data "/tmp/master.csv" --base_title "master" \
    --changed_data "/tmp/changed.csv" --changed_title "${BRANCH}"
