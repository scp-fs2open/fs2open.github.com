# Module: render — `code/render/`

## Purpose
The **legacy 3D pipeline and the 2D batcher**. Two things live here:

1. The `g3_*` API — the original software-era transform, projection, clipping,
   and immediate-mode drawing layer. Modern rendering goes through
   `model_draw_list` and the `gr_*` backend instead, but `g3_*` still owns the
   **view state** (`g3_set_view`, the `Eye_*` / `View_*` matrices) that the
   whole renderer reads, so it is not vestigial.
2. `batching.*` — the **sprite/effect batcher** that collects the frame's
   bitmaps, beams, lines, and polygons into as few draw calls as possible.
   Particles, weapon trails, shockwaves, fireballs, nebula poofs, and the
   starfield all go through it.

> Do not confuse this with `code/graphics/render.cpp`, which is a different
> file in a different module (`modules/graphics.md`).

## Key files
- `3d.h` — the public `g3_*` API and the view-state globals.
- `3dsetup.cpp` — frame setup, view matrices, and instance matrices.
- `3dmath.cpp` — point rotation, projection, and facing tests.
- `3dclipper.cpp` — polygon clipping against the view frustum.
- `3ddraw.cpp` — the immediate-mode draw calls.
- `3dlaser.cpp` — laser/bolt drawing.
- `3dinternal.h` — internals shared between the `3d*` files only.
- `batching.cpp` / `batching.h` — the batcher.

## Core data structures / globals
- View state, all in `3d.h`: `Eye_position`, `Eye_matrix`, `Eye_fov` (the
  viewer in world coordinates); `View_position`, `View_matrix` (world to
  screen); `Light_matrix`, `Light_base` (world into local for lighting);
  `Object_position`, `Object_matrix` (the current instance);
  `Proj_fov`.
- `batch_vertex`, `batch_info` (with its `material_type`), `batch_buffer_key`,
  `primitive_batch`, `primitive_batch_item`, `primitive_batch_buffer` — the
  batcher's internals; a batch is keyed by texture plus material so everything
  sharing both can be drawn at once.

## Major constants
- Vertex projection flags: `PF_PROJECTED` (screen coordinates are valid),
  `PF_OVERFLOW` (cannot be projected), `PF_TEMP_POINT` (created while clipping).
- Clip codes: `CC_OFF_LEFT`, `CC_OFF_RIGHT`, `CC_OFF_BOT`, `CC_OFF_TOP`,
  `CC_OFF_USER`, `CC_BEHIND`, and the `CC_OFF` mask.

## Using it
- A frame is bracketed by `g3_start_frame(zbuffer_flag)` and `g3_end_frame()`.
  Both macros record `__FILE__`/`__LINE__`, and `g3_in_frame()` reports whether
  you are inside one — mismatched pairs are a common bug, so check it rather
  than assuming.
- Set the view with `g3_set_view(camera*)` or `g3_set_view_matrix()`, and the
  field of view with `g3_set_fov()`.
- Draw something in an object's local space between
  `g3_start_instance_matrix(pos, orient)` (or the `matrix4` /
  `g3_start_instance_angles` variants) and `g3_done_instance()`.
- Add to the batcher with the `batching_add_*` calls: `batching_add_bitmap()`,
  `batching_add_beam()`, `batching_add_line()`, `batching_add_polygon()`, and
  the volume and distortion variants.

## Configuration tables
None.

## See also
- `code/graphics/` (the `gr_*` backend this feeds; `modules/graphics.md`),
  `code/model/modelrender.*` (`model_draw_list`, the modern path),
  `code/particle/` and `code/weapon/beam.*` (the batcher's biggest users),
  `code/camera/` (what `g3_set_view` consumes).
