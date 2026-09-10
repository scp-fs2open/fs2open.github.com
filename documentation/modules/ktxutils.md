# Module: ktxutils — `code/ktxutils/`

## Purpose
Reads **KTX1** image files for `code/bmpman/`. KTX is the Khronos texture
container; in FSO it mainly carries **ETC2**-compressed data, the compression
family available on OpenGL ES and mobile-class hardware where the DXT/BC7
formats in `code/ddsutils/` are not.

## Key files
- `ktxutils.cpp` / `ktxutils.h` — header parsing, format mapping, and reading.

## API
The shared decoder contract that `bmpman` calls:
- `ktx1_read_header(filename, img_cfp, w, h, bpp, c_type, mip_levels, total_size)`
  — identify the file, its compression type, mipmap count, and total size.
- `ktx1_read_bitmap(filename, dst, out_bpp)` — read the pixel data.
- `ktx_map_ktx_format_to_gl_internal(ktx_format)` — translate the KTX format
  token into a GL internal format.

`bmpman` maps the reported type onto `BM_TYPE_ETC2_RGB`, `BM_TYPE_ETC2_SRGB`,
`BM_TYPE_ETC2_RGBA1`, or the RGBA8 variant, with the matching `BMP_TEX_ETC2_*`
flag.

## Configuration tables
None.

## See also
- `code/bmpman/` — the caller; `modules/bmpman.md` has the load/lock contract.
- `code/ddsutils/` — the desktop-hardware equivalent (DXT/BC7).
- The `FSO_BUILD_WITH_OPENGL_ES` CMake option, which is why this path exists.
