#ifndef IMAGING_H
#define IMAGING_H

#define USETHREAD 1

#include "../templates/tarrays.h"
#include "../VM/vm.h"

namespace anl
{
typedef TArray2D<double> CArray2Dd;
typedef TArray3D<double> CArray3Dd;
typedef TArray2D<SRGBA> CArray2Drgba;
typedef TArray3D<SRGBA> CArray3Drgba;
enum EMappingModes
{
    SEAMLESS_NONE,
    SEAMLESS_X,
    SEAMLESS_Y,
    SEAMLESS_Z,
    SEAMLESS_XY,
    SEAMLESS_XZ,
    SEAMLESS_YZ,
    SEAMLESS_XYZ
};

struct SMappingRanges
{
    double mapx0,mapy0,mapz0, mapx1,mapy1,mapz1;
    double loopx0,loopy0,loopz0, loopx1,loopy1,loopz1;

    SMappingRanges()
    {
        mapx0=mapy0=mapz0=loopx0=loopy0=loopz0=-1;
        mapx1=mapy1=mapz1=loopx1=loopy1=loopz1=1;
    };

    SMappingRanges(SMappingRanges &rhs)
    {
        mapx0=rhs.mapx0;
        mapx1=rhs.mapx1;
        mapy0=rhs.mapy0;
        mapy1=rhs.mapy1;
        mapz0=rhs.mapz0;
        mapz1=rhs.mapz1;

        loopx0=rhs.loopx0;
        loopx1=rhs.loopx1;
        loopy0=rhs.loopy0;
        loopy1=rhs.loopy1;
        loopz0=rhs.loopz0;
        loopz1=rhs.loopz1;
    }

    SMappingRanges(const anl::SMappingRanges &rhs)
    {
        mapx0=rhs.mapx0;
        mapx1=rhs.mapx1;
        mapy0=rhs.mapy0;
        mapy1=rhs.mapy1;
        mapz0=rhs.mapz0;
        mapz1=rhs.mapz1;

        loopx0=rhs.loopx0;
        loopx1=rhs.loopx1;
        loopy0=rhs.loopy0;
        loopy1=rhs.loopy1;
        loopz0=rhs.loopz0;
        loopz1=rhs.loopz1;
    }

    SMappingRanges(double x0, double x1, double y0, double y1, double z0=0.0, double z1=1.0)
    {
        mapx0=x0;
        mapx1=x1;
        mapy0=y0;
        mapy1=y1;
        mapz0=z0;
        mapz1=z1;

        loopx0=x0;
        loopx1=x1;
        loopy0=y0;
        loopy1=y1;
        loopz0=z0;
        loopz1=z1;
    }

};

struct SChunk
{
    int seamlessmode;
    double *a;
    int awidth, aheight;
    int chunkheight, chunkyoffset;
    CKernel kernel;
    SMappingRanges ranges;
    CInstructionIndex at;
    double z;

    SChunk(CInstructionIndex a) : at(a) {}
};

struct SChunk3D
{
    int seamlessmode;
    double *a;
    int awidth, aheight, adepth;
    int chunkdepth, chunkzoffset;
    CKernel kernel;
    SMappingRanges ranges;
    CInstructionIndex at;

    SChunk3D(CInstructionIndex a) : at(a) {}
};

struct SRGBAChunk
{
    int seamlessmode;
    SRGBA *a;
    int awidth, aheight;
    int chunkheight, chunkyoffset;
    CKernel kernel;
    SMappingRanges ranges;
    double z;
    CInstructionIndex at;

    SRGBAChunk(CInstructionIndex a) : at(a) {}
};

void map2D(int seamlessmode, CArray2Dd &a, CKernel &k, SMappingRanges ranges, double z, CInstructionIndex at);
void map2DNoZ(int seamlessmode, CArray2Dd &a, CKernel &k, SMappingRanges ranges, CInstructionIndex at);
void map3D(int seamlessmode, CArray3Dd &a, CKernel &k, SMappingRanges ranges, CInstructionIndex index);

void mapRGBA2D(int seamlessmode, CArray2Drgba &a, CKernel &k, SMappingRanges ranges, double z, CInstructionIndex at);
void mapRGBA2DNoZ(int seamlessmode, CArray2Drgba &a, CKernel &k, SMappingRanges ranges, CInstructionIndex at);
void mapRGBA3D(int seamlessmode, CArray3Drgba &a, CKernel &k, SMappingRanges ranges, CInstructionIndex index);

void map2DChunk(SChunk chunk);
void map2DChunkNoZ(SChunk chunk);
void mapRGBA2DChunkNoZ(SRGBAChunk chunk);
void mapRGBA2DChunk(SRGBAChunk chunk);
void map3DChunk(SChunk3D chunk);

void calcNormalMap(CArray2Dd *map, CArray2Drgba *bump, float spacing, bool normalize, bool wrap);
void calcBumpMap(CArray2Dd *map, CArray2Dd *bump, float light[3], float spacing, bool wrap);

double highresTime();
};


#endif
