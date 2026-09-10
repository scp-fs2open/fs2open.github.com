# Module: inetfile — `code/inetfile/`

## Purpose
**Downloading a file over the network.** A small, self-contained HTTP and FTP
client used for out-of-band transfers — pilot-tracker and lobby support files,
and other places the engine needs to fetch something by URL. It is unrelated to
multiplayer gameplay traffic, which is UDP and lives in
`code/network/psnet2.*`.

## Key files
- `inetgetfile.cpp` / `inetgetfile.h` — `InetGetFile`, the URL-level front end;
  this is what callers use.
- `chttpget.cpp` / `chttpget.h` — the HTTP client.
- `cftp.cpp` / `cftp.h` — the FTP client (`CFtpGet`).

## Core data structures
- `class InetGetFile` — one download. It runs on its own thread and is polled:
  `GetBytesIn()` and `GetTotalBytes()` drive a progress bar, `GetErrorCode()`
  reports failure, and `AbortGet()` cancels.
- `class CFtpGet` — the FTP equivalent, with its own state machine.

## Major constants
- `InetGetFile` errors: `INET_ERROR_NO_ERROR`, `INET_ERROR_BADPARMS`,
  `INET_ERROR_CANT_WRITE_FILE`, `INET_ERROR_CANT_PARSE_URL`,
  `INET_ERROR_BAD_FILE_OR_DIR`, `INET_ERROR_HOST_NOT_FOUND`,
  `INET_ERROR_UNKNOWN_ERROR`, `INET_ERROR_NO_MEMORY`.
- `CFtpGet` states, which double as error codes: `FTP_STATE_STARTUP`,
  `FTP_STATE_CONNECTING`, `FTP_STATE_LOGGING_IN`, `FTP_STATE_LOGGED_IN`,
  `FTP_STATE_RECEIVING`, `FTP_STATE_FILE_RECEIVED`, and the failures
  (`FTP_STATE_SOCKET_ERROR`, `FTP_STATE_URL_PARSING_ERROR`,
  `FTP_STATE_HOST_NOT_FOUND`, `FTP_STATE_CANT_CONNECT`,
  `FTP_STATE_LOGIN_ERROR`, `FTP_STATE_DIRECTORY_INVALID`,
  `FTP_STATE_FILE_NOT_FOUND`, `FTP_STATE_RECV_FAILED`,
  `FTP_STATE_CANT_WRITE_FILE`, `FTP_STATE_INTERNAL_ERROR`,
  `FTP_STATE_UNKNOWN_ERROR`).

## Conventions
- A download is **asynchronous and polled**. Start it, then check progress each
  frame; never block the frame loop waiting for one.
- Downloading is a network action with privacy implications, so it stays
  confined to the features that genuinely need it. Do not add fetches to
  ordinary asset-loading paths.

## Configuration tables
None.

## See also
- `code/network/multi_pxo.*` (the main caller — the online lobby),
  `code/network/psnet2.*` (multiplayer sockets, a different layer),
  `code/cfile/` (where a downloaded file ends up).
