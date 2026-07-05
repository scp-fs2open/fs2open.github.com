#ifndef PIXEL_SWIZZLE_H
#define PIXEL_SWIZZLE_H

#include <cstddef>
#include <cstdint>

#include "globalincs/pstypes.h"

namespace graphics {
namespace util {

// ---- BGR/BGRA ↔ RGB/RGBA channel swaps ----

// 32-bit BGRA → RGBA (in-place convenience)
void swizzle_bgra_to_rgba(uint8_t* pixels, size_t count);

// 32-bit BGRA → RGBA (copy, safe for overlapping when src==dst)
void convert_BGRA8888_to_RGBA8888(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// 32-bit BGRA → RGB888 (R/B swap, alpha discarded)
void convert_BGRA8888_to_RGB888(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// 32-bit RGBA → RGB888 (alpha stripped, no channel reorder)
void convert_RGBA8888_to_RGB888(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// 24-bit BGR → RGB
void convert_BGR_to_RGB(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// 24-bit BGR → 32-bit RGBA (alpha = 255)
void convert_BGR_to_RGBA(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// 24-bit BGR → 32-bit BGRA (alpha = 255, preserves BGR channel order)
void expand_BGR_to_BGRA(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// ---- 16-bit A1R5G5B5 / BGRA1555_REV expansion ----

// BGRA1555_REV → RGBA5551
void convert_BGRA1555_REV_to_RGBA5551(const uint16_t* RESTRICT src, uint16_t* RESTRICT dst, size_t count);

// BGRA1555_REV → RGBA8888 (5→8 bit expansion)
// Note: BGRA1555_REV layout is identical to A1R5G5B5
void convert_BGRA1555_REV_to_RGBA8888(const uint16_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// BGRA1555_REV → RGB888 (5→8 bit expansion, alpha discarded)
// Note: BGRA1555_REV layout is identical to A1R5G5B5
void convert_BGRA1555_REV_to_RGB888(const uint16_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// ---- Greyscale expansion ----

// R8 → RGBA8888 (R replicated to R/G/B, alpha = 255)
void expand_R8_to_RGBA(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

// R8 → RGB888 (R replicated to R/G/B)
void expand_R8_to_RGB(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count);

} // namespace util
} // namespace graphics

#endif
