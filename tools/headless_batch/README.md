# Headless batch runner

Run many FreeSpace Open missions as fast as the CPU allows, with no window, no rendering and no audio, then
collect one result per run. Each run is a separate engine process with its own workspace, seed and UDP port,
so runs cannot affect each other and the batch scales with the number of cores.

This directory has the runner (`run_batch.py`), its tests (`test_run_batch.py`) and a starting plan
(`example.plan.json`). The engine side of the feature is the set of `-headless` command-line options described
below.

## The engine options

| Option | Effect |
|---|---|
| `-headless` | No window, renderer, audio, speech, joystick, HUD/radar presentation or movies. Graphics runs on the stub backend. Implies `-unthrottled`. |
| `-unthrottled` | Run frames as fast as possible. Mission time comes from a virtual clock that advances only by simulated time, so results do not depend on how fast the machine is. Wall-clock timers (profiling, UI deadlines) keep real time. |
| `-fixed-dt <seconds>` | Length of one simulation step. Default 1/30; must be in (0, 0.25]. It sets the step, it does not enlarge the engine's integration step. |
| `-max-sim-time <seconds>` | End the run once this much mission time has been simulated. Must be above 0. |
| `-sim-result <path>` | When the engine shuts down, write a JSON file describing the run (below). |
| `-seed <n>` | Existing option. With this change it seeds every game RNG: the main generator, the generator that `RandomRange` instances use, and the ship firepoint, asteroid-split and random-message-sender shuffles. |

`-sim-result` file:

```json
{ "schema": 1, "mission": "my-mission", "seed": 1000, "fixed_dt": 0.0333333,
  "max_sim_time": 120.0, "simulated_seconds": 120.03, "wall_seconds": 9.4,
  "time_limit_reached": true, "termination": "time_limit" }
```

`termination` is `time_limit` when `-max-sim-time` ended the run and `mission_exit` when the mission or the game
ended it first.

## What runs and what does not

The authoritative frame is unchanged: `game_do_frame()` calls `game_frame()` and `game_simulation_frame()`, so
object movement, ship and weapon processing, collisions, mission parsing and goals, and scripting all run.

Script frame callbacks (`On Frame`) normally fire from `gr_flip()`, which headless mode never reaches. The
engine therefore runs them once per simulated frame through `gr_process_frame_callbacks()`, the same function the
rendered path uses. A mod that runs its game logic from `On Frame` behaves as usual.

Screens that wait for a person are passed automatically. `-start_mission` normally stops at the fiction, command
briefing, red alert and briefing screens and at the debriefing. In a headless single-player run the engine goes
straight to the briefing state, takes the default loadout through the same `commit_pressed()` call it uses for
missions flagged no-briefing, and enters the mission. When the mission ends it leaves the debriefing by quitting,
so the result file is written. If the default loadout cannot be committed (for example a required weapon is not
on any ship) the run ends with a warning in the engine log.

Skipped: renderer setup and frame rendering, sound and speech, input devices, HUD and radar, camera and photo
mode interpolation, the intro movie, and real-time frame pacing. Particle, trail, shockwave, asteroid and nebula
updates still run, because nothing has shown them to be irrelevant to gameplay. Model and collision data still
load.

## Quick start

1. Make a **base workspace**: a portable FreeSpace Open folder holding the root `.vp` files, any mod folders the
   mission needs, and a stored pilot (`data/players/<pilot>.json`, or `.plr`, in the root `data` folder or in a mod).
   The runner never writes to it.
2. Copy `example.plan.json`, set `engine`, `base_workspace`, `output_dir`, `mission` and `pilot`.
3. Check the plan, then run it:

   ```
   python tools/headless_batch/run_batch.py my.plan.json --dry-run    # validate and print the commands
   python tools/headless_batch/run_batch.py my.plan.json
   ```

Before a long batch, run one short simulation (`"repeats": 1, "max_sim_time": 20`) and confirm the result file
reports `simulated_seconds` close to 20. A run that never writes a result ends as a timeout and is reported as a
failure with the reason.

## Plan reference

Unknown keys are rejected, so a misspelt option fails instead of being ignored.

| Key | Required | Default | Meaning |
|---|---|---|---|
| `engine` | yes | | Path to the engine executable. |
| `base_workspace` | yes | | Portable workspace described above. |
| `output_dir` | yes | | Where runs and results go. Keep the path short on Windows, see Limits. |
| `mission` | yes | | Passed to `-start_mission`. A simple name (letters, digits, `_ . -`). |
| `pilot` | yes | | Stored pilot, passed to `-pilot`. |
| `max_sim_time` | yes | | Simulated seconds to run, passed to `-max-sim-time`. |
| `mods` | no | `[]` | Mod folder names under the base workspace, passed to `-mod` in this order. |
| `seeds` | no | | Explicit list of seeds, integers in 1..2147483647. One run per seed. |
| `seed_start`, `repeats` | no | 1000, 1 | Used when `seeds` is absent: seeds `seed_start` .. `seed_start + repeats - 1`. |
| `workers` | no | 1 | Engine processes running at once. |
| `fixed_dt` | no | 1/30 | Passed to `-fixed-dt`. |
| `timeout_seconds` | no | 600 | Wall-clock limit per run; a run over it is killed and counted as failed. |
| `base_port` | no | 39000 | Run *i* binds UDP port `base_port + i`. Every engine process binds one at startup, so ports must be unique. |
| `extra_args` | no | `[]` | Extra engine arguments appended to the command line. |

## What a run does

For run *i* with seed *s* the runner:

1. creates `output_dir/runs/r000i/w`, hardlinks the root `.vp` files and every `.vp` inside the mod folders,
   and copies the rest, so a run can never write through to the base workspace;
2. starts `engine -portable_mode -parse_cmdline_only -port <base_port+i> -start_mission <mission> -pilot <pilot>
   -headless -fixed-dt <dt> -max-sim-time <t> -sim-result <path> -noninteractive -nomovies -seed <s>`, then
   `-mod` if any, then `extra_args`, with the workspace as the working directory;
3. waits up to `timeout_seconds`, killing the whole process tree if it is exceeded;
4. reads the result file and records the run.

A run is `ok` only if the engine exited with code 0 and wrote a readable result file whose `termination` is
`time_limit` or `mission_exit`. A nonzero exit, a missing or unreadable result, a mission that never started, or a
timeout is a failure and is recorded with the reason; the batch never reports success for
a run it could not verify. On Windows the runner turns off crash and missing-file dialogs for its children so a
crashing engine exits instead of blocking an unattended batch (this changes no registry or machine setting).

## Output

```
output_dir/
  results.jsonl      one JSON record per finished run (index, seed, port, status, exit code, wall seconds,
                     the engine's result, and a list of problems)
  summary.json       runs, ok, failed, timed out, exit codes, terminations, simulated seconds, batch wall
                     seconds and simulated seconds per wall second
  runs/r0000/        result.json (same record), stdout.txt, stderr.txt, engine-result.json, w/ (workspace)
```

`--resume` keeps finished (`ok`) runs whose seed still matches the plan and reruns the others (failed,
interrupted, or run with another seed) from a clean workspace; without it a non-empty
`runs/` folder is refused so earlier results are never overwritten. The exit code is 0 when every run is `ok`,
1 when any failed, and 2 for a plan or usage error.

## Running the engine directly

```
fs2_open_x64.exe -portable_mode -parse_cmdline_only -mod mymod -start_mission my-mission -pilot Tester
  -headless -fixed-dt 0.0333333333 -max-sim-time 120 -sim-result result.json -noninteractive -seed 1000
```

## Reproducibility

`-seed` removes the known engine-side sources of run-to-run variation, but it does not make runs exact replays.
A script that draws its own random numbers (for example Lua `math.random()`, which uses a separate generator)
must seed itself from the run seed. In our tests two short same-seed runs of one scenario were identical, while
two same-seed runs of a long scenario with many AI-driven ships diverged (24 versus 25 losses), and the cause was
not found. Treat same-seed runs as correlated, not identical, and use several seeds for any comparison. The
engine has no fixed-step mode outside this feature, so rendered and headless runs of one mission are not
expected to match frame for frame either.

## Limits and known issues

- **Windows paths.** The engine cannot open files whose full path is 260 characters or longer. The runner checks
  this before it starts: it computes the longest path it will stage under `output_dir` and refuses the plan,
  naming the longest `output_dir` that fits.
- **Hardlinks.** `.vp` files are hardlinked, which needs `base_workspace` and `output_dir` on the same volume.
  The runner checks this before it starts and refuses the plan otherwise.
- **Process-level parallelism only.** The engine has a lot of global state, so each simulation gets its own
  process. More workers than physical cores did not help in our measurements.
- **Not a rendering test.** Anything that depends on the renderer, audio or input does not run.
- **Negative option values.** The engine's command-line parser reads a value that starts with `-` (for example
  `-seed -1`) as a new option and shows an "Unrecognized command line parameter" message box, which blocks a
  headless run. The runner cannot produce this (seeds must be above zero and `fixed_dt` is range-checked); keep to
  positive values when you start the engine by hand.

## Tests

`python -m unittest tools/headless_batch/test_run_batch.py`. They cover plan validation, command construction,
workspace staging, the same-volume and path-length checks, and a stub engine for success, crash, missing result and
timeout. They need no game files.
