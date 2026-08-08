
#include "lens_flare.h"
#include "lens_flare_internal.h"

#include "globalincs/linklist.h"
#include "object/object.h"
#include "render/3d.h"
#include "weapon/beam.h"

#include <algorithm>

// Firing beams as lens-flare sources: one flare at each beam's muzzle, for the
// whole time the beam exists.
//
// Brightness is not invented here. beam_get_muzzle_glow() reports what the
// beam's own muzzle light is emitting, which already ramps up over the warmup,
// holds while the beam fires and ramps back down over the warmdown -- so the
// flare grows and fades with the glow it belongs to instead of following a
// second curve that could disagree with it.
//
// Unlike thruster flares there is no table opt-in, because there is nothing for
// content to opt into that it has not already said: a beam that throws no muzzle
// light throws no flare, and no flare of any kind is drawn unless the mission
// mounts a camera lens in the first place.
//
// beam_get_muzzle_glow() deliberately does not check the Detail.lighting setting
// that gates the dynamic muzzle light itself (beam_light_sanity_and_setup()): a
// lens flare is an artifact of the camera, not a scene light, so lowering the
// lighting detail slider shouldn't make it vanish.

namespace graphics {

void lens_flare_gather_beam_sources(SCP_vector<flare_source>& out, int budget)
{
	if (budget <= 0) {
		return;
	}

	SCP_vector<flare_source> candidates;

	for (const object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->type != OBJ_BEAM || objp->instance < 0 || objp->instance >= MAX_BEAMS) {
			continue;
		}

		beam_muzzle_glow glow;
		if (!beam_get_muzzle_glow(&Beams[objp->instance], &glow)) {
			continue;
		}

		// The muzzle sits somewhere out in front of the camera, so how large it
		// looks matters as much as how hard it is burning -- the same reasoning,
		// and the same calibration, as an engine nozzle
		const float dist_sq = vm_vec_dist_squared(&glow.pos, &Eye_position);
		if (dist_sq <= 0.0f) {
			continue;
		}
		const float ratio = lens_flare_apparent_ratio(PI * glow.radius * glow.radius / dist_sq);

		flare_source src;
		src.pos = glow.pos;
		src.at_infinity = false;
		src.color = glow.color;
		src.intensity = glow.intensity * ratio;
		src.visibility = MIN(glow.intensity, 1.0f); // the warmup/warmdown ramp, for the lab
		src.kind = flare_source_kind::beam;
		src.index = OBJ_INDEX(objp);

		if (src.intensity <= 0.0f) {
			continue;
		}
		candidates.push_back(src);
	}

	lens_flare_commit_candidates(out, candidates, budget);
}

} // namespace graphics
