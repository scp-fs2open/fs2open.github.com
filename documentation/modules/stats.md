# Module: stats — `code/stats/`

## Purpose
**Scoring, ranks, medals, and the traitor rules.** It tracks what the player did
in a mission (kills, assists, shots, score), decides at debriefing whether that
earns a promotion, a medal, or a badge, folds the mission totals into the
all-time totals stored in the pilot, and owns the rules that decide when a
player is declared a traitor.

## Key files
- `scoring.cpp` / `scoring.h` — `scoring_struct` (the whole score record), the
  rank table, the traitor rules, and the accept/back-out logic.
- `medals.cpp` / `medals.h` — the medal table and the medals display screen.
- `stats.cpp` / `stats.h` — drawing statistics on the debriefing and barracks
  screens.

## Core data structures / globals
- `class scoring_struct` — the score record carried inside each `player`. It has
  **two halves**: the all-time fields (`score`, `rank`, `assists`,
  `kill_count`, `kill_count_ok`, `bonehead_kills`) and the current-mission
  fields, all prefixed `m_` (`m_score`, `m_kills`, `m_okKills`,
  `m_kill_count_ok`, `m_assists`, `m_bonehead_kills`, `m_player_deaths`,
  `m_medal_earned`, `m_badge_earned`, `m_promotion_earned`,
  `m_dogfight_kills[MAX_PLAYERS]`). The per-ship-class kill vectors are sized to
  `ship_info_size()`.
  "ok" kills are the ones that count; `bonehead` kills are friendly fire.
- `rank_stuff` + `SCP_vector<rank_stuff> Ranks` — each rank and the `points`
  needed to reach it.
- `medal_stuff` + `SCP_vector<medal_stuff> Medals` — medals, with
  `num_versions` (how many times it can be awarded), `kills_needed` for badges,
  and `mask_index` for the display screen. `Rank_medal_index` is the medal slot
  that shows the current rank.
- `traitor_stuff Traitor` + `SCP_vector<traitor_override_t> Traitor_overrides`
  — the traitor thresholds and per-mission overrides.

## Major constants
- `NUM_MEDALS_FS2` (18), `NUM_MEDALS_FS1` (16).
- Stats flags: `STATS_FLAG_INVALID`, `STATS_FLAG_CAMPAIGN`,
  `STATS_FLAG_MULTIPLAYER`.
- Medal-screen modes: `MM_NORMAL` (run through the state machine) and
  `MM_POPUP` (called from inside another loop, so it must not use the
  `gameseq_*` functions).

## The accept / back-out pattern
Mission stats are **provisional** until the player accepts the debriefing:
- `scoring_level_init()` clears the `m_` fields at mission start.
- `scoring_level_close(accepted)` finalizes them.
- `scoring_do_accept(score)` folds the mission into the all-time totals.
- `scoring_backout_accept(score)` undoes that — used when a mission is replayed
  or a campaign is rewound, so replaying does not inflate a pilot's totals.

Match this pattern when adding a new statistic, or replays will corrupt it.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `rank.tbl` (+ `*-rnk.tbm`) | `parse_rank_table()` / `rank_init()` (`scoring.cpp`) | Rank names and point thresholds |
| `medals.tbl` (+ `*-mdl.tbm`) | `parse_medals_table()` / `medals_init()` (`medals.cpp`) | Medals and badges |
| `traitor.tbl` (+ `*-trtr.tbm`) | `parse_traitor_tbl()` / `traitor_init()` (`scoring.cpp`) | Traitor thresholds and messages |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/playerman/` and `code/pilotfile/` (where `scoring_struct` is stored and
  saved — note `pilotfile::update_stats()` / `update_stats_backout()` mirror the
  accept pattern), `code/missionui/missiondebrief.*` (the screen that shows
  this), `code/menuui/barracks.cpp` (all-time stats and the medal case),
  `code/mission/missiongoals.*` (what scores in the first place).
