# Module: pcxutils — `code/pcxutils/`

## Purpose
Reads and writes **PCX** images for `code/bmpman/`. PCX is the original
FreeSpace 2 image format: 8-bit paletted and run-length encoded. It is kept for
**retail content compatibility** — most retail interface art and many retail
textures are PCX — so this module is a backwards-compatibility requirement
rather than a format anyone would choose today.

## Key files
- `pcxutils.cpp` / `pcxutils.h` — reading, writing, and the RLE codec.

## API
- `pcx_read_header(filename, img_cfp, w, h, bpp, pal)` and
  `pcx_read_bitmap(filename, org_data, pal, byte_size, aabitmap, mask_bitmap, cf_type)`
  — the shared decoder contract `bmpman` calls. The extra `aabitmap` and
  `mask_bitmap` arguments reflect that PCX is the format retail used for
  antialiased interface bitmaps and for mouse-interaction masks.
- `pcx_write_bitmap(filename, w, h, row_ptrs, palette)` — write a PCX.

Because PCX is paletted, this is one of the few decoders that fills in a
palette; see the `palette` member of `struct bitmap` in `code/bmpman/`.

## Configuration tables
None.

## See also
- `code/bmpman/` — the caller; `modules/bmpman.md` has the load/lock contract
  and the `BMP_AABITMAP` / `BMP_MASK_BITMAP` flags this module serves.
- `code/tgautils/` — the other retail-era format.
