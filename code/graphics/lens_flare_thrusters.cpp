
#include "lens_flare.h"
#include "lens_flare_internal.h"

#include "globalincs/systemvars.h"

#include "model/model.h"
#include "object/object.h"
#include "render/3d.h"
#include "ship/ship.h"
#include "species_defs/species_defs.h"

#include <algorithm>
#include <optional>

// Engines as lens-flare sources. Everything here answers one question -- which
// ships' engines are bright enough this frame to be worth imaging, and how
// bright -- and hands the answer to lens_flare.cpp as plain flare_sources. It
// knows nothing about lenses, ghosts or quads.
//
// Every lit nozzle is its own source: a capital ship's engine banks are set far
// enough apart to read as separate points in frame, so one flare at their
// centroid would sit where no engine is. The cost of that is a uniform block and
// a draw call per nozzle, which is what the budget in
// lens_flare_gather_thruster_sources() bounds, and what
// lens_flare_tuning::thruster_ghosts keeps affordable by drawing only the
// starburst of each.
//
// None of modelrender.cpp's thruster geometry is duplicated here: submodel
// rotation, warp-plane clipping and the per-frame glow noise all move a nozzle by
// less than its own apparent size, so a plain rigid transform of the glow points
// is enough.

namespace graphics {
namespace {

// The lab's override of every species' settings; see lens_flare.h
std::optional<thruster_flare_info> Lab_thruster_flare;

// The apparent-size calibration a nozzle is stated against, and the ceiling on
// how far past it one may be driven, are shared with beam muzzles --
// lens_flare_apparent_ratio() in lens_flare_internal.h. Keeping one copy is what
// makes an intensity of 1.0 mean the same brightness whichever kind of source it
// was tabled on.

// Floor below which a source cannot change a pixel: by the time it reaches the
// frame it has also been multiplied by the lens's own intensity, so this is
// already a small fraction of one display level.
//
// Deliberately far below anything you could see, because the budget's ranking --
// not a threshold -- is what decides which flares are worth drawing. A threshold
// set where it could plausibly cull something visible is how normal-throttle
// engines came to look like they did not flare at all.
constexpr float MIN_SOURCE_INTENSITY = 0.002f;

// Is anything at all asking for thruster flares? Almost always no, and that case
// has to cost nothing more than this loop over the (three, usually) species.
bool any_thruster_flares_enabled()
{
	if (Lab_thruster_flare) {
		return Lab_thruster_flare->enabled;
	}
	return std::any_of(Species_info.begin(), Species_info.end(),
		[](const species_info& species) { return species.thruster_flare.enabled; });
}

// Append one source per lit, camera-facing nozzle of this ship. Nothing at all
// when its engines are off, destroyed, or the ship isn't drawn.
void gather_ship_nozzles(const object* objp, SCP_vector<flare_source>& out)
{
	const ship* shipp = &Ships[objp->instance];
	const ship_info* sip = &Ship_info[shipp->ship_info_index];

	const thruster_flare_info flare = lens_flare_thruster_settings(sip->species);
	if (!flare.enabled) {
		return;
	}

	// The gates ship_render() applies before it ever asks for thruster geometry:
	// a ship that isn't drawn has no glow for the camera to image, and engines
	// that are dead or disrupted are exactly the ones the glow is suppressed for.
	if (!(objp->flags[Object::Object_Flags::Renders]) || shipp->flags[Ship::Ship_Flags::Cloaked] ||
		shipp->flags[Ship::Ship_Flags::Disabled] || ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE)) {
		return;
	}
	// The player's own ship isn't drawn from inside its own cockpit, so its
	// engines -- a couple of metres behind the camera -- must not flare either
	if (objp == Viewer_obj && !(Viewer_mode & VM_TOPDOWN)) {
		return;
	}

	// How hard the engines are running, which is the same quantity the thruster
	// geometry is stretched by -- so a nozzle flares exactly when its glow is
	// drawn, at every throttle setting and not only under afterburner.
	const float throttle = std::clamp(vm_vec_mag(&objp->phys_info.linear_thrust), 0.0f, 1.0f);
	if (throttle <= 0.0f) {
		return;
	}
	const bool use_ab = (objp->phys_info.flags & (PF_AFTERBURNER_ON | PF_BOOSTER_ON)) != 0;

	if (sip->model_num < 0) {
		return;
	}
	const polymodel* pm = model_get(sip->model_num);
	if (pm == nullptr || pm->n_thrusters <= 0) {
		return;
	}

	// Ghosts are a property of the whole class of thruster flares, resolved once
	// here rather than per nozzle
	const bool draw_ghosts = lens_flare_get_tuning().thruster_ghosts;
	const float brightness = use_ab ? flare.afterburner_intensity : flare.intensity;

	for (int i = 0; i < pm->n_thrusters; i++) {
		const thruster_bank& bank = pm->thrusters[i];
		if (!bank.points || bank.num_points <= 0) {
			continue;
		}
		if (!model_should_render_engine_glow(OBJ_INDEX(objp), bank.obj_num)) {
			continue;
		}

		for (int j = 0; j < bank.num_points; j++) {
			vec3d world_pnt;
			float apparent;
			if (!lens_flare_nozzle_apparent(bank.points[j], objp->orient, objp->pos, Eye_position, &world_pnt,
					&apparent)) {
				continue;
			}

			// Irradiance at the entrance pupil, as a multiple of the reference
			// source. This is the term "brightness follows the throttle and the
			// afterburner" leaves out, and the one that keeps a battle's worth of
			// distant fighters from each throwing a full-strength flare.
			const float ratio = lens_flare_apparent_ratio(apparent);

			flare_source src;
			src.pos = world_pnt;
			src.at_infinity = false;
			src.color = flare.color;
			src.intensity = brightness * throttle * ratio;
			src.draw_ghosts = draw_ghosts;
			src.visibility = throttle;
			src.kind = flare_source_kind::thruster;
			src.index = OBJ_INDEX(objp);

			if (src.intensity < MIN_SOURCE_INTENSITY) {
				continue;
			}
			out.push_back(src);
		}
	}
}

} // namespace

std::optional<thruster_flare_info>& lens_flare_lab_thruster_flare() { return Lab_thruster_flare; }

thruster_flare_info lens_flare_thruster_settings(int species_idx)
{
	if (Lab_thruster_flare) {
		return *Lab_thruster_flare;
	}
	if (SCP_vector_inbounds(Species_info, species_idx)) {
		return Species_info[species_idx].thruster_flare;
	}
	return {};
}

bool lens_flare_nozzle_apparent(const glow_point& gpt, const matrix& orient, const vec3d& pos, const vec3d& eye,
	vec3d* world_pnt, float* apparent)
{
	vm_vec_unrotate(world_pnt, &gpt.pnt, &orient);
	vm_vec_add2(world_pnt, &pos);

	vec3d to_eye;
	vm_vec_sub(&to_eye, &eye, world_pnt);
	const float dist = vm_vec_normalize_safe(&to_eye, true);
	if (dist <= 0.0f) {
		// the eye is exactly on the nozzle; it has no direction to face
		return false;
	}

	// A null normal is a legal glowpoint, and means the nozzle shines every way --
	// the same reading the thruster renderer gives it.
	float facing = 1.0f;
	if (!IS_VEC_NULL_SQ_SAFE(&gpt.norm)) {
		vec3d world_norm;
		vm_vec_unrotate(&world_norm, &gpt.norm, &orient);
		// model normals are not guaranteed unit-length
		if (vm_vec_normalize_safe(&world_norm, true) <= 0.0f) {
			return false;
		}
		// The glow itself fades in over the first third of the hemisphere (the
		// `d *= 3` in model_queue_render_thrusters), so the flare follows the same
		// curve rather than inventing a second one.
		facing = std::clamp(vm_vec_dot(&to_eye, &world_norm) * 3.0f, 0.0f, 1.0f);
	}
	if (facing <= 0.0f) {
		return false;
	}

	*apparent = facing * PI * gpt.radius * gpt.radius / (dist * dist);
	return true;
}

void lens_flare_gather_thruster_sources(SCP_vector<flare_source>& out, int budget)
{
	if (budget <= 0 || !any_thruster_flares_enabled()) {
		return;
	}

	SCP_vector<flare_source> candidates;

	for (const object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->type != OBJ_SHIP || objp->instance < 0) {
			continue;
		}
		gather_ship_nozzles(objp, candidates);
	}

	lens_flare_commit_candidates(out, candidates, budget);
}

} // namespace graphics
