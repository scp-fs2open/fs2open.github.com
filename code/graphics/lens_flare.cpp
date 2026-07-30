
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

// Overall brightness calibration of the ghost/starburst energy model relative
// to the HDR scene (defaults; runtime-tunable from the lab, and scaled per
// lens via $Intensity:)
float Ghost_brightness = 64.0f;
float Starburst_brightness = 1.6f;

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
float Hdr_flare_headroom = 2.5f; // flare may reach ~this multiple of paper white in HDR

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

// Where a sun's image lands on the film.
struct sun_image {
	float dist_mm = 0.0f;               // distance from the sensor centre
	float theta = 0.0f;                 // matching paraxial field angle
	float axis_x = 1.0f, axis_y = 0.0f; // unit direction the flare is strung along
};

// Project a sun onto the film gate, the same way stars_draw_sun() projects its
// sprite. False when the sun isn't imaged onto the sensor at all.
bool project_sun(const vec3d& sun_pos, const film_gate& gate, float efl, sun_image* out)
{
	vertex sun_vex;
	memset(&sun_vex, 0, sizeof(vertex));
	g3_rotate_faraway_vertex(&sun_vex, &sun_pos);
	if (sun_vex.codes & CC_BEHIND) {
		return false;
	}
	if (!(sun_vex.flags & PF_PROJECTED)) {
		g3_project_vertex(&sun_vex);
	}
	if (sun_vex.flags & PF_OVERFLOW) {
		return false;
	}

	float sx = sun_vex.screen.xyw.x / gate.clip_w * 2.0f - 1.0f;
	float sy = 1.0f - sun_vex.screen.xyw.y / gate.clip_h * 2.0f;

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

float lens_flare_get_ghost_brightness() { return Ghost_brightness; }
void lens_flare_set_ghost_brightness(float value) { Ghost_brightness = MAX(value, 0.0f); }
float lens_flare_get_starburst_brightness() { return Starburst_brightness; }
void lens_flare_set_starburst_brightness(float value) { Starburst_brightness = MAX(value, 0.0f); }
float lens_flare_get_hdr_headroom() { return Hdr_flare_headroom; }
void lens_flare_set_hdr_headroom(float value) { Hdr_flare_headroom = MAX(value, 0.0f); }

// Extra multiplier applied to the whole flare so its brightness reads
// consistently in SDR and HDR output without per-lens re-tuning. SDR is the
// reference (calibration was done there), so it is left at 1.0; HDR is rescaled
// down to sit near paper white. See LENS_FLARE_SDR_REFERENCE_WHITE.
static float lens_flare_output_scale()
{
	if (Gr_hdr_output_active) {
		return Hdr_flare_headroom / LENS_FLARE_SDR_REFERENCE_WHITE;
	}
	return 1.0f;
}

const char* lens_flare_default_name()
{
	return SCP_vector_inbounds(Lens_systems, Default_lens) ? Lens_systems[Default_lens].name.c_str() : "";
}

void lens_flare_switch_to(const char* lens_name)
{
	if (lens_name == nullptr || *lens_name == '\0') {
		Mission_lens = -1;
		return;
	}

	Mission_lens = lens_flare_lookup(lens_name);
	if (Mission_lens < 0) {
		// An unknown lens falls back to the table default rather than to no
		// flares: a typo shouldn't silently look like "the mod wanted none"
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
		auto tex = std::make_unique<lens_flare_textures>();
		lens_flare_generate_textures(lens.aperture, tex.get());
		lens.textures = std::move(tex);
	}
	return lens.textures.get();
}

void lens_flare_reset_for_level()
{
	// Unmount: the mission being loaded sets its own $Camera Lens: right after
	// this (see parse_mission_info), and the lab sets its override on demand
	Mission_lens = Default_lens;
	lens_flare_clear_lab_lens();
	Logged_lens = -2;

	// The next mission's suns are not this one's; drop the published frame so
	// nothing consumes it across the level change
	Frame_draws.clear();
	Sun_starburst_drawn.clear();

	// drop any pending live edit first; it belongs to the mission being left
	Aperture_dirty = false;
	Aperture_dirty_lens = -1;

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

	// Apply straight away unless we just did; the flush below picks up the rest
	if (!Aperture_dirty_stamp.isValid() || ui_timestamp_elapsed(Aperture_dirty_stamp)) {
		flush_pending_aperture_edit();
	}
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
		inst.color.a1d[k] = ghost.reflectance[k] * energy * Ghost_brightness;
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
		inst.color.a1d[k] = Starburst_brightness;
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

// Pack one sun's quads into a uniform block and return how many instance slots
// were written. Depends only on the lens and where the sun's image lands on the
// sensor -- `sdist` is that image's distance from the sensor centre in mm,
// `theta` its paraxial field angle.
static int pack_sun_instances(const lens_system& lens, float theta, float sdist,
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
	for (const auto& ghost : lens.ghosts) {
		if (count >= ghost_budget) {
			break;
		}
		emit_ghost(out->instances[count++], lens, ghost, theta);
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

void lens_flare_frame_update()
{
	Frame_draws.clear();
	Sun_starburst_drawn.clear();

	// live aperture edits from the lab land here, throttled
	flush_pending_aperture_edit();

	const int lens_idx = lens_flare_active_lens();
	if (lens_idx < 0 || !pass_globally_possible()) {
		return;
	}
	const lens_system& lens = Lens_systems[lens_idx];

	// Frame time for visibility smoothing (snap if we haven't run for a while)
	float dt = 0.25f;
	if (Sun_visibility_stamp.isValid()) {
		dt = ui_timestamp_since(Sun_visibility_stamp) * 0.001f;
	}
	Sun_visibility_stamp = ui_timestamp();
	bool snap = (dt > 1.0f) || (dt < 0.0f);

	int num_suns = stars_get_num_suns();
	// Sized up front so the pointers handed out below stay valid: the loop only
	// writes into slots, it never grows this
	if (static_cast<int>(Frame_data.size()) < num_suns) {
		Frame_data.resize(num_suns);
	}
	Sun_starburst_drawn.resize(num_suns, false);

	film_gate gate;
	gate.clip_w = i2fl(gr_screen.clip_width);
	gate.clip_h = i2fl(gr_screen.clip_height);
	if (gate.clip_w <= 0.0f || gate.clip_h <= 0.0f) {
		return;
	}
	gate.half_w = lens.sensor_width * 0.5f;
	gate.half_h = gate.half_w * gate.clip_h / gate.clip_w;

	// Also the camera's, not any sun's, so it is a frame constant too
	const float out_scale = lens_flare_output_scale();

	for (int sun_n = 0; sun_n < num_suns; sun_n++) {
		const auto sun_light = stars_get_sun_rgbi(sun_n);
		if (!sun_light) {
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

		sun_image image;
		if (!project_sun(sun_pos, gate, lens.efl, &image)) {
			continue;
		}

		// The slot is only committed by the push_back below, so a sun that packs
		// nothing leaves it to the next one
		generic_data::lens_flare_data* out = &Frame_data[Frame_draws.size()];

		out->axis.x = image.axis_x;
		out->axis.y = image.axis_y;
		out->ndc_scale.x = 1.0f / gate.half_w;
		out->ndc_scale.y = 1.0f / gate.half_h;
		// Master multiplier for every ghost and the starburst (lensflare-f.sdr
		// applies tint.rgb to both paths). The output scale keeps SDR and HDR
		// visually consistent without per-lens re-tuning.
		const float tint_scale = sun_light->intensity * total_vis * lens.intensity * out_scale;
		out->tint.xyzw.x = sun_light->color.xyz.x * tint_scale;
		out->tint.xyzw.y = sun_light->color.xyz.y * tint_scale;
		out->tint.xyzw.z = sun_light->color.xyz.z * tint_scale;
		out->tint.xyzw.w = 0.0f;

		int count = pack_sun_instances(lens, image.theta, image.dist_mm, out);
		if (count == 0) {
			continue;
		}

		lens_flare_draw draw;
		draw.sun_index = sun_n;
		draw.instances = count;
		draw.data = out;
		draw.visibility = total_vis;
		draw.off_axis_deg = image.theta * (180.0f / PI);
		draw.output_scale = out_scale;
		Frame_draws.push_back(draw);

		// This sun is committed, and pack_sun_instances() reserves the starburst a
		// slot up front, so a starburst-enabled lens has certainly drawn one. The
		// sprite sun can now safely step aside for it.
		Sun_starburst_drawn[sun_n] = lens.starburst;
	}

	if (!Frame_draws.empty() && lens_idx != Logged_lens) {
		Logged_lens = lens_idx;
		mprintf(("Lens flare: rendering through lens '%s' (%d ghosts + %s)\n", lens.name.c_str(),
			static_cast<int>(lens.ghosts.size()), lens.starburst ? "starburst" : "no starburst"));
	}
}


} // namespace graphics
