#ifndef __BLOCKDATA_HPP__
#define __BLOCKDATA_HPP__

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "Bitmap.hpp"
#include "ForceInline.hpp"
#include "Vector.hpp"
#include "TextureHeader.hpp"

struct bc7enc_compress_block_params;

class BlockData
{
public:

    enum Format
    {
        Pvr,
        Dds
    };

    BlockData( const char* fn );
    BlockData( const char* fn, const v2i& size, bool mipmap, CodecType type, Format format );
    BlockData( const v2i& size, bool mipmap, CodecType type );
    ~BlockData();

    BitmapPtr Decode();

    void Process( const uint32_t* src, uint32_t blocks, size_t offset, size_t width, bool dither, bool useHeuristics );
    void ProcessRGBA( const uint32_t* src, uint32_t blocks, size_t offset, size_t width, bool useHeuristics, const bc7enc_compress_block_params* params );

    const v2i& Size() const { return m_size; }

private:
    uint8_t* m_data;
    v2i m_size;
    size_t m_dataOffset;
    FILE* m_file;
    size_t m_maplen;
    CodecType m_type;
};

typedef std::shared_ptr<BlockData> BlockDataPtr;

#endif
