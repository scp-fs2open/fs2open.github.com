#include "graphics/color.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "math/floating.h"

void hdr_color::set_vecf(const SCP_vector<float>& input)
{
	size_t l = input.size();
	Assertion(l >= 3, "attempted to set color values invalid component count");
	this->red = input.at(0);
	this->green = input.at(1);
	this->blue = input.at(2);
	if (l > 3)
		this->alpha = input.at(3);
	if (l > 4)
		this->intensity = input.at(4);
}

/*
 * @brief Fills a vector with the raw float color components of the color, in the order red, green, blue, alpha,
 * intensity
 *
 * @param to_fill: The fector that will be filled with the color compoents
 */
void hdr_color::get_v5f(SCP_vector<float>& to_fill) const
{
	to_fill.clear();
	to_fill.push_back(this->red);
	to_fill.push_back(this->green);
	to_fill.push_back(this->blue);
	to_fill.push_back(this->alpha);
	to_fill.push_back(this->intensity);
}

hdr_color::hdr_color()
{
	this->red = 1.0f;
	this->green = 1.0f;
	this->blue = 1.0f;
	this->alpha = 1.0f;
	this->intensity = 1.0f;
}

hdr_color::hdr_color(float new_r, float new_g, float new_b, float new_a, float new_i)
{
	this->red = new_r;
	this->green = new_g;
	this->blue = new_b;
	this->alpha = new_a;
	this->intensity = new_i;
}

hdr_color::hdr_color(const hdr_color* const source_color)
{
	this->red = source_color->red;
	this->green = source_color->green;
	this->blue = source_color->blue;
	this->alpha = source_color->alpha;
	this->intensity = source_color->intensity;
}

/**
 * @brief Sets RGB values from three 0-255 ints
 */
void hdr_color::set_rgb(int new_r, int new_g, int new_b)
{
	this->red = i2fl(new_r) / 255.0f;
	this->green = i2fl(new_g) / 255.0f;
	this->blue = i2fl(new_b) / 255.0f;
}

/**
 * @brief Sets RGBA values from an old style color object
 */
void hdr_color::set_rgb(const color* const new_color)
{
	this->set_rgb(new_color->red, new_color->green, new_color->blue);
	this->alpha = i2fl(new_color->alpha);
}

/**
 * @brief Sets RGB values from an array of three 0-255 ints
 */
void hdr_color::set_rgb(const int* const new_rgb)
{
	this->set_rgb(new_rgb[0], new_rgb[1], new_rgb[2]);
}


/**
 * @brief retreives unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::r() const
{
	return red;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::r(float in)
{
	red = in;
	return red;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component from 0-255 int.
 */
float hdr_color::r(int in)
{
	red = i2fl(in) / 255.0f;
	return red;
}


/**
 * @brief retreives unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::g() const
{
	return green;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::g(float in)
{
	green = in;
	return green;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component from 0-255 int.
 */
float hdr_color::g(int in)
{
	green = i2fl(in) / 255.0f;
	return green;
}


/**
 * @brief retreives unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::b() const
{
	return blue;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::b(float in)
{
	blue = in;
	return blue;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component from 0-255 int.
 */
float hdr_color::b(int in)
{
	blue = i2fl(in) / 255.0f;
	return blue;
}


/**
 * @brief retreives unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::a() const
{
	return alpha;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::a(float in)
{
	alpha = in;
	return alpha;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component from 0-255 int.
 */
float hdr_color::a(int in)
{
	alpha = i2fl(in) / 255.0f;
	return alpha;
}


/**
 * @brief retreives unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::i() const
{
	return intensity;
}

/**
 * @brief sets and returns unmultiplied 0.0f-1.0f color component.
 */
float hdr_color::i(float in)
{
	intensity = in;
	return intensity;
}
/**
 * @brief Resets to a full alpha, 100% white.
 */
void hdr_color::reset()
{
	this->red = 1.0f;
	this->green = 1.0f;
	this->blue = 1.0f;
	this->alpha = 1.0f;
	this->intensity = 1.0f;
}