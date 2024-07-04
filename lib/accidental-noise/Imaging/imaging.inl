#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <thread>
#include <chrono>

namespace anl
{

void map2DChunk(SChunk chunk)
{
    static double pi2=3.141592*2.0;
    CNoiseExecutor m((chunk.kernel));
    SMappingRanges ranges=chunk.ranges;
    double z=chunk.z;

    for(int x=0; x<chunk.awidth; ++x)
    {
        for(int y=0; y<chunk.chunkheight; ++y)
        {
            int realy=y+chunk.chunkyoffset;
            int index=y*chunk.awidth+x;
            double p=(double) x/ (double)(chunk.awidth);
            double q=(double) realy/(double)(chunk.aheight);
            double r;
            double nx,ny,nz,nw,nu,nv,val=0.0;
            double dx, dy, dz;
            switch(chunk.seamlessmode)
            {
            case SEAMLESS_NONE:
            {
                nx=ranges.mapx0 + p*(ranges.mapx1-ranges.mapx0);
                ny=ranges.mapy0 + q*(ranges.mapy1-ranges.mapy0);
                nz=z;
                CCoordinate coord(nx,ny,nz);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_X:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.mapy0 + q*dy;
                nw=z;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_Y:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0 + p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nw=z;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_Z:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                nx=ranges.mapx0 + p*dx;
                ny=ranges.mapy0 + p*dx;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nz=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nw=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_XY:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nu=z;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outfloat_;

            }
            break;
            case SEAMLESS_XZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapx1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) *dx/pi2;
                nz=ranges.mapy0 + q*dy;
                nw=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_YZ:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0+p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nw=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_XYZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                nu=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nv=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,nv);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;

            default:
                break;
            }
            chunk.a[index]=val;
        }
    }
}

void map2DChunkNoZ(SChunk chunk)
{
    static double pi2=3.141592*2.0;
    CNoiseExecutor m((chunk.kernel));
    SMappingRanges ranges=chunk.ranges;
    double z=chunk.z;

    for(int x=0; x<chunk.awidth; ++x)
    {
        for(int y=0; y<chunk.chunkheight; ++y)
        {
            int realy=y+chunk.chunkyoffset;
            int index=y*chunk.awidth+x;
            double p=(double) x/ (double)(chunk.awidth);
            double q=(double) realy/(double)(chunk.aheight);
            double r;
            double nx,ny,nz,nw,nu,nv,val=0.0;
            double dx, dy, dz;
            switch(chunk.seamlessmode)
            {
            case SEAMLESS_NONE:
            {
                nx=ranges.mapx0 + p*(ranges.mapx1-ranges.mapx0);
                ny=ranges.mapy0 + q*(ranges.mapy1-ranges.mapy0);
                nz=z;
                CCoordinate coord(nx,ny);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_X:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.mapy0 + q*dy;

                CCoordinate coord(nx,ny,nz);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_Y:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0 + p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                CCoordinate coord(nx,ny,nz);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_Z:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                nx=ranges.mapx0 + p*dx;
                ny=ranges.mapy0 + p*dx;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nz=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nw=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_XY:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outfloat_;

            }
            break;
            case SEAMLESS_XZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapx1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) *dx/pi2;
                nz=ranges.mapy0 + q*dy;
                nw=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_YZ:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0+p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nw=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;
            case SEAMLESS_XYZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                nu=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nv=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,nv);
                val=m.evaluateAt(coord,chunk.at).outfloat_;
            }
            break;

            default:
                break;
            }
            chunk.a[index]=val;
        }
    }
}

double highresTime()
{
    using namespace std::chrono;
    high_resolution_clock::time_point t=high_resolution_clock::now();
    high_resolution_clock::duration d=t.time_since_epoch();
    return (double)d.count() * (double)high_resolution_clock::period::num / (double)high_resolution_clock::period::den;
}

void map2D(int seamlessmode, CArray2Dd &a, CKernel &k, SMappingRanges ranges, double z, CInstructionIndex at)
{
#ifndef USETHREAD
    SChunk chunk(at);
    chunk.seamlessmode=seamlessmode;
    chunk.a=a.getData();
    chunk.awidth=a.width();
    chunk.aheight=a.height();
    chunk.chunkheight=a.height();
    chunk.chunkyoffset=0;
    chunk.kernel=k;
    chunk.ranges=ranges;
    chunk.z=z;
    chunk.at=at;

    map2DChunk(chunk);
#else
    unsigned threadcount=std::thread::hardware_concurrency();
    int chunksize=std::floor(a.height() / threadcount);
    std::vector<std::thread> threads;

    for(unsigned int thread=0; thread<threadcount; ++thread)
    {
        SChunk chunk(at);
        chunk.seamlessmode=seamlessmode;
        double *arr=a.getData();
        int offsety=thread*chunksize;
        chunk.a=&arr[offsety*a.width()];
        chunk.awidth=a.width();
        chunk.aheight=a.height();
        if(thread==threadcount-1) chunk.chunkheight=a.height()-(chunksize*(threadcount-1));
        else chunk.chunkheight=chunksize;
        chunk.chunkyoffset=offsety;
        chunk.kernel=k;
        chunk.ranges=ranges;
        chunk.z=z;
        threads.push_back(std::thread(map2DChunk, chunk));
    }

    for(unsigned int c=0; c<threads.size(); ++c)
    {
        threads[c].join();
    }
#endif
}

void map2DNoZ(int seamlessmode, CArray2Dd &a, CKernel &k, SMappingRanges ranges, CInstructionIndex at)
{
#ifndef USETHREAD
    SChunk chunk(at);
    chunk.seamlessmode=seamlessmode;
    chunk.a=a.getData();
    chunk.awidth=a.width();
    chunk.aheight=a.height();
    chunk.chunkheight=a.height();
    chunk.chunkyoffset=0;
    chunk.kernel=k;
    chunk.ranges=ranges;
    chunk.z=0;
    chunk.at=at;

    map2DChunkNoZ(chunk);
#else
    unsigned threadcount=std::thread::hardware_concurrency();
    int chunksize=std::floor(a.height() / threadcount);
    std::vector<std::thread> threads;

    for(unsigned int thread=0; thread<threadcount; ++thread)
    {
        SChunk chunk(at);
        chunk.seamlessmode=seamlessmode;
        double *arr=a.getData();
        int offsety=thread*chunksize;
        chunk.a=&arr[offsety*a.width()];
        chunk.awidth=a.width();
        chunk.aheight=a.height();
        if(thread==threadcount-1) chunk.chunkheight=a.height()-(chunksize*(threadcount-1));
        else chunk.chunkheight=chunksize;
        chunk.chunkyoffset=offsety;
        chunk.kernel=k;
        chunk.ranges=ranges;
        chunk.z=0;
        threads.push_back(std::thread(map2DChunkNoZ, chunk));
    }

    for(unsigned int c=0; c<threads.size(); ++c)
    {
        threads[c].join();
    }
#endif
}

void map3DChunk(SChunk3D chunk)
{
    static double pi2=3.141592*2.0;
    CNoiseExecutor m((chunk.kernel));
    SMappingRanges ranges=chunk.ranges;

    for(int x=0; x<chunk.awidth; ++x)
    {
        for(int y=0; y<chunk.aheight; ++y)
        {
            for(int z=0; z<chunk.chunkdepth; ++z)
            {
                int realz=z+chunk.chunkzoffset;
                int index=(z*chunk.awidth*chunk.aheight) + y*chunk.awidth+x;
                double p=(double) x/ (double)(chunk.awidth);
                double q=(double) y/(double)(chunk.aheight);
                double r=(double) realz/(double)(chunk.adepth);
                double nx,ny,nz,nw,nu,nv,val=0.0;
                double dx, dy, dz;
                switch(chunk.seamlessmode)
                {
                case SEAMLESS_NONE:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.mapy0 + q*dy;
                    nz=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                case SEAMLESS_X:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.mapy0 + q*dy;
                    nw=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz,nw);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                case SEAMLESS_Y:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nz=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nw=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz,nw);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                case SEAMLESS_Z:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.mapy0 + q*dy;
                    nz=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nw=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                case SEAMLESS_XY:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nu=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz,nw,nu,0);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                case SEAMLESS_XZ:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.mapy0 + q*dy;
                    nw=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nu=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw,nu,0);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                case SEAMLESS_YZ:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nz=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nw=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nu=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw,nu,0);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                case SEAMLESS_XYZ:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nu=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nv=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw,nu,nv);
                    val=m.evaluateAt(coord,chunk.at).outfloat_;
                }
                break;
                default:
                    break;
                }
                chunk.a[index]=val;
            }
        }
    }
}

void map3D(int seamlessmode, CArray3Dd &a, CKernel &k, SMappingRanges ranges, CInstructionIndex index)
{
    if(a.getData()==0) return;
#ifndef USETHREAD
    SChunk3D chunk(index);
    chunk.seamlessmode=seamlessmode;
    chunk.a=a.getData();
    chunk.awidth=a.width();
    chunk.aheight=a.height();
    chunk.adepth=a.depth();
    chunk.chunkdepth=a.depth();
    chunk.chunkzoffset=0;
    chunk.kernel=k;
    chunk.ranges=ranges;
    chunk.at=index;

    map3DChunk(chunk);
#else
    unsigned threadcount=std::thread::hardware_concurrency();
    int chunksize=std::floor(a.depth() / threadcount);

    std::vector<std::thread> threads;


    for(unsigned int thread=0; thread<threadcount; ++thread)
    {
        SChunk3D chunk(index);
        chunk.seamlessmode=seamlessmode;
        double *arr=a.getData();
        int offsetz=thread*chunksize;
        chunk.a=&arr[offsetz*a.width()*a.height()];
        chunk.awidth=a.width();
        chunk.aheight=a.height();
        chunk.adepth=a.depth();
        if(thread==threadcount-1) chunk.chunkdepth=a.depth()-(chunksize*(threadcount-1));
        else chunk.chunkdepth=chunksize;
        chunk.chunkzoffset=offsetz;
        chunk.kernel=k;
        chunk.ranges=ranges;
        threads.push_back(std::thread(map3DChunk, chunk));
    }

    for(unsigned int c=0; c<threads.size(); ++c) threads[c].join();
#endif
}

void mapRGBA2DChunk(SRGBAChunk chunk)
{
    static double pi2=3.141592*2.0;
    CNoiseExecutor m((chunk.kernel));
    SMappingRanges ranges=chunk.ranges;
    double z=chunk.z;

    for(int x=0; x<chunk.awidth; ++x)
    {
        for(int y=0; y<chunk.chunkheight; ++y)
        {
            int realy=y+chunk.chunkyoffset;
            int index=y*chunk.awidth+x;
            double p=(double) x/ (double)(chunk.awidth);
            double q=(double) realy/(double)(chunk.aheight);
            double r;
            double nx,ny,nz,nw,nu,nv;
            SRGBA val;
            double dx, dy, dz;
            switch(chunk.seamlessmode)
            {
            case SEAMLESS_NONE:
            {
                nx=ranges.mapx0 + p*(ranges.mapx1-ranges.mapx0);
                ny=ranges.mapy0 + q*(ranges.mapy1-ranges.mapy0);
                nz=z;
                CCoordinate coord(nx,ny,nz);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_X:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.mapy0 + q*dy;
                nw=z;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_Y:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0 + p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nw=z;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_Z:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                nx=ranges.mapx0 + p*dx;
                ny=ranges.mapy0 + p*dx;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nz=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nw=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_XY:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nu=z;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outrgba_;

            }
            break;
            case SEAMLESS_XZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapx1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) *dx/pi2;
                nz=ranges.mapy0 + q*dy;
                nw=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_YZ:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0+p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nw=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_XYZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                nu=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nv=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,nv);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;

            default:
                break;
            }
            chunk.a[index]=val;
        }
    }
}

void mapRGBA2DChunkNoZ(SRGBAChunk chunk)
{
    static double pi2=3.141592*2.0;
    CNoiseExecutor m((chunk.kernel));
    SMappingRanges ranges=chunk.ranges;
    double z=chunk.z;

    for(int x=0; x<chunk.awidth; ++x)
    {
        for(int y=0; y<chunk.chunkheight; ++y)
        {
            int realy=y+chunk.chunkyoffset;
            int index=y*chunk.awidth+x;
            double p=(double) x/ (double)(chunk.awidth);
            double q=(double) realy/(double)(chunk.aheight);
            double r;
            double nx,ny,nz,nw,nu,nv;
            SRGBA val;
            double dx, dy, dz;
            switch(chunk.seamlessmode)
            {
            case SEAMLESS_NONE:
            {
                nx=ranges.mapx0 + p*(ranges.mapx1-ranges.mapx0);
                ny=ranges.mapy0 + q*(ranges.mapy1-ranges.mapy0);
                nz=z;
                CCoordinate coord(nx,ny);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_X:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.mapy0 + q*dy;

                CCoordinate coord(nx,ny,nz);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_Y:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0 + p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                CCoordinate coord(nx,ny,nz);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_Z:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                nx=ranges.mapx0 + p*dx;
                ny=ranges.mapy0 + p*dx;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nz=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nw=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_XY:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) * dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                CCoordinate coord(nx,ny,nz,nw);
                val=m.evaluateAt(coord,chunk.at).outrgba_;

            }
            break;
            case SEAMLESS_XZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.mapy1-ranges.mapy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapx1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                nx=ranges.loopx0 + cos(p*pi2) * dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2) *dx/pi2;
                nz=ranges.mapy0 + q*dy;
                nw=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_YZ:
            {
                dx=ranges.mapx1-ranges.mapx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                nx=ranges.mapx0+p*dx;
                ny=ranges.loopy0 + cos(q*pi2) * dy/pi2;
                nz=ranges.loopy0 + sin(q*pi2) * dy/pi2;
                nw=ranges.loopz0 + cos(zval*pi2) * dz/pi2;
                nu=ranges.loopz0 + sin(zval*pi2) * dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,0);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;
            case SEAMLESS_XYZ:
            {
                dx=ranges.loopx1-ranges.loopx0;
                dy=ranges.loopy1-ranges.loopy0;
                dz=ranges.loopz1-ranges.loopz0;
                p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                r=(z-ranges.mapz0)/(ranges.mapz1-ranges.mapz0);
                double zval=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                nu=ranges.loopz0 + cos(zval*pi2)*dz/pi2;
                nv=ranges.loopz0 + sin(zval*pi2)*dz/pi2;
                CCoordinate coord(nx,ny,nz,nw,nu,nv);
                val=m.evaluateAt(coord,chunk.at).outrgba_;
            }
            break;

            default:
                break;
            }
            //a.set(x,y,val);
            chunk.a[index]=val;
        }
    }
}


void mapRGBA2D(int seamlessmode, CArray2Drgba &a, CKernel &k, SMappingRanges ranges, double z, CInstructionIndex at)
{
#ifndef USETHREAD
    SRGBAChunk chunk(at);
    chunk.seamlessmode=seamlessmode;
    chunk.a=a.getData();
    chunk.awidth=a.width();
    chunk.aheight=a.height();
    chunk.chunkheight=a.height();
    chunk.chunkyoffset=0;
    chunk.kernel=k;
    chunk.ranges=ranges;
    chunk.z=z;
    chunk.at=at;

    mapRGBA2DChunk(chunk);
#else
    unsigned threadcount=std::thread::hardware_concurrency();
    int chunksize=std::floor(a.height() / threadcount);
    std::vector<std::thread> threads;

    for(unsigned int thread=0; thread<threadcount; ++thread)
    {
        SRGBAChunk chunk(at);
        chunk.seamlessmode=seamlessmode;
        SRGBA *arr=a.getData();
        int offsety=thread*chunksize;
        chunk.a=&arr[offsety*a.width()];
        chunk.awidth=a.width();
        chunk.aheight=a.height();
        if(thread==threadcount-1) chunk.chunkheight=a.height()-(chunksize*(threadcount-1));
        else chunk.chunkheight=chunksize;
        chunk.chunkyoffset=offsety;
        chunk.kernel=k;
        chunk.ranges=ranges;
        chunk.z=z;
        threads.push_back(std::thread(mapRGBA2DChunk, chunk));
    }

    for(unsigned int c=0; c<threads.size(); ++c)
    {
        threads[c].join();
    }
#endif
}

void mapRGBA2DNoZ(int seamlessmode, CArray2Drgba &a, CKernel &k, SMappingRanges ranges, CInstructionIndex at)
{
#ifndef USETHREAD
    SRGBAChunk chunk(at);
    chunk.seamlessmode=seamlessmode;
    chunk.a=a.getData();
    chunk.awidth=a.width();
    chunk.aheight=a.height();
    chunk.chunkheight=a.height();
    chunk.chunkyoffset=0;
    chunk.kernel=k;
    chunk.ranges=ranges;
    chunk.z=0;
    chunk.at=at;

    mapRGBA2DChunkNoZ(chunk);
#else
    unsigned threadcount=std::thread::hardware_concurrency();
    int chunksize=std::floor(a.height() / threadcount);
    std::vector<std::thread> threads;

    for(unsigned int thread=0; thread<threadcount; ++thread)
    {
        SRGBAChunk chunk(at);
        chunk.seamlessmode=seamlessmode;
        SRGBA *arr=a.getData();
        int offsety=thread*chunksize;
        chunk.a=&arr[offsety*a.width()];
        chunk.awidth=a.width();
        chunk.aheight=a.height();
        if(thread==threadcount-1) chunk.chunkheight=a.height()-(chunksize*(threadcount-1));
        else chunk.chunkheight=chunksize;
        chunk.chunkyoffset=offsety;
        chunk.kernel=k;
        chunk.ranges=ranges;
        chunk.z=0;
        threads.push_back(std::thread(mapRGBA2DChunkNoZ, chunk));
    }

    for(unsigned int c=0; c<threads.size(); ++c)
    {
        threads[c].join();
    }
#endif
}

struct SRGBAChunk3D
{
    int seamlessmode;
    SRGBA *a;
    int awidth, aheight, adepth;
    int chunkdepth, chunkzoffset;
    CKernel kernel;
    SMappingRanges ranges;
    CInstructionIndex at;

    SRGBAChunk3D(CInstructionIndex a) : at(a) {}
};

void mapRGBA3DChunk(SRGBAChunk3D chunk)
{
    static double pi2=3.141592*2.0;
    CNoiseExecutor m((chunk.kernel));
    SMappingRanges ranges=chunk.ranges;

    for(int x=0; x<chunk.awidth; ++x)
    {
        for(int y=0; y<chunk.aheight; ++y)
        {
            for(int z=0; z<chunk.chunkdepth; ++z)
            {
                int realz=z+chunk.chunkzoffset;
                int index=(z*chunk.awidth*chunk.aheight) + y*chunk.awidth+x;
                double p=(double) x/ (double)(chunk.awidth);
                double q=(double) y/(double)(chunk.aheight);
                double r=(double) realz/(double)(chunk.adepth);
                double nx,ny,nz,nw,nu,nv=0.0;
                SRGBA val;
                double dx, dy, dz;
                switch(chunk.seamlessmode)
                {
                case SEAMLESS_NONE:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.mapy0 + q*dy;
                    nz=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                case SEAMLESS_X:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.mapy0 + q*dy;
                    nw=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz,nw);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                case SEAMLESS_Y:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nz=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nw=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz,nw);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                case SEAMLESS_Z:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.mapy0 + q*dy;
                    nz=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nw=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                case SEAMLESS_XY:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.mapz1-ranges.mapz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nu=ranges.mapz0 + r*dz;
                    CCoordinate coord(nx,ny,nz,nw,nu,0);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                case SEAMLESS_XZ:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.mapy1-ranges.mapy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.mapy0 + q*dy;
                    nw=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nu=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw,nu,0);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                case SEAMLESS_YZ:
                {
                    dx=ranges.mapx1-ranges.mapx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.mapx0 + p*dx;
                    ny=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nz=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nw=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nu=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw,nu,0);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                case SEAMLESS_XYZ:
                {
                    dx=ranges.loopx1-ranges.loopx0;
                    dy=ranges.loopy1-ranges.loopy0;
                    dz=ranges.loopz1-ranges.loopz0;
                    p=p*(ranges.mapx1-ranges.mapx0)/(ranges.loopx1-ranges.loopx0);
                    q=q*(ranges.mapy1-ranges.mapy0)/(ranges.loopy1-ranges.loopy0);
                    r=r*(ranges.mapz1-ranges.mapz0)/(ranges.loopz1-ranges.loopz0);
                    nx=ranges.loopx0 + cos(p*pi2)*dx/pi2;
                    ny=ranges.loopx0 + sin(p*pi2)*dx/pi2;
                    nz=ranges.loopy0 + cos(q*pi2)*dy/pi2;
                    nw=ranges.loopy0 + sin(q*pi2)*dy/pi2;
                    nu=ranges.loopz0 + cos(r*pi2)*dz/pi2;
                    nv=ranges.loopz0 + sin(r*pi2)*dz/pi2;
                    CCoordinate coord(nx,ny,nz,nw,nu,nv);
                    val=m.evaluateAt(coord,chunk.at).outrgba_;
                }
                break;
                default:
                    break;
                }
                chunk.a[index]=val;
            }
        }
    }
}

void mapRGBA3D(int seamlessmode, CArray3Drgba &a, CKernel &k, SMappingRanges ranges, CInstructionIndex index)
{
    if(a.getData()==0) return;
#ifndef USETHREAD
    SRGBAChunk3D chunk(index);
    chunk.seamlessmode=seamlessmode;
    chunk.a=a.getData();
    chunk.awidth=a.width();
    chunk.aheight=a.height();
    chunk.adepth=a.depth();
    chunk.chunkdepth=a.depth();
    chunk.chunkzoffset=0;
    chunk.kernel=k;
    chunk.ranges=ranges;
    chunk.at=index;

    mapRGBA3DChunk(chunk);
#else
    unsigned threadcount=std::thread::hardware_concurrency();
    int chunksize=std::floor(a.depth() / threadcount);

    std::vector<std::thread> threads;


    for(unsigned int thread=0; thread<threadcount; ++thread)
    {
        SRGBAChunk3D chunk(index);
        chunk.seamlessmode=seamlessmode;
        SRGBA *arr=a.getData();
        int offsetz=thread*chunksize;
        chunk.a=&arr[offsetz*a.width()*a.height()];
        chunk.awidth=a.width();
        chunk.aheight=a.height();
        chunk.adepth=a.depth();
        if(thread==threadcount-1) chunk.chunkdepth=a.depth()-(chunksize*(threadcount-1));
        else chunk.chunkdepth=chunksize;
        chunk.chunkzoffset=offsetz;
        chunk.kernel=k;
        chunk.ranges=ranges;
        threads.push_back(std::thread(mapRGBA3DChunk, chunk));
    }

    for(unsigned int c=0; c<threads.size(); ++c)
    {
        threads[c].join();
    }
#endif
}

void normalizeVec3(float v[3])
{
    float len=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    v[0]/=len;
    v[1]/=len;
    v[2]/=len;
}

void calcNormalMap(CArray2Dd *map, CArray2Drgba *bump, float spacing, bool normalize, bool wrap)
{
    if(!map || !bump) return;
    int mw=map->width(), mh=map->height();
    if(mw!=bump->width() || mh!=bump->height()) bump->resize(mw,mh);

    for(int x=0; x<mw; ++x)
    {
        for(int y=0; y<mh; ++y)
        {
            float n[3]= {0.0, 1.0, 0.0};

            if(!wrap)
            {
                if(x==0 || y==0 || x==mw-1 || y==mh-1)
                {
                    n[0]=0.0;
                    n[2]=0.0;
                }
                else
                {
                    n[0]=(map->get(x-1,y)-map->get(x+1,y)) / spacing;
                    n[2]=(map->get(x,y-1)-map->get(x,y+1)) / spacing;
                }
                normalizeVec3(n);
            }
            else
            {
                int x1,x2,y1,y2;
                if(x==0) x1=mw-1;
                else x1=x-1;

                if(y==0) y1=mh-1;
                else y1=y-1;

                if(x==mw-1) x2=0;
                else x2=x+1;

                if(y==mh-1) y2=0;
                else y2=y+1;

                n[0]=(map->get(x1,y)-map->get(x2,y)) / spacing;
                n[2]=(map->get(x,y1)-map->get(x,y2)) / spacing;
                normalizeVec3(n);
            }
            if(normalize)
            {
                n[0]=n[0]*0.5 + 0.5;
                n[1]=n[1]*0.5 + 0.5;
                n[2]=n[2]*0.5 + 0.5;
            }
            bump->set(x,y,SRGBA((float)n[0], (float)n[2], (float)n[1], 1.0));
        }
    }
}

void calcBumpMap(CArray2Dd *map, CArray2Dd *bump, float light[3], float spacing, bool wrap)
{
    if(!map || !bump) return;
    int mw=map->width(), mh=map->height();
    if(mw!=bump->width() || mh!=bump->height()) bump->resize(mw,mh);
    normalizeVec3(light);

    for(int x=0; x<mw; ++x)
    {
        for(int y=0; y<mh; ++y)
        {
            float n[3]= {0.0, 1.0, 0.0};

            if(!wrap)
            {
                if(x==0 || y==0 || x==mw-1 || y==mh-1)
                {
                    n[0]=0.0;
                    n[2]=0.0;
                }
                else
                {
                    n[0]=(map->get(x-1,y)-map->get(x+1,y)) / spacing;
                    n[2]=(map->get(x,y-1)-map->get(x,y+1)) / spacing;
                }
                normalizeVec3(n);
            }
            else
            {
                int x1,x2,y1,y2;
                if(x==0) x1=mw-1;
                else x1=x-1;

                if(y==0) y1=mh-1;
                else y1=y-1;

                if(x==mw-1) x2=0;
                else x2=x+1;

                if(y==mh-1) y2=0;
                else y2=y+1;

                n[0]=(map->get(x1,y)-map->get(x2,y)) / spacing;
                n[2]=(map->get(x,y1)-map->get(x,y2)) / spacing;
                normalizeVec3(n);
            }
            float b = light[0]*n[0] + light[1]*n[1] + light[2]*n[2];
            if(b<0.0) b=0.0;
            if(b>1.0) b=1.0;
            bump->set(x,y,b);
        }
    }
}
};
