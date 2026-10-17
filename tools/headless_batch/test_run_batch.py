"""Tests for run_batch.py. They need no FreeSpace Open install: a stub "engine" (a Python script run by the
current interpreter) stands in for the executable and writes the -sim-result sidecar the real engine writes.

    python -m unittest tools/headless_batch/test_run_batch.py
"""

import functools
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_batch  # noqa: E402

STUB_OK = r"""
import json, sys
args = sys.argv[1:]
path = args[args.index("-sim-result") + 1]
seed = int(args[args.index("-seed") + 1])
json.dump({"schema": 1, "seed": seed, "simulated_seconds": 12.5, "termination": "time_limit"}, open(path, "w"))
"""
STUB_START_FAILED = STUB_OK.replace('"time_limit"', '"start_failed"')
STUB_PLAYER_DIED = STUB_OK.replace('"time_limit"', '"player_died"')
STUB_SLEEP = "import time; time.sleep(60)"
STUB_CRASH = "import sys; sys.exit(3)"


class Fixture(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.base = self.root / "base"
        (self.base / "data" / "players").mkdir(parents=True)
        (self.base / "root_fs2.vp").write_bytes(b"vp")
        (self.base / "data" / "players" / "Tester.json").write_text("{}")
        (self.base / "data" / "debug.log").write_text("noise")
        (self.base / "mymod" / "data").mkdir(parents=True)
        (self.base / "mymod" / "data" / "mod.vp").write_bytes(b"vp")
        (self.base / "mymod" / "data" / "table.tbm").write_text("x")
        self.engine = Path(sys.executable)

    def tearDown(self):
        self._tmp.cleanup()

    def plan_file(self, **overrides):
        plan = {"engine": str(self.engine), "base_workspace": str(self.base), "output_dir": str(self.root / "out"),
                "mission": "test-mission", "pilot": "Tester", "max_sim_time": 20, "mods": ["mymod"],
                "seed_start": 500, "repeats": 3, "workers": 2, "base_port": 40000}
        plan.update(overrides)
        path = self.root / "plan.json"
        path.write_text(json.dumps(plan))
        return path

    def stub(self, source):
        path = self.root / "stub.py"
        path.write_text(source)
        return (str(path),)


class PlanTests(Fixture):
    def test_defaults_and_seed_expansion(self):
        plan = run_batch.load_plan(self.plan_file())
        self.assertEqual(plan["seeds"], [500, 501, 502])
        self.assertAlmostEqual(plan["fixed_dt"], 1 / 30)
        self.assertEqual(plan["workers"], 2)

    def test_explicit_seeds_win_over_seed_start(self):
        self.assertEqual(run_batch.load_plan(self.plan_file(seeds=[7, 9]))["seeds"], [7, 9])

    def test_rejects_bad_plans_with_a_message_that_names_the_problem(self):
        cases = {
            "unknown plan keys": {"fixd_dt": 0.1},
            "fixed_dt must be at least": {"fixed_dt": 0},
            "fixed_dt must be at least 1.52587890625e-05 and at most 0.25": {"fixed_dt": 0.5},
            "max_sim_time": {"max_sim_time": -1},
            "max_sim_time must be above 0.0 and at most 32767": {"max_sim_time": 40000},
            "seeds must be": {"seeds": [0]},
            "simple name": {"mission": "../escape"},
            "mod directory not found": {"mods": ["nope"]},
            "no stored pilot": {"pilot": "Ghost"},
            "port range": {"base_port": 65000, "repeats": 1000},
            "engine must be a path string": {"engine": 5},
            "base_workspace must be a path string": {"base_workspace": ["x"]},
            "output_dir must be a path string": {"output_dir": "a\0b"},
        }
        for fragment, overrides in cases.items():
            with self.subTest(fragment):
                with self.assertRaises(run_batch.PlanError) as ctx:
                    run_batch.load_plan(self.plan_file(**overrides))
                self.assertIn(fragment, str(ctx.exception))

    def test_missing_required_keys_are_listed(self):
        path = self.root / "plan.json"
        path.write_text(json.dumps({"engine": str(self.engine)}))
        with self.assertRaises(run_batch.PlanError) as ctx:
            run_batch.load_plan(path)
        self.assertIn("base_workspace", str(ctx.exception))
        self.assertIn("pilot", str(ctx.exception))

    def test_base_workspace_without_vp_files_is_rejected(self):
        (self.base / "root_fs2.vp").unlink()
        with self.assertRaises(run_batch.PlanError):
            run_batch.load_plan(self.plan_file())

    def test_output_dir_on_another_volume_is_rejected(self):
        original = run_batch._volume_of
        try:
            run_batch._volume_of = lambda path: 1 if path == self.base.resolve() else 2
            with self.assertRaises(run_batch.PlanError) as ctx:
                run_batch.load_plan(self.plan_file())
            self.assertIn("same volume", str(ctx.exception))
        finally:
            run_batch._volume_of = original

    @unittest.skipUnless(sys.platform == "win32", "the path limit is a Windows limit")
    def test_a_too_long_output_dir_is_rejected_with_the_allowed_length(self):
        with self.assertRaises(run_batch.PlanError) as ctx:
            run_batch.check_path_length(self.base, ["mymod"], self.root / "out", limit=60)
        self.assertIn("too long", str(ctx.exception))
        self.assertIn("at most", str(ctx.exception))
        run_batch.check_path_length(self.base, ["mymod"], self.root / "out", limit=10000)  # roomy limit: no error


class RunTests(Fixture):
    def test_command_has_the_headless_flags_and_the_per_run_values(self):
        plan = run_batch.load_plan(self.plan_file(extra_args=["-nosound"]))
        command = run_batch.build_command(plan, 501, 40001, Path("r.json"))
        self.assertEqual(command[0], str(self.engine))
        for flag in ("-headless", "-noninteractive", "-portable_mode", "-nomovies", "-nosound"):
            self.assertIn(flag, command)
        self.assertEqual(command[command.index("-seed") + 1], "501")
        self.assertEqual(command[command.index("-port") + 1], "40001")
        self.assertEqual(command[command.index("-mod") + 1], "mymod")
        self.assertEqual(command[command.index("-max-sim-time") + 1], "20.0")

    def test_workspace_hardlinks_packs_copies_the_rest_and_skips_logs(self):
        destination = self.root / "w"
        run_batch.prepare_workspace(self.base, destination, ["mymod"])
        self.assertEqual((destination / "root_fs2.vp").stat().st_ino, (self.base / "root_fs2.vp").stat().st_ino)
        self.assertEqual((destination / "mymod" / "data" / "mod.vp").stat().st_ino,
                         (self.base / "mymod" / "data" / "mod.vp").stat().st_ino)
        (destination / "mymod" / "data" / "table.tbm").write_text("changed")
        self.assertEqual((self.base / "mymod" / "data" / "table.tbm").read_text(), "x")  # a run cannot write through
        self.assertFalse((destination / "data" / "debug.log").exists())
        self.assertTrue((destination / "data" / "players" / "Tester.json").is_file())

    def test_successful_run_collects_the_sidecar(self):
        plan = run_batch.load_plan(self.plan_file())
        record = run_batch.run_one(plan, 1, 501, launcher=self.stub(STUB_OK))
        self.assertEqual(record["status"], "ok", record["problems"])
        self.assertEqual(record["sim_result"]["seed"], 501)
        self.assertEqual(record["port"], 40001)
        self.assertTrue((plan["output_dir"] / "runs" / "r0001" / "result.json").is_file())

    def test_crash_and_missing_sidecar_are_failures_not_silent(self):
        plan = run_batch.load_plan(self.plan_file())
        record = run_batch.run_one(plan, 0, 500, launcher=self.stub(STUB_CRASH))
        self.assertEqual(record["status"], "failed")
        self.assertEqual(record["exit_code"], 3)
        self.assertTrue(any("no -sim-result" in p for p in record["problems"]))

    def test_relative_output_dir_still_finds_the_engine_result(self):
        # The engine runs in its run folder, so a relative -sim-result path would be written there instead.
        old_cwd = os.getcwd()
        os.chdir(self.root)
        try:
            plan = run_batch.load_plan(self.plan_file(output_dir="out"))
            record = run_batch.run_one(plan, 0, 500, launcher=self.stub(STUB_OK))
        finally:
            os.chdir(old_cwd)
        self.assertEqual(record["status"], "ok", record["problems"])

    def test_a_mission_that_never_started_is_a_failure(self):
        plan = run_batch.load_plan(self.plan_file())
        record = run_batch.run_one(plan, 0, 500, launcher=self.stub(STUB_START_FAILED))
        self.assertEqual(record["status"], "failed")
        self.assertTrue(any("start_failed" in p for p in record["problems"]))

    def test_a_player_death_is_an_outcome_not_a_failure(self):
        plan = run_batch.load_plan(self.plan_file())
        record = run_batch.run_one(plan, 0, 500, launcher=self.stub(STUB_PLAYER_DIED))
        self.assertEqual(record["status"], "ok", record["problems"])

    def test_resume_reruns_interrupted_unreadable_and_reseeded_runs_and_keeps_good_ones(self):
        path = self.plan_file(seeds=[7, 8, 9, 10], repeats=None, seed_start=None)
        runs = self.root / "out" / "runs"
        (runs / "r0000" / "w").mkdir(parents=True)  # interrupted before it wrote result.json
        (runs / "r0001").mkdir()
        (runs / "r0001" / "result.json").write_text('{"status": "ok", "se')  # interrupted mid-write
        (runs / "r0002").mkdir()
        (runs / "r0002" / "result.json").write_text(json.dumps({"index": 2, "seed": 99, "status": "ok"}))  # old plan
        (runs / "r0003").mkdir()
        kept = {"index": 3, "seed": 10, "status": "ok", "exit_code": 0, "timed_out": False,
                "sim_result": {"simulated_seconds": 1.0, "termination": "time_limit"}, "problems": []}
        (runs / "r0003" / "result.json").write_text(json.dumps(kept))
        original = run_batch.run_one
        run_batch.run_one = functools.partial(original, launcher=self.stub(STUB_OK))
        try:
            self.assertEqual(run_batch.main([str(path), "--resume"]), 0)
        finally:
            run_batch.run_one = original
        for index, seed in enumerate([7, 8, 9]):
            record = json.loads((runs / f"r{index:04d}" / "result.json").read_text())
            self.assertEqual((record["status"], record["seed"]), ("ok", seed))
        self.assertEqual(json.loads((runs / "r0003" / "result.json").read_text()), kept)  # not rerun

    def test_timeout_kills_the_process(self):
        plan = run_batch.load_plan(self.plan_file(timeout_seconds=1))
        record = run_batch.run_one(plan, 0, 500, launcher=self.stub(STUB_SLEEP))
        self.assertTrue(record["timed_out"])
        self.assertEqual(record["status"], "failed")
        self.assertLess(record["wall_seconds"], 30)

    def test_summary_counts_and_rate(self):
        ok = {"status": "ok", "exit_code": 0, "timed_out": False, "sim_result": {"simulated_seconds": 10.0, "termination": "time_limit"}}
        bad = {"status": "failed", "exit_code": 3, "timed_out": False, "sim_result": None}
        summary = run_batch.summarize([ok, ok, bad], batch_wall=5.0)
        self.assertEqual((summary["runs"], summary["ok"], summary["failed"]), (3, 2, 1))
        self.assertEqual(summary["simulated_seconds_per_wall_second"], 4.0)
        self.assertEqual(summary["terminations"], {"time_limit": 2})


if __name__ == "__main__":
    unittest.main()
