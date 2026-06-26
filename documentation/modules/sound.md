# Module: sound — `code/sound/`

## Purpose
**Audio**: 2D/3D sound effects, streaming audio (music, voice, movies), the sound
abstraction over OpenAL, FFmpeg-based decoding, text-to-speech, and (Windows)
voice recognition. Note: event music is owned by `code/gamesnd/`, which defines the
sound *entries* this module plays.

## Key files
- `sound.cpp` / `sound.h` — high-level API: load, play, 3D positioning, priorities.
- `ds.cpp` / `ds.h`, `ds3d.*` — low-level device/buffer layer (the "DirectSound"-era API).
- `openal.cpp` / `openal.h` — OpenAL backend.
- `audiostr.*` — streaming audio (music/voice). `ffmpeg/` — decoding.
- `fsspeech.*`, `speech_*` (win/linux/mac) — text-to-speech.
- `voicerec.*`, `rtvoice.*` — voice recognition / real-time voice (multiplayer).

## Core data structures / globals
- `game_snd` (defined in `code/gamesnd/gamesnd.h`) — a named, table-driven sound entry.
- Sound handles returned by `snd_play*` calls.

## Major constants
- Game-sound flags: `GAME_SND_USE_DS3D`, `GAME_SND_VOICE`, `GAME_SND_PRELOAD`,
  `GAME_SND_RETAIL_STYLE`, `GAME_SND_EXPLICITLY_EMPTY`.
- Priorities: `SND_PRIORITY_MUST_PLAY`, `..._SINGLE/DOUBLE/TRIPLE_INSTANCE`.
- Volume buses: `AAV_MUSIC`, `AAV_VOICE`, `AAV_EFFECTS`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `sounds.tbl` | `parse_sound_table()` (`code/gamesnd/gamesnd.cpp`) | Named sound effect entries |
| `music.tbl` | `event_music_parse_musictbl()` (`code/gamesnd/eventmusic.cpp`) | Event/menu music |

Table option reference: https://wiki.hard-light.net/index.php/Tables (see *Sounds.tbl*, *Music.tbl*).

## See also
- `code/gamesnd/` (sound/music table entries), `code/sound/ffmpeg/`, `code/cutscene/`.
