# Repository Guidelines

## Project Structure & Module Organization
- `code/` – Core engine sources (graphics, input, sound, gameplay). Vulkan path under `code/graphics/vulkan/`; OpenGL under `code/graphics/opengl/`.
- `test/` – GoogleTest-based unit and integration tests (see `test/src/graphics/test_vulkan.cpp` for renderer tests).
- `docs/` – Design notes and phase breakdowns (e.g., Vulkan renderer phases).
- `build/` – CMake build tree (out-of-source). Binaries land in `build/bin/<Config>/`.
- Assets and game data live under the game install (e.g., `C:\Program Files (x86)\Steam\steamapps\common\Freespace 2`); config under `%APPDATA%\HardLightProductions\FreeSpaceOpen\`.

## Build, Test, and Development Commands
- Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` (add `-DSCP_RELEASE_LOGGING=ON` for release logs).
- Build: `cmake --build build --config Debug --parallel`.
- Run (Debug Vulkan): `build/bin/Debug/fs2_26_0_0.exe -vulkan` (adjust exe name per target/config).
- Tests: `ctest -C Debug --output-on-failure -VV` from `build/`.

## Coding Style & Naming Conventions
- C++17 with project `.clang-format`; keep existing formatting and brace style.
- Prefer existing naming: `CamelCase` for types, `snake_case` for functions/vars in C++; logging via `mprintf`/`vk_logf`.
- New files should include header guards or `#pragma once`; keep ASCII.
- Use existing managers and helper APIs in `code/graphics/vulkan/` (e.g., `VulkanRenderer`, `RenderFrame`) instead of ad hoc Vulkan calls.

## Testing Guidelines
- Use GoogleTest (see `test/src/graphics`). Name tests `Component_Feature` and keep assertions focused.
- When touching Vulkan/graphics, add/adjust tests if possible and run `ctest` in the affected configuration.
- For crashes/hangs, capture `vulkan_debug.log` and stdout/stderr from the built exe to verify fixes.

## Commit & Pull Request Guidelines
- Commits: concise, imperative summaries (e.g., “Add Vulkan flip logging”). Group related changes; avoid mixing style-only edits with logic changes.
- PRs: describe the change, reproduction steps, and validation (tests run, logs). Link issues/threads if applicable and include screenshots/log snippets for rendering or crash fixes.

## Agent-Specific Notes
- On session start, if a `CONTEXT.md` file exists at the repository root, read it fully before performing any other actions.
- Keep `CONTEXT.md` up to date for the areas it covers (currently the Vulkan backend):
  - When you change the architecture or behavior of systems described there (API version, dynamic rendering, key managers, etc.).
  - When you fix a non-trivial crash/bug or discover a pitfall that future LLM sessions should avoid (add to “Crash fix” or “Mistakes to avoid” sections).
  - When you add/remove important files, tools, or workflows that should be reflected in its “File map” or “Testing & debugging” sections.
- Do not revert user changes. Avoid destructive git commands. Prefer `rg` for search and `cmake --build` for builds. Log and test before handing off.***
- `gr_vulkan_calculate_irrmap` now relies on per-face cubemap framebuffers to pull format/extent metadata; if you touch this path ensure you handle the `irrmapRT->framebuffer` null case by scanning `cubeFaceFramebuffers` before dereferencing and by using `faceFramebuffer->getExtent()` instead of hard-coded sizes.
- When running `git show`/`git diff` in this repo, always pass `--no-pager` (or set `GIT_PAGER=cat`) so automated tooling is not blocked by the interactive pager/“spacebar to continue” prompt.
