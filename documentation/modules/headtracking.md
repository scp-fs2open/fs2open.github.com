# Module: headtracking — `code/headtracking/`

## Purpose
**Head-tracking device support**: reads a head tracker (TrackIR or FreeTrack)
and reports the player's head position and orientation, so the in-cockpit view
can follow where the pilot is looking. It is optional hardware — the engine runs
normally when no device or driver is present, and every entry point is written
to fail softly.

## Key files
- `headtracking.cpp` / `headtracking.h` — the public `headtracking::` API.
- `headtracking_internal.h` — `HeadTrackingProvider`, the interface each device
  backend implements, and `HeadTrackingException`.
- `trackir.cpp` / `trackir.h`, `trackirpublic.cpp` / `trackirpublic.h` — the
  TrackIR provider and its SDK glue.
- `freetrack.cpp` / `freetrack.h` — the FreeTrack provider.

## Core data structures
- `headtracking::HeadTrackingStatus` — the reading: `x`, `y`, `z`, `pitch`,
  `roll`, `yaw`.
- `headtracking::internal::HeadTrackingProvider` — one device backend. Adding
  support for another tracker means adding a provider, not touching callers.

## API
- `init()` — probe for a device; returns false when none is available.
- `isEnabled()` — whether tracking is actually running. **Check this before
  using any reading.**
- `query()` — poll the device for a fresh reading.
- `getStatus()` — the current `HeadTrackingStatus`.
- `shutdown()`.

## Hardware-optional contract
This module is a worked example of the project's rule that hardware-dependent
features must degrade gracefully. `init()` returning false is a normal outcome,
not an error; callers gate on `isEnabled()` and fall back to the ordinary view
controls. Keep that shape when extending it — do not make a missing device
fatal, and do not log per-frame when tracking is off.

## Configuration tables
None.

## See also
- `code/camera/` (where the reading is applied), `code/io/` (the other input
  devices; `code/io/spacemouse.*` is a comparable optional device),
  `code/osapi/` (platform detail), `code/options/` (the player toggle).
