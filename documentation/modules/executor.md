# Module: executor — `code/executor/`

## Purpose
A **deferred work queue** that decouples "something wants to run later" from
"the engine reaches the right point in the frame". Engine code creates an
executor and calls `process()` at a well-defined moment; any other module can
`post()` work to it without either side depending on the other. It is how Lua
coroutines, async operations, and delayed engine work get resumed safely.

An **execution context** adds the second half of the problem: making sure a work
item only runs if the game is still in the state it was posted from, so a
callback queued in the briefing does not fire in the middle of gameplay.

## Key files
- `Executor.cpp` / `Executor.h` — `executor::Executor`: `post()`, `process()`,
  and the `CallbackResult` enum. Thread-safe.
- `global_executors.cpp` / `global_executors.h` — the two engine-wide
  executors.
- `IExecutionContext.cpp` / `IExecutionContext.h` — the `IExecutionContext`
  interface and its `State` enum.
- `GameStateExecutionContext.*` — a context tied to a `GS_STATE`.
- `CombinedExecutionContext.*` — a context that is valid only when several
  others are.

## Core data structures
- `Executor::CallbackResult` — `Done` (drop the item) or `Reschedule` (run it
  again next round). A posted `Callback` keeps running every `process()` until
  it returns `Done`.
- `IExecutionContext::State` — `Valid` (we are in the captured state),
  `Suspended` (not right now, but we may return to it), `Invalid` (an unrelated
  state; the work should be abandoned). A context-aware `Callback` receives this
  state and decides what to do.

## The two global executors
- `executor::OnSimulationExecutor` — runs at the end of every **simulation**
  frame. Use it for work that must see a consistent world state.
- `executor::OnFrameExecutor` — runs just before the frame is presented. Use it
  for work that should happen once per rendered frame, including while the
  simulation is paused.

`executor::currentExecutor()` returns the one currently running, if any.

## Conventions
- Post to an executor rather than adding another call into
  `game_simulation_frame()` — that is the dependency this module exists to
  avoid.
- Wrap a callback in an execution context whenever it captures game state
  (an object, a mission, a screen); otherwise it may run after that state is
  gone.

## Configuration tables
None.

## See also
- `code/scripting/` (Lua async and coroutines are the main user),
  `code/gamesequence/` (the states `GameStateExecutionContext` tracks),
  `code/utils/threading.*` (actual worker threads, a different tool).
