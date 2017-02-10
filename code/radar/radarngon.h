/*
 * z64555's N-gon radar
 *
 * You may not sell or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#ifndef _RADAR_NGON_H
#define _RADAR_NGON_H

#include "radar/radarsetup.h"
#include "radar/radar.h"

const int RADAR_NGON_MIN_SIDES = 3;		//!< Required, things break if n < 3

/**
 * @brief Flavor of the standard radar gauge which has an polygonal plot surface (vs. the standard oval)
 */
class HudGaugeRadarNgon : public HudGaugeRadarStd
{
	float arclen;	// The arc length of each n-gon sector (radians)
	float offset;	// The angle offset of the plot area (+ is counter-clockwise) (radians)
	float r_min;	// The apothem of the ngon (0.0f < r_min <= 1.0f)

protected:
	/**
	* @brief Forms the N-gon edge/line that the blip will be clamped/projected to
	* @param[out] p     Reference point for the line
	* @param[out] v     Directional vector (not normalized) for the line
	* @param[in]  angle The azimuth of the blip on the plot surface;
	*/
	void formLine(vec3d* p, vec3d* v, float angle);

	/**
	* @brief Clamps and scales the blip to be within the plot area
	* @param[in] vec The blip coordinates (only x and y are nonzero)
	*/
	void clampBlip(vec3d* blip);

public:
	/**
	 * @brief Default constructor the Ngon radar.
	 *
	 * @note This shouldn't be actually used, but it breaks compiling if the default constructor isn't present
	 */
	HudGaugeRadarNgon();

	/**
	 * @brief Constructor for the Ngon radar
	 *
	 * @param[in] num_sides Number of sides the n-gon should have
	 * @param[in] offset    Offset angle, in degrees
	 */
	HudGaugeRadarNgon(int num_sides, float offset);
};

#endif

