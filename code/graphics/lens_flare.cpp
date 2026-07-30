
#include "lens_flare.h"
#include "lens_flare_internal.h"

#include "globalincs/systemvars.h"

#include "graphics/2d.h"
#include "graphics/openxr.h"
#include "graphics/util/uniform_structs.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "render/3d.h"
#include "ship/shipfx.h"
#include "starfield/starfield.h"

#include <algorithm>
#include <cmath>

extern int Game_subspace_effect;

namespace graphics {
namespace {

// Overall brightness calibration of the ghost/starburst energy model relative to
// the HDR scene, plus the HDR headroom below. Runtime-tunable from the lab (and
// scaled per lens via $Intensity:); the field defaults are in lens_flare.h.
lens_flare_tuning Tuning;

// SDR/HDR consistency (see lens_flare_output_scale()). The flare composites
// additively into the pre-tonemap HDR scene buffer, so the tonemapper is the
// only thing that differs between the two output paths. In SDR a compressive
// curve (default = Uncharted2) squashes the flare's large linear values toward
// display white; in HDR the forced pass-through HdrScene tonemapper preserves
// them and the encode pass scales by paper-white nits, so a flare calibrated in
// SDR blows out. We rescale the HDR contribution by HEADROOM / reference-white
// so the flare's fraction of SDR display-white maps to the same fraction of HDR
// paper-white, with a little headroom left so it still reads as a highlight.
//
// Reference white is the linear input the default SDR tonemapper (Uncharted2,
// the reset default in lighting_profiles.cpp) maps to display white -- its
// W constant. HDR forces its own tonemapper, so this is a fixed calibration
// reference, not the live SDR curve.
constexpr float LENS_FLARE_SDR_REFERENCE_WHITE = 11.2f;

// Cap on the (entrance pupil / ghost size)^2 energy concentration so nearly
// focused ghosts can't blow out to infinity
constexpr float GHOST_ENERGY_CAP = 400.0f;

SCP_vector<lens_system> Lens_systems;

// Bumped whenever a lens's cached textures are dropped, so the render backends
// can tell a re-generated aperture from the one they already uploaded
unsigned int Texture_generation = 1;

// Live aperture editing in the lab: coalesce a slider drag into one
// regeneration every APERTURE_REGEN_INTERVAL ms
constexpr int APERTURE_REGEN_INTERVAL = 100;
bool Aperture_dirty = false;
int Aperture_dirty_lens = -1;
UI_TIMESTAMP Aperture_dirty_stamp;

// Per-sun occlusion visibility, smoothed over frames. Touched only by
// sun_visibility() below.
SCP_vector<float> Sun_visibility;
UI_TIMESTAMP Sun_visibility_stamp;

// The camera lens: what the table declares as the default, what the mission
// mounted (its "$Camera Lens:", possibly changed by set-camera-lens), and the
// lab's live override of that. -1 means no lens, i.e. no flares.
//
// "$Default Lens:" is kept as a name until every table has been read, since a
// *-lens.tbm may name the default before (or without) defining it.
SCP_string Default_lens_name;
int Default_lens = -1;
int Mission_lens = -1;
std::optional<int> Lab_lens;

// Per-frame flare data of every drawing sun. Kept here (rather than handed out
// by value) because one lens_flare_data is several kilobytes; the draws only
// carry pointers into this, valid until the next build.
SCP_vector<generic_data::lens_flare_data> Frame_data;
SCP_vector<lens_flare_draw> Frame_draws;

// Indexed by sun: did this frame's draw for that sun include a starburst quad?
// Published by lens_flare_frame_update() alongside Frame_draws and read back by
// lens_flare_sun_starburst_drawn(), so the sun renderer and the flare pass can
// never disagree about which suns the starburst has taken over.
SCP_vector<bool> Sun_starburst_drawn;

// Lens the "flares are running through X" breadcrumb last reported. Logged from
// the frame build rather than from lens_flare_switch_to(), because mission-info
// scans (FRED opening a file dialog) mount lenses they never render with.
int Logged_lens = -2;

// True while the global conditions allow the flare pass to render at all
// (independent of any particular sun)
bool pass_globally_possible()
{
	if (gr_screen.mode == GraphicsAPI::Stub || Lens_systems.empty()) {
		return false;
	}
	// The flare composites into the HDR scene buffer from the post-processing
	// chain, so it can only draw when this scene render actually goes through
	// that chain. Both backends raise this in their scene_texture_begin()
	// precisely when post-processing is on and drop it again at
	// scene_texture_end(), which makes it the one authoritative signal -- rather
	// than a second copy of the backends' own conditions. It is also what keeps
	// FRED and qtFRED, which draw the background without ever opening a scene
	// texture, from having their sun sprites step aside for a pass that will
	// never run.
	if (!High_dynamic_range) {
		return false;
	}
	if (Game_subspace_effect) {
		return false;
	}
	if (openxr_enabled()) {
		// A per-eye camera-lens artifact is wrong in VR
		return false;
	}
	// same conditions under which the sun sprites themselves are drawn
	if (The_mission.flags[Mission::Mission_Flags::Fullneb] || !Detail.planets_suns) {
		return false;
	}
	return true;
}

// How visible a sun is to the flare, in 0..1: the eye-in-shadow occlusion test
// smoothed over a few frames so shadow transitions fade instead of popping,
// times the off-axis falloff that takes the whole effect out as the sun leaves
// the frame. `dot` is the sun direction against the view axis, `dt` the frame
// time, and `snap` skips the smoothing when the pass hasn't run for a while.
float sun_visibility(int sun_n, int light_idx, float dot, float dt, bool snap)
{
	if (static_cast<int>(Sun_visibility.size()) <= sun_n) {
		Sun_visibility.resize(sun_n + 1, 0.0f);
	}

	bool occluded = (dot <= 0.0f) ||
		(light_idx >= 0 && shipfx_eye_in_shadow(&Eye_position, Viewer_obj, light_idx));

	float target = occluded ? 0.0f : 1.0f;
	float& vis = Sun_visibility[sun_n];
	if (snap) {
		vis = target;
	} else {
		vis += (target - vis) * MIN(dt * 8.0f, 1.0f);
	}

	float axis_fade = std::clamp((dot - 0.2f) / 0.3f, 0.0f, 1.0f);
	return vis * axis_fade;
}

// The camera's film gate for this frame: the sensor half-extents the lens
// declares, and the screen the projection lands on. One gate for every sun,
// because the gate belongs to the camera.
struct film_gate {
	float clip_w = 0.0f, clip_h = 0.0f; // pixels
	float half_w = 0.0f, half_h = 0.0f; // mm
};

// Where a light source's image lands on the film.
struct film_image {
	float dist_mm = 0.0f;               // distance from the sensor centre
	float theta = 0.0f;                 // matching paraxial field angle
	float axis_x = 1.0f, axis_y = 0.0f; // unit direction the flare is strung along
};

// Project a flare source onto the film gate, each kind the same way the thing it
// stands for is drawn: a sun through the faraway path stars_draw_sun() uses for
// its sprite, an engine as the ordinary finite point its glow is drawn at. False
// when the source isn't imaged onto the sensor at all.
//
// The field angle below is derived from where the image lands, which treats the
// source as collimated -- true for a sun, an approximation for an engine a few
// hundred metres away. Getting it exactly right would mean re-tracing the ghost
// matrices per source and per frame for an object whose flare is a few pixels
// wide, so the ghosts of a near source are placed a little as if it were far.
bool project_source(const flare_source& src, const film_gate& gate, float efl, film_image* out)
{
	vertex vex;
	memset(&vex, 0, sizeof(vertex));
	if (src.at_infinity) {
		g3_rotate_faraway_vertex(&vex, &src.pos);
	} else {
		g3_rotate_vertex(&vex, &src.pos);
	}

	if (vex.codes & CC_BEHIND) {
		return false;
	}
	if (!(vex.flags & PF_PROJECTED)) {
		g3_project_vertex(&vex);
	}
	if (vex.flags & PF_OVERFLOW) {
		return false;
	}

	float sx = vex.screen.xyw.x / gate.clip_w * 2.0f - 1.0f;
	float sy = 1.0f - vex.screen.xyw.y / gate.clip_h * 2.0f;

	float smx = sx * gate.half_w;
	float smy = sy * gate.half_h;

	out->dist_mm = sqrtf(smx * smx + smy * smy);
	out->theta = out->dist_mm / efl;
	out->axis_x = 1.0f;
	out->axis_y = 0.0f;
	if (out->dist_mm > 1e-4f) {
		out->axis_x = smx / out->dist_mm;
		out->axis_y = smy / out->dist_mm;
	}
	return true;
}

// Apply a pending live aperture edit, at most once per APERTURE_REGEN_INTERVAL.
// Regenerating means a 512^2 mask plus its starburst FFT (a good fraction of a
// second in a debug build), while ImGui sliders fire every frame a drag is
// held, so a drag has to be coalesced into a few rebuilds rather than sixty.
void flush_pending_aperture_edit()
{
	if (!Aperture_dirty) {
		return;
	}
	if (Aperture_dirty_stamp.isValid() && !ui_timestamp_elapsed(Aperture_dirty_stamp)) {
		return;
	}
	lens_flare_invalidate_textures(Aperture_dirty_lens);
	Aperture_dirty = false;
	Aperture_dirty_stamp = ui_timestamp(APERTURE_REGEN_INTERVAL);
}

} // namespace

void lens_flare_init()
{
	lens_flare_close();

	lens_flare_parse_tables(Lens_systems, Default_lens_name);

	// Resolved once every table has been read, so the default may be named
	// before it is defined
	if (!Default_lens_name.empty()) {
		Default_lens = lens_flare_lookup(Default_lens_name.c_str());
		if (Default_lens < 0) {
			Warning(LOCATION, "$Default Lens: names '%s', which no lens table defines.", Default_lens_name.c_str());
		}
	}
	Mission_lens = Default_lens;

	mprintf(("Lens flares: %d lens system(s) loaded, default lens '%s'\n", static_cast<int>(Lens_systems.size()),
		lens_flare_default_name()));
}

void lens_flare_close()
{
	Lens_systems.clear();
	Sun_visibility.clear();
	Default_lens_name.clear();
	Default_lens = -1;
	Mission_lens = -1;
	lens_flare_clear_lab_lens();
	lens_flare_lab_thruster_flare().reset();
	Frame_data.clear();
	Frame_draws.clear();
	Sun_starburst_drawn.clear();
	Logged_lens = -2;
}

int lens_flare_lookup(const char* name)
{
	for (int i = 0; i < static_cast<int>(Lens_systems.size()); i++) {
		if (!stricmp(Lens_systems[i].name.c_str(), name)) {
			return i;
		}
	}
	return -1;
}

int lens_flare_num_systems()
{
	return static_cast<int>(Lens_systems.size());
}

const lens_system* lens_flare_get_system(int lens_idx)
{
	if (!SCP_vector_inbounds(Lens_systems, lens_idx)) {
		return nullptr;
	}
	return &Lens_systems[lens_idx];
}

lens_system* lens_flare_get_system_mutable(int lens_idx)
{
	if (!SCP_vector_inbounds(Lens_systems, lens_idx)) {
		return nullptr;
	}
	return &Lens_systems[lens_idx];
}

lens_flare_tuning& lens_flare_get_tuning() { return Tuning; }

// Extra multiplier applied to the whole flare so its brightness reads
// consistently in SDR and HDR output without per-lens re-tuning. SDR is the
// reference (calibration was done there), so it is left at 1.0; HDR is rescaled
// down to sit near paper white. See LENS_FLARE_SDR_REFERENCE_WHITE.
static float lens_flare_output_scale()
{
	if (Gr_hdr_output_active) {
		return MAX(Tuning.hdr_headroom, 0.0f) / LENS_FLARE_SDR_REFERENCE_WHITE;
	}
	return 1.0f;
}

const char* lens_flare_default_name()
{
	return SCP_vector_inbounds(Lens_systems, Default_lens) ? Lens_systems[Default_lens].name.c_str() : "";
}

void lens_flare_switch_to(const char* lens_name)
{
	// No opinion (a mission with no "$Camera Lens:" at all), or the default asked
	// for by name -- see the vocabulary in lens_flare.h
	if (lens_name == nullptr || *lens_name == '\0' || !stricmp(lens_name, LENS_NAME_DEFAULT)) {
		Mission_lens = Default_lens;
		return;
	}

	// The one way to say "no flares even though a default exists"
	if (!stricmp(lens_name, LENS_NAME_NONE)) {
		Mission_lens = -1;
		return;
	}

	Mission_lens = lens_flare_lookup(lens_name);
	if (Mission_lens < 0) {
		// An unknown lens falls back to the table default rather than to no
		// flares: a typo shouldn't silently look like LENS_NAME_NONE
		Warning(LOCATION, "No lens system named '%s' is defined in lens_flares.tbl; using the default lens.",
			lens_name);
		Mission_lens = Default_lens;
	}
}

int lens_flare_active_lens()
{
	int lens_idx = Lab_lens.value_or(Mission_lens);
	return SCP_vector_inbounds(Lens_systems, lens_idx) ? lens_idx : -1;
}

const char* lens_flare_mission_lens_name()
{
	return SCP_vector_inbounds(Lens_systems, Mission_lens) ? Lens_systems[Mission_lens].name.c_str() : "";
}

void lens_flare_set_lab_lens(int lens_idx)
{
	Lab_lens = lens_idx;
}

void lens_flare_clear_lab_lens()
{
	Lab_lens.reset();
}

std::optional<int> lens_flare_get_lab_lens()
{
	return Lab_lens;
}

const SCP_vector<lens_flare_draw>& lens_flare_get_frame_draws()
{
	return Frame_draws;
}

bool lens_flare_sun_starburst_drawn(int sun_n)
{
	return SCP_vector_inbounds(Sun_starburst_drawn, sun_n) && Sun_starburst_drawn[sun_n];
}

const lens_flare_textures* lens_flare_get_textures(int lens_idx)
{
	if (!SCP_vector_inbounds(Lens_systems, lens_idx)) {
		return nullptr;
	}
	lens_system& lens = Lens_systems[lens_idx];
	if (!lens.textures) {
		auto tex = std::make_shared<lens_flare_textures>();
		lens_flare_generate_textures(lens.aperture, tex.get());
		lens.textures = std::move(tex);
	}
	return lens.textures.get();
}

void lens_flare_prime_textures()
{
	// The editors draw the background without ever opening a scene texture, so the
	// flare pass never runs there and generating the pair would be pure waste on
	// every mission load
	if (Fred_running) {
		return;
	}
	lens_flare_get_textures(lens_flare_active_lens());
}

void lens_flare_reset_for_level()
{
	// Unmount: the mission being loaded sets its own $Camera Lens: right after
	// this (see parse_mission_info), and the lab sets its override on demand
	Mission_lens = Default_lens;
	lens_flare_clear_lab_lens();
	lens_flare_lab_thruster_flare().reset();
	Logged_lens = -2;

	// The next mission's suns are not this one's; drop the published frame so
	// nothing consumes it across the level change
	lens_flare_clear_frame();

	// Drop any pending live edit first; it belongs to the mission being left. The
	// throttle stamp goes too, so the next mission's first edit applies at once
	// instead of waiting out an interval started by the previous one.
	Aperture_dirty = false;
	Aperture_dirty_lens = -1;
	Aperture_dirty_stamp = UI_TIMESTAMP::invalid();

	for (int i = 0; i < static_cast<int>(Lens_systems.size()); i++) {
		if (Lens_systems[i].aperture != Lens_systems[i].tabled_aperture) {
			Lens_systems[i].aperture = Lens_systems[i].tabled_aperture;
			lens_flare_invalidate_textures(i);
		}
	}
}

void lens_flare_invalidate_textures(int lens_idx)
{
	if (!SCP_vector_inbounds(Lens_systems, lens_idx)) {
		return;
	}
	Lens_systems[lens_idx].textures.reset();
	Texture_generation++;
}

unsigned int lens_flare_get_texture_generation() { return Texture_generation; }

bool lens_flare_aperture_edit_pending() { return Aperture_dirty; }

void lens_flare_aperture_changed(int lens_idx)
{
	if (!SCP_vector_inbounds(Lens_systems, lens_idx)) {
		return;
	}
	if (Aperture_dirty && Aperture_dirty_lens != lens_idx) {
		// switching lenses mid-edit: flush the pending one first
		lens_flare_invalidate_textures(Aperture_dirty_lens);
	}
	Aperture_dirty = true;
	Aperture_dirty_lens = lens_idx;

	// Apply straight away if the interval has already elapsed; if it hasn't, this
	// is a no-op and the per-frame flush picks the edit up when it does. (The
	// interval check lives in flush_pending_aperture_edit() alone -- repeating it
	// here as a guard would just be the same condition written twice.)
	flush_pending_aperture_edit();
}

namespace {

// The three quad kinds share one instance slot but read its fields differently
// (see lens_flare_instance_data in graphics/util/uniform_structs.h for the
// per-kind table). Each emit_* below is the sole writer of its kind, so the
// convention lives in exactly one place per artifact instead of being spread
// across one long packing function.

// Blank a slot and tag its kind, so each emitter only writes the fields it
// actually means and never has to remember to zero the rest.
void instance_init(generic_data::lens_flare_instance_data& inst, float kind)
{
	inst = {};
	inst.center.xyzw.w = kind;
}

// Sub-pixel guard: a nearly focused ghost would otherwise collapse to a point
// (and its energy concentration to infinity).
float ghost_min_halfext(const lens_system& lens)
{
	return lens.sensor_width * 0.004f;
}

// A ghost: the aperture as imaged by one two-reflection path, evaluated at each
// of the three design wavelengths, so every xyz triple here is per-channel.
void emit_ghost(generic_data::lens_flare_instance_data& inst, const lens_system& lens,
	const lens_flare_ghost& ghost, float theta)
{
	instance_init(inst, generic_data::LENS_QUAD_GHOST);

	const float pupil = lens.entrance_radius;
	const float min_halfext = ghost_min_halfext(lens);

	for (int k = 0; k < 3; k++) {
		// Full path matrix F = Ms * Ma; only row 0 (heights) is needed
		float f_a = ghost.ms[k][0] * ghost.ma[k][0] + ghost.ms[k][1] * ghost.ma[k][2];
		float f_b = ghost.ms[k][0] * ghost.ma[k][1] + ghost.ms[k][1] * ghost.ma[k][3];

		float halfext = MAX(fabsf(f_a) * pupil, min_halfext);
		float energy = MIN((pupil * pupil) / (halfext * halfext), GHOST_ENERGY_CAP);

		inst.center.a1d[k] = f_b * theta;
		inst.halfext.a1d[k] = halfext;
		inst.apscale.a1d[k] = ghost.ma[k][0] * pupil / lens.aperture_radius;
		inst.apoff.a1d[k] = ghost.ma[k][1] * theta / lens.aperture_radius;
		// clamped here rather than at the setter: the lab hands out the tuning
		// struct for direct editing, so this is the boundary that has to hold
		inst.color.a1d[k] = ghost.reflectance[k] * energy * MAX(Tuning.ghost_brightness, 0.0f);
	}
}

// The starburst: the Fraunhofer transform of the iris, sitting exactly on the
// sun's image. Achromatic here, because the texture carries its own per-channel
// diffraction scaling. `sdist` is the image's distance from the sensor centre.
void emit_starburst(generic_data::lens_flare_instance_data& inst, const lens_system& lens, float sdist)
{
	instance_init(inst, generic_data::LENS_QUAD_STARBURST);

	const float halfext = lens.starburst_scale * lens.sensor_width * 0.12f;
	for (int k = 0; k < 3; k++) {
		inst.center.a1d[k] = sdist;
		inst.halfext.a1d[k] = halfext;
		inst.color.a1d[k] = MAX(Tuning.starburst_brightness, 0.0f); // see emit_ghost
	}
}

// The anamorphic streak: screen-horizontal, so unlike the other two kinds it
// reads halfext as a half-length and a half-thickness rather than as three
// chromatic half-widths.
void emit_streak(generic_data::lens_flare_instance_data& inst, const lens_system& lens, float sdist)
{
	instance_init(inst, generic_data::LENS_QUAD_STREAK);

	const float min_halfext = ghost_min_halfext(lens);
	const float half_len = MAX(lens.streak.length * lens.sensor_width * 0.5f, min_halfext);

	inst.center.xyzw.x = sdist; // the sun's image, same as the starburst
	inst.halfext.xyzw.x = half_len;
	inst.halfext.xyzw.y = MAX(half_len * lens.streak.thickness, min_halfext * 0.25f);

	// The lens tint is a colour cast on top of the sun's own colour, which the
	// shared `tint` already applies -- so a red sun keeps a reddish streak
	// instead of the table's blue overriding it
	for (int k = 0; k < 3; k++) {
		inst.color.a1d[k] = lens.streak.tint[k] * lens.streak.strength;
	}
}

} // namespace

// Pack one source's quads into a uniform block and return how many instance
// slots were written. Depends only on the lens and where the source's image
// lands on the sensor -- `sdist` is that image's distance from the sensor centre
// in mm, `theta` its paraxial field angle -- so a sun and an engine that happen
// to land in the same place get the same quads, which is what "one camera, one
// lens" means.
static int pack_source_instances(const lens_system& lens, float theta, float sdist, bool with_ghosts,
	generic_data::lens_flare_data* out)
{
	// Each non-ghost artifact reserves its slot out of the budget up front, so the
	// ghosts can never crowd it out and the emits below need no second bounds
	// check. The predicates are named once and used for both the reservation and
	// the emission, so a new artifact cannot be added to one without the other --
	// which is what lets lens_flare_frame_update() conclude that a
	// starburst-enabled lens has certainly drawn its starburst, and hence what the
	// sprite sun steps aside for.
	const bool wants_starburst = lens.starburst;
	const bool wants_streak = lens.streak.strength > 0.0f;

	const int reserved = (wants_starburst ? 1 : 0) + (wants_streak ? 1 : 0);
	const int ghost_budget = generic_data::MAX_LENS_FLARE_INSTANCES - reserved;

	int count = 0;
	if (with_ghosts) {
		for (const auto& ghost : lens.ghosts) {
			if (count >= ghost_budget) {
				break;
			}
			emit_ghost(out->instances[count++], lens, ghost, theta);
		}
	}
	if (wants_starburst) {
		emit_starburst(out->instances[count++], lens, sdist);
	}
	if (wants_streak) {
		emit_streak(out->instances[count++], lens, sdist);
	}
	Assertion(count <= generic_data::MAX_LENS_FLARE_INSTANCES,
		"Lens flare packed %d instances into %d slots -- the ghost budget no longer reserves the "
		"starburst/streak slots correctly",
		count, generic_data::MAX_LENS_FLARE_INSTANCES);

	out->n_instances = count;
	// Single choke point for the squeeze, so a table typo, a lab slider and any
	// future sexp all get the same guard against a divide by zero in the shader
	out->squeeze = MAX(lens.anamorphic_squeeze, 0.01f);
	out->pad[0] = out->pad[1] = 0.0f;
	return count;
}

void lens_flare_clear_frame()
{
	Frame_draws.clear();
	Sun_starburst_drawn.clear();
}

namespace {

// Every sun the content asked to flare, with its occlusion and off-axis fades
// already folded into the source's brightness.
void gather_sun_sources(SCP_vector<flare_source>& out, float dt, bool snap)
{
	const int num_suns = stars_get_num_suns();

	for (int sun_n = 0; sun_n < num_suns; sun_n++) {
		const auto sun_light = stars_get_sun_rgbi(sun_n);
		if (!sun_light) {
			continue;
		}

		// A sun the content never asked to flare gets nothing from the camera lens.
		// stars.tbl decides whether a sun flares ("+Camera Lens Flare:", or a legacy
		// $Flare: block for tables predating it); the mounted lens only decides how
		// that flare is drawn. Mounting a lens must not invent flares on suns
		// deliberately tabled without one.
		if (!stars_sun_has_camera_lens_flare(sun_n)) {
			continue;
		}

		vec3d sun_pos = vmd_zero_vector;
		sun_pos.xyz.y = 1.0f;
		stars_get_sun_pos(sun_n, &sun_pos);
		vec3d sun_dir = sun_pos;
		vm_vec_normalize(&sun_dir);

		float dot = vm_vec_dot(&sun_dir, &Eye_matrix.vec.fvec);

		// a sun the engine gives no glare gets no flare either
		int light_idx = light_find_for_sun(sun_n);
		if (light_idx >= 0 && !light_has_glare(light_idx)) {
			continue;
		}

		float total_vis = sun_visibility(sun_n, light_idx, dot, dt, snap);
		if (total_vis < 0.005f) {
			continue;
		}

		flare_source src;
		src.pos = sun_pos;
		src.at_infinity = true;
		src.color = sun_light->color;
		src.intensity = sun_light->intensity * total_vis;
		src.visibility = total_vis;
		src.kind = flare_source_kind::sun;
		src.index = sun_n;
		out.push_back(src);
	}
}

// How many nozzles the pass will image at most, brightest first. Every lit
// nozzle is its own source (a capital ship's engines are too far apart to
// average), so this is what stands between a fleet engagement and several
// hundred draws: each source costs a multi-kilobyte uniform block and its own
// instanced draw.
//
// It is generous rather than small because the thruster flares that motivate it
// are the ones on a big ship, where a dozen nozzles are visible at once. What
// makes that affordable is lens_flare_tuning::thruster_ghosts being off, which
// leaves each of them a single starburst quad instead of a full ghost train.
//
// Must stay within what the Vulkan backend's per-frame UBO ring can hold across
// however many times the scene is rendered in a frame -- see
// LENS_FLARE_UBO_SLOTS in VulkanPostProcessingLensFlare.cpp.
constexpr int MAX_THRUSTER_SOURCES = 32;

// Beams are scarce even in a capital-ship engagement, so this is a backstop
// rather than something a normal frame is expected to reach -- which is also
// why they keep their ghost train where nozzles drop theirs: a handful of
// ghost trains reads as an optical effect rather than noise, and ghosts cost
// nothing extra once a source's uniform block is uploaded.
//
// The two budgets plus the suns are what LENS_FLARE_UBO_SLOTS in the Vulkan
// backend has to stay above.
constexpr int MAX_BEAM_SOURCES = 8;

} // namespace

void lens_flare_frame_update()
{
	lens_flare_clear_frame();

	// live aperture edits from the lab land here, throttled
	flush_pending_aperture_edit();

	const int lens_idx = lens_flare_active_lens();
	if (lens_idx < 0 || !pass_globally_possible()) {
		return;
	}
	const lens_system& lens = Lens_systems[lens_idx];

	film_gate gate;
	gate.clip_w = i2fl(gr_screen.clip_width);
	gate.clip_h = i2fl(gr_screen.clip_height);
	if (gate.clip_w <= 0.0f || gate.clip_h <= 0.0f) {
		return;
	}
	gate.half_w = lens.sensor_width * 0.5f;
	gate.half_h = gate.half_w * gate.clip_h / gate.clip_w;

	// Frame time for visibility smoothing (snap if we haven't run for a while)
	float dt = 0.25f;
	if (Sun_visibility_stamp.isValid()) {
		dt = ui_timestamp_since(Sun_visibility_stamp) * 0.001f;
	}
	Sun_visibility_stamp = ui_timestamp();
	bool snap = (dt > 1.0f) || (dt < 0.0f);

	// Everything the camera images this frame, gathered before anything is packed
	// so that the two kinds of light -- one lens, one film gate -- go through the
	// identical projection and packing below
	SCP_vector<flare_source> sources;
	gather_sun_sources(sources, dt, snap);
	lens_flare_gather_thruster_sources(sources, MAX_THRUSTER_SOURCES);
	lens_flare_gather_beam_sources(sources, MAX_BEAM_SOURCES);
	if (sources.empty()) {
		return;
	}

	// Sized once, now that the source list is final: the loop hands out pointers
	// into this and must never grow it afterwards
	if (Frame_data.size() < sources.size()) {
		Frame_data.resize(sources.size());
	}
	Sun_starburst_drawn.resize(stars_get_num_suns(), false);

	// The camera's, not any source's, so it is a frame constant too
	const float out_scale = lens_flare_output_scale();

	for (const auto& src : sources) {
		film_image image;
		if (!project_source(src, gate, lens.efl, &image)) {
			continue;
		}

		// The slot is only committed by the push_back below, so a source that packs
		// nothing leaves it to the next one
		generic_data::lens_flare_data* out = &Frame_data[Frame_draws.size()];

		out->axis.x = image.axis_x;
		out->axis.y = image.axis_y;
		out->ndc_scale.x = 1.0f / gate.half_w;
		out->ndc_scale.y = 1.0f / gate.half_h;
		// Master multiplier for every ghost and the starburst (lensflare-f.sdr
		// applies tint.rgb to both paths). The output scale keeps SDR and HDR
		// visually consistent without per-lens re-tuning.
		const float tint_scale = src.intensity * lens.intensity * out_scale;
		out->tint.xyzw.x = src.color.xyz.x * tint_scale;
		out->tint.xyzw.y = src.color.xyz.y * tint_scale;
		out->tint.xyzw.z = src.color.xyz.z * tint_scale;
		out->tint.xyzw.w = 0.0f;

		int count = pack_source_instances(lens, image.theta, image.dist_mm, src.draw_ghosts, out);
		if (count == 0) {
			continue;
		}

		lens_flare_draw draw;
		draw.kind = src.kind;
		draw.source_index = src.index;
		draw.instances = count;
		draw.data = out;
		draw.visibility = src.visibility;
		draw.off_axis_deg = image.theta * (180.0f / PI);
		draw.output_scale = out_scale;
		Frame_draws.push_back(draw);

		// This sun is committed, and pack_source_instances() reserves the starburst
		// a slot up front, so a starburst-enabled lens has certainly drawn one. The
		// sprite sun can now safely step aside for it. Thrusters are not in this
		// bookkeeping on purpose: an engine's glow is the light the flare is *of*,
		// not a second drawing of the same artifact, so it keeps rendering.
		if (src.kind == flare_source_kind::sun) {
			Sun_starburst_drawn[src.index] = lens.starburst;
		}
	}

	if (!Frame_draws.empty() && lens_idx != Logged_lens) {
		Logged_lens = lens_idx;
		mprintf(("Lens flare: rendering through lens '%s' (%d ghosts + %s)\n", lens.name.c_str(),
			static_cast<int>(lens.ghosts.size()), lens.starburst ? "starburst" : "no starburst"));
	}
}


} // namespace graphics
