#!/usr/bin/env python3
"""Run many headless FreeSpace Open simulations in parallel OS processes.

Each run is one engine process in its own portable workspace, with its own seed
and UDP port, started with -headless, a fixed step and a simulated-time limit.
The engine's -sim-result sidecar is collected into results.jsonl and summary.json.
See README.md for the plan format and the engine options this relies on.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import ctypes
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

NAME = re.compile(r"[A-Za-z0-9_.-]+")
RESERVED_NAMES = {".", ".."}
MAX_SEED = 2**31 - 1
MAX_SIM_TIME = 32767.0  # the engine's mission clock is 16.16 fixed point
FIXED_DT_MIN = 1.0 / 65536  # a shorter step rounds to zero in the engine
FINISHED = {"time_limit", "player_died", "mission_exit"}  # engine terminations that mean the mission ran

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
    return Path(value).resolve()


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

    plan["fixed_dt"] = _number(plan, "fixed_dt", FIXED_DT_MIN, 0.25)
    plan["max_sim_time"] = _number(plan, "max_sim_time", 0.0, MAX_SIM_TIME, low_inclusive=False)
    plan["timeout_seconds"] = _number(plan, "timeout_seconds", 0.0, 1e7, low_inclusive=False)
    plan["workers"] = _number(plan, "workers", 1, 256, integer=True)
    plan["base_port"] = _number(plan, "base_port", 1024, 65000, integer=True)

    if plan["seeds"] is None:
        start = int(_number(plan, "seed_start", 1, MAX_SEED, integer=True))
        repeats = int(_number(plan, "repeats", 1, 100000, integer=True))
        plan["seeds"] = [start + i for i in range(repeats)]
    else:
        seeds = plan["seeds"]
        if (not isinstance(seeds, list) or not seeds
                or not all(isinstance(s, int) and not isinstance(s, bool) and 1 <= s <= MAX_SEED for s in seeds)):
            raise PlanError(f"seeds must be a non-empty list of integers in 1..{MAX_SEED}")
    if plan["base_port"] + len(plan["seeds"]) > 65535:
        raise PlanError("base_port plus the number of runs exceeds the port range")

    # Absolute, because the engine runs in its run folder: a relative -sim-result path would land there.
    plan["engine"] = _path(plan["engine"], "engine")
    plan["base_workspace"] = _path(plan["base_workspace"], "base_workspace")
    plan["output_dir"] = _path(plan["output_dir"], "output_dir")
    if not plan["engine"].is_file():
        raise PlanError(f"engine not found: {plan['engine']}")
    check_workspace(plan["base_workspace"], plan["mods"], plan["pilot"])
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
    if not list(base.glob("*.vp")):
        raise PlanError(f"base_workspace has no root .vp files: {base}")
    for mod in mods:
        if not (base / mod).is_dir():
            raise PlanError(f"mod directory not found under base_workspace: {base / mod}")
    # A stored pilot is mandatory: without one the engine stops at the pilot chooser.
    places = [base / "data" / "players"] + [base / mod / "data" / "players" for mod in mods]
    if not any((place / (pilot + ext)).is_file() for place in places for ext in (".json", ".plr")):
        raise PlanError(f"no stored pilot {pilot!r} (data/players/{pilot}.json or .plr) in the base workspace or its mods")


def link_or_copy(source: str, destination: str) -> None:
    """Hardlink .vp packs (immutable and large); copy everything else so a run can never write through to the base."""
    if source.lower().endswith(".vp"):
        try:
            os.link(source, destination)
            return
        except OSError as exc:
            raise RuntimeError(
                f"cannot hardlink {source}; put base_workspace and output_dir on the same volume") from exc
    shutil.copy2(source, destination)


def _ignore_logs(_directory: str, names: list[str]) -> set[str]:
    return {name for name in names if name.lower().endswith(".log")}


def prepare_workspace(base: Path, destination: Path, mods: list[str]) -> None:
    destination.mkdir(parents=True, exist_ok=False)
    for item in base.iterdir():
        if item.is_file() and item.suffix.lower() == ".vp":
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
               "-headless", "-fixed-dt", repr(plan["fixed_dt"]), "-max-sim-time", repr(plan["max_sim_time"]),
               "-sim-result", str(result_path), "-noninteractive", "-nomovies", "-seed", str(seed)]
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


def kill_tree(process: subprocess.Popen[bytes]) -> None:
    if os.name == "nt":
        subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"], capture_output=True, check=False)
    else:
        try:
            os.killpg(process.pid, 9)
        except ProcessLookupError:
            pass
    process.wait()


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


def run_one(plan: dict[str, Any], index: int, seed: int, launcher: tuple[str, ...] = ()) -> dict[str, Any]:
    """Stage a workspace, run one engine process to completion or timeout, and return its result record."""
    run_dir = plan["output_dir"] / "runs" / f"r{index:04d}"
    workspace = run_dir / "w"
    result_path = run_dir / "engine-result.json"
    port = plan["base_port"] + index
    record: dict[str, Any] = {"index": index, "seed": seed, "port": port, "status": "failed",
                              "exit_code": None, "timed_out": False, "wall_seconds": None,
                              "sim_result": None, "problems": []}
    try:
        prepare_workspace(plan["base_workspace"], workspace, plan["mods"])
        command = build_command(plan, seed, port, result_path, launcher)
        started = time.monotonic()
        with open(run_dir / "stdout.txt", "wb") as out, open(run_dir / "stderr.txt", "wb") as err:
            process = subprocess.Popen(command, cwd=workspace, stdout=out, stderr=err,
                                       start_new_session=(os.name != "nt"))
            try:
                process.wait(timeout=plan["timeout_seconds"])
            except subprocess.TimeoutExpired:
                record["timed_out"] = True
                kill_tree(process)
        record["wall_seconds"] = round(time.monotonic() - started, 3)
        record["exit_code"] = process.returncode
        if result_path.is_file():
            try:
                record["sim_result"] = json.loads(result_path.read_text(encoding="utf-8"))
            except json.JSONDecodeError as exc:
                record["problems"].append(f"unreadable engine result: {exc}")
            else:
                termination = record["sim_result"].get("termination")
                if termination not in FINISHED:
                    record["problems"].append(f"mission did not run to an end (termination {termination!r})")
        else:
            record["problems"].append("engine wrote no -sim-result file")
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
    return record


def read_record(path: Path) -> dict[str, Any] | None:
    """A run's result.json, or None if it is missing or unreadable (an interrupted write)."""
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None


def summarize(records: list[dict[str, Any]], batch_wall: float) -> dict[str, Any]:
    ok = [r for r in records if r["status"] == "ok"]
    sim = sum(r["sim_result"].get("simulated_seconds", 0.0) for r in ok)
    exits: dict[str, int] = {}
    ends: dict[str, int] = {}
    for r in records:
        exits[str(r["exit_code"])] = exits.get(str(r["exit_code"]), 0) + 1
        if r["sim_result"]:
            term = str(r["sim_result"].get("termination"))
            ends[term] = ends.get(term, 0) + 1
    return {"runs": len(records), "ok": len(ok), "failed": len(records) - len(ok),
            "timed_out": sum(1 for r in records if r["timed_out"]),
            "exit_codes": exits, "terminations": ends,
            "simulated_seconds_total": round(sim, 3), "batch_wall_seconds": round(batch_wall, 3),
            "simulated_seconds_per_wall_second": round(sim / batch_wall, 3) if batch_wall > 0 else None}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("plan", type=Path, help="plan JSON file (see README.md)")
    parser.add_argument("--resume", action="store_true", help="keep finished runs in output_dir and run only the rest")
    parser.add_argument("--dry-run", action="store_true", help="validate the plan and print the commands without running")
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

    done: dict[int, dict[str, Any]] = {}
    if runs_dir.exists() and any(runs_dir.iterdir()):
        if not args.resume:
            print(f"plan error: {runs_dir} is not empty; use --resume or a new output_dir", file=sys.stderr)
            return 2
        for index, seed in enumerate(plan["seeds"]):
            run_dir = runs_dir / f"r{index:04d}"
            if not run_dir.exists():
                continue
            previous = read_record(run_dir / "result.json")
            if previous and previous.get("status") == "ok" and previous.get("seed") == seed:
                done[index] = previous
            else:
                # failed, interrupted before it wrote a record, or run with another seed: rerun from a clean workspace
                shutil.rmtree(run_dir)
    runs_dir.mkdir(parents=True, exist_ok=True)

    suppress_crash_dialogs()
    pending = [(i, s) for i, s in enumerate(plan["seeds"]) if i not in done]
    records = dict(done)
    started = time.monotonic()
    with concurrent.futures.ThreadPoolExecutor(max_workers=plan["workers"]) as pool:
        futures = [pool.submit(run_one, plan, i, s) for i, s in pending]
        for future in concurrent.futures.as_completed(futures):
            record = future.result()
            records[record["index"]] = record
            with open(plan["output_dir"] / "results.jsonl", "a", encoding="utf-8") as sink:
                sink.write(json.dumps(record, sort_keys=True) + "\n")
            print(f"run {record['index']:>4} seed {record['seed']}: {record['status']}"
                  + (f" ({'; '.join(record['problems'])})" if record["problems"] else ""), flush=True)
    ordered = [records[i] for i in sorted(records)]
    summary = summarize(ordered, time.monotonic() - started)
    (plan["output_dir"] / "summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True), encoding="utf-8")
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0 if summary["failed"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
