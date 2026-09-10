# Module: windows_stub — `code/windows_stub/`

## Purpose
**Platform compatibility shims.** FSO began as a Windows-only codebase, so
engine code still calls a number of Win32 and MSVC-specific functions. This
module supplies those on non-Windows platforms, and papers over the smaller
differences on Windows itself, so the rest of `code/` does not need `#ifdef`
blocks scattered through it.

Together with `code/osapi/`, this is where platform-specific code is *supposed*
to live.

## Key files
- `config.h` — the compatibility macros. Byte-order detection via SDL
  (`BYTE_ORDER`, `LITTLE_ENDIAN`, `BIG_ENDIAN`), socket type differences
  (`SOCKLEN_T`, `NETCALL_WOULDBLOCK`), name differences (`strtok_r` to
  `strtok_s`, `filelength` to `_filelength`), and the `STUB_FUNCTION` macro,
  which logs an unimplemented call through `nprintf` under the `Warning`
  category.
- `stubs.cpp` — non-Windows implementations of the MSVC C-runtime functions the
  engine uses: `filelength()`, `_getcwd()`, `_chdir()`, `_mkdir()`,
  `_splitpath()`, and `MulDiv()`.

## Conventions
- If you need a platform-specific call, add it here or in `code/osapi/` rather
  than adding a `#ifdef _WIN32` to a subsystem file.
- Mark a knowingly unimplemented path with `STUB_FUNCTION` so it shows up in the
  log rather than failing silently.
- Everything here must compile on GCC, Clang, and MSVC — this module is where
  compiler differences are most likely to surface first.

## Configuration tables
None.

## See also
- `code/osapi/` (the real OS abstraction — window, events, config, logging),
  `code/exceptionhandler/` (Windows crash reporting), root `AGENTS.md` for the
  cross-platform rules.
