#pragma once
#include "graphics/2d.h"
#include "graphics/color.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

class hdr_color{
	private:
	float red,green,blue,alpha,intensity;
public:
	hdr_color();
	hdr_color(const  float new_r, const  float new_g, const  float new_b, const float new_a = 1.0f, const  float new_i =1.0f);
	hdr_color(const hdr_color* const source_color);

	void reset();

	void get_v5f(SCP_vector<float>& outvec) const;
	void set_vecf(const SCP_vector<float>& input);

	void set_rgb(const int new_r,const int new_g,const int new_b);
	void set_rgb(const color* const new_rgb);
	void set_rgb(const int* const new_rgb);

	void fill_rgba_8bpp(ubyte * r_io, ubyte * b_io, ubyte * g_io, ubyte * a_io);

	float r() const;
	float r(const float in);
	float r(const int in);

	float g() const;
	float g(const float in);
	float g(const int in);

	float b() const;
	float b(const float in);
	float b(const int in);

	float a() const;
	float a(const float in);
	float a(const int in);

	float i() const;
	float i(const float in);
};
