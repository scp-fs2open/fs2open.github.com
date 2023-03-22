
#include <vector>
#include <algorithm>
#include <math.h>

namespace anl
{

double noInterp(double t)
{
    return 0;
}

double linearInterp(double t)
{
    return t;
}

double hermiteInterp(double t)
{
    return (t*t*(3-2*t));
}

double quinticInterp(double t)
{
    return t*t*t*(t*(t*6-15)+10);

}

inline int fast_floor(double t)
{
    return (t>0 ? (int)t : (int)t - 1);
}

inline double array_dot2(double *arr, double a, double b)
{
    return a*arr[0] + b*arr[1];
}

inline double array_dot3(double *arr, double a, double b, double c)
{
    return a*arr[0]+b*arr[1]+c*arr[2];
}

inline double array_dot4(double *arr, double x, double y, double z, double w)
{
    return x*arr[0]+y*arr[1]+z*arr[2]+w*arr[3];
}

inline double array_dot6(double *arr, double x, double y, double z, double w, double u, double v)
{
    return x*arr[0]+y*arr[1]+z*arr[2]+w*arr[3]+u*arr[4]+v*arr[5];
}

// Distance

double distEuclid2(double x1, double y1, double x2, double y2)
{
    double dx=x2-x1;
    double dy=y2-y1;
    return sqrt(dx*dx+dy*dy);
}

double distEuclid3(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double dx=x2-x1;
    double dy=y2-y1;
    double dz=z2-z1;
    return sqrt(dx*dx+dy*dy+dz*dz);
}

double distEuclid4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2)
{
    double dx=x2-x1;
    double dy=y2-y1;
    double dz=z2-z1;
    double dw=w2-w1;
    return sqrt(dx*dx+dy*dy+dz*dz+dw*dw);
}

double distEuclid6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2)
{
    double dx=x2-x1;
    double dy=y2-y1;
    double dz=z2-z1;
    double dw=w2-w1;
    double du=u2-u1;
    double dv=v2-v1;
    return sqrt(dx*dx+dy*dy+dz*dz+dw*dw+du*du+dv*dv);
}


double distManhattan2(double x1, double y1, double x2, double y2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    //return std::max(dx,dy);
    return dx+dy;
}

double distManhattan3(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    //return std::max(dz,std::max(dx,dy));
    return dx+dy+dz;
}

double distManhattan4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    double dw=fabs(w2-w1);
    //return std::max(dw,std::max(dz,std::max(dx,dy)));
    return dx+dy+dz+dw;
}

double distManhattan6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    double dw=fabs(w2-w1);
    double du=fabs(u2-u1);
    double dv=fabs(v2-v1);
    //return std::max(du,std::max(dv,std::max(dw,std::max(dz,std::max(dx,dy)))));
    return dx+dy+dz+dw+du+dv;
}

double distGreatestAxis2(double x1, double y1, double x2, double y2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    return std::max(dx,dy);
}

double distGreatestAxis3(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    return std::max(dz,std::max(dx,dy));
}

double distGreatestAxis4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    double dw=fabs(w2-w1);
    return std::max(dw,std::max(dz,std::max(dx,dy)));
}

double distGreatestAxis6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    double dw=fabs(w2-w1);
    double du=fabs(u2-u1);
    double dv=fabs(v2-v1);
    return std::max(du,std::max(dv,std::max(dw,std::max(dz,std::max(dx,dy)))));
}

double distLeastAxis2(double x1, double y1, double x2, double y2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    return std::min(dx,dy);
}

double distLeastAxis3(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    return std::min(dz,std::min(dx,dy));
}

double distLeastAxis4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    double dw=fabs(w2-w1);
    return std::min(dw,std::min(dz,std::min(dx,dy)));
}

double distLeastAxis6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2)
{
    double dx=fabs(x2-x1);
    double dy=fabs(y2-y1);
    double dz=fabs(z2-z1);
    double dw=fabs(w2-w1);
    double du=fabs(u2-u1);
    double dv=fabs(v2-v1);
    return std::min(du,std::min(dv,std::min(dw,std::min(dz,std::min(dx,dy)))));
}

unsigned int compute_hash_double_2(double x, double y, unsigned int seed)
{
    //double d[3]={x,y,(double)seed};
    //unsigned int s=sizeof(d) / sizeof(unsigned int);
    //return xor_fold_hash(fnv_32_a_buf(d, s));

    unsigned int hash=FNV_32_INIT;
    hash=fnv_32_a_combine(hash, (unsigned int)(x*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(y*(double)1000000));
    hash=fnv_32_a_combine(hash, seed);
    return xor_fold_hash(hash);
}


unsigned int compute_hash_double_3(double x, double y, double z, unsigned int seed)
{
    //double d[4]={x,y,z,(double)seed};
    //unsigned int s=sizeof(d) / sizeof(unsigned int);
    //return xor_fold_hash(fnv_32_a_buf(d, s));
    unsigned int hash=FNV_32_INIT;
    hash=fnv_32_a_combine(hash, (unsigned int)(x*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(y*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(z*(double)1000000));
    hash=fnv_32_a_combine(hash, seed);
    return xor_fold_hash(hash);

}

unsigned int compute_hash_double_4(double x, double y, double z, double w, unsigned int seed)
{
    //double d[5]={x,y,z,w,(double)seed};
    //unsigned int s=sizeof(d) / sizeof(unsigned int);
    //return xor_fold_hash(fnv_32_a_buf(d, s));
    unsigned int hash=FNV_32_INIT;
    hash=fnv_32_a_combine(hash, (unsigned int)(x*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(y*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(z*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(w*(double)1000000));
    hash=fnv_32_a_combine(hash, seed);
    return xor_fold_hash(hash);
}

unsigned int compute_hash_double_6(double x, double y, double z, double w, double u, double v, unsigned int seed)
{
    //double d[7]={x,y,z,w,u,v,(double)seed};
    //unsigned int s=sizeof(d) / sizeof(unsigned int);
    //return xor_fold_hash(fnv_32_a_buf(d, s));
    unsigned int hash=FNV_32_INIT;
    hash=fnv_32_a_combine(hash, (unsigned int)(x*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(y*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(z*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(w*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(u*(double)1000000));
    hash=fnv_32_a_combine(hash, (unsigned int)(v*(double)1000000));
    hash=fnv_32_a_combine(hash, seed);
    return xor_fold_hash(hash);
}



// Worker noise functions
typedef double (*worker_noise_2)(double, double, int, int, unsigned int);
typedef double (*worker_noise_3)(double, double, double, int, int, int, unsigned int);
typedef double (*worker_noise_4)(double,double,double,double,int,int,int,int,unsigned int);
typedef double (*worker_noise_6)(double,double,double,double,double,double,int,int,int,int,int,int,unsigned int);


double value_noise_2( double x, double y, int ix, int iy, unsigned int seed)
{
    unsigned int n = (hash_coords_2(ix,iy,seed)) % 256;
    double noise = (double)n / 255.0;
    return noise*2.0 - 1.0;
}
double value_noise_3(double x, double y, double z, int ix, int iy, int iz, unsigned int seed)
{
    unsigned int n = (hash_coords_3(ix,iy,iz,seed)) % 256;
    double noise=(double)n/(255.0);
    return noise*2.0-1.0;
}
double value_noise_4(double x, double y, double z, double w, int ix, int iy,int iz,int iw, unsigned int seed)
{
    unsigned int n = (hash_coords_4(ix,iy,iz,iw,seed)) % 256;
    double noise = (double)n / 255.0;
    return noise*2.0 - 1.0;
}
double value_noise_6(double x, double y, double z, double w, double u, double v, int ix, int iy,int iz,int iw, int iu, int iv, unsigned int seed)
{
    unsigned int n = (hash_coords_6(ix,iy,iz,iw,iu,iv,seed)) % 256;
    double noise = (double)n / 255.0;
    return noise*2.0 - 1.0;
}

double grad_noise_2(double x, double y, int ix, int iy, unsigned int seed)
{
    unsigned int hash=hash_coords_2(ix,iy,seed)%8;
    double *vec=&gradient2D_lut[hash][0];

    double dx=x-(double)ix;
    double dy=y-(double)iy;

    return (dx*vec[0] + dy*vec[1]);
}

double grad_noise_3(double x, double y, double z, int ix, int iy, int iz, unsigned int seed)
{
    unsigned int hash=hash_coords_3(ix,iy,iz,seed)%12;
    double *vec=&gradient3D_lut[hash][0];

    double dx=x-(double)ix;
    double dy=y-(double)iy;
    double dz=z-(double)iz;
    return (dx*vec[0]+dy*vec[1]+dz*vec[2]);
}

double grad_noise_4(double x, double y, double z, double w, int ix, int iy, int iz, int iw, unsigned int seed)
{
    unsigned int hash=hash_coords_4(ix,iy,iz,iw,seed)%64;
    double *vec=&gradient4D_lut[hash][0];

    double dx=x-(double)ix;
    double dy=y-(double)iy;
    double dz=z-(double)iz;
    double dw=w-(double)iw;

    return (dx*vec[0] + dy*vec[1] + dz*vec[2] + dw*vec[3]);

}

double grad_noise_6(double x, double y, double z, double w, double u, double v, int ix, int iy, int iz, int iw, int iu, int iv, unsigned int seed)
{
    unsigned int hash=hash_coords_6(ix,iy,iz,iw,iu,iv,seed)%192;
    double *vec=&gradient6D_lut[hash][0];

    double dx=x-(double)ix;
    double dy=y-(double)iy;
    double dz=z-(double)iz;
    double dw=w-(double)iw;
    double du=u-(double)iu;
    double dv=v-(double)iv;

    return (dx*vec[0] + dy*vec[1] + dz*vec[2] + dw*vec[3] + du*vec[4] + dv*vec[5]);

}



// Edge/Face/Cube/Hypercube interpolation
double lerp(double s, double v1, double v2)
{
    return v1+s*(v2-v1);
}

double interp_X_2(double x, double y, double xs, int x0, int x1, int iy, unsigned int seed, worker_noise_2 noisefunc)
{
    double v1=noisefunc(x,y,x0,iy,seed);
    double v2=noisefunc(x,y,x1,iy,seed);
    return lerp(xs,v1,v2);
}

double interp_XY_2(double x, double y, double xs, double ys, int x0, int x1, int y0, int y1, unsigned int seed, worker_noise_2 noisefunc)
{
    double v1=interp_X_2(x,y,xs,x0,x1,y0,seed,noisefunc);
    double v2=interp_X_2(x,y,xs,x0,x1,y1,seed,noisefunc);
    return lerp(ys,v1,v2);
}

double interp_X_3(double x, double y, double z, double xs, int x0, int x1, int iy, int iz, unsigned int seed, worker_noise_3 noisefunc)
{
    double v1=noisefunc(x,y,z,x0,iy,iz,seed);
    double v2=noisefunc(x,y,z,x1,iy,iz,seed);
    return lerp(xs,v1,v2);
}

double interp_XY_3(double x, double y, double z, double xs, double ys, int x0, int x1, int y0, int y1, int iz, unsigned int seed, worker_noise_3 noisefunc)
{
    double v1=interp_X_3(x,y,z,xs,x0,x1,y0,iz,seed,noisefunc);
    double v2=interp_X_3(x,y,z,xs,x0,x1,y1,iz,seed,noisefunc);
    return lerp(ys,v1,v2);
}

double interp_XYZ_3(double x, double y, double z, double xs, double ys, double zs, int x0, int x1, int y0, int y1, int z0, int z1, unsigned int seed, worker_noise_3 noisefunc)
{
    double v1=interp_XY_3(x,y,z,xs,ys,x0,x1,y0,y1,z0,seed,noisefunc);
    double v2=interp_XY_3(x,y,z,xs,ys,x0,x1,y0,y1,z1,seed,noisefunc);
    return lerp(zs,v1,v2);
}

double interp_X_4(double x, double y, double z, double w, double xs, int x0, int x1, int iy, int iz, int iw, unsigned int seed, worker_noise_4 noisefunc)
{
    double v1=noisefunc(x,y,z,w,x0,iy,iz,iw,seed);
    double v2=noisefunc(x,y,z,w,x1,iy,iz,iw,seed);
    return lerp(xs,v1,v2);
}

double interp_XY_4(double x, double y, double z, double w, double xs, double ys, int x0, int x1, int y0, int y1, int iz, int iw, unsigned int seed, worker_noise_4 noisefunc)
{
    double v1=interp_X_4(x,y,z,w,xs,x0,x1,y0,iz,iw,seed,noisefunc);
    double v2=interp_X_4(x,y,z,w,xs,x0,x1,y1,iz,iw,seed,noisefunc);
    return lerp(ys,v1,v2);
}

double interp_XYZ_4(double x, double y, double z, double w, double xs, double ys, double zs, int x0, int x1, int y0, int y1, int z0, int z1, int iw, unsigned int seed, worker_noise_4 noisefunc)
{
    double v1=interp_XY_4(x,y,z,w,xs,ys,x0,x1,y0,y1,z0,iw,seed,noisefunc);
    double v2=interp_XY_4(x,y,z,w,xs,ys,x0,x1,y0,y1,z1,iw,seed,noisefunc);
    return lerp(zs,v1,v2);
}

double interp_XYZW_4(double x, double y, double z, double w, double xs, double ys, double zs, double ws, int x0, int x1, int y0, int y1, int z0, int z1, int w0, int w1, unsigned int seed, worker_noise_4 noisefunc)
{
    double v1=interp_XYZ_4(x,y,z,w,xs,ys,zs,x0,x1,y0,y1,z0,z1,w0,seed,noisefunc);
    double v2=interp_XYZ_4(x,y,z,w,xs,ys,zs,x0,x1,y0,y1,z0,z1,w1,seed,noisefunc);
    return lerp(ws,v1,v2);
}


double interp_X_6(double x, double y, double z, double w, double u, double v, double xs, int x0, int x1, int iy, int iz, int iw, int iu, int iv, unsigned int seed, worker_noise_6 noisefunc)
{
    double v1=noisefunc(x,y,z,w,u,v,x0,iy,iz,iw,iu,iv,seed);
    double v2=noisefunc(x,y,z,w,u,v,x1,iy,iz,iw,iu,iv,seed);
    return lerp(xs,v1,v2);
}

double interp_XY_6(double x, double y, double z, double w, double u, double v, double xs, double ys, int x0, int x1, int y0, int y1, int iz, int iw, int iu, int iv, unsigned int seed, worker_noise_6 noisefunc)
{
    double v1=interp_X_6(x,y,z,w,u,v,xs, x0,x1,y0,iz,iw,iu,iv,seed,noisefunc);
    double v2=interp_X_6(x,y,z,w,u,v,xs,x0,x1,y1,iz,iw,iu,iv,seed,noisefunc);
    return lerp(ys,v1,v2);
}

double interp_XYZ_6(double x, double y, double z, double w, double u, double v, double xs, double ys, double zs, int x0, int x1, int y0, int y1, int z0, int z1, int iw, int iu, int iv, unsigned int seed, worker_noise_6 noisefunc)
{
    double v1=interp_XY_6(x,y,z,w,u,v,xs,ys,x0,x1,y0,y1,z0,iw,iu,iv,seed,noisefunc);
    double v2=interp_XY_6(x,y,z,w,u,v,xs,ys,x0,x1,y0,y1,z1,iw,iu,iv,seed,noisefunc);
    return lerp(zs,v1,v2);
}

double interp_XYZW_6(double x, double y, double z, double w, double u, double v, double xs, double ys, double zs, double ws, int x0, int x1, int y0, int y1, int z0, int z1, int w0, int w1, int iu, int iv, unsigned int seed,worker_noise_6 noisefunc)
{
    double v1=interp_XYZ_6(x,y,z,w,u,v,xs,ys,zs,x0,x1,y0,y1,z0,z1,w0,iu,iv,seed,noisefunc);
    double v2=interp_XYZ_6(x,y,z,w,u,v,xs,ys,zs,x0,x1,y0,y1,z0,z1,w1,iu,iv,seed,noisefunc);
    return lerp(ws,v1,v2);
}

double interp_XYZWU_6(double x, double y, double z, double w, double u, double v, double xs, double ys, double zs, double ws, double us, int x0, int x1, int y0, int y1, int z0, int z1, int w0, int w1, int u0, int u1, int iv, unsigned int seed, worker_noise_6 noisefunc)
{
    double v1=interp_XYZW_6(x,y,z,w,u,v,xs,ys,zs,ws,x0,x1,y0,y1,z0,z1,w0,w1,u0,iv,seed,noisefunc);
    double v2=interp_XYZW_6(x,y,z,w,u,v,xs,ys,zs,ws,x0,x1,y0,y1,z0,z1,w0,w1,u1,iv,seed,noisefunc);
    return lerp(us,v1,v2);
}

double interp_XYZWUV_6(double x, double y, double z, double w, double u, double v, double xs, double ys, double zs, double ws, double us, double vs, int x0, int x1, int y0, int y1, int z0, int z1, int w0, int w1, int u0, int u1, int v0, int v1, unsigned int seed, worker_noise_6 noisefunc)
{
    double val1=interp_XYZWU_6(x,y,z,w,u,v,xs,ys,zs,ws,us,x0,x1,y0,y1,z0,z1,w0,w1,u0,u1,v0,seed,noisefunc);
    double val2=interp_XYZWU_6(x,y,z,w,u,v,xs,ys,zs,ws,us,x0,x1,y0,y1,z0,z1,w0,w1,u0,u1,v1,seed,noisefunc);
    return lerp(vs,val1,val2);
}



// The usable noise functions

double value_noise2D(double x, double y, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);

    int x1=x0+1;
    int y1=y0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));

    return interp_XY_2(x,y,xs,ys,x0,x1,y0,y1,seed,value_noise_2);
}

double value_noise3D(double x, double y, double z, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);
    int z0=fast_floor(z);
    int x1=x0+1;
    int y1=y0+1;
    int z1=z0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));
    double zs=interp((z-(double)z0));

    return interp_XYZ_3(x,y,z,xs,ys,zs,x0,x1,y0,y1,z0,z1,seed,value_noise_3);
}

double value_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);
    int z0=fast_floor(z);
    int w0=fast_floor(w);

    int x1=x0+1;
    int y1=y0+1;
    int z1=z0+1;
    int w1=w0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));
    double zs=interp((z-(double)z0));
    double ws=interp((w-(double)w0));

    return interp_XYZW_4(x,y,z,w,xs,ys,zs,ws,x0,x1,y0,y1,z0,z1,w0,w1,seed,value_noise_4);
}

double value_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);
    int z0=fast_floor(z);
    int w0=fast_floor(w);
    int u0=fast_floor(u);
    int v0=fast_floor(v);

    int x1=x0+1;
    int y1=y0+1;
    int z1=z0+1;
    int w1=w0+1;
    int u1=u0+1;
    int v1=v0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));
    double zs=interp((z-(double)z0));
    double ws=interp((w-(double)w0));
    double us=interp((u-(double)u0));
    double vs=interp((v-(double)v0));

    return interp_XYZWUV_6(x,y,z,w,u,v,xs,ys,zs,ws,us,vs,x0,x1,y0,y1,z0,z1,w0,w1,u0,u1,v0,v1,seed,value_noise_6);
}

double gradient_noise2D(double x, double y, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);

    int x1=x0+1;
    int y1=y0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));

    return interp_XY_2(x,y,xs,ys,x0,x1,y0,y1,seed,grad_noise_2);
}

double gradient_noise3D(double x, double y, double z, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);
    int z0=fast_floor(z);

    int x1=x0+1;
    int y1=y0+1;
    int z1=z0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));
    double zs=interp((z-(double)z0));

    return interp_XYZ_3(x,y,z,xs,ys,zs,x0,x1,y0,y1,z0,z1,seed,grad_noise_3);
}

double gradient_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);
    int z0=fast_floor(z);
    int w0=fast_floor(w);

    int x1=x0+1;
    int y1=y0+1;
    int z1=z0+1;
    int w1=w0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));
    double zs=interp((z-(double)z0));
    double ws=interp((w-(double)w0));

    return interp_XYZW_4(x,y,z,w,xs,ys,zs,ws,x0,x1,y0,y1,z0,z1,w0,w1,seed,grad_noise_4);
}
double gradient_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp)
{
    int x0=fast_floor(x);
    int y0=fast_floor(y);
    int z0=fast_floor(z);
    int w0=fast_floor(w);
    int u0=fast_floor(u);
    int v0=fast_floor(v);

    int x1=x0+1;
    int y1=y0+1;
    int z1=z0+1;
    int w1=w0+1;
    int u1=u0+1;
    int v1=v0+1;

    double xs=interp((x-(double)x0));
    double ys=interp((y-(double)y0));
    double zs=interp((z-(double)z0));
    double ws=interp((w-(double)w0));
    double us=interp((u-(double)u0));
    double vs=interp((v-(double)v0));

    return interp_XYZWUV_6(x,y,z,w,u,v,xs,ys,zs,ws,us,vs,x0,x1,y0,y1,z0,z1,w0,w1,u0,u1,v0,v1,seed,grad_noise_6);
}

double gradval_noise2D(double x, double y, unsigned int seed, interp_func interp)
{
    return value_noise2D(x,y,seed,interp)+gradient_noise2D(x,y,seed,interp);
}

double gradval_noise3D(double x, double y, double z, unsigned int seed, interp_func interp)
{
    return value_noise3D(x,y,z,seed,interp)+gradient_noise3D(x,y,z,seed,interp);
}

double gradval_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp)
{
    return value_noise4D(x,y,z,w,seed,interp)+gradient_noise4D(x,y,z,w,seed,interp);
}

double gradval_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp)
{
    return value_noise6D(x,y,z,w,u,v,seed,interp)+gradient_noise6D(x,y,z,w,u,v,seed,interp);
}

double white_noise2D(double x, double y, unsigned int seed, interp_func interp)
{
    unsigned char hash=compute_hash_double_2(x,y,seed);
    return whitenoise_lut[hash];
}

double white_noise3D(double x, double y, double z, unsigned int seed, interp_func interp)
{
    unsigned char hash=compute_hash_double_3(x,y,z,seed);
    //std::cout << (unsigned int)hash << std::endl;
    return whitenoise_lut[hash];
}

double white_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp)
{
    unsigned char hash=compute_hash_double_4(x,y,z,w,seed);
    return whitenoise_lut[hash];
}

double white_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp)
{
    unsigned char hash=compute_hash_double_6(x,y,z,w,u,v,seed);
    return whitenoise_lut[hash];
}

void add_dist(double *f, double *disp, double testdist, double testdisp)
{
    int index;
    // Compare the given distance to the ones already in f
    if(testdist < f[3])
    {
        index=3;
        while(index>0 && testdist<f[index-1]) index--;
        for(int i=3; i-->index;)
        {
            f[i+1]=f[i];
            disp[i+1]=disp[i];
        }
        f[index]=testdist;
        disp[index]=testdisp;
    }
}


// Cellular functions. Compute distance (for cellular modules) and displacement (for voronoi modules)



void cellular_function2D(double x, double y, unsigned int seed, double *f, double *disp, dist_func2 distance)
{
    int xint = fast_floor(x);
    int yint = fast_floor(y);

    for(int c=0; c<4; ++c)
    {
        f[c]=99999.0;
        disp[c]=0.0;
    }

    {
        for(int ycur=yint-3; ycur<=yint+3; ++ycur)
        {
            for(int xcur=xint-3; xcur<=xint+3; ++xcur)
            {
                double xpos = (double)xcur + value_noise_2(x,y,xcur,ycur,seed);
                double ypos = (double)ycur + value_noise_2(x,y,xcur,ycur,seed+1);
                //double xdist=xpos-x;
                //double ydist=ypos-y;
                //double dist=(xdist*xdist + ydist*ydist);
                double dist=distance(xpos,ypos,x,y);
                int xval=fast_floor(xpos);
                int yval=fast_floor(ypos);
                double dsp=value_noise_2(x,y,xval,yval,seed+3);
                add_dist(f,disp, dist, dsp);
            }
        }
    }
}

void cellular_function3D(double x, double y, double z, unsigned int seed, double *f, double *disp, dist_func3 distance)
{
    int xint = fast_floor(x);
    int yint = fast_floor(y);
    int zint = fast_floor(z);

    for(int c=0; c<4; ++c)
    {
        f[c]=99999.0;
        disp[c]=0.0;
    }

    for (int zcur=zint-2; zcur<=zint+2; ++zcur)
    {
        for(int ycur=yint-2; ycur<=yint+2; ++ycur)
        {
            for(int xcur=xint-2; xcur<=xint+2; ++xcur)
            {
                double xpos = (double)xcur + value_noise_3(x,y,z,xcur,ycur,zcur,seed);
                double ypos = (double)ycur + value_noise_3(x,y,z,xcur,ycur,zcur,seed+1);
                double zpos = (double)zcur + value_noise_3(x,y,z,xcur,ycur,zcur,seed+2);
                //double xdist=xpos-x;
                //double ydist=ypos-y;
                //double zdist=zpos-z;
                //double dist=(xdist*xdist + ydist*ydist + zdist*zdist);
                double dist=distance(xpos,ypos,zpos,x,y,z);
                int xval=fast_floor(xpos);
                int yval=fast_floor(ypos);
                int zval=fast_floor(zpos);
                double dsp=value_noise_3(x,y,z,xval,yval,zval,seed+3);
                add_dist(f,disp, dist, dsp);
            }
        }
    }
}

void cellular_function4D(double x, double y, double z, double w, unsigned int seed, double *f, double *disp, dist_func4 distance)
{
    int xint = fast_floor(x);
    int yint = fast_floor(y);
    int zint = fast_floor(z);
    int wint = fast_floor(w);

    for(int c=0; c<4; ++c)
    {
        f[c]=99999.0;
        disp[c]=0.0;
    }

    for (int wcur=wint-2; wcur<=wint+2; ++wcur)
    {
        for (int zcur=zint-2; zcur<=zint+2; ++zcur)
        {
            for(int ycur=yint-2; ycur<=yint+2; ++ycur)
            {
                for(int xcur=xint-2; xcur<=xint+2; ++xcur)
                {
                    double xpos = (double)xcur + value_noise_4(x,y,z,w,xcur,ycur,zcur,wcur,seed);
                    double ypos = (double)ycur + value_noise_4(x,y,z,w,xcur,ycur,zcur,wcur,seed+1);
                    double zpos = (double)zcur + value_noise_4(x,y,z,w,xcur,ycur,zcur,wcur,seed+2);
                    double wpos = (double)wcur + value_noise_4(x,y,z,w,xcur,ycur,zcur,wcur,seed+3);
                    //double xdist=xpos-x;
                    //double ydist=ypos-y;
                    //double zdist=zpos-z;
                    //double wdist=wpos-w;
                    //double dist=(xdist*xdist + ydist*ydist + zdist*zdist + wdist*wdist);
                    double dist=distance(xpos,ypos,zpos,wpos,x,y,z,w);
                    int xval=fast_floor(xpos);
                    int yval=fast_floor(ypos);
                    int zval=fast_floor(zpos);
                    int wval=fast_floor(wpos);
                    double dsp=value_noise_4(x,y,z,w,xval,yval,zval,wval,seed+3);
                    add_dist(f,disp, dist, dsp);
                }
            }
        }
    }
}

void cellular_function6D(double x, double y, double z, double w, double u, double v, unsigned int seed, double *f, double *disp, dist_func6 distance)
{
    int xint = fast_floor(x);
    int yint = fast_floor(y);
    int zint = fast_floor(z);
    int wint = fast_floor(w);
    int uint = fast_floor(u);
    int vint = fast_floor(v);

    for(int c=0; c<4; ++c)
    {
        f[c]=99999.0;
        disp[c]=0.0;
    }

    for (int vcur=vint-1; vcur<=vint+1; ++vcur)
    {
        for(int ucur=uint-1; ucur<=uint+1; ++ucur)
        {

            for (int wcur=wint-2; wcur<=wint+2; ++wcur)
            {
                for (int zcur=zint-2; zcur<=zint+2; ++zcur)
                {
                    for(int ycur=yint-2; ycur<=yint+2; ++ycur)
                    {
                        for(int xcur=xint-2; xcur<=xint+2; ++xcur)
                        {
                            double xpos = (double)xcur + value_noise_6(x,y,z,w,u,v,xcur,ycur,zcur,wcur,ucur,vcur,seed);
                            double ypos = (double)ycur + value_noise_6(x,y,z,w,u,v,xcur,ycur,zcur,wcur,ucur,vcur,seed+1);
                            double zpos = (double)zcur + value_noise_6(x,y,z,w,u,v,xcur,ycur,zcur,wcur,ucur,vcur,seed+2);
                            double wpos = (double)wcur + value_noise_6(x,y,z,w,u,v,xcur,ycur,zcur,wcur,ucur,vcur,seed+3);
                            double upos = (double)ucur + value_noise_6(x,y,z,w,u,v,xcur,ycur,zcur,wcur,ucur,vcur,seed+4);
                            double vpos = (double)vcur + value_noise_6(x,y,z,w,u,v,xcur,ycur,zcur,wcur,ucur,vcur,seed+5);
                            //double xdist=xpos-x;
                            //double ydist=ypos-y;
                            //double zdist=zpos-z;
                            //double wdist=wpos-w;
                            //double udist=upos-u;
                            //double vdist=vpos-v;
                            //double dist=(xdist*xdist + ydist*ydist + zdist*zdist + wdist*wdist + udist*udist + vdist*vdist);
                            double dist=distance(xpos,ypos,zpos,wpos,upos,vpos,x,y,z,w,u,v);
                            int xval=fast_floor(xpos);
                            int yval=fast_floor(ypos);
                            int zval=fast_floor(zpos);
                            int wval=fast_floor(wpos);
                            int uval=fast_floor(upos);
                            int vval=fast_floor(vpos);
                            double dsp=value_noise_6(x,y,z,w,u,v,xval,yval,zval,wval,uval,vval,seed+6);
                            add_dist(f,disp, dist, dsp);
                        }
                    }
                }
            }
        }
    }
}


const double F2=0.36602540378443864676372317075294;
const double G2=0.21132486540518711774542560974902;
const double F3=1.0/3.0;
const double G3=1.0/6.0;


double simplex_noise2D(double x, double y, unsigned int seed, interp_func interp)
{
    double s = (x+y)*F2;
    int i=fast_floor(x+s);
    int j=fast_floor(y+s);

    double t = (i+j)*G2;
    double X0=i-t;
    double Y0=j-t;
    double x0=x-X0;
    double y0=y-Y0;

    int i1,j1;
    if(x0>y0)
    {
        i1=1;
        j1=0;
    }
    else
    {
        i1=0;
        j1=1;
    }

    double x1=x0-(double)i1+G2;
    double y1=y0-(double)j1+G2;
    double x2=x0-1.0+2.0*G2;
    double y2=y0-1.0+2.0*G2;

    // Hash the triangle coordinates to index the gradient table
    unsigned int h0=hash_coords_2(i,j,seed)%8;
    unsigned int h1=hash_coords_2(i+i1,j+j1,seed)%8;
    unsigned int h2=hash_coords_2(i+1,j+1,seed)%8;

    // Now, index the tables
    double *g0 = &gradient2D_lut[h0][0];
    double *g1 = &gradient2D_lut[h1][0];
    double *g2 = &gradient2D_lut[h2][0];

    double n0,n1,n2;
    // Calculate the contributions from the 3 corners
    double t0=0.5-x0*x0-y0*y0;
    if(t0<0) n0=0;
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * array_dot2(g0,x0,y0);
    }

    double t1=0.5-x1*x1-y1*y1;
    if(t1<0) n1=0;
    else
    {
        t1*=t1;
        n1=t1*t1*array_dot2(g1,x1,y1);
    }

    double t2=0.5-x2*x2-y2*y2;
    if(t2<0) n2=0;
    else
    {
        t2*=t2;
        n2=t2*t2*array_dot2(g2,x2,y2);
    }

    // Add contributions together
    return (40.0 * (n0+n1+n2));
}


double simplex_noise3D(double x, double y, double z, unsigned int seed, interp_func interp)
{
    //static double F3 = 1.0/3.0;
    //static double G3 = 1.0/6.0;
    double n0,n1,n2,n3;

    double s=(x+y+z)*F3;
    int i=fast_floor(x+s);
    int j=fast_floor(y+s);
    int k=fast_floor(z+s);

    double t=(i+j+k)*G3;
    double X0=i-t;
    double Y0=j-t;
    double Z0=k-t;

    double x0=x-X0;
    double y0=y-Y0;
    double z0=z-Z0;

    int i1, j1, k1;
    int i2, j2, k2;

    if(x0>=y0)
    {
        if(y0>=z0)
        {
            i1=1;
            j1=0;
            k1=0;
            i2=1;
            j2=1;
            k2=0;
        }
        else if(x0>=z0)
        {
            i1=1;
            j1=0;
            k1=0;
            i2=1;
            j2=0;
            k2=1;
        }
        else
        {
            i1=0;
            j1=0;
            k1=1;
            i2=1;
            j2=0;
            k2=1;
        }
    }
    else
    {
        if(y0<z0)
        {
            i1=0;
            j1=0;
            k1=1;
            i2=0;
            j2=1;
            k2=1;
        }
        else if(x0<z0)
        {
            i1=0;
            j1=1;
            k1=0;
            i2=0;
            j2=1;
            k2=1;
        }
        else
        {
            i1=0;
            j1=1;
            k1=0;
            i2=1;
            j2=1;
            k2=0;
        }
    }

    double x1=x0-i1+G3;
    double y1=y0-j1+G3;
    double z1=z0-k1+G3;
    double x2=x0-i2+2.0*G3;
    double y2=y0-j2+2.0*G3;
    double z2=z0-k2+2.0*G3;
    double x3=x0-1.0+3.0*G3;
    double y3=y0-1.0+3.0*G3;
    double z3=z0-1.0+3.0*G3;

    unsigned int h0,h1,h2,h3;

    h0=hash_coords_3(i,j,k,seed)%24;
    h1=hash_coords_3(i+i1,j+j1,k+k1,seed)%24;
    h2=hash_coords_3(i+i2,j+j2,k+k2,seed)%24;
    h3=hash_coords_3(i+1,j+1,k+1,seed)%24;

    double *g0=&gradient3D_lut[h0][0];
    double *g1=&gradient3D_lut[h1][0];
    double *g2=&gradient3D_lut[h2][0];
    double *g3=&gradient3D_lut[h3][0];

    double t0=0.6-x0*x0-y0*y0-z0*z0;
    if(t0<0.0) n0=0.0;
    else
    {
        t0 *= t0;
        n0 = t0*t0*array_dot3(g0,x0,y0,z0);
    }

    double t1=0.6-x1*x1-y1*y1-z1*z1;
    if(t1<0.0) n1=0.0;
    else
    {
        t1*=t1;
        n1=t1*t1*array_dot3(g1,x1,y1,z1);
    }

    double t2=0.6-x2*x2-y2*y2-z2*z2;
    if(t2<0) n2=0.0;
    else
    {
        t2 *= t2;
        n2 = t2*t2*array_dot3(g2,x2,y2,z2);
    }

    double t3=0.6-x3*x3-y3*y3-z3*z3;
    if(t3<0) n3=0.0;
    else
    {
        t3*=t3;
        n3=t3*t3*array_dot3(g3,x3,y3,z3);
    }

    return (32.0*(n0+n1+n2+n3))*1.25086885 + 0.0003194984;
}

double simplex_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp)
{
    static int simplex[64][4] =
    {
        {0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
        {0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
        {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
        {1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
        {1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
        {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
        {2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
        {2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}
    };

    double F4 = (sqrt(5.0)-1.0)/4.0;
    double G4 = (5.0-sqrt(5.0))/20.0;
    double n0, n1, n2, n3, n4; // Noise contributions from the five corners
    // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
    double s = (x + y + z + w) * F4; // Factor for 4D skewing
    int i = fast_floor(x + s);
    int j = fast_floor(y + s);
    int k = fast_floor(z + s);
    int l = fast_floor(w + s);
    double t = (i + j + k + l) * G4; // Factor for 4D unskewing
    double X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
    double Y0 = j - t;
    double Z0 = k - t;
    double W0 = l - t;
    double x0 = x - X0; // The x,y,z,w distances from the cell origin
    double y0 = y - Y0;
    double z0 = z - Z0;
    double w0 = w - W0;
// For the 4D case, the simplex is a 4D shape I won't even try to describe.
// To find out which of the 24 possible simplices we're in, we need to
// determine the magnitude ordering of x0, y0, z0 and w0.
// The method below is a good way of finding the ordering of x,y,z,w and
// then find the correct traversal order for the simplex we’re in.
// First, six pair-wise comparisons are performed between each possible pair
// of the four coordinates, and the results are used to add up binary bits
// for an integer index.
    int c1 = (x0 > y0) ? 32 : 0;
    int c2 = (x0 > z0) ? 16 : 0;
    int c3 = (y0 > z0) ? 8 : 0;
    int c4 = (x0 > w0) ? 4 : 0;
    int c5 = (y0 > w0) ? 2 : 0;
    int c6 = (z0 > w0) ? 1 : 0;
    int c = c1 + c2 + c3 + c4 + c5 + c6;
    int i1, j1, k1, l1; // The integer offsets for the second simplex corner
    int i2, j2, k2, l2; // The integer offsets for the third simplex corner
    int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner
// simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
// impossible. Only the 24 indices which have non-zero entries make any sense.
// We use a thresholding to set the coordinates in turn from the largest magnitude.
// The number 3 in the "simplex" array is at the position of the largest coordinate.
    i1 = simplex[c][0]>=3 ? 1 : 0;
    j1 = simplex[c][1]>=3 ? 1 : 0;
    k1 = simplex[c][2]>=3 ? 1 : 0;
    l1 = simplex[c][3]>=3 ? 1 : 0;
// The number 2 in the "simplex" array is at the second largest coordinate.
    i2 = simplex[c][0]>=2 ? 1 : 0;
    j2 = simplex[c][1]>=2 ? 1 : 0;
    k2 = simplex[c][2]>=2 ? 1 : 0;
    l2 = simplex[c][3]>=2 ? 1 : 0;
// The number 1 in the "simplex" array is at the second smallest coordinate.
    i3 = simplex[c][0]>=1 ? 1 : 0;
    j3 = simplex[c][1]>=1 ? 1 : 0;
    k3 = simplex[c][2]>=1 ? 1 : 0;
    l3 = simplex[c][3]>=1 ? 1 : 0;
// The fifth corner has all coordinate offsets = 1, so no need to look that up.
    double x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
    double y1 = y0 - j1 + G4;
    double z1 = z0 - k1 + G4;
    double w1 = w0 - l1 + G4;
    double x2 = x0 - i2 + 2.0*G4; // Offsets for third corner in (x,y,z,w) coords
    double y2 = y0 - j2 + 2.0*G4;
    double z2 = z0 - k2 + 2.0*G4;
    double w2 = w0 - l2 + 2.0*G4;
    double x3 = x0 - i3 + 3.0*G4; // Offsets for fourth corner in (x,y,z,w) coords
    double y3 = y0 - j3 + 3.0*G4;
    double z3 = z0 - k3 + 3.0*G4;
    double w3 = w0 - l3 + 3.0*G4;
    double x4 = x0 - 1.0 + 4.0*G4; // Offsets for last corner in (x,y,z,w) coords
    double y4 = y0 - 1.0 + 4.0*G4;
    double z4 = z0 - 1.0 + 4.0*G4;
    double w4 = w0 - 1.0 + 4.0*G4;
// Work out the hashed gradient indices of the five simplex corners
    unsigned int h0,h1,h2,h3,h4;
    h0=hash_coords_4(i,j,k,l,seed)%64;
    h1=hash_coords_4(i+i1,j+j1,k+k1,l+l1,seed)%64;
    h2=hash_coords_4(i+i2,j+j2,k+k2,l+l2,seed)%64;
    h3=hash_coords_4(i+i3,j+j3,k+k3,l+l3,seed)%64;
    h4=hash_coords_4(i+1,j+1,k+1,l+1,seed)%64;

    double *g0=&gradient4D_lut[h0][0];
    double *g1=&gradient4D_lut[h1][0];
    double *g2=&gradient4D_lut[h2][0];
    double *g3=&gradient4D_lut[h3][0];
    double *g4=&gradient4D_lut[h4][0];


// Calculate the contribution from the five corners
    double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0 - w0*w0;
    if(t0<0) n0 = 0.0;
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * array_dot4(g0, x0, y0, z0, w0);
    }
    double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1 - w1*w1;
    if(t1<0) n1 = 0.0;
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * array_dot4(g1, x1, y1, z1, w1);
    }
    double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2 - w2*w2;
    if(t2<0) n2 = 0.0;
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * array_dot4(g2, x2, y2, z2, w2);
    }
    double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3 - w3*w3;
    if(t3<0) n3 = 0.0;
    else
    {
        t3 *= t3;
        n3 = t3 * t3 * array_dot4(g3, x3, y3, z3, w3);
    }
    double t4 = 0.6 - x4*x4 - y4*y4 - z4*z4 - w4*w4;
    if(t4<0) n4 = 0.0;
    else
    {
        t4 *= t4;
        n4 = t4 * t4 * array_dot4(g4, x4, y4, z4, w4);
    }
// Sum up and scale the result to cover the range [-1,1]
    return 27.0 * (n0 + n1 + n2 + n3 + n4);
}



struct SVectorOrdering4
{
    SVectorOrdering4(double v, int X, int Y, int Z, int W)
    {
        coord=v;
        x=X;
        y=Y;
        z=Z;
        w=W;
    };
    double coord;
    int x,y,z,w;
};

bool svec4Compare(SVectorOrdering4 v1, SVectorOrdering4 v2)
{
    if(v1.coord > v2.coord) return true;
    return false;
}

struct SVectorOrdering
{
    SVectorOrdering()
    {
        val=0;
        axis=0;
    }
    SVectorOrdering(double v, int a)
    {
        val=v;
        axis=a;
    };
    double val;
    int axis;
};

bool vectorOrderingCompare(SVectorOrdering v1, SVectorOrdering v2)
{
    if(v1.val > v2.val) return true;
    return false;
}

void sortBy_4(double *l1, int *l2)
{
    SVectorOrdering a[4];
    for(int c=0; c<4; ++c)
    {
        a[c].val=l1[c];
        a[c].axis=l2[c];
    }

    std::sort(&a[0],&a[4],vectorOrderingCompare);
    for(int c=0; c<4; ++c) l2[c]=a[c].axis;
}
void sortBy_6(double *l1, int *l2)
{
    SVectorOrdering a[6];
    for(int c=0; c<6; ++c)
    {
        a[c].val=l1[c];
        a[c].axis=l2[c];
    }
    std::sort(&a[0],&a[6],vectorOrderingCompare);
    for(int c=0; c<6; ++c) l2[c]=a[c].axis;
}

double new_simplex_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp)
{
    // f = ((self.d + 1) ** .5 - 1) / self.d
    double F4=(sqrt(5.0)-1.0)/4.0;

    // g=self.f/(1+self.d*self.f)
    double G4 = F4/(1.0+4.0*F4);

    static double sideLength=2.0 / (4.0 * F4 + 1.0);
    static double a=sqrt((sideLength*sideLength)-((sideLength/2.0)*(sideLength/2.0)));
    static double cornerToFace=sqrt((a*a+(a/2.0)*(a/2.0)));
    static double cornerToFaceSquared=cornerToFace*cornerToFace;

    static double valueScaler=pow(3.0, -0.5);
    // Rough estimated/expirmentally determined function
    // for scaling output to be -1 to 1
    valueScaler*=pow(3.0, -3.5)*100.0 + 13.0;

    double loc[4]= {x,y,z,w};
    double s=0;
    for (int c=0; c<4; ++c) s+=loc[c];
    s*=F4;

    int skewLoc[4]= {fast_floor(x+s),fast_floor(y+s),fast_floor(z+s),fast_floor(w+s)};
    int intLoc[4]= {fast_floor(x+s),fast_floor(y+s),fast_floor(z+s),fast_floor(w+s)};
    double unskew=0.0;
    for(int c=0; c<4; ++c) unskew+=skewLoc[c];
    unskew*=G4;
    double cellDist[4]= {loc[0]-(double)skewLoc[0]+unskew, loc[1]-(double)skewLoc[1]+unskew,
                         loc[2]-(double)skewLoc[2]+unskew, loc[3]-(double)skewLoc[3]+unskew
                        };
    int distOrder[4]= {0,1,2,3};
    sortBy_4(cellDist,distOrder);

    int newDistOrder[5]= {-1,distOrder[0],distOrder[1],distOrder[2],distOrder[3]};

    double n=0.0;
    double skewOffset=0.0;

    for(int c=0; c<5; ++c)
    {
        int i=newDistOrder[c];
        if(i!=-1) intLoc[i]+=1;

        double u[4];
        for(int d=0; d<4; ++d)
        {
            u[d]=cellDist[d]-(intLoc[d]-skewLoc[d])+skewOffset;
        }

        double t=cornerToFaceSquared;

        for(int d=0; d<4; ++d)
        {
            t-=u[d]*u[d];
        }

        if(t>0.0)
        {
            unsigned int h=hash_coords_4(intLoc[0],intLoc[1],intLoc[2],intLoc[3],seed)%64;
            double *vec=&gradient4D_lut[h][0];
            double gr=0.0;
            for(int d=0; d<4; ++d)
            {
                gr+=vec[d]*u[d];
            }

            n+=gr*t*t*t*t;
        }
        skewOffset+=G4;
    }
    n*=valueScaler;
    return n;
}



double simplex_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp)
{
    // Skew
    //self.f = ((self.d + 1) ** .5 - 1) / self.d

    static double F4 = (sqrt(7.0)-1.0)/6.0; //(sqrt(5.0)-1.0)/4.0;

    // Unskew
    // self.g=self.f/(1+self.d*self.f)
    static double G4 = F4/(1.0+6.0*F4);

    static double sideLength=sqrt(6.0)/ (6.0 * F4 +1.0);
    static double a=sqrt((sideLength*sideLength)-((sideLength/2.0)*(sideLength/2.0)));
    static double cornerFace=sqrt(a*a + (a/2.0)*(a/2.0));

    static double cornerFaceSqrd=cornerFace*cornerFace;

    //self.valueScaler=(self.d-1)**-.5
    double valueScaler=pow(5.0, -0.5);
    valueScaler*=pow(5.0, -3.5)*100+13;

    double loc[6]= {x,y,z,w,u,v};
    double s=0;
    for (int c=0; c<6; ++c) s+=loc[c];
    s*=F4;

    int skewLoc[6]= {fast_floor(x+s),fast_floor(y+s),fast_floor(z+s),fast_floor(w+s),fast_floor(u+s),fast_floor(v+s)};
    int intLoc[6]= {fast_floor(x+s),fast_floor(y+s),fast_floor(z+s),fast_floor(w+s),fast_floor(u+s),fast_floor(v+s)};
    double unskew=0.0;
    for(int c=0; c<6; ++c) unskew+=skewLoc[c];
    unskew*=G4;
    double cellDist[6]= {loc[0]-(double)skewLoc[0]+unskew, loc[1]-(double)skewLoc[1]+unskew,
                         loc[2]-(double)skewLoc[2]+unskew, loc[3]-(double)skewLoc[3]+unskew,
                         loc[4]-(double)skewLoc[4]+unskew, loc[5]-(double)skewLoc[5]+unskew
                        };
    int distOrder[6]= {0,1,2,3,4,5};
    sortBy_6(cellDist,distOrder);

    int newDistOrder[7]= {-1,distOrder[0],distOrder[1],distOrder[2],distOrder[3],distOrder[4],distOrder[5]};

    double n=0.0;
    double skewOffset=0.0;

    for(int c=0; c<7; ++c)
    {
        int i=newDistOrder[c];
        if(i!=-1) intLoc[i]+=1;

        double u[6];
        for(int d=0; d<6; ++d)
        {
            u[d]=cellDist[d]-(intLoc[d]-skewLoc[d])+skewOffset;
        }

        double t=cornerFaceSqrd;

        for(int d=0; d<6; ++d)
        {
            t-=u[d]*u[d];
        }

        if(t>0.0)
        {
            unsigned int h=hash_coords_6(intLoc[0],intLoc[1],intLoc[2],intLoc[3],intLoc[4],intLoc[5],seed)%192;
            double *vec=&gradient6D_lut[h][0];
            double gr=0.0;
            for(int d=0; d<6; ++d)
            {
                gr+=vec[d]*u[d];
            }

            n+=gr*t*t*t*t*t;
        }
        skewOffset+=G4;
    }
    n*=valueScaler;
    return n;
}
}
