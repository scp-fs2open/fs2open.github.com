# Module: camera — `code/camera/`

## Purpose
Owns the **viewpoint**: named camera objects that can be attached to an object
or submodel, moved and rotated over time with acceleration/deceleration curves,
and given a field of view. Also owns the warp-effect camera, on-screen
**subtitles**, and **photo mode** (the free-look screenshot tool).

## Key files
- `camera.cpp` / `camera.h` — the `camera` class, the camera registry, the
  `warp_camera`, and the `subtitle` class.
- `photomode.cpp` / `photomode.h` — photo mode: freeze the scene, fly the camera
  freely, cycle post-processing filters, queue a screenshot.

## Core data structures / globals
- `class camera` — one camera: name, signature, flags, an optional host object
  (+ submodel) it rides on, an optional target object it looks at, and optional
  custom position/orientation callbacks. Position, rotation, and FOV are all
  set with a duration plus acceleration/deceleration times, so
  `camera::do_frame()` interpolates them.
- `camid` — the handle you get back from `cam_create()`; look up a camera with
  `cam_lookup(name)`, `cam_get_camera(index)`, or `cam_get_current()`.
- `fov_t` — `std::variant<float, asymmetric_fov>`; a camera's FOV is either a
  single angle or an asymmetric (off-axis) frustum.
- `warp_camera Warp_camera` — the dedicated, damped camera used by the warp-out
  effect.
- `SCP_vector<subtitle> Subtitles` — active on-screen text/image overlays
  (driven by SEXPs and Lua).
- `VIEWER_ZOOM_DEFAULT`, `COCKPIT_ZOOM_DEFAULT`, `Sexp_fov`.

## Major constants
- `CAM_STATIONARY_FOV`, `CAM_STATIONARY_ORI`, `CAM_STATIONARY_POS`,
  `CAM_DEFAULT_FLAGS` — pin an aspect of the camera so a host object's motion
  does not drag it.
- `DEFAULT_FOV` (0.75f).
- `EXTERN_CAM_BBOX_CONSTANT_PADDING` (5.0f),
  `EXTERN_CAM_BBOX_MULTIPLIER_PADDING` (1.5f) — external-view framing padding.

## Frame integration
- `cam_init()` / `cam_close()` at level boundaries.
- `cam_do_frame(flFrametime)` is the **first** thing `game_simulation_frame()`
  does, so everything simulated afterwards sees the new viewpoint. Photo mode
  runs right after it, on `flRealframetime`, so it keeps working while time is
  compressed or stopped.

## Configuration tables
None. Cameras are created at runtime by SEXPs, Lua, and cutscene code.

## See also
- `code/graphics/` (the view matrix built from the camera),
  `code/parse/sexp.*` (camera and subtitle SEXPs),
  `code/scripting/api/objs/` (the Lua camera bindings),
  `code/cutscene/` (in-engine cutscenes that drive cameras).
