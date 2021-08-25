#!/usr/bin/env bash

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

export LD_LIBRARY_PATH=$(pwd)/bin/lib:$LD_LIBRARY_PATH

if [ "$CONFIGURATION" = "Debug" ]; then
    valgrind --leak-check=full --error-exitcode=1 --gen-suppressions=all \
        --suppressions="$HERE/valgrind.supp" ./bin/unittests --gtest_shuffle
else
    ./bin/unittests --gtest_shuffle
fi
