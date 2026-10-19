# Headless batch runner

Run many FreeSpace Open missions as fast as the CPU allows, with no window, no rendering and no audio, then
collect one result per run. Each run is a separate engine process with its own workspace, seed and UDP port,
so runs share no in-process state and the batch scales with the number of cores.

This directory has the runner (`run_batch.py`), its tests (`test_run_batch.py`) and a starting plan
(`example.plan.json`). The engine side of the feature is the set of `-headless` command-line options described
below.

## The engine options

| Option | Effect |
|---|---|
| `-headless` | No window, renderer, audio, speech, joystick, HUD/radar presentation or movies. Graphics runs on the stub backend. Frames run as fast as possible, and mission time comes from a virtual clock that advances only by simulated time, which removes the main dependence of results on machine speed (see Reproducibility for what remains). Wall-clock timers (profiling, UI deadlines) keep real time. Implies `-nomovies` and `-noninteractive` (warnings are only logged; an error or a Lua error ends the process instead of opening a dialog). |
| `-fixed-dt <seconds>` | Length of one simulation step. Default 1/30; must be in [1/65536, 0.25]. It sets the step, it does not enlarge the engine's integration step. |
| `-max-sim-time <seconds>` | End the run once this much mission time has been simulated. Must be above 0 and at most 32767 (the range of the engine's fixed-point mission clock). |
| `-sim-result <path>` | When the engine shuts down, write a JSON file describing the run (below). |
| `-seed <n>` | Existing option. With this change it seeds the generators the simulation uses: the main generator, the generator that `RandomRange` instances use, and the ship firepoint, asteroid-split and random-message-sender shuffles. |

`-sim-result` file:

```json
{ "schema": 1, "mission": "my-mission", "seed": 1000, "fixed_dt": 0.0333333,
  "max_sim_time": 120.0, "simulated_seconds": 120.03, "wall_seconds": 9.4,
  "time_limit_reached": true, "termination": "time_limit",
  "goals": [ { "name": "Destroy the convoy", "type": "primary", "status": "complete" } ] }
```

`termination` is `time_limit` when `-max-sim-time` ended the run, `start_failed` when the default loadout could not
be committed so the mission never started, `player_died` when the player's ship blew up, and `mission_exit` when the
mission or the game ended it first. `goals` lists the mission's goals (invalidated ones left out) with `type`
`primary`, `secondary` or `bonus` and `status` `complete`, `failed` or `incomplete`, as they stood when the run
ended.

## What runs and what does not

The authoritative frame is unchanged: `game_do_frame()` calls `game_frame()` and `game_simulation_frame()`, so
object movement, ship and weapon processing, collisions, mission parsing and goals, and scripting all run.

Script frame callbacks (`On Frame`) normally fire from `gr_flip()`, which headless mode never reaches. The
engine therefore runs them once per simulated frame through `gr_process_frame_callbacks()`, the same function the
rendered path uses. A mod that runs its game logic from `On Frame` behaves as usual.

Keyboard, mouse and joystick input is not read, but the player's flight controls still run, so a script can fly
the player ship (scripted flight control) and the player warp-out sequence works.

Screens that wait for a person are passed automatically. `-start_mission` normally stops at the fiction, command
briefing, red alert and briefing screens and at the debriefing. In a headless single-player run the engine skips
the fiction and command briefing. At the briefing it takes the default loadout through the same `commit_pressed()`
call it uses for missions flagged no-briefing; at the red alert screen it continues into the mission as that
screen's button does. When the mission ends it leaves the debriefing by quitting, so the result file is written.
If the default loadout cannot be committed (for example a required weapon is not on any ship) the run ends with a
warning in the engine log and `start_failed` in the result. The death popup cannot be answered either, so when the
player's ship blows up the run ends with `player_died`. Any other modal popup is logged and answered as if
cancelled, unless a script handles `On Dialog Frame`.

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
   and copies the rest, so a run does not write through to the base workspace (the engine only reads `.vp` files;
   a run that rewrote one in place would change the base too);
2. starts `engine -portable_mode -parse_cmdline_only -port <base_port+i> -start_mission <mission> -pilot <pilot>
   -headless -fixed-dt <dt> -max-sim-time <t> -sim-result <path> -noninteractive -nomovies -seed <s>`, then
   `-mod` if any, then `extra_args`, with the workspace as the working directory, stdout and stderr going to the
   run folder, and `FSO_KEEP_STDOUT=1` in the environment (without it the Windows engine reattaches its output to
   the parent's console, if there is one, and the files stay empty);
3. waits up to `timeout_seconds`, killing the whole process tree if it is exceeded;
4. reads the result file and records the run.

A run is `ok` only if the engine exited with code 0 and wrote a readable result file whose `termination` is
`time_limit`, `player_died` or `mission_exit` (a player death is an outcome of the mission, not a failure of the
run). A nonzero exit, a missing or unreadable result, a mission that never started, or a
timeout is a failure and is recorded with the reason; a run whose result could not be read is not
reported as a success. With `-noninteractive` the engine prints an error, or a Lua error and its traceback, to
stderr before it exits, so a failed run's problems quote that message (`engine said: ...`) and `stderr.txt` has it
in full. Release builds write no `fs2_open.log`, so stderr is often the only record of why a run stopped. On Windows the runner turns off crash and missing-file dialogs for its children so a
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

On Windows, set `FSO_KEEP_STDOUT=1` if you redirect the engine's stdout or stderr to a file; otherwise, started
from a console, the engine writes them to that console instead.

## Reproducibility

`-seed` removes the known engine-side sources of run-to-run variation, but it does not make runs exact replays.
A script that draws its own random numbers (for example Lua `math.random()`, which uses a separate generator)
must seed itself from the run seed. In our tests two short same-seed runs of one scenario were identical, while
two same-seed runs of a long scenario with many AI-driven ships diverged (24 versus 25 losses), and the cause was
not found (a real-clock afterburner cooldown, since moved to the mission clock, is a possible but unconfirmed
cause). Treat same-seed runs as correlated, not identical, and use several seeds for any comparison. The
generators behind `RandomRange` are seeded once, when the tables load, so a mission restarted inside one process
does not repeat; that is why the runner starts one process per run. The
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
- **Single player, through `-start_mission`.** The screens are only passed automatically on the way into and out of
  a single-player mission. Without `-start_mission` the engine waits at the main hall, unless a script drives it.
- **Negative option values.** The engine's command-line parser reads a value that starts with `-` (for example
  `-seed -1`) as a new option and shows an "Unrecognized command line parameter" message box, which blocks a
  headless run. The runner cannot produce this (seeds must be above zero and `fixed_dt` is range-checked); keep to
  positive values when you start the engine by hand.

## Tests

`python -m unittest tools/headless_batch/test_run_batch.py`. They cover plan validation, command construction,
workspace staging, the same-volume and path-length checks, and a stub engine for success, crash, missing result and
timeout. They need no game files.
