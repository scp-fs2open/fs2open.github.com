# Module: globalincs — `code/globalincs/`

## Purpose
**Foundation headers** included almost everywhere. Defines the base types, the
project-wide container aliases, the type-safe flagset, intrusive linked lists,
global size limits, debug/assert macros, and shared global game-state variables.
By the include rules (`.clang-format`), `globalincs/*.h` always sort first.

## Key files
- `pstypes.h` — base types, `MAX_PLAYERS`, fixed-point `fix`, `Int3()`/`Assert()`,
  `UNINITIALIZED`, `FSO_DEBUG`, dir separators.
- `globals.h` — central `MAX_*` size limits shared across modules (see below).
- `vmallocator.h` — `SCP_vector`, `SCP_string`, `SCP_map`, `SCP_unordered_map`,
  etc. (the aliases you should use instead of raw `std::`).
- `flagset.h` — `flagset<Enum>` type-safe bitflag container (paired with the
  per-module `*_flags.h` files).
- `linklist.h` — intrusive doubly-linked list macros + `list_range()` iteration.
- `systemvars.h` / `systemvars.cpp` — global runtime state: `Game_mode`,
  `Missiontime`, `flFrametime`, `flRealframetime`, detail/skill levels.
- `alphacolors.*` — standard named UI colors (from `colors.tbl`).
- `version.*`, `safe_strings.*`, `scp_defines.h`, `utility.h`, `type_traits.h`.

## Major constants (the canonical limits)
- `MAX_OBJECTS` (5000), `MAX_SHIPS` (500), `MAX_SHIP_CLASSES` (500),
  `MAX_WEAPONS` (3000), `MAX_WEAPON_TYPES` (500).
- `MAX_WINGS` (75), `MAX_SHIPS_PER_WING` (6).
- `MAX_SHIP_PRIMARY_BANKS` (3), `MAX_SHIP_SECONDARY_BANKS` (4), `MAX_SHIP_WEAPONS` (7).
- `MAX_POLYGON_MODELS` (300), `MAX_MODEL_TEXTURES` (64).
- String lengths: `NAME_LENGTH` (32), `TOKEN_LENGTH` (32), `PATHNAME_LENGTH` (192),
  `MESSAGE_LENGTH` (512), `MULTITEXT_LENGTH` (4096).
- `MAX_PLAYERS` (12, in `pstypes.h`).

## Conventions
- Prefer the `SCP_*` aliases over raw `std::` types.
- Change a flag with the owning module's setter (e.g. `obj_set_flags`), not by
  poking the `flagset` directly, when side effects are involved.

## Configuration tables
`colors.tbl` (named UI colors) via `alphacolors.cpp`.

## See also
- Engine-wide tunables table lives in `code/mod_table/` (`game_settings.tbl`).
- Table option reference: https://wiki.hard-light.net/index.php/Tables
