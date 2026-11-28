# Investigation: integrate CORTAO screen-space AO

Source: [theagentd/CORTAO](https://github.com/theagentd/CORTAO), reference implementation v1.0.3,
MIT license. Analysis date: 2026-11-16, on master `51041cfa81`. No code was changed.

## Status (2026-11-16)

Phase 0 code is on the branch `feat/cortao-ssao` (from master `51041cfa81`). It is not committed.

- The prepass and the main pass are fragment passes in `VulkanPostProcessingCortao.cpp`. They
  run before the deferred light pass. The ambient light reads the result in the `SSAO` variant of
  `deferred-f.sdr`.
- The prepass does not use `CORTAOPrepass.h`. It is one draw for each mip level, and it follows
  the mip selection rule of the reference prepass back to mip 0. The packed data is `R32_SFLOAT`
  that holds the uint bits.
- The depth comes from the G-buffer position target (design decision 1). `flipY` is true, because
  the G-buffer pass uses a negative viewport height.
- The setting is the "Screen-Space Ambient Occlusion" option (Off/On, Vulkan only). The F3 lab has
  a checkbox and sliders for the screen radius, the max radius, the intensity and animated noise.
- The `code` target builds with `-Werror`, with and without Vulkan. The three changed shaders
  compile with `glslc`. There was no runtime test yet.

## Status (2026-11-17)

Phase 0 is commit `3ada3c4504`. Phase 1 is in the working tree of `feat/cortao-ssao`, not committed.

- Filter pass: `cortao-filter-f.sdr` with `CORTAOFilterPass.h` (not changed). Each fragment runs
  the filter for its 2x2 block and keeps its own pixel. The gather callbacks use `texelFetch`,
  because a gather with offsets that are not constant needs `shaderImageGatherExtended`.
- Quality levels: the option is Off, Low, Medium and High (4, 8 and 16 samples). The main pass has
  the `SAMPLES_4` and `SAMPLES_8` variants.
- Filter and noise modes: Auto selects the 5-tap filter and animated noise when FSR runs, and the
  13-tap filter and constant noise when it does not. The lab can select each mode.
- Lighting profile: `$SSAO Max Radius:`, `$SSAO Screen Radius:` and `$SSAO Intensity:`. The lab
  sliders override them for the session.
- Not done: Release-build timing. It needs a runtime test.

Phase 1 is commit `6a57fe83bf`. Phase 2 (OpenGL) is in the working tree, not committed.

- `gropenglcortao.cpp` runs the same three fragment shaders on OpenGL, at the start of
  `gr_opengl_deferred_lighting_finish()`. The deferred light shader reads the result on texture
  unit 12.
- The uniform structs and their fill functions are now in `graphics/cortao_params.cpp`, shared by
  both backends. OpenGL uses `flipY = false`.
- New capability `CAPABILITY_SCREEN_SPACE_AO`: always true on Vulkan, true on OpenGL with GLSL 4.20
  or higher (the filter header swizzles scalars), false on GLES.
- The shaders enable `GL_EXT_control_flow_attributes` when the macro is defined, because the
  CORTAO headers use `[[unroll]]` when the macro is defined.

Phase 2 is commit `c7ae172e7a`. The user skipped Phase 3 and chose Phase 4 (bent normals). Phase 4
is in the working tree, not committed.

- The CORTAO targets are now RGBA8: RGB is the view-space bent normal as `n * 0.5 + 0.5`, A is the
  occlusion. The main pass and the filter pass have `BENT_NORMAL` variants. The filter uses
  `CORTAO_FILTER_BENT_NORMAL`.
- No bent normal lookup table: CORTAO sums the ray directions in the shader. When all rays are
  occluded, that sum is NaN, so the main pass writes the surface normal.
- The deferred light pass (`SSAO_BENT_NORMAL` variant) uses the normalized bent normal only for the
  irradiance map lookup in `ComputeEnvLight`. The specular environment light keeps the normal.
- Bent normals are active only when an environment map is present (`ssao_bent_normals_active()`).
  The lab has a checkbox to turn them off for a comparison.

Phase 4 is commit `b66d175e36`. After it, the user asked to remove RTAO. In the working tree, not
committed:

- `graphics/rtao.h/.cpp`, the "Raytraced Ambient Occlusion" option, the `RTAO` variant of
  `deferred-f.sdr` and `traceAmbientOcclusion()` in `shadows.sdr` are removed.
- `deferred_global_data` loses `rtaoSampleCount` (now `padding0`), `rtaoRadius` and `rtaoStrength`.
  The struct is now 104 bytes. The shader block has the same change.
- `$RTAO Radius:` and `$RTAO Strength:` are no longer parsed. A table that has them gets the
  "Unhandled line in lighting profile" warning.
- The ray-tracing TLAS is now built only for raytraced shadows. The SSAO deferred flags are now
  1 << 2, 1 << 3 and 1 << 4.

## Summary

- CORTAO is a screen-space ambient occlusion (SSAO) technique. It reads a depth buffer and a normal
  buffer, and it writes occlusion and an optional bent normal.
- The engine has all the inputs. The G-buffer holds view-space positions and view-space normals in
  16-bit float targets.
- The engine has one AO hook already: `ao *= ...` in the `LT_AMBIENT` branch of `deferred-f.sdr`.
  RTAO uses this hook. CORTAO can use the same hook.
- CORTAO can be the AO for hardware that cannot use RTAO: all of OpenGL, and Vulkan without ray
  query.
- The two large risks are (1) the engine has no compute shader path, and (2) the engine has no
  temporal filter, except FSR on Vulkan. Item 2 decides how good CORTAO looks for most users.
- Recommendation: do a fragment-shader port first. Keep the four CORTAO core headers without
  changes. Write engine-specific callbacks around them.

## What CORTAO is

CORTAO has three compute passes:

1. Prepass. It reads depth and normal, and writes an `R32_UINT` "packed data" buffer with up to
   8 mip levels. Each texel holds the linear depth as float bits. The low 10 bits hold a
   sphere-map normal. The mip chain uses point samples. It does not use shared memory or barriers.
2. Main pass. It samples the packed mips, makes circular occluders, and traces 8 to 32 rays
   against them with a 3D occluder lookup table (LUT). It writes occlusion, and optionally a bent
   normal and the ray bits. Each pixel is independent. It does not use shared memory.
3. Filter pass (optional). A 5-tap or 13-tap sparse bilateral blur. Each thread writes a 2x2 pixel
   block with `textureGatherOffset`. The author designed it to feed a temporal filter, not to
   remove all noise.

The code structure is good for integration:

- The core is in `CORTAOBase.h`, `CORTAOPrepass.h`, `CORTAOMainPass.h` and `CORTAOFilterPass.h`.
  These files support GLSL and HLSL through `#define`s.
- All texture reads and writes go through user callbacks (`cortao_loadDepth`,
  `cortao_samplePackedData`, `cortao_writeResult`, and others). The engine supplies these.
- The GLSL reference shaders are Vulkan GLSL: push constants, separate samplers and
  `GL_EXT_samplerless_texture_functions`. We cannot use them as they are. We use them only as
  examples.
- `CORTAOParameters.h` is C++. It makes the per-pass constant buffers from a configuration struct.
- The 16-ray occluder LUT (`24x24x16x16_R16_UINT`) is 18 KB. The bent-normal LUT for 16 rays is
  800 KB. The bent-normal LUT for 8 rays is 3 KB.

The [Carbon Engine fork](https://github.com/carbonengine/CORTAO) is for EVE Frontier. It has a
change for very large depth ranges. But it is HLSL only and has no GLSL support. Use the reference
implementation. Copy only the large-depth-range idea (see "Scale" below).

## Engine facts that affect the design

| Fact | Where | Effect |
|---|---|---|
| G-buffer position = view-space xy + compressed z, AO map in w | `main-f.sdr:555` | We can get linear depth without the hardware depth buffer. |
| G-buffer normal = view-space normal, `R16G16B16A16_SFLOAT` | `main-v.sdr:234`, `VulkanPostProcessing.h:315` | CORTAO wants view-space normals. 16-bit is the recommended precision. No conversion is necessary. |
| Cockpit pass clears the Z buffer and uses `Min_draw_distance_cockpit` | `ship.cpp:8396`, `ship.cpp:8320` | The hardware depth buffer holds two different depth ranges in one frame. Do not use it as input. |
| AO hook in the ambient light | `deferred-f.sdr:300-313` | CORTAO multiplies into `ao` like RTAO does. Env-map ambient also gets the AO through `ComputeEnvLight`. |
| No compute pipelines in the Vulkan backend | `VulkanPipeline.cpp:514` (graphics only), `pPushConstantRanges = nullptr` | A compute port needs new pipeline, descriptor and barrier code. |
| OpenGL minimum is 3.2. The shader `#version` is the driver's GLSL version. | `gropengl.cpp:54`, `gropenglshader.cpp:180` | Compute needs GL 4.3. The core uses `fma` and `bitCount`, which need GLSL 4.00. |
| glad has GL 4.0 to 4.3 symbols | `lib/opengl/gl/glad/include/glad/glad.h` | A runtime check for `GLAD_GL_VERSION_4_0` (or `4_3`) is possible. |
| Vulkan shaders compile at runtime with shaderc, with `#define` variants | `VulkanShaderCompiler.h` | Sample-count variants (4, 8, 16) are easy as shader flags. |
| Temporal filter: only FSR, only on Vulkan | `VulkanUpscaler.cpp` | Without FSR, CORTAO noise stays on screen. |
| Motion vectors exist (`fragOut6`) | `main-f.sdr:551` | A small AO-only temporal accumulation pass is possible later. |

## Design decisions

### 1. Depth input: use the G-buffer position, not the hardware depth

The main pass uses only the x/y terms of the projection matrix (`projectionMatrix[0][0]`, `[1][1]`,
`[2][0]`, `[2][1]`). It does not use the depth terms. The world pass and the cockpit pass use the
same FOV and aspect, so one projection is correct for both. Only the near plane is different.

The prepass converts depth with `linearDepth = 1.0 / fma(depth, depthScale, depthOffset)`. Use this
callback and these values:

- `cortao_loadDepth` returns `1.0 / -viewZ`, where `viewZ = uncompress_depth_value(position.z)`.
  For an empty G-buffer texel (sky, background), it returns `0.0`.
- Set `depthScale = 1.0` and `depthOffset = 0.0` in the prepass constants. Do not use
  `populatePrepassParameters()` for these two values.
- Then `linearDepth = -viewZ`, and the sky test (`depth == 0.0` when `depthScale >= 0`) still works.

With this method, the vendored headers stay without changes, and the cockpit and the world use one
consistent depth.

Check before implementation: the cockpit may use a different view transform (eye offset). The
memory note on RT shadows says that the shared coordinate space of the deferred pass is a known gap.
If the cockpit positions are in a different view space, CORTAO shows a wrong occlusion at the
cockpit edges only. The world is not affected.

### 2. Noise and temporal filtering

CORTAO expects a temporal filter (TAA, DLSS, FSR) after it. The engine has FXAA and SMAA, which do
not accumulate frames. FSR is only on Vulkan and is optional.

- FSR on: re-randomize `randomSeed` each frame. Use the 5-tap filter. FSR removes the rest.
- FSR off (all of OpenGL, most of Vulkan): use a constant `randomSeed`. If the seed changes, the
  noise flickers. Use the 13-tap filter and a higher sample count (16). Expect a static noise
  pattern that moves with the camera.
- Optional later step: an AO-only temporal accumulation pass. It reprojects the last AO with the
  existing motion vectors and rejects history on a depth difference. This is approximately one
  fullscreen pass and one history texture.

Do a visual test of item 2 before you do any other work. If the static noise is not acceptable,
the temporal accumulation pass becomes necessary, not optional.

### 3. Fragment shaders or compute shaders

Option A: fragment-shader port (recommended first).

- The main pass is per pixel. Run it as a fullscreen fragment pass. The `cortao_writeResult`
  callback writes to a global variable, and `main()` writes it to the output.
- The prepass: one fullscreen pass for mip 0, then one small pass for each mip 1 to 7. Each mip
  pass copies the texel that the original dither rule selects. The rule is
  `x&1 == (nextY ^ dither.x)&1` and `y&1 == (nextX ^ dither.y)&1`, so the source texel is easy to
  calculate. This shader is ours, not vendored.
- The filter pass: implement `cortao_gatherPackedData` and `cortao_gatherSSAO` with four
  `texelFetch` calls each. Run one fragment for each pixel and keep only the result for that
  pixel. This uses 4 times the filter work, but the filter is the cheapest pass.
- It fits the existing post-processing pass model in both backends.
- It needs GLSL 4.00 for `fma` and `bitCount`. It does not need GL 4.3.
- We lose the thread re-order in the main pass (`deinterleave`). Fragment rasterization already
  gives tile locality, so the cost is probably small. Measure it.

Option B: compute shaders.

- Closest to the reference. Best cache behavior.
- Vulkan needs: compute pipelines, storage-image descriptors, an `R32_UINT` image with one image
  view for each mip, barriers between the passes, and push constants or a new UBO.
- OpenGL needs GL 4.3 and `glDispatchCompute`. The feature is then not available on GL 3.x
  drivers.
- Do this only if Option A is too slow in a Release build.

### 4. Samplers and LUT

- The packed-data sampler needs nearest filtering and border clamp with an integer border of `0u`.
  Vulkan: `eIntTransparentBlack`. OpenGL: `GL_CLAMP_TO_BORDER` with `glTexParameterIuiv`. Or do a
  bounds test in the callback and return `0u`. The bounds test is the simplest for both backends.
- Put the 18 KB occluder LUT in the code as a `const uint16_t[]` array. Upload it once as a
  3D `R16_UINT` texture (`24x24x16`). Do not put it in `def_files`, because it is not a text file.
- Do not add the bent-normal LUT in the first phase.

### 5. Scale and parameters

Ships are from approximately 10 m to more than 2 km. A fixed world-space radius is not correct for
all of them. Do what the Carbon fork does:

- Set `maxRadius` to a large value, so that `maxScreenspaceRadiusHeightRatio` controls the radius.
- Put the values in the lighting profile, next to `$RTAO Radius:` and `$RTAO Strength:`. Proposed
  names: `$SSAO Max Radius:`, `$SSAO Screen Radius:`, `$SSAO Intensity:`. The intensity is the
  `pow(occlusion, intensity)` that the CORTAO README recommends for a non-physical look.
- The linear depth is stored as float bits with 10 low bits removed. The relative error is
  approximately 2^-13. This is correct at all distances, so the large depth range of space scenes
  is not a problem for the packed buffer.

### 6. Relation to RTAO

- CORTAO and RTAO use the same `ao *=` hook. Use only one of them at a time. If `rtao_enabled()`
  is true, do not run CORTAO.
- Use one options-menu entry for AO with these values: Off, SSAO Low (4 samples), SSAO Medium (8),
  SSAO High (16), and the RTAO values when `rtao_supported()` is true.

### 7. Other checks

- MSAA: CORTAO must read single-sample inputs. Run it after the G-buffer resolve (`msaa-f.sdr`).
  Make sure that the resolved position and normal are available at that point in both backends.
- Y direction: Vulkan uses a negative viewport for the screen and a positive viewport for render
  targets (`VulkanState.cpp:462-470`). Find the convention of the G-buffer and set `flipY`. Test
  with a model that is near the top of the screen. A wrong `flipY` gives a vertical mirror of the
  occlusion.
- FSR: CORTAO runs at the render resolution, before the upscale. The deferred light pass is also at
  the render resolution, so no change is necessary.
- License: the vendored files must keep the MIT notice. The rule "no copyright banner in new files"
  applies to our own files only. Add the license to the third-party license list.

## Phases and effort

Estimates are for one developer who knows the renderer.

| Phase | Work | Estimate |
|---|---|---|
| 0 | Prototype on Vulkan only. Option A, occlusion only, 16 samples, no filter, constant seed. Visual test in the F3 lab and in a mission, with and without FSR. Decide on item 2. | 3 to 4 days |
| 1 | Filter pass, sample-count variants, lighting-profile values, options menu, mutual exclusion with RTAO, Release-build timing. | 3 to 4 days |
| 2 | OpenGL port of the same passes. Feature check for GLSL 4.00. | 2 to 3 days |
| 3 (optional) | AO-only temporal accumulation with motion vectors. Only if phase 0 shows that static noise is not acceptable. | 3 to 5 days |
| 4 (optional) | Bent normals for the diffuse env-map lookup in `ComputeEnvLight`. Needs the 8-ray LUT or the 800 KB 16-ray LUT. | 2 to 3 days |
| 5 (optional) | Compute port (Option B) on Vulkan, only if profiles show that it is necessary. | 4 to 6 days |

## Open questions

1. Is a screen-space AO wanted next to RTAO? In space scenes, most occlusion is on ship surfaces
   (greebles, hangars, turrets). SSAO shows these well. SSAO cannot show occlusion from objects
   that are not on the screen.
2. Is static noise without FSR acceptable, or is phase 3 necessary? Phase 0 answers this.
3. Do the cockpit positions use the same view space as the world? This affects only the cockpit
   edges.
