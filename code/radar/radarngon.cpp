#include "radar/radarngon.h"

#include "math/vecmat.h"

#include <cmath>

void HudGaugeRadarNgon::clampBlip(vec3d* blip_instance) {
	float hypotenuse;

	hypotenuse = hypotf(blip_instance->xyz.x, blip_instance->xyz.y);

	// Only do this math if the blip hypotenuse is longer than or equal to the apothem
	if (hypotenuse >= r_min) {
		float angle = atan2(blip_instance->xyz.y, blip_instance->xyz.x);
		vec3d Line_p;
		vec3d Line_v;
		formLine(&Line_p, &Line_v, angle);

		float s;
		int success = find_intersection(&s, &vmd_zero_vector, &Line_p, blip_instance, &Line_v);

		if ((success == 0) && (s < 1.0f)) {
			// Clamp the blip to the edge
			vm_vec_scale(blip_instance, s);
		}
	}

	// Use Std's method of clamping and scaling, we'll get some nice rounded corners in the process
	HudGaugeRadarStd::clampBlip(blip_instance);
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
	: arclen(PI2 / i2fl(6)), r_min(cos(PI / i2fl(6))) {}

HudGaugeRadarNgon::HudGaugeRadarNgon(int num_sides, float offset_incoming)
	: arclen(PI2 / i2fl(num_sides)), offset(offset_incoming * (PI / 180.0f)), r_min(cos(PI / i2fl(num_sides)))
{
}
