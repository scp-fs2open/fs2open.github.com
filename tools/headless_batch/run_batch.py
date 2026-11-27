#!/usr/bin/env python3
"""Run many headless FreeSpace Open simulations in parallel OS processes.

Each run is one engine process in its own portable workspace, with its own seed
and UDP port, started with -headless, a fixed step and a simulated-time limit.
The engine's -sim_result sidecar is collected into results.jsonl and summary.json.
See README.md for the plan format and the engine options this relies on.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import ctypes
import functools
import hashlib
import json
import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path
from typing import Any

NAME = re.compile(r"[A-Za-z0-9_.-]+")
RESERVED_NAMES = {".", ".."}
MAX_SEED = 2**31 - 1
MAX_SIM_TIME = 32767.0  # the engine's mission clock is 16.16 fixed point
FIXED_DT_MIN = 1.0 / 65536  # a shorter step rounds to zero in the engine
# engine terminations that mean the mission ran
FINISHED = {"time_limit", "player_died", "mission_exit", "left_mission"}
PACK_SUFFIXES = (".vp", ".vpc")  # root packs, plain and compressed
FLAG_CHECK_TIMEOUT = 60.0
# The plan keys that change what a run does. A kept run must have been made with the same values (and its seed).
FINGERPRINT_KEYS = ("engine", "base_workspace", "mission", "pilot", "mods", "fixed_dt", "max_sim_time", "extra_args")

# Plan keys, with None meaning "required". Unknown keys are rejected so typos fail loudly.
PLAN_DEFAULTS: dict[str, Any] = {
    "engine": None,
    "base_workspace": None,
    "output_dir": None,
    "mission": None,
    "pilot": None,
    "max_sim_time": None,
    "mods": [],
    "seeds": None,
    "seed_start": 1000,
    "repeats": 1,
    "workers": 1,
    "fixed_dt": 1.0 / 30.0,
    "timeout_seconds": 600.0,
    "base_port": 39000,
    "extra_args": [],
    "keep_workspaces": False,
}


class PlanError(ValueError):
    """The plan file or the workspace it names is not usable."""


def _name(value: Any, what: str) -> str:
    if not isinstance(value, str) or not NAME.fullmatch(value) or value in RESERVED_NAMES:
        raise PlanError(f"{what} must be a simple name (letters, digits, _ . -), got {value!r}")
    return value


def _path(value: Any, what: str) -> Path:
    if not isinstance(value, str) or not value or "\0" in value:
        raise PlanError(f"{what} must be a path string, got {value!r}")
    # absolute, but symlinks are kept: an engine or interpreter reached through a link may depend on its own path
    return Path(os.path.abspath(value))


def _number(plan: dict[str, Any], key: str, low: float, high: float, integer: bool = False,
            low_inclusive: bool = True) -> float | int:
    """A finite number in the given range (a duration or step must be strictly above zero: low_inclusive=False)."""
    value = plan[key]
    if isinstance(value, bool) or not isinstance(value, (int, float)) or value != value or abs(value) == float("inf"):
        raise PlanError(f"{key} must be a finite number, got {value!r}")
    if integer and int(value) != value:
        raise PlanError(f"{key} must be a whole number, got {value!r}")
    above_low = value >= low if low_inclusive else value > low
    if not (above_low and value <= high):
        raise PlanError(f"{key} must be {'at least' if low_inclusive else 'above'} {low} and at most {high}, got {value!r}")
    return int(value) if integer else float(value)


def load_plan(path: Path) -> dict[str, Any]:
    """Read, validate and normalise a plan. Raises PlanError with a message that names the problem."""
    try:
        raw = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise PlanError(f"cannot read plan {path}: {exc}") from exc
    if not isinstance(raw, dict):
        raise PlanError("the plan must be a JSON object")
    unknown = sorted(set(raw) - set(PLAN_DEFAULTS))
    if unknown:
        raise PlanError(f"unknown plan keys: {', '.join(unknown)}")
    plan = {**PLAN_DEFAULTS, **raw}
    missing = [key for key in ("engine", "base_workspace", "output_dir", "mission", "pilot", "max_sim_time")
               if plan[key] is None]
    if missing:
        raise PlanError(f"missing required plan keys: {', '.join(missing)}")

    plan["mission"] = _name(plan["mission"], "mission")
    plan["pilot"] = _name(plan["pilot"], "pilot")
    if not isinstance(plan["mods"], list):
        raise PlanError("mods must be a list of directory names")
    plan["mods"] = [_name(mod, "mod") for mod in plan["mods"]]
    if not isinstance(plan["extra_args"], list) or not all(isinstance(a, str) for a in plan["extra_args"]):
        raise PlanError("extra_args must be a list of strings")
    if not isinstance(plan["keep_workspaces"], bool):
        raise PlanError(f"keep_workspaces must be true or false, got {plan['keep_workspaces']!r}")

    plan["fixed_dt"] = _number(plan, "fixed_dt", FIXED_DT_MIN, 0.25)
    plan["max_sim_time"] = _number(plan, "max_sim_time", 0.0, MAX_SIM_TIME, low_inclusive=False)
    plan["timeout_seconds"] = _number(plan, "timeout_seconds", 0.0, 1e7, low_inclusive=False)
    plan["workers"] = _number(plan, "workers", 1, 256, integer=True)
    plan["base_port"] = _number(plan, "base_port", 1024, 65000, integer=True)

    if plan["seeds"] is None:
        start = int(_number(plan, "seed_start", 1, MAX_SEED, integer=True))
        repeats = int(_number(plan, "repeats", 1, 100000, integer=True))
        if start + repeats - 1 > MAX_SEED:
            raise PlanError(f"seed_start plus repeats would make seeds above {MAX_SEED}")
        plan["seeds"] = [start + i for i in range(repeats)]
    else:
        seeds = plan["seeds"]
        if (not isinstance(seeds, list) or not seeds
                or not all(isinstance(s, int) and not isinstance(s, bool) and 1 <= s <= MAX_SEED for s in seeds)):
            raise PlanError(f"seeds must be a non-empty list of integers in 1..{MAX_SEED}")
    if plan["base_port"] + len(plan["seeds"]) > 65535:
        raise PlanError("base_port plus the number of runs exceeds the port range")

    # Absolute, because the engine runs in its run folder: a relative -sim_result path would land there.
    plan["engine"] = _path(plan["engine"], "engine")
    plan["base_workspace"] = _path(plan["base_workspace"], "base_workspace")
    plan["output_dir"] = _path(plan["output_dir"], "output_dir")
    if not plan["engine"].is_file():
        raise PlanError(f"engine not found: {plan['engine']}")
    check_workspace(plan["base_workspace"], plan["mods"], plan["pilot"])
    check_separate(plan["base_workspace"], plan["output_dir"])
    check_same_volume(plan["base_workspace"], plan["output_dir"])
    check_path_length(plan["base_workspace"], plan["mods"], plan["output_dir"])
    return plan


# The engine cannot open a file whose full path reaches this many characters (Windows MAX_PATH, terminator included).
WINDOWS_PATH_LIMIT = 260
RUN_FOLDER_SUFFIX = os.sep + os.path.join("runs", "r0000", "w")  # the longest run folder name the runner makes


def _volume_of(path: Path) -> int:
    """Device id of the nearest existing ancestor (the folder may not exist yet)."""
    probe = path.resolve()
    while not probe.exists():
        probe = probe.parent
    return probe.stat().st_dev


def check_separate(base: Path, output_dir: Path) -> None:
    """Neither folder may contain the other: staging would copy the output into itself, or runs would write into the
    base workspace."""
    base_real, output_real = base.resolve(), output_dir.resolve()
    if base_real == output_real or base_real in output_real.parents or output_real in base_real.parents:
        raise PlanError(f"base_workspace and output_dir must not contain each other ({base} and {output_dir})")


def check_same_volume(base: Path, output_dir: Path) -> None:
    """.vp packs are hardlinked into each run, which only works inside one volume. Fail now, not on run 1."""
    if _volume_of(base) != _volume_of(output_dir):
        raise PlanError(f"base_workspace and output_dir must be on the same volume so the .vp packs can be hardlinked "
                        f"({base} and {output_dir} are not)")


def longest_relative_path(base: Path, mods: list[str]) -> int:
    """Length of the longest path, relative to a workspace root, among the files a run will have."""
    longest = 0
    for top in [base / "data", *(base / mod for mod in mods)]:
        for folder, _dirs, files in os.walk(top):
            relative = os.path.relpath(folder, base)
            for name in files:
                longest = max(longest, len(os.path.join(relative, name)))
    return longest


def check_path_length(base: Path, mods: list[str], output_dir: Path, limit: int = WINDOWS_PATH_LIMIT) -> None:
    """Windows only: refuse an output_dir so deep that the engine could not open some staged file."""
    if os.name != "nt":
        return
    worst = len(str(output_dir.resolve())) + len(RUN_FOLDER_SUFFIX) + 1 + longest_relative_path(base, mods)
    if worst >= limit:
        allowed = len(str(output_dir.resolve())) - (worst - limit + 1)
        raise PlanError(f"output_dir is too long: the longest staged file path would be {worst} characters but the engine "
                        f"cannot open paths of {limit} or more. Use an output_dir of at most {allowed} characters "
                        f"(it is {len(str(output_dir.resolve()))} now)")


def check_workspace(base: Path, mods: list[str], pilot: str) -> None:
    if not base.is_dir():
        raise PlanError(f"base_workspace not found: {base}")
    if not root_packs(base):
        raise PlanError(f"base_workspace has no root .vp or .vpc files: {base}")
    for mod in mods:
        if not (base / mod).is_dir():
            raise PlanError(f"mod directory not found under base_workspace: {base / mod}")
    # A stored pilot is mandatory: without one the engine stops at the pilot chooser. -pilot only opens
    # <pilot>.json in the root data/players folder; it does not look in mods or convert .plr files.
    if not (base / "data" / "players" / (pilot + ".json")).is_file():
        raise PlanError(f"no stored pilot {pilot!r}: -pilot needs data/players/{pilot}.json in the base workspace")


def root_packs(base: Path) -> list[Path]:
    return [item for item in base.iterdir() if item.is_file() and item.suffix.lower() in PACK_SUFFIXES]


def link_or_copy(source: str, destination: str) -> None:
    """Hardlink packs (immutable and large); copy everything else so a run can never write through to the base."""
    if source.lower().endswith(PACK_SUFFIXES):
        try:
            os.link(source, destination)
            return
        except OSError as exc:
            raise RuntimeError(
                f"cannot hardlink {source} ({exc}); base_workspace and output_dir must be on the same volume, and "
                "NTFS allows at most 1023 links to one file") from exc
    shutil.copy2(source, destination)


# Serialises the read-only handling below, which briefly clears an attribute that all links to a pack share.
_READ_ONLY_LOCK = threading.Lock()


def _retry_writable(workspace: Path, base: Path, function: Any, path: str, error: Any) -> None:
    """rmtree error handler: if `path` is read-only, make it writable and retry `function`. A hardlink shares its mode
    with the base copy, so that copy gets its own mode back. Any other failure is raised as it was.

    `error` is the exception (onexc, Python 3.12+) or the sys.exc_info() tuple (onerror)."""
    twin = base / os.path.relpath(path, workspace)
    with _READ_ONLY_LOCK:
        mode = stat.S_IMODE(os.lstat(path).st_mode)
        if mode & stat.S_IWRITE:
            # not read-only, so it failed for another reason, such as the file being open
            raise error[1] if isinstance(error, tuple) else error
        shared = twin.is_file() and os.path.samefile(path, twin)
        os.chmod(path, mode | stat.S_IWRITE)
        try:
            function(path)
        finally:
            if shared:
                os.chmod(twin, mode)


def remove_workspace(workspace: Path, base: Path) -> None:
    """Delete a run workspace. Windows refuses to delete a read-only file, and a hardlinked pack shares the read-only
    attribute with the base copy, so such a link is made writable, deleted, and the base copy's mode restored."""
    handler = functools.partial(_retry_writable, workspace, base)
    if sys.version_info >= (3, 12):
        shutil.rmtree(workspace, onexc=handler)
    else:
        shutil.rmtree(workspace, onerror=handler)


def save_logs(workspace: Path, destination: Path) -> None:
    """Copy the engine's .log files out of a workspace, keeping their paths relative to it."""
    for log in workspace.rglob("*.log"):
        target = destination / log.relative_to(workspace)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(log, target)


def _ignore_logs(_directory: str, names: list[str]) -> set[str]:
    return {name for name in names if name.lower().endswith(".log")}


def prepare_workspace(base: Path, destination: Path, mods: list[str]) -> None:
    destination.mkdir(parents=True, exist_ok=False)
    for item in root_packs(base):
        link_or_copy(str(item), str(destination / item.name))
    if (base / "data").is_dir():
        shutil.copytree(base / "data", destination / "data", ignore=_ignore_logs, copy_function=link_or_copy)
    else:
        (destination / "data").mkdir()
    for mod in mods:
        shutil.copytree(base / mod, destination / mod, ignore=_ignore_logs, copy_function=link_or_copy)


def build_command(plan: dict[str, Any], seed: int, port: int, result_path: Path, launcher: tuple[str, ...] = ()) -> list[str]:
    """The engine command line for one run. `launcher` is test plumbing (an interpreter argument before the flags)."""
    command = [str(plan["engine"]), *launcher,
               "-portable_mode", "-parse_cmdline_only", "-port", str(port),
               "-start_mission", plan["mission"], "-pilot", plan["pilot"],
               "-headless", "-fixed_dt", repr(plan["fixed_dt"]), "-max_sim_time", repr(plan["max_sim_time"]),
               "-sim_result", str(result_path), "-noninteractive", "-nomovies", "-seed", str(seed)]
    if plan["mods"]:
        command += ["-mod", ",".join(plan["mods"])]
    return command + plan["extra_args"]


def suppress_crash_dialogs() -> None:
    """Windows only: stop a crashing engine from blocking an unattended batch on a dialog.

    Process-wide for this runner and inherited by its children; it changes no registry or machine setting.
    """
    if os.name == "nt":
        # SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX
        ctypes.windll.kernel32.SetErrorMode(0x0001 | 0x0002 | 0x8000)


def kill_tree(process: subprocess.Popen[Any]) -> None:
    if os.name == "nt":
        subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"], capture_output=True, check=False)
    else:
        try:
            os.killpg(process.pid, 9)
        except ProcessLookupError:
            pass
    process.wait()


class LiveEngines:
    """The engine processes running now, so an interrupted batch can kill them. Once stopped, no new run starts."""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._processes: set[subprocess.Popen[Any]] = set()
        self._killed: set[subprocess.Popen[Any]] = set()
        self._stopped = False

    def start(self, command: list[str], **popen_args: Any) -> subprocess.Popen[Any] | None:
        with self._lock:
            if self._stopped:
                return None
            process = subprocess.Popen(command, **popen_args)
            self._processes.add(process)
            return process

    def finished(self, process: subprocess.Popen[Any]) -> None:
        with self._lock:
            self._processes.discard(process)

    def stop_all(self) -> None:
        with self._lock:
            self._stopped = True
            processes = [process for process in self._processes if process.poll() is None]
            self._killed.update(processes)
        for process in processes:
            kill_tree(process)

    def was_killed(self, process: subprocess.Popen[Any]) -> bool:
        """Whether stop_all() killed this process (it was still running when the batch was interrupted)."""
        with self._lock:
            return process in self._killed


def engine_flags(engine: Path, launcher: tuple[str, ...], parent: Path) -> set[str] | None:
    """The command-line flags the engine knows, from its -help listing (lower case), or None if it printed none.

    An unknown flag opens a message box that ignores -noninteractive, so each run would wait for its timeout.
    The engine runs in a new temporary folder under `parent`, removed afterwards."""
    parent.mkdir(parents=True, exist_ok=True)
    workdir = tempfile.mkdtemp(prefix="flag-check-", dir=parent)
    try:
        completed = subprocess.run([str(engine), *launcher, "-portable_mode", "-parse_cmdline_only", "-help"],
                                   cwd=workdir, capture_output=True, timeout=FLAG_CHECK_TIMEOUT, check=False,
                                   env={**os.environ, "FSO_KEEP_STDOUT": "1"})
    except (OSError, subprocess.TimeoutExpired):
        return None
    finally:
        # a file the engine still holds open is left behind rather than failing the check
        shutil.rmtree(workdir, ignore_errors=True)
    flags = {flag.lower() for flag in re.findall(r"\[ (-\S+) \]", completed.stdout.decode(errors="replace"))}
    return flags or None


def unknown_flags(command: list[str], known: set[str]) -> list[str]:
    """Arguments that start with a dash and that the engine does not list. The engine reads every such argument as a
    flag, a value like -0.5 included, and opens a message box for an unknown one."""
    return sorted({argument for argument in command[1:]
                   if argument.startswith("-") and argument.lower() not in known})


def plan_fingerprint(plan: dict[str, Any]) -> str:
    """A short hash of the plan values that change what a run does (FINGERPRINT_KEYS)."""
    values = {key: str(plan[key]) if isinstance(plan[key], Path) else plan[key] for key in FINGERPRINT_KEYS}
    return hashlib.sha256(json.dumps(values, sort_keys=True).encode("utf-8")).hexdigest()[:16]


def engine_reason(stderr_text: str) -> str | None:
    """Why the engine stopped, from its stderr: with -noninteractive, Error() prints "Error: <message>", and a Lua
    error prints its message followed by a dashed separator and the traceback. Warnings are skipped."""
    lines = [line.strip() for line in stderr_text.splitlines()]
    errors = [line for line in lines if line.startswith("Error: ")]
    if errors:
        return errors[-1]
    separator = next((i for i, line in enumerate(lines) if len(line) > 10 and set(line) == {"-"}), len(lines))
    candidates = [line for line in lines[:separator] if line and not line.startswith("WARNING:")]
    return candidates[-1] if candidates else None


def read_engine_result(path: Path) -> tuple[dict[str, Any] | None, str | None]:
    """The engine's -sim_result sidecar, or None and why it is not usable."""
    try:
        result = json.loads(path.read_text(encoding="utf-8"))
    except ValueError as exc:  # truncated or not UTF-8 (json.JSONDecodeError and UnicodeDecodeError)
        return None, f"unreadable engine result: {exc}"
    if not isinstance(result, dict):
        return None, f"engine result is not a JSON object: {type(result).__name__}"
    return result, None


def run_one(plan: dict[str, Any], index: int, seed: int, launcher: tuple[str, ...] = (),
            engines: LiveEngines | None = None) -> dict[str, Any]:
    """Stage a workspace, run one engine process to completion or timeout, and return its result record."""
    engines = engines or LiveEngines()
    run_dir = plan["output_dir"] / "runs" / f"r{index:04d}"
    workspace = run_dir / "w"
    result_path = run_dir / "engine-result.json"
    port = plan["base_port"] + index
    record: dict[str, Any] = {"index": index, "seed": seed, "port": port, "status": "failed",
                              "exit_code": None, "timed_out": False, "wall_seconds": None,
                              "sim_result": None, "problems": [], "plan_fingerprint": plan_fingerprint(plan)}
    try:
        prepare_workspace(plan["base_workspace"], workspace, plan["mods"])
        command = build_command(plan, seed, port, result_path, launcher)
        started = time.monotonic()
        with open(run_dir / "stdout.txt", "wb") as out, open(run_dir / "stderr.txt", "wb") as err:
            # On Windows the engine reattaches stdout/stderr to the parent's console unless FSO_KEEP_STDOUT is 1,
            # and the files above would stay empty
            process = engines.start(command, cwd=workspace, stdout=out, stderr=err,
                                    env={**os.environ, "FSO_KEEP_STDOUT": "1"},
                                    start_new_session=(os.name != "nt"))
            if process is None:
                raise RuntimeError("the batch was interrupted before this run started")
            try:
                process.wait(timeout=plan["timeout_seconds"])
            except subprocess.TimeoutExpired:
                record["timed_out"] = True
                kill_tree(process)
            finally:
                engines.finished(process)
        record["wall_seconds"] = round(time.monotonic() - started, 3)
        record["exit_code"] = process.returncode
        if engines.was_killed(process):
            record["problems"].append("killed: the batch was interrupted")
        if result_path.is_file():
            record["sim_result"], problem = read_engine_result(result_path)
            if problem:
                record["problems"].append(problem)
            else:
                termination = record["sim_result"].get("termination")
                if not isinstance(termination, str) or termination not in FINISHED:
                    record["problems"].append(f"mission did not run to an end (termination {termination!r})")
        else:
            record["problems"].append("engine wrote no -sim_result file")
        if record["timed_out"]:
            record["problems"].append(f"killed after {plan['timeout_seconds']} s")
        if record["exit_code"] not in (0, None) and not record["timed_out"]:
            record["problems"].append(f"engine exit code {record['exit_code']}")
        reason = engine_reason((run_dir / "stderr.txt").read_text(encoding="utf-8", errors="replace"))
        if record["problems"] and reason:
            record["problems"].append(f"engine said: {reason[:300]}")
        if not record["problems"]:
            record["status"] = "ok"
    except (OSError, RuntimeError) as exc:
        record["problems"].append(str(exc))
    (run_dir / "result.json").write_text(json.dumps(record, indent=2, sort_keys=True), encoding="utf-8")
    if not plan["keep_workspaces"] and workspace.exists():
        # each run hardlinks every pack again, and NTFS allows a file at most 1023 links
        try:
            if record["status"] != "ok":
                save_logs(workspace, run_dir / "logs")  # the engine log, when the build writes one
            remove_workspace(workspace, plan["base_workspace"])
        except OSError as exc:
            print(f"warning: could not delete the workspace of run {index}: {exc}", file=sys.stderr, flush=True)
    return record


def read_record(path: Path) -> dict[str, Any] | None:
    """A run's result.json, or None if it is missing or unreadable (an interrupted write)."""
    try:
        record = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None
    return record if isinstance(record, dict) else None


def _simulated_seconds(record: dict[str, Any]) -> float:
    value = (record.get("sim_result") or {}).get("simulated_seconds", 0.0)
    return float(value) if isinstance(value, (int, float)) and not isinstance(value, bool) else 0.0


def summarize(records: list[dict[str, Any]], batch_wall: float, kept: frozenset[int] = frozenset()) -> dict[str, Any]:
    """Counts over all records. The rate counts only the runs made by this invocation, not those kept by --resume."""
    ok = [r for r in records if r["status"] == "ok"]
    sim = sum(_simulated_seconds(r) for r in ok)
    sim_now = sum(_simulated_seconds(r) for r in ok if r.get("index") not in kept)
    exits: dict[str, int] = {}
    ends: dict[str, int] = {}
    for r in records:
        exits[str(r["exit_code"])] = exits.get(str(r["exit_code"]), 0) + 1
        if r["sim_result"]:
            term = str(r["sim_result"].get("termination"))
            ends[term] = ends.get(term, 0) + 1
    return {"runs": len(records), "ok": len(ok), "failed": len(records) - len(ok),
            "kept_from_previous": len(kept),
            "timed_out": sum(1 for r in records if r["timed_out"]),
            "exit_codes": exits, "terminations": ends,
            "simulated_seconds_total": round(sim, 3), "batch_wall_seconds": round(batch_wall, 3),
            "simulated_seconds_per_wall_second": round(sim_now / batch_wall, 3) if batch_wall > 0 else None}


def write_results(path: Path, records: dict[int, dict[str, Any]]) -> None:
    """Rewrite results.jsonl from the given records, in run order, so no row from an earlier invocation remains."""
    temporary = path.with_suffix(".jsonl.tmp")
    with open(temporary, "w", encoding="utf-8") as sink:
        for index in sorted(records):
            sink.write(json.dumps(records[index], sort_keys=True) + "\n")
    os.replace(temporary, path)


INTERRUPT_GRACE_SECONDS = 10.0  # how long Ctrl-C waits for the killed runs to write their records


def _add_record(record: dict[str, Any], records: dict[int, dict[str, Any]], results_path: Path) -> None:
    records[record["index"]] = record
    with open(results_path, "a", encoding="utf-8") as sink:
        sink.write(json.dumps(record, sort_keys=True) + "\n")
    print(f"run {record['index']:>4} seed {record['seed']}: {record['status']}"
          + (f" ({'; '.join(record['problems'])})" if record["problems"] else ""), flush=True)


def run_all(plan: dict[str, Any], pending: list[tuple[int, int]], records: dict[int, dict[str, Any]],
            engines: LiveEngines) -> None:
    """Run the pending runs on the worker pool, adding each record as it finishes. On Ctrl-C, cancel the queued runs,
    kill the running engines, record the runs that finish within a short grace period, and re-raise."""
    results_path = plan["output_dir"] / "results.jsonl"
    pool = concurrent.futures.ThreadPoolExecutor(max_workers=plan["workers"])
    submitted: list[concurrent.futures.Future[dict[str, Any]]] = []
    recorded: set[concurrent.futures.Future[dict[str, Any]]] = set()
    try:
        submitted = [pool.submit(run_one, plan, i, s, engines=engines) for i, s in pending]
        waiting = set(submitted)
        while waiting:
            # a timed wait, so Ctrl-C reaches this thread on Windows too
            finished, waiting = concurrent.futures.wait(waiting, timeout=0.5,
                                                        return_when=concurrent.futures.FIRST_COMPLETED)
            for future in finished:
                _add_record(future.result(), records, results_path)
                recorded.add(future)
    except BaseException:
        pool.shutdown(wait=False, cancel_futures=True)
        engines.stop_all()
        # cancelled futures never complete, so wait only for the runs that had started
        started = [future for future in submitted if future not in recorded and not future.cancelled()]
        finished, _ = concurrent.futures.wait(started, timeout=INTERRUPT_GRACE_SECONDS)
        for future in finished:
            if future.exception() is None:
                _add_record(future.result(), records, results_path)
        raise
    pool.shutdown(wait=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("plan", type=Path, help="plan JSON file (see README.md)")
    parser.add_argument("--resume", action="store_true", help="keep finished runs in output_dir and run only the rest")
    parser.add_argument("--dry-run", action="store_true", help="validate the plan and print the commands without running")
    parser.add_argument("--skip-flag-check", action="store_true",
                        help="do not compare the command line with the engine's -help listing (it omits deprecated flags)")
    args = parser.parse_args(argv)

    try:
        plan = load_plan(args.plan)
    except PlanError as exc:
        print(f"plan error: {exc}", file=sys.stderr)
        return 2
    runs_dir = plan["output_dir"] / "runs"

    if args.dry_run:
        for index, seed in enumerate(plan["seeds"]):
            print(subprocess.list2cmdline(build_command(plan, seed, plan["base_port"] + index, runs_dir / f"r{index:04d}" / "engine-result.json")))
        return 0

    if runs_dir.exists() and any(runs_dir.iterdir()) and not args.resume:
        print(f"plan error: {runs_dir} is not empty; use --resume or a new output_dir", file=sys.stderr)
        return 2

    suppress_crash_dialogs()
    if not args.skip_flag_check:
        known = engine_flags(plan["engine"], (), plan["output_dir"])
        if known is None:
            print(f"plan error: {plan['engine']} -help listed no flags, so the command line cannot be checked; "
                  "use --skip-flag-check to run anyway", file=sys.stderr)
            return 2
        unknown = unknown_flags(build_command(plan, plan["seeds"][0], plan["base_port"], runs_dir / "x.json"), known)
        if unknown:
            print(f"plan error: the engine does not know {', '.join(unknown)} (an older engine, or a typo in "
                  "extra_args?); each run would stop at a message box. Use --skip-flag-check if the flag is "
                  "deprecated but still accepted", file=sys.stderr)
            return 2

    fingerprint = plan_fingerprint(plan)
    done: dict[int, dict[str, Any]] = {}
    if runs_dir.exists():
        for index, seed in enumerate(plan["seeds"]):
            run_dir = runs_dir / f"r{index:04d}"
            if not run_dir.exists():
                continue
            previous = read_record(run_dir / "result.json")
            if (previous and previous.get("status") == "ok" and previous.get("seed") == seed
                    and previous.get("plan_fingerprint") == fingerprint):
                done[index] = previous
            else:
                # failed, interrupted before it wrote a record, or made by another plan or seed: rerun it cleanly
                try:
                    workspace = run_dir / "w"
                    if workspace.exists():
                        # a kept workspace may hold read-only pack links
                        remove_workspace(workspace, plan["base_workspace"])
                    shutil.rmtree(run_dir)
                except OSError as exc:
                    print(f"plan error: cannot clear {run_dir} for a rerun: {exc}", file=sys.stderr)
                    return 2
    runs_dir.mkdir(parents=True, exist_ok=True)

    pending = [(i, s) for i, s in enumerate(plan["seeds"]) if i not in done]
    records = dict(done)
    results_path = plan["output_dir"] / "results.jsonl"
    write_results(results_path, records)  # drop rows left by an earlier invocation
    # an earlier summary would disagree with results.jsonl if this invocation is interrupted
    (plan["output_dir"] / "summary.json").unlink(missing_ok=True)
    started = time.monotonic()
    try:
        run_all(plan, pending, records, LiveEngines())
    except KeyboardInterrupt:
        write_results(results_path, records)
        print(f"interrupted: {len(records)} of {len(plan['seeds'])} runs have records; use --resume to continue",
              file=sys.stderr)
        return 130
    write_results(results_path, records)
    ordered = [records[i] for i in sorted(records)]
    summary = summarize(ordered, time.monotonic() - started, frozenset(done))
    (plan["output_dir"] / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True), encoding="utf-8")
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0 if summary["failed"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
