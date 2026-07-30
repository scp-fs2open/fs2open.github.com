#pragma once

#include "globalincs/pstypes.h"

#include <complex>
#include <memory>
#include <optional>

// Physically-based lens flares (Lee & Eisemann 2013 matrix approximation).
//
// A lens system is an ordered stack of spherical surfaces parsed from
// lens_flares.tbl / *-lens.tbm. Every ordered pair of refractive surfaces
// produces one two-reflection "ghost" image of the aperture; each ghost is
// reduced at table-load time to a handful of paraxial ray-transfer matrices
// so the render backends only have to draw one textured quad per ghost.
//
// A mission mounts exactly one of these as the camera lens (see "the camera
// lens" below); with none mounted nothing changes.

namespace graphics {

namespace generic_data {
struct lens_flare_data; // graphics/util/uniform_structs.h
}

struct lens_surface {
	float radius = 0.0f;             // signed curvature radius in mm, 0 = flat
	float thickness = 0.0f;          // distance to the next surface in mm
	float n = 1.0f;                  // refractive index behind the surface (1.0 = air)
	float abbe = 0.0f;               // Abbe V-number for dispersion, 0 = dispersion-free
	float coating_wavelength = -1.0f; // AR coating tuning in nm; < 0 = use lens default, 0 = uncoated
	bool is_stop = false;            // aperture stop (flat, non-refracting)
};

// The iris: a stack of multiplicative layers rendered into one R8 transmission
// mask. Ported from realflare's aperture kernels, with one deliberate
// difference: realflare keeps a separate aperture for ghosts and for the
// starburst, while here a single definition drives both (the starburst is the
// Fraunhofer transform of this very mask), so a lens has one iris and cannot
// contradict itself.
//
// Every layer below the shape defaults to strength 0 (off), which reproduces
// the plain-iris look of tables written before they existed.
struct lens_aperture {
	// Iris opening. Blade count/rotation/curvature are the original fields;
	// curvature 0 = straight blades, 1 = circular, and negative values bow the
	// blades inward for a star-shaped iris.
	int blades = 6;
	float rotation = 0.0f;   // degrees
	float curvature = 0.0f;  // -1 = concave .. 0 = straight .. 1 = circular
	// Edge feather, as a fraction of the iris radius. The default is also the
	// floor the generator clamps to (a ~2px ramp, so the mask never aliases),
	// and reproduces the edge the iris had before this was tunable. A soft edge
	// visibly weakens the starburst spikes, so it is not a free parameter.
	float softness = 0.0039f;

	// Diffraction grating around the rim: fine radial ridges that throw extra
	// spikes into the starburst.
	struct {
		float strength = 0.0f; // 0 = off
		float density = 0.5f;  // fraction of the 360 possible ridges
		float length = 0.5f;   // how far in the ridges reach, as a fraction of the iris radius
		float width = 0.25f;   // ridge width as a duty cycle of the spacing between ridges
		float softness = 0.0f;
	} grating;

	// Scratches on the glass: randomly placed and oriented slivers.
	struct {
		float strength = 0.0f; // 0 = off
		float density = 0.5f;  // fraction of the 1000 possible scratches
		float length = 0.5f;
		float width = 0.25f;
		float rotation = 0.0f;           // degrees
		float rotation_variation = 0.0f; // 0 = all parallel, 1 = fully random
		float softness = 0.0f;
	} scratches;

	// Dust on the glass: randomly placed specks.
	struct {
		float strength = 0.0f; // 0 = off
		float density = 0.5f;  // fraction of the 1000 possible specks
		float radius = 0.5f;
		float softness = 0.0f;
	} dust;

	// Compared field by field so a caller that reassigns the whole aperture can
	// tell whether anything actually moved -- regenerating costs a 512^2 mask
	// plus an FFT, which a repeating mission event must not pay every frame.
	bool operator==(const lens_aperture& o) const
	{
		return blades == o.blades && rotation == o.rotation && curvature == o.curvature &&
			   softness == o.softness && grating.strength == o.grating.strength &&
			   grating.density == o.grating.density && grating.length == o.grating.length &&
			   grating.width == o.grating.width && grating.softness == o.grating.softness &&
			   scratches.strength == o.scratches.strength && scratches.density == o.scratches.density &&
			   scratches.length == o.scratches.length && scratches.width == o.scratches.width &&
			   scratches.rotation == o.scratches.rotation &&
			   scratches.rotation_variation == o.scratches.rotation_variation &&
			   scratches.softness == o.scratches.softness && dust.strength == o.dust.strength &&
			   dust.density == o.dust.density && dust.radius == o.dust.radius && dust.softness == o.dust.softness;
	}
	bool operator!=(const lens_aperture& o) const { return !(*this == o); }
};

// One two-reflection ghost, precomputed per wavelength (index 0 = red 656nm,
// 1 = green 588nm, 2 = blue 486nm). Matrices are row-major 2x2 ray-transfer
// matrices acting on [height; angle] column vectors: [0]=A [1]=B [2]=C [3]=D.
struct lens_flare_ghost {
	float ma[3][4];        // entrance plane -> aperture stop (last stop crossing)
	float ms[3][4];        // aperture stop -> sensor plane
	float reflectance[3];  // product of the two (coated) Fresnel reflectances
	int surf_first = -1;   // surface indices of the reflection pair (diagnostics)
	int surf_second = -1;
};

// CPU-generated texture payloads for one lens system (created on demand by
// lens_flare_get_textures(), uploaded by each render backend).
struct lens_flare_textures {
	int aperture_size = 0;
	SCP_vector<ubyte> aperture;      // R8 iris transmission mask
	int starburst_size = 0;
	SCP_vector<float> starburst;     // RGBA32F Fraunhofer starburst
};

struct lens_system {
	SCP_string name;
	SCP_vector<lens_surface> surfaces;

	float entrance_radius = 10.0f;   // entrance pupil (front element) radius, mm
	float aperture_radius = 5.0f;    // iris half-opening, mm
	float sensor_width = 36.0f;      // film-gate width, mm

	// Anamorphic squeeze: how much wider than tall the flare footprints are,
	// 1.0 = spherical. A front anamorphot is an afocal cylindrical telescope, so
	// to first order all it does is magnify one meridian -- which is why this is
	// a single number and not a second set of ray-transfer matrices. The lens
	// behind it stays rotationally symmetric, so the iris (and hence the mask and
	// its transform) is unaffected; only the imaging of it is stretched.
	//
	// Note that FSO draws the flare into an already-composed frame with no
	// desqueeze stage, so this is a look control rather than a 2x-squeeze
	// capture pipeline: ghost positions follow the sun as always, and only their
	// footprints are stretched, which is what a desqueezed anamorphic frame
	// shows.
	float anamorphic_squeeze = 1.0f;

	// The anamorphic streak: the long horizontal flare a cylindrical element
	// throws across the frame, and the half of the look the squeeze alone does
	// not buy -- stretching the starburst only ever reads as a wider sun, since
	// a real streak runs 20-50 times longer than it is thick.
	//
	// It is its own quad rather than a reshaped starburst because it is a
	// different artifact: the starburst is the iris seen end-on and rotates with
	// the sun, while the streak lies along the cylindrical element and so stays
	// horizontal wherever the sun is. Off by default, which keeps every lens
	// written before it existed untouched.
	struct {
		float strength = 0.0f;                // 0 = off
		float length = 1.0f;                  // half-length as a fraction of the sensor width
		float thickness = 0.02f;              // as a fraction of the length, so 0.02 = 50:1
		float tint[3] = {0.35f, 0.55f, 1.0f}; // multiplies the sun's own colour
	} streak;

	float coating_wavelength = 540.0f; // default AR coating tuning, nm (0 = uncoated)

	lens_aperture aperture;          // iris shape + imperfections, shared by ghosts and starburst

	bool starburst = true;
	float starburst_scale = 1.0f;
	float intensity = 0.2f;
	int max_ghosts = 40;

	// The aperture exactly as the table declared it. Missions and the lab edit
	// `aperture` in place, so this is what lens_flare_reset_for_level() restores
	// between missions -- one mission's lens must never carry into the next.
	lens_aperture tabled_aperture;

	// --- filled by lens_flare_precompute() ---
	SCP_vector<lens_flare_ghost> ghosts;
	float efl = 50.0f;               // effective focal length (green), mm
	float bfd = 40.0f;               // back focal distance last surface -> sensor (green), mm

	// Iris/starburst textures of this lens, generated on demand by
	// lens_flare_get_textures() and dropped by lens_flare_invalidate_textures().
	// Owning them here rather than in a vector alongside Lens_systems is what
	// keeps them from ever going out of step with the lens they belong to; it
	// makes lens_system move-only, which is all the engine ever needs.
	std::unique_ptr<lens_flare_textures> textures;
};

// Parse lens_flares.tbl + *-lens.tbm (embedded default as fallback) and
// precompute all ghost data. Called once from stars_init(); safe to call again
// (reloads).
void lens_flare_init();
void lens_flare_close();

// Index of a tabled lens system by name, -1 if unknown.
int lens_flare_lookup(const char* name);

int lens_flare_num_systems();
const lens_system* lens_flare_get_system(int lens_idx);

// Lazily generate (and cache) the aperture/starburst textures of a lens.
// Returns nullptr for an invalid index.
const lens_flare_textures* lens_flare_get_textures(int lens_idx);

// Drop a lens's cached textures so the next lens_flare_get_textures() rebuilds
// them (after its lens_aperture was edited).
void lens_flare_invalidate_textures(int lens_idx);

// Bumped on every invalidation. Render backends cache the uploaded textures of
// the mounted lens, so they must key that cache on this as well to notice a
// rebuild.
unsigned int lens_flare_get_texture_generation();

// Live aperture editing (lab): note that a lens's aperture changed, and poll
// whether a regeneration is still pending. Regenerating a 512^2 mask and its
// starburst FFT is far too slow to do on every frame of a slider drag, so edits
// are coalesced and applied a few times a second. Call the poll once per frame.
void lens_flare_aperture_changed(int lens_idx);
bool lens_flare_aperture_edit_pending();

// Undo everything a mission or the lab did to the lens state: unmount whatever
// lens was mounted (back to $Default Lens:) and restore every lens's aperture to
// what its table declared, so one mission's camera can't carry into the next.
// Called from stars_pre_level_init(), which runs before the mission's
// $Camera Lens: is parsed.
void lens_flare_reset_for_level();

// Mutable access for live tuning in the lab (e.g. a lens's $Intensity:).
// Changes take effect the next frame; they are lost on table reload.
lens_system* lens_flare_get_system_mutable(int lens_idx);

// Global brightness calibration multipliers applied to every ghost/starburst
// (defaults match the shipped calibration; tunable live from the lab)
float lens_flare_get_ghost_brightness();
void lens_flare_set_ghost_brightness(float value);
float lens_flare_get_starburst_brightness();
void lens_flare_set_starburst_brightness(float value);

// HDR consistency headroom: how many multiples of paper white the flare is
// allowed to reach in HDR output. SDR is the calibration reference and is
// unaffected; this only rescales the HDR path so an SDR-tuned flare doesn't
// blow out. Defaults to a value that keeps a little HDR "pop".
float lens_flare_get_hdr_headroom();
void lens_flare_set_hdr_headroom(float value);

// ---- the camera lens ----
//
// There is one lens, because there is one camera: every light source in the
// scene is imaged through the same glass, so the flares of all suns share a
// prescription, an iris and a starburst. What differs per sun is only where it
// sits in the frame and how bright it is.
//
// The mounted lens comes from the mission's "$Camera Lens:" (defaulting to
// "$Default Lens:" in lens_flares.tbl), can be changed at runtime by the
// set-camera-lens sexp, and can be overridden live in the lab.

// The name in "$Default Lens:", or "" when the table declares no default (in
// which case missions without a "$Camera Lens:" render no flares at all).
const char* lens_flare_default_name();

// Mount a lens by name: "" / <none> for no flares, <default> or an unknown name
// for the table default (unknown names warn). Called from mission parse and by
// the set-camera-lens sexp.
void lens_flare_switch_to(const char* lens_name);

// The lens actually in use (a lab override beats the mission's), -1 = none.
int lens_flare_active_lens();

// Name of the mission's own camera lens, ignoring any lab override. What the lab
// shows as the entry to fall back to, and what <default> restores.
const char* lens_flare_mission_lens_name();

// Lab override of the mission's camera lens: unset means the mission's choice
// stands, a value of -1 forces "no flares". Cleared by
// lens_flare_reset_for_level().
void lens_flare_set_lab_lens(int lens_idx);
void lens_flare_clear_lab_lens();
std::optional<int> lens_flare_get_lab_lens();

// True when the last lens_flare_frame_update() actually put a starburst quad in
// this sun's draw. Used by the sun renderer to skip the sprite sun and its glow
// so the two starbursts don't stack.
//
// This reports what the flare pass *is drawing*, read back out of the frame data
// below -- it does not re-derive it. Predicting it independently is how the
// sprite and the flare came to disagree about occluded and off-screen suns.
bool lens_flare_sun_starburst_drawn(int sun_n);

// One sun's worth of flare quads. All suns share the mounted lens (hence one
// aperture/starburst texture for the whole pass), but each has its own flare axis
// and tint, so each gets its own uniform block and instanced draw.
//
// The trailing fields are diagnostics for the lab; the renderer ignores them.
struct lens_flare_draw {
	int sun_index = -1;
	int instances = 0; // quads to draw (ghosts + optional starburst)
	// Uniform block for this draw, owned by lens_flare.cpp; valid until the
	// next lens_flare_frame_update() call.
	const generic_data::lens_flare_data* data = nullptr;

	float visibility = 0.0f;   // smoothed occlusion * off-axis fade
	float off_axis_deg = 0.0f; // paraxial field angle of the sun
	float output_scale = 1.0f; // SDR/HDR consistency multiplier applied this frame
};

// Decide what the flare pass will draw this frame and publish it, once per scene
// render. Does all the game-state access (sun projection, occlusion raycast,
// gates) and all the deferred work (flushing throttled aperture rebuilds), so
// that everything downstream is a pure read.
//
// Called from stars_draw(), which is the one place that both runs after the view
// and projection matrices are live and runs before the sun sprites and the
// post-processing pass consume the result. Deliberately *not* called while
// rendering an environment map: that path never reaches the flare pass, so
// publishing there would tell the sun renderer to step aside for a starburst
// that never gets drawn.
void lens_flare_frame_update();

// What the last lens_flare_frame_update() published: one entry per sun that has
// something to draw, in sun order (empty = skip the pass entirely). Every entry
// is drawn with the textures of lens_flare_active_lens().
//
// The single source of truth for the pass -- the render backends draw exactly
// these, the sun renderer asks lens_flare_sun_starburst_drawn() about them, and
// the lab reports on them.
const SCP_vector<lens_flare_draw>& lens_flare_get_frame_draws();

// ---- internals exposed for unit testing ----

// Run the ghost/matrix precompute on a hand-built lens_system.
// Returns false (with ghosts cleared) if the prescription is unusable.
bool lens_flare_precompute(lens_system& lens);

// Normal-incidence reflectance of an interface n1 -> n2 with an optional
// quarter-wave AR coating tuned to lambda0_nm (0 = uncoated), evaluated at
// lambda_nm. Coating index is max(1.38, sqrt(n1*n2)).
float lens_flare_fresnel_reflectance(float n1, float n2, float lambda0_nm, float lambda_nm);

// In-place radix-2 2D FFT of a size x size complex grid (size must be a power
// of two). Used for the starburst; exposed for tests.
void lens_flare_fft2d(SCP_vector<std::complex<float>>& data, int size, bool inverse);

// Render just the iris mask of an aperture, skipping the starburst transform
// the full generation path would also run. Tests that only care about the mask
// use this to avoid paying for the FFT.
void lens_flare_generate_aperture_mask(const lens_aperture& ap, lens_flare_textures* out);

} // namespace graphics
