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
| `-fixed_dt <seconds>` | Length of one simulation step. Default 1/30; must be in [1/65536, 0.25]. It sets the step, it does not enlarge the engine's integration step. |
| `-max_sim_time <seconds>` | End the run once this much mission time has been simulated. Must be above 0 and at most 32767 (the range of the engine's fixed-point mission clock). |
| `-sim_result <path>` | When the engine shuts down, write a JSON file describing the run (below). |
| `-seed <n>` | Existing option. With this change it seeds the generators the simulation uses: the main generator, the generator that `RandomRange` instances use, and the ship firepoint, asteroid-split and random-message-sender shuffles. |

`-sim_result` file:

```json
{ "schema": 1, "mission": "my-mission", "seed": 1000, "fixed_dt": 0.0333333,
  "max_sim_time": 120.0, "simulated_seconds": 120.03, "wall_seconds": 9.4,
  "time_limit_reached": true, "termination": "time_limit",
  "goals": [ { "name": "Destroy the convoy", "type": "primary", "status": "complete" } ] }
```

`termination` is one of:

- `time_limit`: `-max_sim_time` ended the run;
- `player_died`: the player's ship blew up, or a supernova killed the player;
- `mission_exit`: the mission ended normally and reached the debriefing;
- `left_mission`: gameplay ended some other way, for example `end-mission` back to the main hall, a red alert, or
  the end of a campaign;
- `start_failed`: the default loadout could not be committed, so the mission never started;
- `load_failed`: the mission could not be loaded;
- `quit`: the engine quit by another route (the window closed, a signal, a script quitting the game).

The result is taken when the run's end is decided, so `mission`, `simulated_seconds` and `goals` describe the
mission that was played. A campaign red alert has by then already advanced the campaign in the run's own copy of the
pilot file; the next mission is not loaded.

`goals` lists the mission's goals (invalidated ones left out) with `type` `primary`, `secondary` or `bonus` and
`status` `complete`, `failed` or `incomplete`, as they stood when the run ended. Unfinished goals are marked failed
only by the debriefing and by a campaign red alert, so they are `failed` after `mission_exit` and after a campaign
red alert (`left_mission`), and `incomplete` after `time_limit`, `player_died` and any other `left_mission`.

The three options above require `-headless`, and `-headless` cannot be combined with `-standalone`, `-connect` or
`-startgame`; the engine refuses such a command line with an error.

## What runs and what does not

The authoritative frame is unchanged: `game_do_frame()` calls `game_frame()` and `game_simulation_frame()`, so
object movement, ship and weapon processing, collisions, mission parsing and goals, and scripting all run.

Script frame callbacks (`On Frame`) normally fire from `gr_flip()`, which headless mode never reaches. The
engine therefore runs them once per simulated frame through `gr_process_frame_callbacks()`, the same function the
rendered path uses. A mod that runs its game logic from `On Frame` behaves as usual.

Keyboard, mouse and joystick input is not read, but the player's flight controls and the Lua button actions
(targeting, weapon cycling, ETS, orders) still run, so a script can fly and operate the player ship, and the
player warp-out sequence works. The frame counter advances once per simulated frame, so mission messages play and
events that wait on them proceed.

Screens that wait for a person are passed automatically. `-start_mission` normally stops at the fiction, command
briefing, red alert and briefing screens and at the debriefing. In a headless single-player run the engine skips
the fiction and command briefing. At the briefing it takes the default loadout through the same `commit_pressed()`
call it uses for missions flagged no-briefing; at the red alert screen it continues into the mission as that
screen's button does. When the mission ends it leaves the debriefing by quitting, so the result file is written.
If the default loadout cannot be committed (for example a required weapon is not on any ship) the run ends with a
warning on stderr and `start_failed` in the result. The death popup cannot be answered either, so when the
player dies the run ends with `player_died`. Any way out of gameplay other than the debriefing ends the run as
`left_mission`, so a red alert does not play on into the next mission. Any other modal popup is answered as if
cancelled and reported on stderr, unless a script handles `On Dialog Frame`; a popup that script has not answered
after 10 s of real time is given up the same way.

Missile locks and cargo scans are processed in the HUD code that headless mode skips, so a scripted player never
locks on and goals that need the player to scan cargo cannot complete.

Skipped: renderer setup and frame rendering, sound and speech, input devices, HUD and radar, camera and photo
mode interpolation, the intro movie, and real-time frame pacing. Particle, trail, shockwave, asteroid and nebula
updates still run, because nothing has shown them to be irrelevant to gameplay. Model and collision data still
load.

## Quick start

1. Make a **base workspace**: a portable FreeSpace Open folder holding the root `.vp` or `.vpc` files, any mod
   folders the mission needs, and a stored pilot, `data/players/<pilot>.json` in the root `data` folder (`-pilot`
   does not look in mods or read `.plr` files). The runner never writes to its files; the only change it makes
   is to clear a read-only pack's read-only attribute for a moment while deleting a run's link to it, then put
   the pack's mode back. `output_dir` must not be inside it (or contain it).
2. Use Python 3.9 or later (standard library only). Copy `example.plan.json`, set `engine`, `base_workspace`, `output_dir`, `mission` and `pilot`.
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
| `max_sim_time` | yes | | Simulated seconds to run, passed to `-max_sim_time`. |
| `mods` | no | `[]` | Mod folder names under the base workspace, passed to `-mod` in this order. |
| `seeds` | no | | Explicit list of seeds, integers in 1..2147483647. One run per seed. |
| `seed_start`, `repeats` | no | 1000, 1 | Used when `seeds` is absent: seeds `seed_start` .. `seed_start + repeats - 1`. |
| `workers` | no | 1 | Engine processes running at once. |
| `fixed_dt` | no | 1/30 | Passed to `-fixed_dt`. |
| `timeout_seconds` | no | 600 | Wall-clock limit per run; a run over it is killed and counted as failed. |
| `base_port` | no | 39000 | Run *i* binds UDP port `base_port + i`. Every engine process binds one at startup, so ports must be unique. |
| `extra_args` | no | `[]` | Extra engine arguments appended to the command line. |
| `keep_workspaces` | no | `false` | Keep each run's `w/` workspace after the run instead of deleting it. Without it, a failed run keeps only its `.log` files, and files a mod or script writes into the workspace (its own logs or telemetry) are deleted with it. Each kept workspace holds a link to every pack, and NTFS allows 1023 per file. |

## What a run does

Before the first run, the runner starts the engine once with `-help` and checks every flag on the run command
line against that listing. An unknown flag (an engine without `-headless`, or a typo in `extra_args`) would open
an "Unrecognized command line parameter" message box that `-noninteractive` does not suppress, so every run would
wait for its timeout; the runner refuses the plan instead. `-help` omits deprecated flags, so pass
`--skip-flag-check` if you rely on one.

For run *i* with seed *s* the runner:

1. creates `output_dir/runs/r000i/w`, hardlinks the root `.vp` and `.vpc` files and every pack inside the mod folders,
   and copies the rest, so a run does not write through to the base workspace (the engine only reads `.vp` files;
   a run that rewrote one in place would change the base too);
2. starts `engine -portable_mode -parse_cmdline_only -port <base_port+i> -start_mission <mission> -pilot <pilot>
   -headless -fixed_dt <dt> -max_sim_time <t> -sim_result <path> -noninteractive -nomovies -seed <s>`, then
   `-mod` if any, then `extra_args`, with the workspace as the working directory, stdout and stderr going to the
   run folder, and `FSO_KEEP_STDOUT=1` in the environment (without it the Windows engine reattaches its output to
   the parent's console, if there is one, and the files stay empty);
3. waits up to `timeout_seconds`, killing the whole process tree if it is exceeded;
4. reads the result file and records the run;
5. deletes the workspace `w/`, unless `keep_workspaces` is set (each run adds a hardlink to every pack, and NTFS
   allows at most 1023 links to one file). A failed run first copies the workspace's `.log` files (the engine log,
   if the build writes one) to `logs/`. A read-only pack link is made writable to delete it, and the base copy
   gets its mode back. A workspace that cannot be deleted for another reason (a file still open) is left behind,
   possibly in part, with a warning.

Ctrl-C stops the batch: queued runs are cancelled and running engines are killed. The runner waits up to 10 s for
the killed runs to write their records (marked `killed: the batch was interrupted`), keeps every finished run in
`results.jsonl`, and exits with code 130 without writing `summary.json` (one left by an earlier invocation is
deleted when the batch starts). `--resume` continues from there.

A run is `ok` only if the engine exited with code 0 and wrote a readable result file whose `termination` is
`time_limit`, `player_died`, `mission_exit` or `left_mission` (a player death is an outcome of the mission, not a
failure of the run). A nonzero exit, a missing or unreadable result, a mission that never started or never loaded,
an engine that quit by another route (`quit`), or a timeout is a failure and is recorded with the reason; a run whose result could not be read is not
reported as a success. With `-noninteractive` the engine prints an error, or a Lua error and its traceback, to
stderr before it exits, so a failed run's problems quote that message (`engine said: ...`) and `stderr.txt` has it
in full. Release builds write no `fs2_open.log`, so stderr is often the only record of why a run stopped. On Windows the runner turns off crash and missing-file dialogs for its children so a
crashing engine exits instead of blocking an unattended batch (this changes no registry or machine setting).

## Output

```
output_dir/
  results.jsonl      one JSON record per run of the plan, in run order (index, seed, port, status, exit code,
                     wall seconds, the engine's result, a list of problems and the plan fingerprint)
  summary.json       runs, ok, failed, kept from a previous invocation, timed out, exit codes, terminations,
                     simulated seconds, batch wall seconds and simulated seconds per wall second
  runs/r0000/        result.json (same record), stdout.txt, stderr.txt, engine-result.json,
                     logs/ (a failed run's engine logs), w/ (workspace, only with keep_workspaces)
```

`--resume` keeps a finished (`ok`) run only if its seed and its plan fingerprint (a hash of `engine`,
`base_workspace`, `mission`, `pilot`, `mods`, `fixed_dt`, `max_sim_time` and `extra_args`) still match the plan,
and reruns the others (failed, interrupted, or made by another plan or seed) from a clean workspace. The
fingerprint holds paths, not file contents: a rebuilt engine or changed data at the same paths keeps the old runs,
so use a new `output_dir` after changing either. Run folders past the end of the plan's seed list are left in
place and not reported. It rewrites
`results.jsonl` from the final records, so no row from an earlier invocation remains, and the simulated seconds per
wall second count only the runs made by this invocation. Without `--resume` a non-empty `runs/` folder is refused so
earlier results are never overwritten. The exit code is 0 when every run is `ok`, 1 when any failed, 2 for a plan
or usage error and 130 after Ctrl-C.

## Running the engine directly

```
fs2_open_x64.exe -portable_mode -parse_cmdline_only -mod mymod -start_mission my-mission -pilot Tester
  -headless -fixed_dt 0.0333333333 -max_sim_time 120 -sim_result result.json -noninteractive -seed 1000
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
workspace staging, the same-volume and path-length checks, resume, and a stub engine for success, crash, missing
result and timeout. They need no game files. CI does not run them yet.
