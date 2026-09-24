# Module: lighting — `code/lighting/`

## Purpose
Two related jobs. First, the **dynamic light list**: every frame, code that
wants to light the scene (suns, explosions, weapon glows, thrusters) adds a
light here, and the renderer consumes the collected list. Second,
**lighting profiles**: the table-driven tone-mapping and exposure settings that
control how the HDR scene is mapped to the display.

## Key files
- `lighting.cpp` / `lighting.h` — the light list, the `light` struct, the
  `light_add_*` API, and `scene_lights` (the per-draw-call light selection).
- `lighting_profiles.cpp` / `lighting_profiles.h` — `lighting_profiles.tbl`
  parsing, the tone-mapper selection, and exposure.

## Core data structures / globals
- `struct light` — one light: `Light_Type type`, `vec` / `vec2` (position or
  direction; `vec2` is the tube's second point or the cone's direction) with
  their view-space `local_vec` counterparts, `intensity`, the `rada`/`radb`
  falloff radii, `r`/`g`/`b`, cone angles, `source_radius`, `flags`, and
  `sun_index` when it corresponds to a background sun.
- `SCP_vector<light> Static_light` — lights that do not change per frame
  (the mission's suns).
- `class scene_lights` — collects the frame's lights and, per object, picks the
  relevant subset: `addLight()`, `setLightFilter(pos, rad)`,
  `setLights(light_indexing_info*)`, `resetLightState()`. `light_indexing_info`
  is the per-draw slice handed to the shader.
- `lighting_mode Lighting_mode` — `NORMAL` or `COCKPIT`; the cockpit is lit
  separately so its lighting does not fight the world's.
- `lighting_profiles::profile` — one parsed profile: `exposure`, the tone-mapper
  choice, and its parameters.
- `piecewise_power_curve_values` / `piecewise_power_curve_intermediates` — the
  toe/shoulder parameters of the piecewise power-curve tone mapper, and the
  precomputed form used at runtime.

## API
- Per frame: `light_reset()`, then `light_add_directional()`,
  `light_add_point()`, `light_add_tube()`, `light_add_cone()` — each has an
  `hdr_color` overload and a loose `intensity, r, g, b` overload.
- `light_rotate_all()` moves the list into view space;
  `light_apply_rgb()` is the legacy per-vertex path.
- Background suns: `light_get_global_count()`, `light_get_global_dir()`.
- Profiles: `load_profiles()`, `current_exposure()`,
  `name_to_tonemapper(name)`. The `lab_set_*` helpers exist so the Lab can
  override exposure and tone mapping live without editing the table.

## Major constants
- `enum class Light_Type` — `Directional`, `Point`, `Tube`, `Cone`, `Ambient`.
  The older `LT_DIRECTIONAL`/`LT_POINT`/`LT_TUBE`/`LT_CONE` defines carry the
  same values 0-3; `Ambient` (4) has no legacy define. New code should use the
  enum class.
- Light flags: `LF_DUAL_CONE`, `LF_NO_GLARE` (a sun with `$NoGlare`),
  `LF_NO_RT_SHADOW` (never a raytraced-shadow candidate), `LF_DEFAULT`.
- `TonemapperAlgorithm` — the selectable tone-mapping curves.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `lighting_profiles.tbl` (+ `*-ltp.tbm`) | `lighting_profiles::load_profiles()` (`lighting_profiles.cpp`) | Exposure and tone-mapper presets |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/graphics/` (consumes the light list; deferred lighting, shadows, and
  post-processing all read from here), `code/starfield/` (background suns),
  `code/model/modelrender.*` (per-object light selection via `scene_lights`),
  `code/lab/` (the live lighting/tone-mapper controls).
