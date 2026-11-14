# Module: cfileextractor — `code/cfileextractor/`

## Purpose
A **standalone command-line tool that unpacks VP archives** — the counterpart to
`code/cfilearchiver/`. It reads a VP built for FSO and writes its contents back
out as loose files. Like the archiver, it is a separate executable rather than
part of the game.

## Key files
- `cfileextractor.cpp` — the whole tool, `main()` included.

## Build
- Built only when the `FSO_BUILD_TOOLS` CMake option is ON (default OFF), and
  excluded from the `all` target:
  ```bash
  cmake -DFSO_BUILD_TOOLS=ON ..
  ninja cfileextractor
  ```
- Target name `cfileextractor`, in the `FSOTools` IDE folder.

## Notes
- It has its own `main()` and does not use the engine's logging or error
  reporting.
- Not built by CI and not covered by the unit tests; verify changes by running
  it against a real VP.

## Configuration tables
None.

## See also
- `code/cfilearchiver/` (builds a VP), `code/cfile/`
  (`modules/cfile.md` — the VP format and the VFS).
