# Module: tgautils — `code/tgautils/`

## Purpose
Reads and writes **TGA (Targa)** images for `code/bmpman/`. TGA is a retail-era
format still used by older content; it is uncompressed or run-length encoded,
and simple enough that this module implements the RLE codec itself rather than
using a library.

## Key files
- `tgautils.cpp` / `tgautils.h` — reading, writing, and the RLE codec.

## API
- `targa_read_header(filename, img_cfp, w, h, bpp, palette)` and
  `targa_read_bitmap(filename, data, palette, dest_size, cf_type)` — the shared
  decoder contract `bmpman` calls.
- `targa_write_bitmap(filename, data, palette, w, h, bpp)` — write a TGA.
- `targa_compress()` / `targa_uncompress()` — the run-length codec.

## Configuration tables
None.

## See also
- `code/bmpman/` — the caller; `modules/bmpman.md` has the load/lock contract.
- `code/pcxutils/` — the other retail-era format.
