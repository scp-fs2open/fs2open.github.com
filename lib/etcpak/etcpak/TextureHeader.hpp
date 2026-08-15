#ifndef __TEXTUREHEADER_HPP__
#define __TEXTUREHEADER_HPP__

#include "ForceInline.hpp"
#include <stdint.h>
#include <stddef.h>

enum CodecType
{
    Etc1,
    Etc2_RGB,
    Etc2_RGBA,
    Etc2_R11,
    Etc2_RG11,
    Bc1,
    Bc3,
    Bc4,
    Bc5,
    Bc7
};

// Public interface for processing header
etcpak_no_inline void ProcessHeader(uint8_t* data, CodecType& type, int32_t& width, int32_t& height, size_t& dataOffset);
#endif