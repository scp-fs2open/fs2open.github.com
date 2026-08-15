#include <assert.h>
#include <string.h>

#include "bcdec.h"
#include "BlockData.hpp"
#include "ColorSpace.hpp"
#include "Debug.hpp"
#include "MipMap.hpp"
#include "mmap.hpp"
#include "ProcessRGB.hpp"
#include "ProcessDxtc.hpp"
#include "Tables.hpp"
#include "TaskDispatch.hpp"
#include "Decode.hpp"

#ifdef __ARM_NEON
#  include <arm_neon.h>
#endif

#if defined __AVX__ && !defined __SSE4_1__
#  define __SSE4_1__
#endif

#if defined __SSE4_1__ || defined __AVX2__ || defined _MSC_VER
#  ifdef _MSC_VER
#    include <intrin.h>
#    include <Windows.h>
#    define _bswap(x) _byteswap_ulong(x)
#    define _bswap64(x) _byteswap_uint64(x)
#  else
#    include <x86intrin.h>
#  endif
#endif

#ifndef _bswap
#  define _bswap(x) __builtin_bswap32(x)
#  define _bswap64(x) __builtin_bswap64(x)
#endif

static uint8_t table59T58H[8] = { 3,6,11,16,23,32,41,64 };

BlockData::BlockData( const char* fn )
    : m_file( fopen( fn, "rb" ) )
{
    assert( m_file );
    fseek( m_file, 0, SEEK_END );
    m_maplen = ftell( m_file );
    fseek( m_file, 0, SEEK_SET );
    m_data = (uint8_t*)mmap( nullptr, m_maplen, PROT_READ, MAP_SHARED, fileno( m_file ), 0 );
    ProcessHeader(m_data, m_type, m_size.x, m_size.y, m_dataOffset);
}

static void WritePvrHeader( uint32_t* dst, CodecType type, const v2i& size, int levels )
{
    *dst++ = 0x03525650;  // version
    *dst++ = 0;           // flags
    switch( type )        // pixelformat[0]
    {
    case CodecType::Etc1:
        *dst++ = 6;
        break;
    case CodecType::Etc2_RGB:
        *dst++ = 22;
        break;
    case CodecType::Etc2_RGBA:
        *dst++ = 23;
        break;
    case CodecType::Etc2_R11:
        *dst++ = 25;
        break;
    case CodecType::Etc2_RG11:
        *dst++ = 26;
        break;
    case CodecType::Bc1:
        *dst++ = 7;
        break;
    case CodecType::Bc3:
        *dst++ = 11;
        break;
    case CodecType::Bc4:
        *dst++ = 12;
        break;
    case CodecType::Bc5:
        *dst++ = 13;
        break;
    case CodecType::Bc7:
        *dst++ = 15;
        break;
    default:
        assert( false );
        break;
    }
    *dst++ = 0;           // pixelformat[1]
    *dst++ = 0;           // colourspace
    *dst++ = 0;           // channel type
    *dst++ = size.y;      // height
    *dst++ = size.x;      // width
    *dst++ = 1;           // depth
    *dst++ = 1;           // num surfs
    *dst++ = 1;           // num faces
    *dst++ = levels;      // mipmap count
    *dst++ = 0;           // metadata size
}

static void WriteDdsHeader( uint32_t* dst, CodecType type, const v2i& size, int levels )
{
    const uint32_t flags = levels == 1 ? 0x1007 : 0x21007;
    uint32_t pitch = size.x * size.y / 2;
    if( type == CodecType::Etc2_RGBA || type == CodecType::Bc3 || type == CodecType::Bc5 || type == CodecType::Bc7 || type == CodecType::Etc2_RG11 ) pitch *= 2;
    const uint32_t caps = levels == 1 ? 0x1000 : 0x401008;

    *dst++ = 0x20534444;  // magic
    *dst++ = 124;         // size
    *dst++ = flags;       // flags
    *dst++ = size.y;      // height
    *dst++ = size.x;      // width
    *dst++ = pitch;       // pitch
    *dst++ = 0;           // depth
    *dst++ = levels;      // mipmap count
    memset( dst, 0, 44 );
    dst += 11;
    *dst++ = 32;          // size
    *dst++ = 4;           // flags
    switch( type )
    {
    case CodecType::Bc1:
        memcpy( dst++, "DXT1", 4 );
        break;
    case CodecType::Bc3:
        memcpy( dst++, "DXT5", 4 );
        break;
    case CodecType::Bc4:
        memcpy( dst++, "DX10", 4 );
        break;
    case CodecType::Bc5:
        memcpy( dst++, "DX10", 4 );
        break;
    case CodecType::Bc7:
        memcpy( dst++, "DX10", 4 );
        break;
    default:
        assert( false );
        break;
    }
    memset( dst, 0, 20 );
    dst += 5;
    *dst++ = caps;
    memset( dst, 0, 16 );
    dst+= 4;

    if( type == CodecType::Bc1 || type == CodecType::Bc3 ) return;

    switch( type )
    {
    case CodecType::Bc4:
        *dst++ = 80; // DXGI_FORMAT_BC4_UNORM
        break;
    case CodecType::Bc5:
        *dst++ = 83; // DXGI_FORMAT_BC5_UNORM
        break;
    case CodecType::Bc7:
        *dst++ = 98; // DXGI_FORMAT_BC7_UNORM
        break;
    default:
        assert( false );
        break;
    }
    *dst++ = 3; // DXGI_FORMAT_DIMENSION_TEXTURE2D
    *dst++ = 0; // miscFlag
    *dst++ = 1; // arraySize
    *dst++ = 0; // miscFlags2
}

static uint8_t* OpenForWriting( const char* fn, size_t len, const v2i& size, FILE** f, int levels, CodecType type, BlockData::Format format )
{
    *f = fopen( fn, "wb+" );
    assert( *f );
    fseek( *f, len - 1, SEEK_SET );
    const char zero = 0;
    fwrite( &zero, 1, 1, *f );
    fseek( *f, 0, SEEK_SET );

    auto ret = (uint8_t*)mmap( nullptr, len, PROT_WRITE, MAP_SHARED, fileno( *f ), 0 );
    auto dst = (uint32_t*)ret;

    switch( format )
    {
    case BlockData::Pvr:
        WritePvrHeader( dst, type, size, levels );
        break;
    case BlockData::Dds:
        WriteDdsHeader( dst, type, size, levels );
        break;
    default:
        assert( false );
        break;
    }

    return ret;
}

static int AdjustSizeForMipmaps( const v2i& size, int levels )
{
    int len = 0;
    v2i current = size;
    for( int i=1; i<levels; i++ )
    {
        assert( current.x != 1 || current.y != 1 );
        current.x = std::max( 1, current.x / 2 );
        current.y = std::max( 1, current.y / 2 );
        len += ( ( current.x + 3 ) & ~3 ) * ( ( current.y + 3 ) & ~3 ) / 2;
    }
    assert( current.x == 1 && current.y == 1 );
    return len;
}

BlockData::BlockData( const char* fn, const v2i& size, bool mipmap, CodecType type, Format format )
    : m_size( size )
    , m_dataOffset( 52 )
    , m_maplen( m_size.x*m_size.y/2 )
    , m_type( type )
{
    assert( m_size.x%4 == 0 && m_size.y%4 == 0 );

    uint32_t cnt = m_size.x * m_size.y / 16;
    DBGPRINT( cnt << " blocks" );

    int levels = 1;

    if( mipmap )
    {
        levels = NumberOfMipLevels( size );
        DBGPRINT( "Number of mipmaps: " << levels );
        m_maplen += AdjustSizeForMipmaps( size, levels );
    }

    if( type == Etc2_RGBA || type == Bc3 || type == Bc5 || type == Bc7 || type == Etc2_RG11 ) m_maplen *= 2;

    switch( format )
    {
    case Pvr:
        m_dataOffset = 52;
        break;
    case Dds:
        m_dataOffset = 128;
        if( type == Bc4 || type == Bc5 || type == Bc7 ) m_dataOffset += 20;
        break;
    default:
        assert( false );
        break;
    }

    m_maplen += m_dataOffset;
    m_data = OpenForWriting( fn, m_maplen, m_size, &m_file, levels, type, format );
}

BlockData::BlockData( const v2i& size, bool mipmap, CodecType type )
    : m_size( size )
    , m_dataOffset( 52 )
    , m_file( nullptr )
    , m_maplen( m_size.x*m_size.y/2 )
    , m_type( type )
{
    assert( m_size.x%4 == 0 && m_size.y%4 == 0 );
    if( mipmap )
    {
        const int levels = NumberOfMipLevels( size );
        m_maplen += AdjustSizeForMipmaps( size, levels );
    }

    if( type == Etc2_RGBA || type == Bc3 || type == Bc5 || type == Bc7 || type == Etc2_RG11 ) m_maplen *= 2;

    m_maplen += m_dataOffset;
    m_data = new uint8_t[m_maplen];
}

BlockData::~BlockData()
{
    if( m_file )
    {
        munmap( m_data, m_maplen );
        fclose( m_file );
    }
    else
    {
        delete[] m_data;
    }
}

void BlockData::Process( const uint32_t* src, uint32_t blocks, size_t offset, size_t width, bool dither, bool useHeuristics )
{
    auto dst = ((uint64_t*)( m_data + m_dataOffset )) + offset;

    switch( m_type )
    {
    case Etc1:
        if( dither )
        {
            CompressEtc1RgbDither( src, dst, blocks, width );
        }
        else
        {
            CompressEtc1Rgb( src, dst, blocks, width );
        }
        break;
    case Etc2_RGB:
        CompressEtc2Rgb( src, dst, blocks, width, useHeuristics );
        break;
    case Etc2_R11:
        CompressEacR( src, dst, blocks, width );
        break;
    case Etc2_RG11:
        dst = ((uint64_t*)( m_data + m_dataOffset )) + offset * 2;
        CompressEacRg( src, dst, blocks, width );
        break;
    case Bc1:
        if( dither )
        {
            CompressBc1Dither( src, dst, blocks, width );
        }
        else
        {
            CompressBc1( src, dst, blocks, width );
        }
        break;
    case Bc4:
        CompressBc4( src, dst, blocks, width );
        break;
    case Bc5:
        dst = ((uint64_t*)( m_data + m_dataOffset )) + offset * 2;
        CompressBc5( src, dst, blocks, width );
        break;
    default:
        assert( false );
        break;
    }
}

void BlockData::ProcessRGBA( const uint32_t* src, uint32_t blocks, size_t offset, size_t width, bool useHeuristics, const bc7enc_compress_block_params* params )
{
    auto dst = ((uint64_t*)( m_data + m_dataOffset )) + offset * 2;

    switch( m_type )
    {
    case Etc2_RGBA:
        CompressEtc2Rgba( src, dst, blocks, width, useHeuristics );
        break;
    case Bc3:
        CompressBc3( src, dst, blocks, width );
        break;
    case Bc7:
        CompressBc7( src, dst, blocks, width, params );
        break;
    default:
        assert( false );
        break;
    }
}

BitmapPtr BlockData::Decode()
{
    auto ret = std::make_shared<Bitmap>(m_size);
    const uint64_t* src = (const uint64_t*)(m_data + m_dataOffset);

    switch( m_type )
    {
    case Etc1:
    case Etc2_RGB:
        ::DecodeRGB(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Etc2_RGBA:
        ::DecodeRGBA(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Etc2_R11:
        ::DecodeR(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Etc2_RG11:
        ::DecodeRG(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Bc1:
        ::DecodeBc1(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Bc3:
        ::DecodeBc3(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Bc4:
        ::DecodeBc4(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Bc5:
        ::DecodeBc5(src, ret->Data(), m_size.x, m_size.y);
        break;
    case Bc7:
        ::DecodeBc7(src, ret->Data(), m_size.x, m_size.y);
        break;
    default:
        assert( false );
        return nullptr;
    }
    return ret;
}
