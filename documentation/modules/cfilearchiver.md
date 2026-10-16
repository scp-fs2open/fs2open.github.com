# Module: cfilearchiver — `code/cfilearchiver/`

## Purpose
A **standalone command-line tool that builds VP archives**. A VP is the package
format FSO content ships in (see `code/cfile/`); this tool packs a directory
tree into one. It is not part of the game: it is a separate executable that
links only `cfile` and SDL.

## Key files
- `cfilearchiver.cpp` — the whole tool, `main()` included.

## Build
- Built only when the `FSO_BUILD_TOOLS` CMake option is ON (default OFF), and
  the target is `EXCLUDE_FROM_ALL`, so `ninja all` does not produce it. Build it
  explicitly:
  ```bash
  cmake -DFSO_BUILD_TOOLS=ON ..
  ninja cfilearchiver
  ```
- Target name `cfilearchiver`, in the `FSOTools` IDE folder. It is compiled with
  `-DNO_SAFE_STRINGS`.

## Notes
- Because it is a separate executable with its own `main()`, it does not
  participate in the engine's init, logging, or error-reporting conventions —
  it prints to stdout.
- It is not covered by the unit tests, and CI does not build it (no CI
  configuration sets `FSO_BUILD_TOOLS`). Verify changes by running it.

## Configuration tables
None.

## See also
- `code/cfileextractor/` (the counterpart that unpacks a VP),
  `code/cfile/` (`modules/cfile.md` — the VP format and the VFS that reads it).
