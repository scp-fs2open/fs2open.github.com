#include "radar/radarngon.h"

#include "math/vecmat.h"

#include <math.h>

void HudGaugeRadarNgon::clampBlip(vec3d* blip) {
	float hypotenuse;

	hypotenuse = _hypotf(blip->xyz.x, blip->xyz.y);

	// Only do this math if the blip hypotenuse is longer than or equal to the apothem
	if (hypotenuse >= r_min) {
		float angle = atan2(blip->xyz.y, blip->xyz.x);
		vec3d Line_p;
		vec3d Line_v;
		formLine(&Line_p, &Line_v, angle);

		float s;
		int success = find_intersection(&s, &vmd_zero_vector, &Line_p, blip, &Line_v);

		if ((success == 0) && (s < 1.0f)) {
			// Clamp the blip to the edge
			vm_vec_scale(blip, s);
		}
	}

	// Use Std's method of clamping and scaling, we'll get some nice rounded corners in the process
	HudGaugeRadarStd::clampBlip(blip);
}

void HudGaugeRadarNgon::formLine(vec3d* p, vec3d* v, float angle) {
	int n;  // sector the blip is in

	angle -= offset;
	n = fl2i(angle / arclen);

	p->xyz.x = cos((arclen * n) + offset);
	p->xyz.y = sin((arclen * n) + offset);
	p->xyz.z = 0.0f;

	(angle >= 0.0f) ? (n += 1) : (n -= 1);

	v->xyz.x = cos((arclen * n) + offset);
	v->xyz.y = sin((arclen * n) + offset);
	v->xyz.z = 0.0f;

	vm_vec_sub2(v, p);
}

// Default to a hexagon
HudGaugeRadarNgon::HudGaugeRadarNgon()
	: arclen(PI2 / i2fl(6)), offset(0.0f), r_min(cos(PI / i2fl(6))) {}

HudGaugeRadarNgon::HudGaugeRadarNgon(int num_sides, float offset)
	: arclen(PI2 / i2fl(num_sides)), offset(offset * (PI / 180.0f)), r_min(cos(PI / i2fl(num_sides)))
{
}
