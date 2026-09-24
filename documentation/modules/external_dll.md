# Module: external_dll — `code/external_dll/`

## Purpose
**Loading an optional shared library at runtime.** One small base class,
`SCP_ExternalCode`, that wraps SDL's dynamic-loading calls so a feature can bind
to a library that may or may not be installed, and disable itself cleanly when
it is not. Using SDL rather than `dlopen`/`LoadLibrary` directly is what makes
it portable.

The module is a single header of about 125 lines — read it directly.

## Key files
- `externalcode.h` — `class SCP_ExternalCode`. There is no `.cpp`.

## Core data structure
- `class SCP_ExternalCode` — holds the loaded library handle and unloads it in
  its destructor (`SDL_UnloadObject`). Derive from it and call the protected
  `LoadExternal(externlib, basePath)` in your constructor, then resolve the
  entry points you need.

## Who uses it
- `code/headtracking/trackirpublic.h` and `freetrack.h` — the head-tracking
  device SDKs.
- `code/graphics/vulkan/VulkanShaderCompiler.cpp` — the shader compiler
  library.

All three are optional features, which is the point: a missing library must
leave the engine running with that feature off, never fail to start.

## Conventions
- Always treat a failed load as a normal outcome and fall back, in line with the
  project's rule that hardware and driver dependent features degrade gracefully.
- Let the destructor unload; do not call `SDL_UnloadObject` yourself.

## Configuration tables
None.

## See also
- `code/headtracking/` and `code/graphics/vulkan/` (the users),
  `code/libs/` (glue for libraries linked at build time rather than loaded at
  runtime), `code/osapi/` (the rest of the platform layer).
