#include "TextureHeader.hpp"
#include <cassert>

void ProcessHeader(uint8_t* data, CodecType& type, int32_t& width, int32_t& height, size_t& dataOffset){
    auto data32 = (uint32_t*)data;
    if( *data32 == 0x03525650 )
    {
        // PVR
        switch( *(data32+2) )
        {
        case 6:
            type = Etc1;
            break;
        case 7:
            type = Bc1;
            break;
        case 11:
            type = Bc3;
            break;
        case 12:
            type = Bc4;
            break;
        case 13:
            type = Bc5;
            break;
        case 15:
            type = Bc7;
            break;
        case 22:
            type = Etc2_RGB;
            break;
        case 23:
            type = Etc2_RGBA;
            break;
        case 25:
            type = Etc2_R11;
            break;
        case 26:
            type = Etc2_RG11;
            break;
        default:
            assert( false );
            break;
        }

        height = *(data32+6);
        width = *(data32+7);
        dataOffset = 52 + *(data32+12);
    }
    else if( *data32 == 0x58544BAB )
    {
        // KTX
        switch( *(data32+7) )
        {
        case 0x9274:
            type = Etc2_RGB;
            break;
        case 0x9278:
            type = Etc2_RGBA;
            break;
        case 0x9270:
            type = Etc2_R11;
            break;
        case 0x9272:
            type = Etc2_RG11;
            break;
        default:
            assert( false );
            break;
        }

        width = *(data32+9);
        height = *(data32+10);
        dataOffset = sizeof( uint32_t ) * 17 + *(data32+15);
    }
    else if( *data32 == 0x20534444 )
    {
        // DDS
        switch( *(data32+21) )
        {
        case 0x31545844:
            type = Bc1;
            dataOffset = 128;
            break;
        case 0x35545844:
            type = Bc3;
            dataOffset = 128;
            break;
        case 0x30315844:
            dataOffset = 148;
            // DXGI_FORMAT_BCn
            switch( *(data32+32) )
            {
            case 71:
            case 72:
                type = Bc1;
                break;
            case 77:
            case 78:
                type = Bc3;
                break;
            case 80:
                type = Bc4;
                break;
            case 83:
                type = Bc5;
                break;
            case 98:
            case 99:
                type = Bc7;
                break;
            default:
                assert( false );
                break;
            };
            break;
        default:
            assert( false );
            break;
        };

        width = *(data32+4);
        height = *(data32+3);
    }
    else
    {
        assert( false );
    }
}
