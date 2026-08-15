#ifndef __DECODE_HPP__
#define __DECODE_HPP__

#include "ForceInline.hpp"
#include <stdint.h>

// Public interface for decoding functions
etcpak_no_inline void DecodeBc1(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeBc3(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeBc4(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeBc5(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeBc7(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeRGB(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeRGBA(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height);
etcpak_no_inline void DecodeR(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height, bool isSigned = false);
etcpak_no_inline void DecodeRG(const uint64_t* src, uint32_t* dst, int32_t width, int32_t height, bool isSigned = false);

#endif

