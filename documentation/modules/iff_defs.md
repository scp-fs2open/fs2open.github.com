# Module: iff_defs — `code/iff_defs/`

## Purpose
**Teams and who shoots whom.** An IFF ("identification friend or foe") is a
team: Friendly, Hostile, Neutral, Traitor, and whatever else a mod defines. This
module owns the team list, the attack relationships between them, and the colours
each team is drawn in on the HUD and radar. Almost every "is this a target?"
question in the engine ends up here.

## Key files
- `iff_defs.cpp` / `iff_defs.h` — the whole module.

## Core data structures / globals
- `iff_info` + `SCP_vector<iff_info> Iff_info` — one team: its name, colour
  indices, flags, and the **attackee bitmasks** that say which other teams it
  attacks. There are two masks — the normal one and
  `attackee_bitmask_all_teams_at_war` — because a mission can put every team at
  war. The header marks `color_index` and both bitmasks as effectively private:
  go through the accessors below.
- `Iff_info_names` — the name list.
- `Iff_traitor` — the index of the traitor team, referenced all over the engine.
- `radar_iff_color[5][2][4]` — radar colours per team, per brightness.
- `IFF_COLOR_SELECTION`, `IFF_COLOR_MESSAGE`, `IFF_COLOR_TAGGED` — the
  non-team colour slots.

## Major constants
- IFF flags: `IFFF_SUPPORT_ALLOWED` (this team can call for support),
  `IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR`, `IFFF_ORDERS_HIDDEN` /
  `IFFF_ORDERS_SHOWN` (whether the HUD reveals a targeted ship's orders —
  friendly shows them by default), `IFFF_WING_NAME_HIDDEN`;
  `MAX_IFF_FLAGS` (5).

## The accessors to use
- Relationships: `iff_x_attacks_y(team_x, team_y)` — the one most callers want;
  `iff_get_attackee_mask(attacker_team)`, `iff_get_attacker_mask(attackee_team)`,
  `iff_get_mask(team)`, `iff_matches_mask(team, mask)`.
- Colours: `iff_get_color(color_index, is_bright)`,
  `iff_get_color_by_team(team, seen_from_team, is_bright)`, and
  `iff_get_color_by_team_and_object(...)`. Colour depends on **who is looking**,
  which is why `seen_from_team` is a parameter — do not cache the result across
  viewers.
- `iff_lookup(name)`, `iff_init()`, `iff_init_color()`,
  `iff_get_alpha_value(is_bright)`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `iff_defs.tbl` (+ `*-iff.tbm`) | `iff_init()` (`iff_defs.cpp`) | Team definitions, attack relationships, and colours |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/ship/` (`ship.team`), `code/ai/` (target selection asks
  `iff_x_attacks_y`), `code/radar/` (blip colours), `code/hud/`,
  `code/stats/` (the traitor rules use `Iff_traitor`),
  `code/species_defs/` (species is a separate axis from team).
