# Module: anim — `code/anim/`

## Purpose
The **legacy `.ani` animation player**: FreeSpace 2's in-house animated-bitmap
format and the code that decodes and plays it. `.ani` files hold retail's
animated interface elements — talking heads, briefing animations, button
highlights — and are kept for retail content compatibility.

> **Not to be confused with `code/model/animation/`**, which is subobject and
> procedural *model* animation (turret rotation, gear deployment). This module
> is only about 2D animated bitmaps.

## Key files
- `packunpack.cpp` / `packunpack.h` — the `.ani` file format: the `anim`
  structure, the run-length codecs, and keyframe handling.
- `animplay.cpp` / `animplay.h` — playback: creating an instance of a loaded
  animation and drawing it each frame.

## Core data structures / globals
- `struct anim` — a loaded animation: `width`, `height`, `total_frames`,
  `packer_code`, keyframe table, 768-byte `palette`, plus `ref_count` (how many
  times it was loaded) and `instance_count` (how many are playing).
- `struct anim_instance` — one playing copy; several instances can share one
  `anim`.
- `anim_play_struct` — the playback request you fill in and pass to
  `anim_play_init()`: position, `start_at`/`stop_at`, `screen_id`,
  `framerate_independent`, `skip_frames`, `looped`, `ping_pong`, colour.
- `struct key_frame` — `frame_num` plus the `offset` into the data block.
- `Anim_paused`.

## Major constants
- Packing methods: `PACKING_METHOD_RLE` (Hoffoss's RLE),
  `PACKING_METHOD_RLE_KEY` (its keyframe variant), `PACKING_METHOD_STD_RLE`
  (high-bit count), `PACKING_METHOD_STD_RLE_KEY`; `PACKER_CODE` (0xEE) and
  `STD_RLE_CODE` (0x80).
- Animation flags: `ANF_MEM_MAPPED`, `ANF_STREAMED`, `ANF_XPARENT`,
  `ANF_ALL_KEYFRAMES` (required to play a file backwards).
- `ANI_STREAM_CACHE_SIZE` (4096) — the streaming read buffer.

## Playback flow
`anim_init()` / `anim_level_init()` / `anim_level_close()` bracket the system.
Fill an `anim_play_struct` via `anim_play_init()`, then `anim_render_all(screen_id,
frametime)` draws every instance registered for that screen; `anim_render_one()`
draws a single instance. Animations are grouped by `screen_id` so a screen can
draw exactly its own.

## Relationship to bmpman
`.ani` is also a `bmpman` type (`BM_TYPE_ANI`), so an animation can be loaded as
a numbered frame sequence through `bm_load_animation()` instead of played
through this module. New code should prefer the `bmpman` route — `EFF` and PNG
sequences are the modern equivalents; this module exists for the retail playback
paths that still use it.

## Configuration tables
None.

## See also
- `code/bmpman/` (`BM_TYPE_ANI`, `bm_load_animation`),
  `code/model/animation/` (the unrelated model-animation system),
  `code/menuui/` and `code/missionui/` (the retail screens that play `.ani`).
