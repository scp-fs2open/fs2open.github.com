# Module: bmpman — `code/bmpman/`

## Purpose
The **bitmap manager**: the single owner of every 2D image the engine loads. It
hides the file format behind one handle type, tracks how much texture memory is
in use, pages textures in and out around a mission, and hands the data to the
renderer. Every texture, interface bitmap, HUD graphic, and animation frame in
the game comes through here.

## Key files
- `bmpman.cpp` / `bmpman.h` — the whole public API and the `BM_TYPE`/flag sets.
- `bm_internal.h` — `bitmap_entry`, `bitmap_slot`, `gr_bitmap_info`, and the
  `bm_blocks` storage the graphics backends need but ordinary code does not.
- `bm_examples.cpp` — worked examples of the load/lock/unload sequence.

## Core data structures / globals
- **The handle is the currency.** `bm_load()` and friends return an `int`
  handle, not a pointer. Everything else in the engine stores that handle;
  `bm_is_valid()` checks one.
- `struct bitmap` — what `bm_lock()` gives you: `w`, `h`, `d`, `rowsize`,
  `bpp` (requested bit depth), `true_bpp` (the image's actual depth), `flags`,
  `data`, and an optional `palette`.
- `bitmap_entry` / `bitmap_slot` (`bm_internal.h`) — the manager's own record
  per slot. Storage is `SCP_vector<std::array<bitmap_slot, BM_BLOCK_SIZE>>
  bm_blocks`: it **grows in blocks** of `BM_BLOCK_SIZE` (4096), so there is no
  fixed `MAX_BITMAPS` cap.
- `bm_texture_ram` — bytes of texture memory currently in use.
- `Bm_paging` — true while a paging pass is running.

## Major constants
- `enum BM_TYPE` — the format of a slot: `BM_TYPE_NONE`, `BM_TYPE_USER`
  (in-memory), `BM_TYPE_PCX`, `BM_TYPE_TGA`, `BM_TYPE_DDS`, `BM_TYPE_PNG`,
  `BM_TYPE_JPG`, `BM_TYPE_ANI`, `BM_TYPE_EFF`, `BM_TYPE_KTX`, `BM_TYPE_3D`,
  the render-target types, the compressed types (`BM_TYPE_DXT1`/`DXT3`/`DXT5`,
  `BM_TYPE_BC7`, the `BM_TYPE_ETC2_*` family), and the `BM_TYPE_CUBEMAP_*`
  variants.
- Texture-use flags: `BMP_AABITMAP` (antialiased interface bitmap),
  `BMP_TEX_XPARENT`, `BMP_TEX_OTHER`, `BMP_MASK_BITMAP` (a mouse-interaction
  mask, not drawn), `BMP_TEX_CUBEMAP`, and the per-compression
  `BMP_TEX_DXT1`/`DXT3`/`DXT5`/`BC7`/`ETC2_*` bits.
- Convenience masks: `BMP_TEX_COMP` (all compressed), `BMP_TEX_NONCOMP`,
  `BMP_TEX_ANY`.
- Render-target flags: `BMP_FLAG_RENDER_TARGET_STATIC`,
  `BMP_FLAG_RENDER_TARGET_DYNAMIC`, `BMP_FLAG_CUBEMAP`.

## The load/lock/unload contract
1. `bm_load(filename, dir_type)` — resolves the extension, picks the decoder,
   reads only the *header*, and reserves a slot. It does not read pixels.
   `bm_load_animation()` and `bm_load_either()` do the same for animations.
2. `bm_lock(handle, bpp, flags)` — this is where the pixels are actually read
   and converted. The `flags` you pass decide the in-memory form, which is why
   the same file can be loaded once as a texture and once as an `BMP_AABITMAP`.
3. `bm_unlock(handle)` — always pair it with the lock.
4. `bm_unload()` / `bm_release()` — release the data or the slot itself.

`bm_page_in_start()` … `bm_page_in_texture()` … `bm_page_in_stop()` is the
mission-load paging pass that decides what actually reaches the GPU.
`bm_create()` makes a `BM_TYPE_USER` bitmap from memory you already have.

## Supported formats
Each file format lives in its own module and exposes the same two-call
contract, `<fmt>_read_header()` and `<fmt>_read_bitmap()`, which `bmpman` calls:

| Format | Module | Guide |
| --- | --- | --- |
| DDS (incl. DXT/BC7 and cubemaps) | `code/ddsutils/` | `modules/ddsutils.md` |
| KTX1 (incl. ETC2) | `code/ktxutils/` | `modules/ktxutils.md` |
| PNG | `code/pngutils/` | `modules/pngutils.md` |
| TGA | `code/tgautils/` | `modules/tgautils.md` |
| PCX | `code/pcxutils/` | `modules/pcxutils.md` |
| JPEG | `code/jpgutils/` | `modules/jpgutils.md` |
| ANI (in-house animation) | `code/anim/` | `modules/anim.md` |

`EFF` is not an image format: it is a small text file naming a frame sequence,
handled inside `bmpman` itself.

## Adding a format
Add a module exposing `<fmt>_read_header()` / `<fmt>_read_bitmap()`, add a
`BM_TYPE_*` value, and extend the extension table and the load/lock switches in
`bmpman.cpp`. Compressed formats also need the matching `BMP_TEX_*` bit and
renderer support, so they must degrade gracefully where the hardware cannot take
them.

## Configuration tables
None.

## See also
- `code/graphics/` (uploads these bitmaps as textures), `code/cfile/` (where the
  files come from), `code/model/` (ship textures are bitmap handles),
  `code/lab/` (texture-override controls exercise this module).
