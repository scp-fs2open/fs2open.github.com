# Module: exceptionhandler — `code/exceptionhandler/`

## Purpose
**Windows crash reporting.** When the game hits an unhandled structured
exception, this writes a human-readable crash report — the exception type, the
faulting address, register contents, and a stack walk — so a user can attach it
to a bug report. It is a diagnostic of last resort, reached only after the
engine has already failed.

## Scope and platform
- **Windows only**, and only when `GAME_ERRORLOG_TXT` is defined: the entire
  header is inside that guard. On other platforms the module compiles to
  nothing and the operating system's own crash handling applies.
- It uses Win32 structured exception handling, so it is one of the few places
  platform-specific code is expected rather than avoided.

## Key files
- `exceptionhandler.cpp` / `exceptionhandler.h` — the whole module.

## API
- `RecordExceptionInfo(PEXCEPTION_POINTERS data, const char* Message)` — the
  single entry point, installed as the unhandled-exception filter. The header
  forward-declares `_EXCEPTION_POINTERS` itself specifically to avoid pulling
  `windows.h` into anything that includes it.

## Relationship to the other error paths
This is not the normal way to report a problem. In order of increasing severity:
`Warning()` for a recoverable user-data problem, `Error()` for unrecoverable bad
user data, `Assert()`/`Assertion()` for a programmer invariant — all in
`code/osapi/dialogs.h`. This module only catches what none of those did.

## Configuration tables
None.

## See also
- `code/osapi/dialogs.*` (the normal error and assert reporting),
  `code/osapi/outwnd.*` (the log a crash report accompanies),
  `code/windows_stub/` (the other Windows-specific corner),
  root `AGENTS.md` for the error-reporting conventions.
