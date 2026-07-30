#pragma once

#include "graphics/lens_flare.h"

// Private interface between the six lens-flare translation units. Nothing here
// is part of the module's API -- see graphics/lens_flare.h for that.
//
//   lens_flare.cpp            module state, the camera lens, texture cache, and
//                             the per-frame build the render backends consume
//   lens_flare_optics.cpp     the paraxial model: ray-transfer matrices, ghost
//                             enumeration, coated-Fresnel reflectance
//   lens_flare_aperture.cpp   image synthesis: the iris mask and the starburst
//                             that is its Fraunhofer transform
//   lens_flare_table.cpp      lens_flares.tbl / *-lens.tbm parsing
//   lens_flare_thrusters.cpp  finding and ranking the nozzles bright enough to
//                             flare
//   lens_flare_beams.cpp      the same for firing beam weapons
//
// The optics and image-synthesis halves touch no engine state at all; they are
// pure functions of a lens_system / lens_aperture.

struct glow_point; // model/model.h

namespace graphics {

// One light the camera images this frame, already reduced to a world position, a
// colour and a brightness. The frame build in lens_flare.cpp projects and packs
// these without caring where they came from, which is what lets a sun, an engine
// nozzle and a firing beam share every step below the gather.
struct flare_source {
	// World position -- or, when at_infinity, a direction from the eye. Suns are
	// at infinity and engines are not, and it changes which g3 projection applies,
	// so the two cannot simply be the same vector.
	vec3d pos = {{{0.0f, 0.0f, 1.0f}}};
	bool at_infinity = false;

	vec3d color = {{{1.0f, 1.0f, 1.0f}}}; // linear rgb, 0..1
	float intensity = 0.0f;               // multiplies the colour; every fade is already folded in

	// Whether this source draws the lens's ghost train as well as its starburst.
	// Decided by the gather -- suns and beams always do, thrusters follow the
	// lab's lens_flare_tuning::thruster_ghosts, since there are dozens of them
	// and a ghost train each is noise -- so the packing below stays a plain
	// function of the lens, the geometry and this flag.
	bool draw_ghosts = true;

	// Diagnostics for the lab, reported straight through to lens_flare_draw.
	// `visibility` is the fade that `intensity` above already accounts for, kept
	// separately only so the lab can show it.
	float visibility = 0.0f;
	flare_source_kind kind = flare_source_kind::sun;
	int index = -1; // sun index, or objnum for a thruster or beam source
};

// Append the nozzles bright enough to be worth a flare, brightest first and at
// most `budget` of them. A no-op when no species tables one.
//
// Budgeted rather than unbounded because every source costs a multi-kilobyte
// uniform block and its own instanced draw, and a fleet engagement has hundreds
// of lit nozzles on screen. The budget is a hard cap on the pass, not a hint.
void lens_flare_gather_thruster_sources(SCP_vector<flare_source>& out, int budget);

// Where one nozzle images and how large it appears from `eye`: the solid angle it
// subtends (pi*r^2 over the square of its distance), scaled by how squarely it
// faces the camera. Nothing here is normalized against the calibration reference
// -- the caller does that -- so this stays a plain statement of geometry.
//
// `orient`/`pos` place the model in the world, and nothing else about the ship
// matters, which is what lets this be tested without a scene.
//
// Returns false when the nozzle faces away from the camera, or when the eye sits
// exactly on it and it therefore has no direction to face.
bool lens_flare_nozzle_apparent(const glow_point& gpt, const matrix& orient, const vec3d& pos, const vec3d& eye,
	vec3d* world_pnt, float* apparent);

// Append the muzzles of every firing beam, brightest first and at most `budget`
// of them. Their brightness follows the beam's own muzzle light, so a beam ramps
// its flare up over its warmup and back down over its warmdown exactly as it
// ramps that light.
void lens_flare_gather_beam_sources(SCP_vector<flare_source>& out, int budget);

// Whether the eye has an unobstructed line of sight to a finite world point --
// the same segment/model test AI targeting uses to decide whether a shot has a
// clear path to its target (test_line_of_sight(), ai/aicode.cpp). A nozzle or
// beam muzzle is bright and squarely faced just as often tucked behind its own
// ship's hull, a wing, or another ship entirely, so both gathers call this on
// the sources that survive their budget cut -- after the cut, not before, since
// this costs a scene-wide raycast and the budget is what bounds how many of
// those a frame can afford. Deliberately does not exclude the emitting ship:
// a nozzle or muzzle on the far side of its own hull should occlude exactly
// like it would behind anything else.
bool lens_flare_point_visible(const vec3d& world_pos);

// Fraunhofer C / d / F lines (red / green / blue), in micrometers. The three
// wavelengths everything chromatic in the flare is evaluated at: dispersion and
// coating reflectance in the optics, diffraction scaling in the starburst.
constexpr float Wavelengths_um[3] = {0.65627f, 0.58756f, 0.48613f};

// Every finite source -- an engine nozzle, a beam muzzle -- is calibrated
// against one reference: a disc of radius r seen from thirty-two of its own
// radii away. Keeping it here rather than per source kind is what makes an
// intensity of 1.0 mean the same brightness whatever it was stated on, and means
// re-tuning the calibration moves one constant.
constexpr float Reference_radius = 1.0f;
constexpr float Reference_distance = 32.0f;
constexpr float Reference_apparent = PI * Reference_radius * Reference_radius /
									 (Reference_distance * Reference_distance);

// Ceiling on how far past that reference a source may be driven. Flying down a
// destroyer's exhaust, or standing next to a firing beam, would otherwise put an
// unbounded number into the tint and white out the frame; a flare that has
// already saturated cannot usefully get brighter anyway.
constexpr float Max_apparent_ratio = 3.0f;

// The solid angle a finite source subtends (pi*r^2 over the square of its
// distance), as the multiple of the reference above that an intensity of 1.0 is
// stated against.
inline float lens_flare_apparent_ratio(float solid_angle)
{
	return MIN(solid_angle / Reference_apparent, Max_apparent_ratio);
}

// Render the iris mask of an aperture and the starburst that follows from it,
// filling both halves of `out`. This is the expensive one: a 512^2 mask plus a
// 2D FFT of it. (graphics/lens_flare.h's lens_flare_generate_aperture_mask()
// stops after the mask, for callers that don't need the transform.)
void lens_flare_generate_textures(const lens_aperture& ap, lens_flare_textures* out);

// Parse lens_flares.tbl (falling back to the embedded default) plus every
// *-lens.tbm, appending to `systems` -- a later table redefining a lens by name
// replaces the earlier entry -- and precomputing each one's ghosts. Also reports
// the "$Default Lens:" name, unresolved: it may be declared before, or by a
// different table than, the lens it names.
void lens_flare_parse_tables(SCP_vector<lens_system>& systems, SCP_string& default_lens_name);

} // namespace graphics
