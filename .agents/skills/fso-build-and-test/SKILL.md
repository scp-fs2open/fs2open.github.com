---
name: fso-build-and-test
description: >-
  Configure, build, and test the FSO engine with CMake/Ninja and run the
  GoogleTest unit tests, mirroring CI so changes stay warning-clean across
  GCC/Clang/MSVC. Use when building FSO, running unittests, reproducing a CI
  failure, or checking clang-format/clang-tidy compliance before a PR.
---

# FSO: Build and Test

FSO uses CMake (out-of-source builds required). CI builds with
`FSO_FATAL_WARNINGS=ON` across GCC, Clang, and MSVC, so a local clean build is the
minimum bar. Prefer Ninja for speed.

## Prerequisites

Submodules must be present:

```bash
git submodule update --init --recursive
```

## Configure + build (Linux/macOS, Ninja)

```bash
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DFSO_BUILD_TESTS=ON -DFSO_FATAL_WARNINGS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
ninja -k 20 all
```

- Build types: `Debug`, `Release`, `FastDebug`.
- Existing build dirs in-tree: `cmake-build-debug/`, `cmake-build-release/`,
  `cmake-build-relwithdebinfo/` — reuse one instead of reconfiguring if present.
- The CI configure script is `ci/linux/configure_cmake.sh` (adds ccache, SIMD,
  AppImage flags); match its options when reproducing a CI build.

## Run unit tests

Tests build only with `FSO_BUILD_TESTS=ON`; the binary is `unittests` in `build/bin/`.

```bash
./bin/unittests --gtest_shuffle
# subset:
./bin/unittests --gtest_filter='SomeSuite.*'
```

CI test runner: `ci/linux/run_tests.sh` (Debug non-macOS runs under valgrind).

## Formatting & static analysis (pre-PR)

```bash
# Format only files you changed:
clang-format -i path/to/changed_file.cpp
```

- `.clang-format` (LLVM-based) and `.clang-tidy` are enforced; CI runs clang-tidy
  on changed code for clang builds (`ci/linux/clang_tidy.sh`).
- Indentation is tabs (width 4); column limit 120; left pointer alignment.

## Workflow

```
- [ ] Submodules updated
- [ ] Configure (Ninja, FSO_BUILD_TESTS=ON, FSO_FATAL_WARNINGS=ON)
- [ ] ninja all builds with zero warnings
- [ ] unittests pass (--gtest_shuffle)
- [ ] clang-format applied to changed files
```

## Notes

- Keep builds out-of-source (top-level CMake refuses in-source builds).
- Code must compile on GCC, Clang, and MSVC; avoid platform APIs outside
  `code/osapi/`, `code/windows_stub/`, and platform guards.
- For build options reference see the top-level `CMakeLists.txt` and root `AGENTS.md`.
