# Module: species_defs — `code/species_defs/`

## Purpose
**Species**: the faction a ship class *belongs to* — Terran, Vasudan, Shivan, or
whatever a mod defines. Species is what supplies the per-faction look and feel:
thruster animations, debris models, briefing icons, flyby sounds, the default
countermeasure and support ship. It is a property of a ship **class**, and is
distinct from IFF/team, which is a property of a ship **instance** in a mission.

## Key files
- `species_defs.cpp` / `species_defs.h` — the whole module.

## Core data structures / globals
- `class species_info` + `SCP_vector<species_info> Species_info` — one species:
  - `default_iff` — the team a ship of this species gets when nothing else says
    otherwise.
  - the species colour.
  - `thrust_info` / `thrust_pair` / `thrust_pair_bitmap` — the normal and
    afterburner thruster animations, each with a bitmap and glow pair.
  - `warpin_params_index` / `warpout_params_index` — the warp effect used.
  - `bii_indices[MIN_BRIEF_ICONS]` — briefing icons, with
    `borrows_bii_index_species` so a species can reuse another's icons rather
    than duplicating them. `borrows_flyby_sounds_species` does the same for
    flyby sounds.
  - `cmeasure_index`, `support_ship_index` — the default countermeasure and
    support ship class.
- `species_info_lookup(name)`, `species_init()`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `species_defs.tbl` (+ `*-sdf.tbm`) | `species_init()` (`species_defs.cpp`) | Species definitions |

This module also references `asteroid.tbl` entries, because the debris types a
species produces are asteroid/debris classes.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/ship/` (`ship_info.species`), `code/iff_defs/` (team, the other axis —
  a Vasudan ship can be on the hostile team), `code/debris/` and
  `code/asteroid/` (species debris), `code/fireball/` (warp effects),
  `code/mission/missionbriefcommon.*` (briefing icons),
  `code/cmeasure/` (the default countermeasure).
