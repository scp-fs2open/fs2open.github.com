# Module: jpgutils — `code/jpgutils/`

## Purpose
Reads **JPEG** images for `code/bmpman/`. It is the smallest of the image
modules and read-only: JPEG is lossy and has no alpha channel, so it is used for
large opaque images (backgrounds, briefing art) and never for anything needing
transparency.

## Key files
- `jpgutils.cpp` / `jpgutils.h` — the two read entry points. Uses the bundled
  libjpeg.

## API
- `jpeg_read_header(filename, img_cfp, w, h, bpp, palette)`
- `jpeg_read_bitmap(filename, image_data, palette, dest_size, cf_type)`

That is the whole module — the shared decoder contract and nothing else. There
is no write path.

## Configuration tables
None.

## See also
- `code/bmpman/` — the caller; `modules/bmpman.md` has the load/lock contract.
- `code/pngutils/` — use PNG instead when alpha or lossless data is needed.
