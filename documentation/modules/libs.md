# Module: libs — `code/libs/`

## Purpose
**Engine-side glue for optional third-party libraries.** The libraries
themselves are vendored under the top-level `lib/` directory or come from the
system; what lives here is the thin FSO-facing wrapper that initializes each
one, adapts it to engine conventions, and lets the feature compile out when the
library is not available.

> Not to be confused with the top-level `lib/`, which holds the vendored library
> sources and must not be edited.

## Key files
- `jansson.cpp` / `jansson.h` — JSON, via the jansson library. Used by the
  options system, pilot files, and control presets.
- `ffmpeg/` — `FFmpeg`, `FFmpegContext`, `FFmpegHeaders.h`, and
  `LibAVCompatibility.h`. The last one absorbs API differences between FFmpeg
  versions, which is why engine code should include these headers rather than
  FFmpeg's directly.
- `discord/` — Discord rich presence. Gated by the `FSO_BUILD_WITH_DISCORD`
  CMake option (default ON).
- `antlr/` — `ErrorListener`, routing ANTLR parser errors into the engine's
  error reporting.
- `renderdoc/` — RenderDoc in-application capture (`renderdoc_app.h` is the
  vendored API header), for debugging the renderer.

## Conventions
- Every one of these is **optional**. Guard use behind the matching CMake option
  or a runtime availability check; the engine must build and run without any of
  them.
- Include the wrapper header, not the library's own headers, so version
  differences stay in one place.

## Configuration tables
None.

## See also
- `code/options/`, `code/pilotfile/`, `code/controlconfig/presets.*` (jansson
  users), `code/cutscene/ffmpeg/` and `code/sound/ffmpeg/` (FFmpeg users),
  `code/graphics/` (RenderDoc), the top-level `lib/` directory, and the
  `FSO_BUILD_WITH_FFMPEG` / `FSO_BUILD_WITH_DISCORD` CMake options.
