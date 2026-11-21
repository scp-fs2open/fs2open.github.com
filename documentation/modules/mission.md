# Module: mission — `code/mission/`

## Purpose
Loads and runs **missions and campaigns**. Parses `.fs2` mission files into spawn
data, evaluates arrivals/departures and goals each frame, and owns mission-time
features: objectives, in-mission messages/personas, training directives,
briefings/debriefings, the mission log, and campaign progression.

## Key files
- `missionparse.cpp` / `missionparse.h` — `.fs2` parsing, `p_object`,
  `Parse_objects`, arrival/departure evaluation, ship/wing spawning.
- `missiongoals.*` — objectives/goals (`mission_eval_goals()`).
- `missionmessage.*` — in-mission messages, personas, voice.
- `missiontraining.*` — training directives/objectives.
- `missionbriefcommon.*` — briefing/debriefing data and icons.
- `missioncampaign.*` — campaign state and progression.
- `missionload.*`, `missionlog.*`, `missionhotkey.*`, `missiongrid.*`,
  `mission_flags.h`.

## Core data structures / globals
- `SCP_vector<p_object> Parse_objects` — parsed-but-not-yet-spawned ship entries.
- Mission globals such as `The_mission`, plus time globals `Missiontime`
  (in `code/globalincs/systemvars.*`).

## Major constants
- `MISSION_DESC_LENGTH` (512), message/training lengths in `code/globalincs/globals.h`.
- Mission/parse flags in `mission_flags.h`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `messages.tbl` | `parse_msgtbl()` / `parse_custom_message_table()` | Built-in & custom messages |
| `icons.tbl` | `brief_parse_icon_tbl()` | Briefing icons |

Mission content itself comes from `.fs2` files (not `.tbl`).

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/parse/` (parsing + SEXPs that drive mission logic), `code/ship/` (spawned entities),
  `code/missionui/` & `code/menuui/` (briefing/debrief screens).
