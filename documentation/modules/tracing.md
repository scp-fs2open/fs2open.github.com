# Module: tracing — `code/tracing/`

## Purpose
**Performance instrumentation.** It records timed scopes, asynchronous spans,
and counters, and writes them out in the Chrome trace-event format so a profile
can be opened in a trace viewer. It also drives the in-game frame profiler and
the on-screen monitor counters. This is the module to reach for when answering
"where is the frame time going?", rather than adding timing `mprintf` calls.

## Key files
- `tracing.cpp` / `tracing.h` — the public API and the `TRACE_SCOPE` macro.
- `categories.cpp` / `categories.h` — the `Category` type and the full list of
  named trace categories (`LuaOnFrame`, `DrawSceneTexture`, `Tonemapping`,
  `Bloom`, `FXAA`, `SMAA`, and many more).
- `scopes.cpp` / `scopes.h` — `Scope`, used to name an asynchronous span.
- `FrameProfiler.cpp` / `FrameProfiler.h` — the in-game per-frame breakdown.
- `MainFrameTimer.*` — overall frame timing.
- `Monitor.cpp` / `Monitor.h` — `Monitor<T>` and `RunningCounter`: cheap
  named counters with the `MONITOR` / `MONITOR_INC` macros.
- `TraceEventWriter.*` — writes the trace file.
- `ThreadedEventProcessor.h` — moves event processing off the main thread.

## Core data structures
- `tracing::Category` — a named measurement point, constructed with a name and
  an `is_graphics` flag so GPU-side categories can be timed differently. Declare
  new ones in `categories.h`/`.cpp` rather than inventing strings at the call
  site.
- `tracing::Scope` — identifies one instance of an asynchronous span, so
  overlapping spans of the same category can be told apart.
- `tracing::trace_event`, `tracing::EventType` — the recorded event.
- `tracing::complete::ScopedCompleteEvent` — the RAII object behind
  `TRACE_SCOPE`.

## Using it
- **A timed scope:** `TRACE_SCOPE(tracing::Simulation);` at the top of a block.
  This is the common case, and it is what `game_simulation_frame()` uses.
- **A span across frames:** `tracing::async::begin(category, scope)`, then
  `step()`, then `end()`.
- **A counter:** `tracing::counter::value(category, value)`, or the
  `MONITOR(name)` / `MONITOR_INC(name, inc)` macro pair for a simple tally.
- Lifecycle: `tracing::init()`, `process_events()`,
  `frame_profile_process_frame()`, `shutdown()`.

## Configuration tables
None.

## See also
- `code/osapi/` (logging, the other diagnostic channel — `modules/osapi.md`),
  `code/graphics/` (the largest set of categories),
  `code/debugconsole/` (the commands that turn tracing and the frame profiler
  on).
