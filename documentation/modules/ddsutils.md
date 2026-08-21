# Module: ddsutils — `code/ddsutils/`

## Purpose
Reads **DDS** image files for `code/bmpman/`. DDS is the format FSO uses for
most in-game textures because it can carry GPU-ready block-compressed data
(DXT1/3/5, BC7) with mipmaps already generated, so the file can go to the GPU
with no decompression on load. It also carries cubemaps, which FSO uses for
environment maps.

## Key files
- `ddsutils.cpp` / `ddsutils.h` — header parsing, format identification, and
  reading.
- `bcdec.h` — a vendored single-header block-compression decoder, used when the
  data has to be decompressed on the CPU rather than handed to the GPU.

## API
This module follows the shared decoder contract that `bmpman` calls:
- `dds_read_header(filename, cfp, w, h, bpp, compression_type, levels, size)` —
  identify the file and report its dimensions, the compression type (which
  `bmpman` maps onto a `BM_TYPE_DXT*` / `BM_TYPE_BC7` / `BM_TYPE_CUBEMAP_*`),
  the mipmap count, and the total data size.
- `dds_read_bitmap(filename, data, bpp, cf_type)` — read the pixel data.
- `dds_decompress_top_mip_bgra(...)` — decompress only the top mip to BGRA,
  for the paths that need real pixels (screenshots, CPU-side inspection).
- `dds_save_image(...)` — write a DDS, including cubemaps.

Note that `dds_read_header` reports a **mipmap count**: DDS is the one input
format that arrives with mipmaps already in the file.

## Configuration tables
None.

## See also
- `code/bmpman/` — the caller; read `modules/bmpman.md` for the load/lock
  contract and the `BM_TYPE_*` / `BMP_TEX_*` sets this module feeds.
- `code/ktxutils/` — the other compressed-texture container (KTX1/ETC2).
- `code/graphics/` — consumes the compressed blocks directly where the hardware
  supports them.
