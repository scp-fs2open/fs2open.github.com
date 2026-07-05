# AGENTS.md

Guidance for AI coding agents working in the FreeSpace 2 Open (FSO) source tree.

## Project overview

FSO is the open-source continuation of the FreeSpace 2 game engine (C++, ~C++17),
maintained by the Source Code Project (SCP). It builds the game engine, the
FreeSpace 2 executable, and the FRED mission editors. The codebase is large,
legacy-heavy, and cross-platform (Windows, Linux, macOS).

For a map of the engine's subsystems and where concepts live (so you can navigate
without blind grepping), read **`documentation/ARCHITECTURE.md`** first. Per-module
entry-point guides live under **`documentation/modules/`**.

## Core Philosophies

FSO is, at its core, a project dedicated to preserving the FreeSpace 2 game
experience. We support a vast ecosystem of user-made content as well as the original
retail release, and our foremost aim is to maintain backwards compatibility with that content.

When making gameplay changes, existing behaviours should be preserved - changes that
might affect existing gameplay should be gated behind optional table flags, new scripting API
calls or new sexp functions. 


## Repository layout

- `code/` — Core engine source. Organized by subsystem (e.g. `ship/`, `weapon/`,
  `ai/`, `graphics/`, `model/`, `mission/`, `network/`, `scripting/`, `parse/`,
  `math/`, `physics/`, `sound/`, `hud/`, `ui/`). Built as a static library.
- `code/globalincs/` — Globally-included headers (`pstypes.h`, etc.). Included
  first everywhere; treat as the foundation.
- `freespace2/` — The game executable (entry point `freespace.cpp`).
- `fred2/` — Windows (MFC) mission editor.
- `qtfred/` — Cross-platform Qt mission editor (opt-in build).
- `wxfred2/` — Legacy wxWidgets editor.
- `lib/` — Bundled third-party libraries (vendored; avoid editing).
- `parsers/` — Generated/parser code.
- `test/` — GoogleTest unit tests (`test/src`, `test/test_data`).
- `cmake/` — CMake modules, toolchain, finders, external submodules.
- `ci/`, `.github/workflows/` — CI scripts and workflow definitions.

## Setup

Submodules are required. After cloning:

```bash
git submodule update --init --recursive
```

## Build

The project uses CMake. Build types: `Debug`, `Release`, `FastDebug`.
Always build out-of-source (the top-level `CMakeLists.txt` refuses in-source builds).
There are already configured build dirs: `cmake-build-debug/`,
`cmake-build-release/`, `cmake-build-relwithdebinfo/`.

Typical Linux configure + build (mirrors CI in `ci/linux/configure_cmake.sh`):

```bash
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFSO_BUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
ninja -k 20 all
```

Useful CMake options (see top-level `CMakeLists.txt` for the full list):

- `FSO_BUILD_TESTS` — build unit tests (default OFF; turn ON to run tests).
- `FSO_BUILD_FRED2` (Win) / `FSO_BUILD_QTFRED` — build the editors.
- `FSO_BUILD_WITH_OPENGL` / `FSO_BUILD_WITH_VULKAN` — renderer backends.
- `FSO_FATAL_WARNINGS` — warnings become errors (used in CI; keep code clean).
- `FSO_BUILD_INCLUDED_LIBS` — build bundled libs instead of system libs
  (default ON for Win/macOS, OFF for Linux).

## Tests

Unit tests use GoogleTest and are only built when `FSO_BUILD_TESTS=ON`.
The produced binary is `unittests` (under `build/bin/`).

```bash
./bin/unittests --gtest_shuffle
# Run a subset:
./bin/unittests --gtest_filter='SomeSuite.*'
```

CI reference: `ci/linux/run_tests.sh` (Debug builds on non-macOS run under valgrind).

## Code style

Formatting is enforced via `.clang-format` (LLVM-based, customized) and
`.editorconfig`. Key conventions:

- **Indentation:** tabs (width 4) for `.h`/`.cpp`/`.sdr`; spaces (2) for YAML.
- **Column limit:** 120.
- **Pointer alignment:** left (`int* p`).
- **Braces:** opening brace on its own line for functions; same line for
  control statements/namespaces/classes.
- **Includes:** sorted and regrouped; `globalincs/*.h` always come first.
  Let clang-format handle include ordering.

Run formatting on changed files before submitting:

```bash
clang-format -i path/to/changed_file.cpp
```

`.clang-tidy` is also configured and run in CI on clang builds for changed code.

## CI pipeline requirements

Every PR is built by `.github/workflows/test-pull_request.yaml` across a full
platform/compiler matrix. A change isn't CI-clean until it satisfies all of the
following:

- **Warnings are fatal.** All CI configs build with `-DFSO_FATAL_WARNINGS=ON`,
  so any compiler warning fails the build. Compile locally with that flag
  before pushing.
- **Compiler/platform matrix:** Linux (gcc-9, gcc-13, clang-16) × (Debug,
  Release), plus extra gcc-13 legs with OpenGL/Vulkan toggled off; Windows
  (MSVC, Win32 and x64) × (Debug, FastDebug, Release); macOS (clang, x86_64 and
  arm64) × (Debug, Release). Code must compile clean on GCC, Clang, and MSVC —
  don't rely on one compiler's extensions or warning behavior.
- **Tests must build and pass:** every CI config builds with
  `-DFSO_BUILD_TESTS=ON` and runs `unittests --gtest_shuffle`. Linux Debug
  additionally runs the suite under valgrind (`--leak-check=full
  --error-exitcode=1`, see `ci/linux/run_tests.sh`), so leaks or
  uninitialized-memory reads in new code fail CI even if the test assertions
  pass.
- **clang-tidy** runs on clang builds (Linux clang-16, macOS clang), scoped to
  lines changed relative to the PR base branch (`ci/linux/clang_tidy.sh`,
  checks defined in `.clang-tidy`). Don't introduce new findings in touched
  code.
- **clang-format is not enforced by CI** — there's no automated formatting
  job — but reviewers expect it. Always run `clang-format -i` on changed files;
  formatting drift is a common review comment even though it won't fail the
  pipeline.
- **Registered source files:** if a new file isn't added to the relevant
  `CMakeLists.txt`/`source_groups.cmake`, it silently isn't compiled or tested
  at all rather than causing an obvious CI error — verify it's picked up by
  building locally.
- Closest local reproduction of the Linux CI leg:
  ```bash
  cmake -G Ninja -DFSO_FATAL_WARNINGS=ON -DFSO_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
  ninja -k 20 all && ./bin/unittests --gtest_shuffle
  ```

## Conventions for agents

- This is legacy game-engine code: prefer **minimal, surgical changes** that match
  surrounding style over broad refactors.
- Do **not** edit vendored code under `lib/` or generated files unless explicitly
  required.
- Keep changes warning-clean — CI builds with `FSO_FATAL_WARNINGS=ON`.
- When adding source files, register them with the appropriate `CMakeLists.txt`
  / `source_groups.cmake` in the relevant subdirectory.
- Cross-platform matters: code must compile on GCC, Clang, and MSVC. Avoid
  platform-specific APIs outside `osapi/`, `windows_stub/`, and platform guards.
- Add or update unit tests under `test/src` when changing testable logic.

## Useful references

- Build wiki: https://github.com/scp-fs2open/fs2open.github.com/wiki/Building
- Community/forums: https://www.hard-light.net/forums/
- `Changelog.md` for recent notable changes.
