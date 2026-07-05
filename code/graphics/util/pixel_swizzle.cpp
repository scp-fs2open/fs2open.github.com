#include "graphics/util/pixel_swizzle.h"

namespace graphics {
namespace util {

void swizzle_bgra_to_rgba(uint8_t* pixels, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		size_t off = i * 4;
		uint8_t tmp = pixels[off + 0];
		pixels[off + 0] = pixels[off + 2];
		pixels[off + 2] = tmp;
	}
}

void convert_BGRA8888_to_RGBA8888(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		const uint8_t b = src[4 * i + 0];
		const uint8_t g = src[4 * i + 1];
		const uint8_t r = src[4 * i + 2];
		const uint8_t a = src[4 * i + 3];
		dst[4 * i + 0] = r;
		dst[4 * i + 1] = g;
		dst[4 * i + 2] = b;
		dst[4 * i + 3] = a;
	}
}

void convert_RGBA8888_to_RGB888(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		dst[i * 3 + 0] = src[i * 4 + 0];
		dst[i * 3 + 1] = src[i * 4 + 1];
		dst[i * 3 + 2] = src[i * 4 + 2];
	}
}

void convert_BGRA8888_to_RGB888(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		dst[i * 3 + 0] = src[i * 4 + 2]; // R
		dst[i * 3 + 1] = src[i * 4 + 1]; // G
		dst[i * 3 + 2] = src[i * 4 + 0]; // B
	}
}

void convert_BGR_to_RGB(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		size_t s = i * 3;
		size_t t = i * 3;
		uint8_t b = src[s + 0];
		uint8_t g = src[s + 1];
		uint8_t r = src[s + 2];
		dst[t + 0] = r;
		dst[t + 1] = g;
		dst[t + 2] = b;
	}
}

void convert_BGR_to_RGBA(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		size_t s = i * 3;
		size_t t = i * 4;
		dst[t + 0] = src[s + 2]; // R <- B
		dst[t + 1] = src[s + 1]; // G
		dst[t + 2] = src[s + 0]; // B <- R
		dst[t + 3] = 255;        // A
	}
}

void expand_BGR_to_BGRA(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = 255;
		src += 3;
		dst += 4;
	}
}

void convert_BGRA1555_REV_to_RGBA5551(const uint16_t* RESTRICT src, uint16_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		const uint16_t s = src[i];
		uint16_t a = (s >> 15) & 0x1;
		uint16_t r = (s >> 10) & 0x1F;
		uint16_t g = (s >> 5) & 0x1F;
		uint16_t b = (s >> 0) & 0x1F;
		dst[i] = static_cast<uint16_t>((r << 11) | (g << 6) | (b << 1) | a);
	}
}

void convert_BGRA1555_REV_to_RGBA8888(const uint16_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		uint16_t s = src[i];
		uint8_t A = (s >> 15) ? 255 : 0;
		uint8_t R5 = (s >> 10) & 0x1F;
		uint8_t G5 = (s >> 5) & 0x1F;
		uint8_t B5 = s & 0x1F;
		uint8_t R = (R5 << 3) | (R5 >> 2);
		uint8_t G = (G5 << 3) | (G5 >> 2);
		uint8_t B = (B5 << 3) | (B5 >> 2);
		size_t o = 4 * i;
		dst[o + 0] = R;
		dst[o + 1] = G;
		dst[o + 2] = B;
		dst[o + 3] = A;
	}
}

void convert_BGRA1555_REV_to_RGB888(const uint16_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		uint16_t s = src[i];
		uint8_t R5 = (s >> 10) & 0x1F;
		uint8_t G5 = (s >> 5) & 0x1F;
		uint8_t B5 = s & 0x1F;
		uint8_t R = (R5 << 3) | (R5 >> 2);
		uint8_t G = (G5 << 3) | (G5 >> 2);
		uint8_t B = (B5 << 3) | (B5 >> 2);
		size_t o = 3 * i;
		dst[o + 0] = R;
		dst[o + 1] = G;
		dst[o + 2] = B;
	}
}

void expand_R8_to_RGBA(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		const uint8_t r = src[i];
		dst[i * 4 + 0] = r;
		dst[i * 4 + 1] = r;
		dst[i * 4 + 2] = r;
		dst[i * 4 + 3] = 255;
	}
}

void expand_R8_to_RGB(const uint8_t* RESTRICT src, uint8_t* RESTRICT dst, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		const uint8_t r = src[i];
		dst[i * 3 + 0] = r;
		dst[i * 3 + 1] = r;
		dst[i * 3 + 2] = r;
	}
}

} // namespace util
} // namespace graphics
