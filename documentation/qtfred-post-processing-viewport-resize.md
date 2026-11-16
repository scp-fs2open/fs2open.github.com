# qtFred post-processing viewport resize issues

Notes on two bugs hit while adding the qtFred "Enable Post Processing" View-menu
toggle. Both are fixed; kept here so the next person who reopens this doesn't
have to re-derive the diagnosis from scratch, and so the dead ends are on record.

## Background

qtFred's `FredRenderer::render_frame()` calls `gr_screen_resize()` every frame
to match whatever size its dockable/resizable 3D viewport widget currently is.
The game does this too, but only on an SDL window-resize event
(`osapi.cpp`) — for most of its life `gr_screen` is fixed after `gr_init()`.
That difference in *frequency* is the root of everything below; the underlying
bug was reachable from the game's resizable window as well.

## Bug 1: `u_scale`/`v_scale` copy-paste in the post-processing passes

**Symptom:** with post-processing enabled, sun sprites and lens flares landed
at different screen positions depending on where in the viewport the sun was
— vertical-only offset top-left, both-axes top-right, horizontal-only
bottom-right, near-perfect bottom-left. Bloom was also misaligned the same way.

**Cause:** `code/graphics/opengl/gropenglpostprocessing.cpp` had eight call
sites of the form:

```cpp
opengl_draw_full_screen_textured(0.0f, 0.0f, Scene_texture_u_scale, Scene_texture_u_scale);
```

`u_scale` was passed for *both* the horizontal and vertical scale argument —
a copy-paste error. In the game this was a no-op (`u_scale == v_scale` there,
since the scene texture is always exactly screen-sized). In qtFred, where the
scene texture can be a different aspect ratio than the current viewport, this
silently applied the horizontal crop fraction to the vertical axis (or vice
versa) at each pass (tonemap, FXAA prepass, both SMAA passes, cockpit
lightshafts, and the final composite-to-screen blit), compounding into a
direction- and position-dependent drift.

**Fix:** those sites now call `opengl_draw_full_screen_scene_texture()`
(`gropengldraw.cpp`), which supplies both scales itself. Open-coding the extents
is what allowed one axis to be wrong, so the helper exists to make the whole
class of bug unreachable — prefer it over literal extents in any new pass that
samples a scene or post-processing texture.

Fixing bug 1 alone was **not** sufficient — the user confirmed misalignment
persisted afterwards, which is what led to bug 2.

(Three further `u_scale`-twice sites survive at the bottom of
`gr_opengl_post_process_end()`; they are inside a `/* */` block of dead debug
code and were deliberately left alone.)

### Related sites found later

The same "sample the full [0,1] range of a partially-filled target" bug existed
outside the post-processing module and was fixed alongside the resize work:

- `gropengldeferred.cpp` — the MSAA scene-colour copy, the MSAA resolve, and
  the fog pass all passed literal `1.0f` extents while sampling scene targets.
- `deferred-f.sdr` reconstructs a G-buffer texture coordinate from
  `gl_FragCoord` and `invScreenWidth`/`invScreenHeight`, which described
  `gr_screen` rather than the G-buffer. Fixed in both the OpenGL
  (`gropengldeferred.cpp`) and Vulkan (`VulkanPostProcessingLighting.cpp`)
  backends. It is currently a no-op under Vulkan, whose `resize()` keeps the
  scene extent equal to `gr_screen`, but it states the requirement rather than
  depending on that staying true.
- `fxaa-v.sdr` derived its texcoord from `vertPosition` instead of the
  `vertTexCoord` attribute, ignoring whatever sub-rectangle the draw call asked
  for. Every other post-process vertex shader already used the attribute.

**Deliberately left alone:** the volumetric nebula pass. `volumetric-f.sdr` uses
`fragTexCoord` for two incompatible purposes — reconstructing an eye-space ray
direction, which needs the full 0..1 range across the viewport, and sampling
composite/depth/emissive, which needs the rendered sub-rectangle. Scaling the
draw would fix the sampling and skew every ray. Separating them needs a second
varying (or a scale uniform) in the shader. Until then volumetrics are only
correct while the targets exactly match the viewport, which is the normal case.

## Bug 2: stale scene-texture allocation when the viewport grows

**Symptom:** after the bug-1 fix, misalignment (and bloom stretching) still
appeared, but only after the qtFred window had been resized/maximized/
fullscreened *larger* than it was earlier in the session. A manual resize
cycle would often fix it; going fullscreen would reintroduce it.

**Root cause:** `Scene_texture_width`/`Scene_texture_height`
(`opengl_setup_scene_textures()`, `gropengldraw.cpp`) and the post-processing
surfaces sized off `gr_screen.max_w`/`max_h`
(`opengl_post_init_framebuffer()`, `gropenglpostprocessing.cpp`) were allocated
exactly once, at `gr_init()`, and never revisited. When the viewport grows past
that original allocation the render still only writes into the old (smaller)
texture: everything past its edge is silently clipped, and the final blit —
unaware anything was clipped — stretches that smaller, cropped result back over
the new, larger viewport. That non-uniform stretch is what reads as a position-
and axis-dependent drift, magnified further at fullscreen. Shrinking the
viewport back below the allocation is unaffected, since
`Scene_texture_u_scale`/`v_scale` already crop correctly for a viewport smaller
than the allocation.

Diagnosed empirically (no way to run the qtFred GUI directly) via two
throttled `mprintf` diagnostics temporarily added to `project_source()`
(`lens_flare.cpp`) and `gr_opengl_scene_texture_begin()` (`gropengldraw.cpp`),
comparing `Scene_texture_width/height` against `gr_screen.max_w/h` and
`Canvas_width/height`. Log evidence
(`Scene_texture=3072x1728 gr_screen.max=3512x1910`) confirmed the texture was
smaller than the live viewport, and the user's own manual testing (resize
fixes it, fullscreen re-breaks it) confirmed the allocate-once behavior.
Both diagnostics were removed once the root cause was confirmed; they are not
in the tree.

### Fix: grow the render targets when the viewport outgrows them

`gr_screen.gf_resize_render_targets` (`2d.h`) is a backend hook called from
`gr_screen_resize()` (`2d.cpp`) after `gr_setup_viewport()`. OpenGL implements
it as `gr_opengl_resize_render_targets()` (`gropengldraw.cpp`); Vulkan leaves it
unset, because `VulkanRenderer::recreateSwapChain()` already owns resizing its
extent-sized targets and a second entry point would risk double-resizing.

The OpenGL implementation rebuilds only the resolution-dependent resources:

- `opengl_scene_texture_shutdown()` + `opengl_setup_scene_textures(w, h)` for
  the scene textures. The latter now takes explicit dimensions rather than
  reading `gr_screen` itself, so the sizing policy lives in one named place.
- `opengl_post_resize_render_targets()` (`gropenglpostprocessing.cpp`) for the
  bloom mip chain and the SMAA surfaces. It sizes `Post_texture_*` to match
  `Scene_texture_*` — they consume those textures pass by pass, so the two
  sizes diverging is what bug 1 looked like.

The post-processing table, the compiled shaders and the SMAA area/search lookup
textures are all resolution-independent and stay alive. That is what keeps this
cheap enough to run off a window drag, and it mirrors what
`VulkanPostProcessor::resize()` has always done — the OpenGL backend was the
odd one out.

Three properties the implementation depends on:

- **Grow only.** `gr_screen_resize()` runs every frame in qtFred, and
  `BriefingMapWidget` resizes down and back repeatedly. Tracking the high-water
  mark avoids thrashing, and the shrunk state is already correct via
  `Scene_texture_u_scale`/`v_scale`.
- **Clamp before comparing.** `GL_max_renderbuffer_size` is applied to the
  requested size *inside* `gr_opengl_resize_render_targets()`, before it decides
  whether anything changed. Clamping inside the allocator instead would leave a
  viewport larger than the hardware limit requesting a resize that can never be
  satisfied — a full teardown and rebuild every single frame.
- **Never resize mid-frame.** The function refuses (with an `Assertion`) while
  `Scene_framebuffer_in_frame` is set. That flag covers the post-processing
  passes too: they only run inside `gr_scene_texture_begin()`/`end()`, and it is
  cleared at the very end of `gr_opengl_scene_texture_end()`.

If the larger allocation fails outright — most likely precisely when growing —
`opengl_setup_scene_textures()` reports it by leaving `Scene_texture_initialized`
at 0, having already turned post-processing and soft particles off. The resize
stops there rather than rebuilding the post-processing targets on top of scene
textures that no longer exist.

This replaced an earlier `Gr_min_render_target_w`/`_h` floor, which sized the
targets up front for the largest attached display. That worked, but every
qtFred user paid for it at launch whether or not they ever enabled
post-processing: on a 4K display at 2x scaling the floor was 7680x4320, which
across the nine scene textures (most of them `RGBA16F`) plus the post-processing
surfaces is on the order of a gigabyte of VRAM — and `-msaa 8` multiplied the
six multisample targets on top of that. AGENTS.md is explicit that FSO must run
across the whole hardware range, so an unconditional worst-case allocation for
an off-by-default feature was the wrong trade.

### Verifying it from a log

`opengl_setup_scene_textures()` reports each allocation:

```
  Scene textures: 3840x2160 (screen 3840x2160, max renderbuffer 16384)
```

and `gr_opengl_resize_render_targets()` reports each growth:

```
Growing render targets from 1024x768 to 3840x2160 to cover the new 3840x2160 viewport.
```

Launch qtFred, enable post-processing, and drag the viewport dock larger. The
growth line should appear **once per growth step and never per frame** — a
per-frame stream means the clamp/early-out logic is wrong. The one-shot
`nprintf(("OpenGL", "Viewport (...) is larger than the scene texture backing
it ..."))` in `gr_opengl_scene_texture_begin()` should not appear at all; if it
does, the targets could not grow (`GL_max_renderbuffer_size` on low-end
hardware) and the old stretching is back. It must stay `nprintf` and stay
one-shot: that function runs every frame.

**Not yet checked in-editor.** None of this has been exercised at runtime. Note
that the CLion `qtfred` run configuration passes `-vulkan`, which exercises
`VulkanPostProcessor::resize()` rather than any of the OpenGL code above — drop
that flag to test this path. Worth eyeballing once someone does: bloom radius
and SMAA quality, since the bright pass renders into the full
`Post_texture_width >> 1` viewport while sampling only the cropped
sub-rectangle, and SMAA's RT-metrics are likewise derived from `Post_texture_*`.
Worst case there is a cosmetic difference, not misalignment.

### History: why reallocation was rejected twice before

Two earlier attempts at exactly the approach now implemented both regressed to a
black viewport and were reverted:

1. A size check inside `gr_opengl_scene_texture_begin()` that tore down and
   rebuilt in place, every frame.
2. A cross-backend `gr_scene_texture_grow()` entry point wired through the
   function-pointer table and called from `FredRenderer::render_frame()` after
   `gr_screen_resize()` — structurally the same as the current hook.

The second failed on *every* post-processing frame, not just grown ones, which
is consistent: qtFred's first post-processing frame is almost always already
larger than the `gr_init()` allocation.

**Why it works now.** The previous version of this document identified the
prerequisite correctly, and it turned out to be the whole problem:
`opengl_scene_texture_shutdown()` did not delete or zero `Scene_ldr_texture`,
`Scene_composite_texture`, `Scene_luminance_texture`, `Cockpit_depth_texture`,
or any of the six `_ms` objects and `Scene_framebuffer_ms`, while
`opengl_setup_scene_textures()` re-`glGenTextures`'d over those handles. Any FBO
left attached to a stale or deleted texture is incomplete, and draws to an
incomplete FBO go nowhere — black viewport. (At shutdown this was merely a leak,
which is why it went unnoticed.)

The teardown now releases everything setup allocates, and post-processing
shutdown releases the SMAA lookup textures it was also leaking. Two further
prerequisites had to be met:

- **Deletion goes through the state cache.** `GL_state.Texture.Delete()` unbinds
  a texture from every unit before `glDeleteTextures()`. Without it the cache
  can still hold a freed name, and since the driver is free to hand that name
  straight back out, a later `Enable()` of the recycled texture is silently
  skipped. Use `opengl_delete_render_texture()` /
  `opengl_delete_render_framebuffer()` (`gropengldraw.cpp`) rather than calling
  the GL entry points directly. The framebuffer cache has no equivalent unbind,
  so the resize path binds 0 before deleting anything.
- **Only the size-dependent work is redone.** `opengl_post_process_init()`
  re-parses `post_processing.tbl` and rebuilds
  `graphics::Post_processing_manager` from scratch, which is not something to do
  mid-session; the resolution-dependent half was split out into
  `opengl_post_resize_render_targets()` precisely so the resize does not touch
  it.

The earlier document also floated `GL_state`'s framebuffer-binding cache as the
cause of the black viewport and proposed an explicit
`GL_state.BindFrameBufferBoth(0, 0)`. That diagnosis did not hold up on its own —
both setup functions already end with `GL_state.BindFrameBuffer(0)` from a
non-zero cache, so the bind does get issued. The call is nonetheless present in
the resize path, for the different and real reason given above: to keep the
cache off a framebuffer name that is about to be deleted.
