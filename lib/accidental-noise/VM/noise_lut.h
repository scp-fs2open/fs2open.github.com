// Lookup tables for 2D, 4D and 6D gradient oise.
// Generated with boost::random, using a lagged Fibonacci generator and a uniform_on_sphere distribution.


#ifndef NOISE_LUT_H
#define NOISE_LUT_H

extern double gradient2D_lut[8][2];
extern double gradient3D_lut[24][3];
extern double gradient4D_lut[64][4];
extern double gradient6D_lut[192][6];
extern double whitenoise_lut[256];


#endif
