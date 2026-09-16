# Module: gamesnd — `code/gamesnd/`

## Purpose
The **sound and music content layer**. It owns the named, table-driven sound
entries the whole engine refers to by id, and the **event music** system that
picks a track based on what is happening in the mission. It sits on top of
`code/sound/`, which actually plays the audio.

## Key files
- `gamesnd.cpp` / `gamesnd.h` — `sounds.tbl` parsing, the two sound registries
  (gameplay and interface), and the lookup/play helpers.
- `eventmusic.cpp` / `eventmusic.h` — `music.tbl` parsing, soundtracks, and the
  state machine that switches patterns during a mission.

## Core data structures / globals
- `game_snd` (defined in `code/sound/sound.h`) — one named sound entry: one or
  more `game_snd_entry` files, min/max distance, pitch and volume ranges, and a
  `GameSoundCycleType` (`RandomCycle` or `SequentialCycle`) picking between them.
- `gamesnd_id` / `interface_snd_id` — type-safe handles (`util::ID`) into the
  gameplay and interface registries. They are **not interchangeable**, and each
  has its own lookup and play path.
- `enum class GameSounds` / `enum class InterfaceSounds` — the built-in retail
  sound ids that engine code refers to by name.
- `SOUNDTRACK_INFO` + `SCP_vector<SOUNDTRACK_INFO> Soundtracks` — a soundtrack
  is a set of patterns; `Current_soundtrack_num` selects one.
- `Mission_music[NUM_SCORES]` — the non-pattern scores (briefing, the three
  debriefing outcomes, fiction viewer) as indices into the spooled music list.
- `Event_music_enabled`, `Master_event_music_volume`, `Default_music_volume`.

## Major constants
- Music patterns (`eventmusic.h`): `SONG_NRML_1` … `SONG_NRML_3`,
  `SONG_AARV_1` / `SONG_AARV_2` (allied arrival), `SONG_EARV_1` /
  `SONG_EARV_2` (enemy arrival), `SONG_BTTL_1` … `SONG_BTTL_3`,
  `SONG_FAIL_1`, `SONG_VICT_1` / `SONG_VICT_2`, `SONG_DEAD_1`;
  `MAX_PATTERNS` (14).
- Scores: `NUM_SCORES` (5) — `SCORE_BRIEFING`, `SCORE_DEBRIEFING_SUCCESS`,
  `SCORE_DEBRIEFING_AVERAGE`, `SCORE_DEBRIEFING_FAILURE`,
  `SCORE_FICTION_VIEWER`.
- `BATTLE_START_MIN_TARGET_DIST` (500) — how close a hostile must be before
  battle music starts.
- Game-sound flags (`code/sound/sound.h`): `GAME_SND_USE_DS3D`,
  `GAME_SND_VOICE`, `GAME_SND_PRELOAD`, `GAME_SND_RETAIL_STYLE`,
  `GAME_SND_EXPLICITLY_EMPTY`.

## Using a sound
- Resolve a handle once — `gamesnd_get_by_name()` / `gamesnd_get_by_tbl_index()`
  for gameplay sounds, `gamesnd_get_by_iface_name()` /
  `gamesnd_get_by_iface_tbl_index()` for interface ones, or a
  `GameSounds::`/`InterfaceSounds::` constant — then play it through
  `code/sound/`'s `snd_play*`. Interface sounds have their own shortcut,
  `gamesnd_play_iface()`.
- `gamesnd_get_game_sound(handle)` / `gamesnd_get_interface_sound(handle)`
  return the `game_snd`; `gamesnd_choose_entry()` applies the cycle type.
- Loading is staged: `gamesnd_parse_soundstbl(first_stage)` runs twice, and
  gameplay vs. interface sounds are loaded and unloaded separately
  (`gamesnd_load_gameplay_sounds()`, `gamesnd_load_interface_sounds()`).

## Event music flow
`event_music_level_start()` picks the soundtrack; the engine then calls the
notification helpers — `event_music_battle_start()`,
`event_music_enemy_arrival()`, `event_music_friendly_arrival()`,
`event_music_primary_goals_met()`, `event_music_primary_goal_failed()`,
`event_music_player_death()` — and `event_music_do_frame()` cross-fades between
patterns. SEXPs change the soundtrack through `event_sexp_change_soundtrack()`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `sounds.tbl` (+ `*-snd.tbm`) | `gamesnd_parse_soundstbl()` (`gamesnd.cpp`) | Named gameplay and interface sound entries |
| `music.tbl` (+ `*-mus.tbm`) | `event_music_parse_musictbl()` (`eventmusic.cpp`) | Soundtracks, patterns, and menu music |

Table option reference: https://wiki.hard-light.net/index.php/Tables
(see *Sounds.tbl*, *Music.tbl*).

## See also
- `code/sound/` (playback, streaming, OpenAL — the layer below this one),
  `code/parse/sexp.*` (music and sound SEXPs),
  `code/scripting/api/libs/audio.cpp` (Lua bindings).
