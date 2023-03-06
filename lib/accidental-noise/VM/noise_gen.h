#ifndef NOISE_GEN_H
#define NOISE_GEN_H

namespace anl
{

typedef double (*interp_func)(double);
typedef double (*noise_func2)(double, double, unsigned int, interp_func);
typedef double (*noise_func3)(double,double,double,unsigned int, interp_func);
typedef double (*noise_func4)(double,double,double,double,unsigned int, interp_func);
typedef double (*noise_func6)(double,double,double,double,double,double,unsigned int,interp_func);
typedef double (*dist_func2)(double, double, double, double);
typedef double (*dist_func3)(double, double, double, double, double, double);
typedef double (*dist_func4)(double, double, double, double, double, double, double, double);
typedef double (*dist_func6)(double, double, double, double, double, double, double, double, double, double, double, double);

// Interpolation functions
double noInterp(double t);
double linearInterp(double t);
double hermiteInterp(double t);
double quinticInterp(double t);

// Distance functions
double distEuclid2(double x1, double y1, double x2, double y2);
double distEuclid3(double x1, double y1, double z1, double x2, double y2, double z2);
double distEuclid4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2);
double distEuclid6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2);

double distManhattan2(double x1, double y1, double x2, double y2);
double distManhattan3(double x1, double y1, double z1, double x2, double y2, double z2);
double distManhattan4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2);
double distManhattan6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2);

double distGreatestAxis2(double x1, double y1, double x2, double y2);
double distGreatestAxis3(double x1, double y1, double z1, double x2, double y2, double z2);
double distGreatestAxis4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2);
double distGreatestAxis6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2);

double distLeastAxis2(double x1, double y1, double x2, double y2);
double distLeastAxis3(double x1, double y1, double z1, double x2, double y2, double z2);
double distLeastAxis4(double x1, double y1, double z1, double w1, double x2, double y2, double z2, double w2);
double distLeastAxis6(double x1, double y1, double z1, double w1, double u1, double v1, double x2, double y2, double z2, double w2, double u2, double v2);


// Noise generators
double value_noise2D(double x, double y, unsigned int seed, interp_func interp);
double value_noise3D(double x, double y, double z, unsigned int seed, interp_func interp);
double value_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp);
double value_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp);

double gradient_noise2D(double x, double y, unsigned int seed, interp_func interp);
double gradient_noise3D(double x, double y, double z, unsigned int seed, interp_func interp);
double gradient_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp);
double gradient_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp);

double gradval_noise2D(double x, double y, unsigned int seed, interp_func interp);
double gradval_noise3D(double x, double y, double z, unsigned int seed, interp_func interp);
double gradval_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp);
double gradval_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp);

double white_noise2D(double x, double y, unsigned int seed, interp_func interp);
double white_noise3D(double x, double y, double z, unsigned int seed, interp_func interp);
double white_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp);
double white_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp);

double simplex_noise2D(double x, double y, unsigned int seed, interp_func interp);
double simplex_noise3D(double x, double y, double z, unsigned int seed, interp_func interp);
double simplex_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp);
double simplex_noise6D(double x, double y, double z, double w, double u, double v, unsigned int seed, interp_func interp);
double new_simplex_noise4D(double x, double y, double z, double w, unsigned int seed, interp_func interp);

void cellular_function2D(double x, double y, unsigned int seed, double *f, double *disp, dist_func2 dist);
void cellular_function3D(double x, double y, double z, unsigned int seed, double *f, double *disp, dist_func3 dist);
void cellular_function4D(double x, double y, double z, double w, unsigned int seed, double *f, double *disp, dist_func4 dist);
void cellular_function6D(double x, double y, double z, double w, double u, double v, unsigned int seed, double *f, double *disp, dist_func6 dist);
// Hash
unsigned int FNV1A_3d(double x, double y, double z, unsigned int seed);

};

#endif
