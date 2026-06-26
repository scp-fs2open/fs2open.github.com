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
