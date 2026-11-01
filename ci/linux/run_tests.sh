#!/usr/bin/env bash

SCRIPT=$(readlink -f "$0")
HERE=$(dirname "$SCRIPT")

export LD_LIBRARY_PATH=$(pwd)/bin/lib:$LD_LIBRARY_PATH

# The unit tests never open a window, but a few of them reach code that brings SDL's video
# subsystem up (os_init() and resolution_default(), the latter via gr_init()). Left to itself SDL
# probes its real backends and dlopens libX11, which leaks a handful of blocks per init/quit cycle
# that SDL_QuitSubSystem() does not reclaim -- those are the "definitely lost" contexts memcheck
# fails the Debug legs on. Pinning the dummy driver skips the probing entirely, so the leaks never
# happen, and it makes the tests behave the same way whether or not the runner has a display.
export SDL_VIDEODRIVER=dummy

# if on mac and building for architecture that doesn't match host
# then unittests will fail so we just need to skip it (for now)
if [ "$RUNNER_OS" = "macOS" ] && [ "$ARCH" != "$(uname -m)" ]; then
    echo "Skipping tests due to architecture mismatch!"
    exit 0
fi

if [ "$CONFIGURATION" = "Debug" ] && [[ "$RUNNER_OS" != "macOS" ]] ; then
    valgrind --leak-check=full --error-exitcode=1 --gen-suppressions=all \
        --suppressions="$HERE/valgrind.supp" ./bin/unittests --gtest_shuffle
else
    ./bin/unittests --gtest_shuffle
fi
