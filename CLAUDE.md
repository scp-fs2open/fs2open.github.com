# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FreeSpace 2 Open Source Code Project (FS2_Open) - A comprehensive 3D space combat simulation game engine based on the original FreeSpace 2 source code. This is a mature C++17 codebase with ~449,000 lines of code across 79 subsystems.

## Fork Information

This is a fork of FSO with version **26.0.0**. Key differences from upstream:
- Removed joystick support
- Enhanced graphics defaults (TAA, SSAO Ultra, Auto-exposure enabled by default)
- Output binary: `fs2_26_0_0.exe`

### Graphics Enhancements

**Auto-Exposure (Eye Adaptation):**
- Enabled by default for realistic HDR rendering
- Prevents sun/bright light whitewash by dynamically adjusting exposure
- Uses mipmap-based luminance averaging with async GPU readback
- Configurable via `lighting_profiles.tbl` with `auto_exposure_settings`
- Disable with `-no_auto_exposure` command-line flag
- Implementation: `code/graphics/opengl/gropenglpostprocessing.cpp`

**Default Post-Processing:**
- **TAA (Temporal Anti-Aliasing)**: Default AA mode (was None)
- **SSAO**: Default Ultra quality (16 samples, 2 blur passes)
- **Tonemapper**: Uncharted2 (best highlight rolloff)

### Vulkan Renderer Development (In Progress)

The Vulkan renderer is under active development. Located in `code/graphics/vulkan/`.

**Completed Infrastructure:**
- **VulkanRenderer** - Core renderer with swapchain, surface, device selection, HDR10 support
- **VulkanBuffer** - GPU buffer management (vertex, index, uniform, staging with ring buffer)
- **VulkanShader** - Runtime GLSL->SPIR-V compilation via shaderc, reflection via SPIRV-Cross, disk caching
- **VulkanDescriptorManager** - Descriptor set/pool management with per-frame cycling
- **VulkanPipelineManager** - Graphics pipeline creation with hash-based caching, VkPipelineCache persistence
- **VulkanFramebuffer** - Framebuffer attachments (owned or external), image/memory/view management
- **VulkanRenderPassManager** - Scene pass (color+depth) and present pass (color only)
- **RenderFrame** - Per-frame synchronization (semaphores, fences, command buffers)

**Not Yet Implemented:**
- **VulkanTexture** - Image creation, texture uploads, samplers, mipmap generation
- Model rendering integration (connecting to FSO's model/batch systems)
- Post-processing pipeline (bloom, SSAO, TAA, tonemapping)
- MSAA support

**Build:** Enable with `-DFSO_BUILD_WITH_VULKAN=ON` (requires Vulkan SDK)

### Version Override

Version is set via `version_override.cmake` in the project root:
```cmake
set(FSO_VERSION_MAJOR 26)
set(FSO_VERSION_MINOR 0)
set(FSO_VERSION_BUILD 0)
```

## Essential Setup

**CRITICAL: Before any build, update git submodules:**
```bash
git submodule update --init --recursive
```

The project will not build without submodules (21 external libraries including Lua, libRocket, imgui, OpenXR, Discord RPC, FFmpeg, etc.).

## Build Commands

### Linux/macOS Build

```bash
# Configure
mkdir build && cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DFSO_BUILD_TESTS=ON \
  -DFSO_BUILD_INCLUDED_LIBS=ON \
  ..

# Compile
ninja -k 20 all

# Run tests
./bin/unittests --gtest_shuffle

# Optional: Run with valgrind (Debug builds only)
valgrind --leak-check=full ./bin/unittests --gtest_shuffle
```

### Windows Build

```bash
mkdir build && cd build

# 64-bit
cmake -G "Visual Studio 17 2022" -A x64 \
  -DFSO_BUILD_TESTS=ON \
  -DFSO_USE_SPEECH=ON \
  -DFSO_USE_VOICEREC=ON \
  ..

# 32-bit
cmake -G "Visual Studio 17 2022" -A Win32 \
  -DFSO_BUILD_TESTS=ON \
  -DFSO_USE_SPEECH=ON \
  -DFSO_USE_VOICEREC=ON \
  -DFSO_BUILD_WITH_VULKAN=OFF \
  ..

# Compile
cmake --build . --config Release --target INSTALL

# Run tests
./bin/Release/unittests --gtest_shuffle
```

### Build Configurations

- **Release** - Optimized production build
- **Debug** - Full debug symbols, no optimizations
- **FastDebug** - Debug symbols with some optimizations

### Important CMake Options

- `FSO_BUILD_TESTS=ON` - Build unit tests (recommended for development)
- `FSO_BUILD_FRED2=ON` - Build legacy Windows mission editor
- `FSO_BUILD_QTFRED=ON` - Build Qt-based mission editor (cross-platform)
- `FSO_BUILD_TOOLS=ON` - Build development tools
- `FSO_BUILD_WITH_FFMPEG=ON` - Enable FFmpeg audio/video support (default ON)
- `FSO_BUILD_WITH_DISCORD=ON` - Enable Discord Rich Presence (default ON)
- `FSO_BUILD_WITH_VULKAN=ON` - Enable Vulkan renderer (default OFF on macOS)
- `FSO_BUILD_WITH_OPENXR=ON` - Enable OpenXR VR support (default ON except macOS)
- `FSO_FATAL_WARNINGS=ON` - Treat warnings as errors (used in CI)
- `FSO_BUILD_APPIMAGE=ON` - Build AppImage package (Linux only)

### Code Formatting with Clang-Format

Before committing, format changed files:

```bash
# Format a single file
clang-format -i code/path/to/file.cpp

# Format multiple files
clang-format -i code/**/*.cpp code/**/*.h
```

### Code Analysis with Clang-Tidy

Run static analysis on modified code:

```bash
cd build

# Check a single file
clang-tidy-16 ../code/ship/ship.cpp -- -I. $(cat compile_commands.json | jq -r '.[] | select(.file=="'$(realpath ../code/ship/ship.cpp)'") | .command' | sed 's/ -c.*//')

# Check changed files (PR workflow)
git diff -U0 --no-color origin/master..HEAD | \
  ../ci/linux/clang-tidy-diff.py -path "$(pwd)" -p1 \
  -clang-tidy-binary clang-tidy-16 -j$(nproc)
```

The project treats clang-tidy warnings as errors in CI (via GitHub Actions), so fix warnings before pushing.

## High-Level Architecture

### Main Directory Structure

- **code/** - Main engine (79 subsystems, static library)
- **freespace2/** - Game executable entry point
- **lib/** - External dependencies (21 libraries)
- **fred2/** - Legacy MFC mission editor (Windows only)
- **wxfred2/** - wxWidgets mission editor (cross-platform)
- **qtfred/** - Qt mission editor (in development)
- **parsers/** - ANTLR4-based table parsers
- **tools/** - Build tools (embedfile, strings_tool)
- **test/** - Google Test unit tests
- **ci/** - CI/CD scripts

### Critical Engine Subsystems (code/)

#### Core Game Loop
- **gamesequence/** - State machine managing game flow (49 states, 94 events)
  - Event-driven architecture with state stacking
  - All game modes route through `game_do_state()` in freespace2/freespace.cpp

#### Object & Physics System
- **object/** - Central manager for all entities (ships, weapons, debris, asteroids)
  - All in-game entities flow through the object system
  - Collision detection happens here via `objcollide_check_all()`
- **physics/** - Movement and physics calculations
- **ship/** - Ship systems (shields, damage, subsystems) - 755KB codebase
- **weapon/** - Weapon systems (projectiles, beams, missiles)

#### AI System
- **ai/** - 1.4MB of AI code
  - `aicode.cpp` (581KB) - Core AI decision making
  - `aigoals.cpp` - Goal-based AI system
  - Fully scriptable via Lua

#### Mission System
- **mission/** - Mission loading and execution
  - `missionparse.cpp` (325KB) - Parses .fs2 mission files
  - Campaign management, briefings, goals
- **parse/** - Table file parsing
  - `sexp.cpp` (1.3MB) - S-expression evaluation for mission logic
  - `parselo.cpp` (122KB) - Low-level parser

#### Scripting & Modding
- **scripting/** - Lua engine integration
  - **scripting/api/** - Extensive Lua bindings for game objects
  - Coroutine support for async mission logic
  - 20+ event hooks ($on-mission-start, $on-frame, $on-weapon-fire, etc.)
- **mod_table/** - Mod management system

#### Graphics Pipeline
- **graphics/** - 2D graphics, fonts, batch rendering
- **render/** - 3D rendering pipeline, clipping, transformations
- **model/** - 3D model system (POF format) - 199KB
  - Model loading, rendering, animation, collision
- **particle/** - Particle effects
- **lighting/** - Lighting system
- **decals/** - Surface decal rendering
- **nebula/** - Volumetric nebula effects

#### User Interface
- **hud/** - In-game HUD (targeting, damage, messages)
- **menuui/** - Main menu system (barracks, readyroom, tech room)
- **missionui/** - Mission-related UI (briefing, debrief, ship selection)
- **scpui/** - Modern libRocket-based UI framework

#### Audio
- **sound/** - Multi-threaded audio (DirectSound/OpenAL)
  - 3D spatial audio
  - FFmpeg integration for various formats
  - Voice recognition (Windows) and text-to-speech
- **gamesnd/** - Event-based music system

#### Networking (Multiplayer)
- **network/** - Comprehensive multiplayer (100+ files)
  - `multimsgs.cpp` (263KB) - 100+ network message types
  - `multi_obj.cpp` (111KB) - Object synchronization
  - UDP-based with lag compensation
  - Voice communication, file transfer

#### File System
- **cfile/** - Virtual file system
  - Archive support (VP files)
  - Compression (LZ4)
  - Multi-path searching for mod support
- **bmpman/** - Texture/bitmap manager
- Image loaders: jpgutils, pngutils, pcxutils, tgautils, ddsutils

#### System Integration
- **osapi/** - OS abstraction layer (Windows, macOS, Linux)
- **io/** - Input handling (keyboard, mouse, joystick, force feedback)
- **exceptionhandler/** - Crash dumps and stack traces
- **cmdline/** - Command-line argument parsing
- **debugconsole/** - In-game debug console

### Architectural Patterns

**1. Subsystem Initialization Pattern**
Every major subsystem follows:
```cpp
void subsystem_init();      // Initialize on game start
void subsystem_close();     // Cleanup on exit
void subsystem_do_frame();  // Called every frame
void subsystem_reset();     // Reset to defaults
```

**2. Object Manager Pattern**
- Central `object_create()` factory in object/object.cpp
- Each object type has type-specific `<type>_create()`, `<type>_delete()`, `<type>_move()`, `<type>_collide()`
- All entities route through the unified object system

**3. Virtual File System (VFS)**
- All file access goes through cfile/ subsystem
- Transparent archive support (VP files are like ZIP files)
- Multiple search paths enable modding
- Never use direct file I/O; always use cfile API

**4. Table-Driven Data**
- Mission definitions: .fs2 files
- Ship/weapon stats: .tbl files
- All parsed by parse/parselo.cpp
- Enables extensive modding without code changes

**5. Event-Driven State Machine**
- Game states (GS_STATE_*): main menu, gameplay, briefing, debrief, etc.
- Game events (GS_EVENT_*): state transitions
- State stacking for nested menus
- See gamesequence/gamesequence.cpp

**6. Scripting-First Extensibility**
- AI is fully Lua-scriptable
- Hook system for event-driven mission logic
- LuaPromise for async operations
- Community creates entire campaigns via scripts

### System Interaction Flow

**Typical Game Frame:**
```
main() → game_main()
  ↓
while(!quit) {
  gameseq_process_events()     // Handle state transitions
  ↓
  game_do_state()               // Execute current state logic
    ├─ object_move_all()        // Update all objects
    │   ├─ ai_process()         // Process AI
    │   ├─ physics_step()       // Update physics
    │   └─ weapon_move()        // Move projectiles
    ├─ objcollide_check_all()   // Collision detection
    ├─ hud_render_all()         // Render HUD
    ├─ render_frame()           // 3D graphics
    └─ sound_update()           // Update audio
  ↓
  swap buffers
}
```

**Mission Execution Chain:**
```
mission_load() → missionparse_load()
  ↓
object_create() → ship_create() / weapon_create()
  ↓
Per-frame loop:
  ├─ AI System (ailua_process → aicode_execute → aigoals_process)
  ├─ Physics (physics_step)
  ├─ Collision (objcollide_check_all)
  ├─ Rendering (model_render with effects)
  ├─ Audio (spatial 3D sound)
  └─ Network (if multiplayer: multi_obj_send_all)
  ↓
missiongoals_process() → SEXP evaluation
```

### Large Monolithic Files

These files contain tightly coupled logic that's expensive to refactor:
- **ai/aicode.cpp** (581 KB) - Core AI decision making
- **parse/sexp.cpp** (1.3 MB) - SEXP expression evaluation
- **weapon/weapons.cpp** (360 KB) - Weapon behavior
- **ship/ship.cpp** (755 KB) - Ship systems
- **network/multimsgs.cpp** (263 KB) - Network protocol

When working with these files, be careful as changes can have far-reaching effects.

## Testing

### Running Tests

```bash
# From build directory
./bin/unittests                           # Run all tests
./bin/unittests --gtest_shuffle          # Randomize test order (CI uses this)
./bin/unittests --gtest_filter=math.*    # Run tests matching pattern
./bin/unittests --gtest_filter=SuiteName.TestName  # Run single test
./bin/unittests --gtest_repeat=10 --gtest_shuffle  # Run multiple times with random order
```

**Test subsystems in `test/src/`:**
- `actions/` - Action system tests
- `cfile/` - File I/O and archive tests
- `graphics/` - Graphics rendering tests
- `math/` - Mathematics library tests
- `parse/` - Parser tests (SEXP, table parsing)
- `scripting/` - Lua scripting and API tests
- `weapon/` - Weapon system tests

Test data location: `test/test_data/`

### Running Tests with Memory Checking

For Debug builds on Linux only:

```bash
valgrind --leak-check=full --error-exitcode=1 \
  --gen-suppressions=all \
  --suppressions="ci/linux/valgrind.supp" \
  ./bin/unittests --gtest_shuffle
```

## Common Development Workflows

### Pre-Commit Checklist

Before pushing your changes:

```bash
# 1. Build with warnings as errors
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DFSO_BUILD_TESTS=ON -DFSO_FATAL_WARNINGS=ON ..
ninja -k 20 all

# 2. Run all tests
./bin/unittests --gtest_shuffle

# 3. Format your changes
clang-format -i ../code/path/to/your/file.cpp

# 4. Check for static analysis issues
git diff -U0 --no-color origin/master..HEAD | \
  ../ci/linux/clang-tidy-diff.py -path "$(pwd)" -p1 \
  -clang-tidy-binary clang-tidy-16 -j$(nproc)
```

All of these checks run in GitHub Actions CI, so fixing them locally saves time.

### Adding a New Ship Subsystem

1. Define in `ship/ship.h`
2. Implement in `ship/ship.cpp`
3. Add table parsing in `parse/parselo.cpp`
4. Add damage handling in `ship/ship.cpp:do_subobj_hit_stuff()`
5. Add rendering in `ship/shipfx.cpp` or `model/modelrender.cpp`
6. Add tests in `test/src/ship/`

### Modifying Mission Logic (SEXP Scripting)

**WARNING:** `parse/sexp.cpp` (1.3 MB) is tightly coupled with mission execution. Changes here affect campaign branching and condition evaluation.

1. SEXP operator changes go in `parse/sexp.cpp` (read carefully, changes have far-reaching effects)
2. New hook types in `scripting/hook_api.h`
3. Lua scripting changes in `scripting/api/`
4. Mission parser changes in `mission/missionparse.cpp`
5. Test with existing missions to avoid breaking campaigns

### Adding a New Weapon Type

1. Define in `weapon/weapon.h`
2. Implement behavior in `weapon/weapons.cpp`
3. Add collision handling in `object/objcollide.cpp`
4. Add rendering in `weapon/weapons.cpp` or `particle/particle.cpp`
5. Add table parsing for weapon stats in `parse/parselo.cpp`
6. Add AI behavior in `ai/aicode.cpp` (note: highly coupled, be careful)

### Working with Lua Scripting

- API bindings are in `scripting/api/objs/` and `scripting/api/libs/`
- Add new hooks in `scripting/hook_api.h`
- Test scripts should go in `test/src/scripting/`
- All game objects (ships, weapons, etc.) can be exposed to Lua
- Full documentation of Lua API available at https://github.com/scp-fs2open/fs2open.github.com/wiki

## CI/CD & Quality Assurance

### GitHub Actions Workflow

On every pull request, the following runs automatically:

**Linux (multiple compilers/configurations):**
- Debug and Release builds
- GCC 9, GCC 13, clang-16 compilers
- With and without Vulkan support
- Unit tests with `--gtest_shuffle`
- clang-tidy static analysis

**Windows:**
- Visual Studio 2022 MSVC compiler
- 32-bit and 64-bit builds
- Debug, FastDebug, and Release configurations

**macOS:**
- Xcode compiler
- Universal build (x86_64 + ARM64)

**Fix clang-tidy warnings:** The CI pipeline fails on clang-tidy violations. Run the pre-commit checks above to catch these before pushing.

### Compilation Performance Tips

- **Cotire (Precompiled Headers):** Enabled by default, significantly speeds up builds. Disable with `-DENABLE_COTIRE=OFF` if issues arise.
- **Link-Time Optimization (LTO):** Enabled by default in Release builds, adds 30-60% to link time but produces smaller binaries. Disable with `-DFSO_USE_LTO=OFF`.
- **ccache:** Install `ccache` to cache object files between builds. CI uses this automatically.
- **Parallel builds:** `ninja -k 20 all` uses aggressive parallelism. Adjust `-k` value if system runs out of memory.

## Important Build Notes

- **Precompiled headers**: The project uses prefix_header.h for faster compilation
- **Cotire**: Enabled by default for compilation speed (can disable with ENABLE_COTIRE=OFF)
- **LTO**: Link-time optimization enabled by default for Release builds
- **Platform-specific**: Windows uses DirectSound/MFC, others use OpenAL/SDL2
- **Submodules**: If you see mysterious build failures, 99% chance you need to update submodules

## Platform-Specific Notes

### Windows
- Requires Visual Studio 2022 or newer
- Can build FRED2 (legacy mission editor) with FSO_BUILD_FRED2=ON
- Speech and voice recognition available

### macOS
- Vulkan support is disabled by default (OpenGL only)
- OpenXR not supported
- Use Homebrew to install ninja

### Linux
- Can build AppImage with FSO_BUILD_APPIMAGE=ON
- Requires SDL2, OpenAL-Soft
- Works on FreeBSD and Solaris with minor adjustments

## Additional Resources

- Wiki: https://github.com/scp-fs2open/fs2open.github.com/wiki/Building
- Total C++ lines: ~449,000
- Main subsystems: 79 directories in code/
- Supported platforms: Windows, macOS, Linux, FreeBSD, Solaris
# === END USER INSTRUCTIONS ===