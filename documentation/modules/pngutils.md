# Module: pngutils — `code/pngutils/`

## Purpose
Reads and writes **PNG** images for `code/bmpman/`. PNG is the lossless
uncompressed-source format used for interface art and for textures that need
full alpha without compression artefacts. It is also the format the engine
writes screenshots in, and — uniquely among the image modules — it can work on
**base64 strings** as well as files, which is how images reach the libRocket UI
and Lua.

## Key files
- `pngutils.cpp` / `pngutils.h` — reading, writing, and the base64 variants.
  Uses the bundled libpng.

## API
The shared decoder contract, plus base64 and write support:
- `png_read_header(filename, img_cfp, w, h, bpp, palette)` and
  `png_read_bitmap(filename, image_data, bpp, dest_size, cf_type)` — the
  file path that `bmpman` calls.
- `png_read_header(b64, w, h, bpp, palette)` and
  `png_read_bitmap(b64, image_data, bpp)` — the same, from a base64 string.
- `png_write_bitmap(filename, width, height, y_flip, data)` — write a PNG.
- `png_b64_bitmap(width, height, y_flip, data)` — encode to a base64 string.

The `y_flip` argument exists because the renderer's framebuffer origin does not
match PNG's row order.

## Configuration tables
None.

## See also
- `code/bmpman/` — the caller; `modules/bmpman.md` has the load/lock contract.
  Note PNG is also one of the animation types in `bm_ani_type_list`, so an
  animation can be a PNG sequence.
- `code/scpui/` and `code/scripting/` — the base64 consumers.
