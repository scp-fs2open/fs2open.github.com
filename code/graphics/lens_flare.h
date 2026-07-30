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
//
// Each layer carries its own operator== because the iris is the one part of the
// camera whose textures cost real time to build (a 512^2 mask plus a 2D FFT), so
// the texture cache keys on it to tell a genuine edit from a slider that landed
// back where it started. Keeping each layer's comparison next to its own fields
// is what stops a newly added field from being silently left out of that check.
// (C++20 would make all four `= default`.)

// Diffraction grating around the rim: fine radial ridges that throw extra spikes
// into the starburst.
struct lens_aperture_grating {
	float strength = 0.0f; // 0 = off
	float density = 0.5f;  // fraction of the 360 possible ridges
	float length = 0.5f;   // how far in the ridges reach, as a fraction of the iris radius
	float width = 0.25f;   // ridge width as a duty cycle of the spacing between ridges
	float softness = 0.0f;

	bool operator==(const lens_aperture_grating& o) const
	{
		return strength == o.strength && density == o.density && length == o.length && width == o.width &&
			   softness == o.softness;
	}
	bool operator!=(const lens_aperture_grating& o) const { return !(*this == o); }
};

// Scratches on the glass: randomly placed and oriented slivers.
struct lens_aperture_scratches {
	float strength = 0.0f; // 0 = off
	float density = 0.5f;  // fraction of the 1000 possible scratches
	float length = 0.5f;
	float width = 0.25f;
	float rotation = 0.0f;           // degrees
	float rotation_variation = 0.0f; // 0 = all parallel, 1 = fully random
	float softness = 0.0f;

	bool operator==(const lens_aperture_scratches& o) const
	{
		return strength == o.strength && density == o.density && length == o.length && width == o.width &&
			   rotation == o.rotation && rotation_variation == o.rotation_variation && softness == o.softness;
	}
	bool operator!=(const lens_aperture_scratches& o) const { return !(*this == o); }
};

// Dust on the glass: randomly placed specks.
struct lens_aperture_dust {
	float strength = 0.0f; // 0 = off
	float density = 0.5f;  // fraction of the 1000 possible specks
	float radius = 0.5f;
	float softness = 0.0f;

	bool operator==(const lens_aperture_dust& o) const
	{
		return strength == o.strength && density == o.density && radius == o.radius && softness == o.softness;
	}
	bool operator!=(const lens_aperture_dust& o) const { return !(*this == o); }
};

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

	lens_aperture_grating grating;
	lens_aperture_scratches scratches;
	lens_aperture_dust dust;

	// Each layer compares itself, so this only has to cover the iris fields and
	// the three layers.
	bool operator==(const lens_aperture& o) const
	{
		return blades == o.blades && rotation == o.rotation && curvature == o.curvature &&
			   softness == o.softness && grating == o.grating && scratches == o.scratches && dust == o.dust;
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
struct lens_streak {
	float strength = 0.0f;                // 0 = off
	float length = 1.0f;                  // half-length as a fraction of the sensor width
	float thickness = 0.02f;              // as a fraction of the length, so 0.02 = 50:1
	float tint[3] = {0.35f, 0.55f, 1.0f}; // multiplies the sun's own colour

	bool operator==(const lens_streak& o) const
	{
		return strength == o.strength && length == o.length && thickness == o.thickness &&
			   tint[0] == o.tint[0] && tint[1] == o.tint[1] && tint[2] == o.tint[2];
	}
	bool operator!=(const lens_streak& o) const { return !(*this == o); }
};

// The anamorphic look -- squeeze plus streak -- bundled the same way
// lens_aperture bundles the iris and its layers, so that it can be tabled,
// overridden and compared against its own neutral defaults as one thing.
//
// Anamorphic squeeze is how much wider than tall the flare footprints are,
// 1.0 = spherical. A front anamorphot is an afocal cylindrical telescope, so to
// first order all it does is magnify one meridian -- which is why this is a
// single number and not a second set of ray-transfer matrices. The lens behind
// it stays rotationally symmetric, so the iris (and hence the mask and its
// transform) is unaffected; only the imaging of it is stretched.
//
// Note that FSO draws the flare into an already-composed frame with no desqueeze
// stage, so this is a look control rather than a 2x-squeeze capture pipeline:
// ghost positions follow the sun as always, and only their footprints are
// stretched, which is what a desqueezed anamorphic frame shows.
struct lens_anamorphic {
	float squeeze = 1.0f;
	lens_streak streak;

	bool operator==(const lens_anamorphic& o) const { return squeeze == o.squeeze && streak == o.streak; }
	bool operator!=(const lens_anamorphic& o) const { return !(*this == o); }
};

// A lens as the tables declare it. Everything here is either the lens's
// prescription -- the part that makes it *this* lens, and that nothing may
// override -- or the tabled baseline of a knob that lens_overrides can restyle.
struct lens_system {
	SCP_string name;

	// ---- the prescription: a lens's identity, never overridden ----
	SCP_vector<lens_surface> surfaces;
	float entrance_radius = 10.0f;     // entrance pupil (front element) radius, mm
	float aperture_radius = 5.0f;      // iris half-opening, mm
	float sensor_width = 36.0f;        // film-gate width, mm
	float coating_wavelength = 540.0f; // default AR coating tuning, nm (0 = uncoated)

	// ---- the tabled look: the baseline lens_overrides lays over ----
	lens_aperture aperture;     // iris shape + imperfections, shared by ghosts and starburst
	lens_anamorphic anamorphic; // squeeze + streak
	float intensity = 0.2f;
	bool starburst = true;
	float starburst_scale = 1.0f;
	int max_ghosts = 40;

	// --- filled by lens_flare_precompute() ---
	// Brightest first, so max_ghosts can be applied at draw time by simply
	// taking a prefix of this.
	SCP_vector<lens_flare_ghost> ghosts;
	float efl = 50.0f;               // effective focal length (green), mm
	float bfd = 40.0f;               // back focal distance last surface -> sensor (green), mm
};

// ---- restyling the camera ----
//
// A mission, the lab, the set-lens-* sexps and both editors all restyle the same
// one camera, so they all write to the same one set of overrides: an unset field
// means "whatever the mounted lens tables", exactly the way an unset Lab_lens
// means "whatever lens the mission mounted" (see lens_flare_active_lens()).
//
// Overriding rather than stamping the values into the lens is what keeps one
// mission's camera out of the next: lens_flare_reset_for_level() clears these
// and every lens is untouched, with nothing to restore and no backup copy that
// could go stale when a field is added.
// The most ghosts a single source's uniform block can ever hold, once the
// starburst and streak have taken their slots. Restated here rather than reached
// through graphics/util/uniform_structs.h so that the editors and the sexps can
// bound "$Max Ghosts:" without pulling the whole uniform layout in; lens_flare.cpp
// static_asserts the two against each other.
constexpr int MAX_LENS_FLARE_GHOSTS = 62;

struct lens_overrides {
	std::optional<lens_aperture> aperture;
	std::optional<lens_anamorphic> anamorphic;
	std::optional<float> intensity;
	std::optional<bool> starburst;
	std::optional<float> starburst_scale;
	std::optional<int> max_ghosts;
	// The energy-model calibration. Per-camera rather than per-lens, since it
	// scales the model itself rather than describing any particular glass.
	std::optional<float> ghost_brightness;
	std::optional<float> starburst_brightness;

	bool any() const
	{
		return aperture || anamorphic || intensity || starburst || starburst_scale || max_ghosts ||
			   ghost_brightness || starburst_brightness;
	}
	void clear() { *this = lens_overrides(); }
};

// The camera as it actually is: a lens's tabled look with the overrides above
// laid over it. Resolved in one place by lens_flare_effective_settings(), so no
// caller re-implements the precedence, and passed down by value so that a
// consumer physically cannot read the un-overridden value off the lens instead.
struct lens_settings {
	lens_aperture aperture;
	lens_anamorphic anamorphic;
	float intensity = 0.2f;
	bool starburst = true;
	float starburst_scale = 1.0f;
	int max_ghosts = 40;
	float ghost_brightness = 64.0f;
	float starburst_brightness = 1.6f;
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

// The overrides in force, for reading and for editing in place. One set, because
// there is one camera: mission load, the set-lens-* sexps, the lab and both
// editors are all restyling the same glass.
//
// Anything that edits these must follow up with lens_flare_overrides_changed().
lens_overrides& lens_flare_overrides();

// Note that the overrides were edited. Only the iris costs anything to change --
// a 512^2 mask plus its 2D FFT -- so this schedules a texture rebuild, which the
// per-frame flush then performs at most once every few frames and only if the
// effective aperture really did move. Everything else takes effect next frame at
// no cost, so calling this after any edit is always correct and never wasteful.
void lens_flare_overrides_changed();

// Whether a scheduled iris rebuild is still outstanding. For the lab, which
// shows it while a slider drag is being coalesced. Call once per frame.
bool lens_flare_aperture_edit_pending();

// The mounted lens's tabled look with the overrides above laid over it -- the
// single resolver, so no caller re-implements the precedence. Returns the plain
// defaults for an invalid index.
lens_settings lens_flare_effective_settings(int lens_idx);

// Lazily generate (and cache) the iris/starburst textures of the effective
// aperture of a lens. Returns nullptr for an invalid index.
//
// There is one cache, because there is one camera: whichever lens is mounted,
// with whatever iris is in force, is the only pair anything ever draws with.
const lens_flare_textures* lens_flare_get_textures(int lens_idx);

// Generate the mounted lens's textures now, so the render backends find them
// already cached instead of paying for them mid-frame.
//
// Building them is a 512^2 iris mask plus a 2D FFT of it -- a visible hitch if it
// lands on the first frame a sun flares. Call it from wherever a lens has just
// been mounted for a scene that is about to be rendered and a moment's work is
// already expected: stars_post_level_init() for a mission, the lab's
// useBackground(), and qtFred's Background Editor when it switches the mission's
// lens interactively.
//
// A no-op everywhere the flare pass never runs regardless: plain FRED, and
// qtFred unless its View menu's "Enable Post Processing" toggle is on (qtFred
// otherwise draws the background without a scene texture, so High_dynamic_range
// never goes true -- see lens_flare.cpp's pass_globally_possible()).
void lens_flare_prime_textures();

// Bumped whenever the cached textures are rebuilt. Render backends cache the
// uploaded copy, so they must key that cache on this to notice a rebuild --
// which lens_flare_textures_if_changed() below does for them.
unsigned int lens_flare_get_texture_generation();

// The whole staleness protocol a render backend needs, in one call: returns the
// textures to upload, or nullptr when the ones the caller already holds are
// still current. `cached_lens` / `cached_generation` are the backend's own record
// of what it last uploaded, and are updated on a non-null return.
//
// Backends own their GPU handles; they do not each need to re-derive when those
// handles went stale, which is a rule about this module and belongs here.
const lens_flare_textures* lens_flare_textures_if_changed(int lens_idx, int& cached_lens,
	unsigned int& cached_generation);

// Undo everything a mission or the lab did to the camera: unmount whatever lens
// was mounted (back to $Default Lens:) and drop every override, so one mission's
// camera can't carry into the next. Called from stars_pre_level_init(), which
// runs before the mission's $Camera Lens: is parsed.
void lens_flare_reset_for_level();

// The calibration that is neither per-lens nor per-mission: how the flare is fitted
// to HDR output, and whether nozzles draw ghosts. Handed out mutably for the lab
// to edit in place; values are sanitized where they are consumed, so a caller
// cannot break the renderer by writing a silly number here.
struct lens_flare_tuning {
	// How many multiples of paper white the flare may reach in HDR output. SDR is
	// the calibration reference and is unaffected; this only rescales the HDR path
	// so an SDR-tuned flare doesn't blow out. The default keeps a little HDR "pop".
	// Not overridable per mission: it describes the display, not the camera.
	float hdr_headroom = 2.5f;

	// Whether a thruster flare draws the ghost train as well as its starburst.
	// Off, because unlike a sun an engine is one of dozens of small sources in
	// frame: a ghost train each is both the expensive part of the pass and, at
	// that count, visual noise rather than an optical effect you can read.
	// Exposed so the lab can turn them on and show what they cost and look like.
	// Suns are unaffected and always draw theirs.
	bool thruster_ghosts = false;
};

lens_flare_tuning& lens_flare_get_tuning();

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

// The two names that stand in for a lens instead of naming one. The mission's
// "$Camera Lens:", the set-camera-lens sexp and both editors all speak this same
// vocabulary, so it lives here with the code that resolves it rather than being
// re-spelled at each of those.
#define LENS_NAME_NONE    "<none>"
#define LENS_NAME_DEFAULT "<default>"

// Mount a lens, resolving the whole vocabulary above in one place:
//
//   ""/nullptr      the caller has no opinion -> the table default. This is what
//                   a mission without a "$Camera Lens:" gets, which is why it
//                   means "default" and not "none".
//   <none>          no lens, hence no flares, even when a default exists. The
//                   only way to say that, and the reason it is a token rather
//                   than an empty string.
//   <default>       the table default, said explicitly.
//   a lens name     that lens; an unknown name warns and falls back to the
//                   default, since a typo shouldn't silently look like <none>.
//
// Called from mission parse, the set-camera-lens sexp, the lab and both editors.
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
//
// Suns only: an engine's glow is the light source the flare is *of*, not a
// competing sprite of the same artifact, so a thruster flare never makes the
// thruster glow step aside.
bool lens_flare_sun_starburst_drawn(int sun_n);

// What a draw images. Diagnostics for the lab -- the pass draws every kind the
// same way, through the same lens.
enum class flare_source_kind {
	sun,
	thruster,
	beam,
};

// One light source's worth of flare quads. Every source shares the mounted lens
// (hence one aperture/starburst texture for the whole pass), but each has its own
// flare axis and tint, so each gets its own uniform block and instanced draw.
//
// The trailing fields are diagnostics for the lab; the renderer ignores them.
struct lens_flare_draw {
	flare_source_kind kind = flare_source_kind::sun;
	// Which sun (a stars.tbl instance index), which ship (an objnum, for a
	// thruster) or which beam (an objnum, for a beam) this draw images.
	int source_index = -1;
	int instances = 0; // quads to draw (ghosts + optional starburst)
	// Uniform block for this draw, owned by lens_flare.cpp; valid until the
	// next lens_flare_frame_update() call.
	const generic_data::lens_flare_data* data = nullptr;

	// The 0..1 fade already folded into this draw's tint that isn't the source's
	// own tabled brightness: for a sun, smoothed occlusion times the off-axis
	// fade; for a thruster, the throttle; for a beam, its warmup/warmdown ramp.
	float visibility = 0.0f;
	float off_axis_deg = 0.0f; // paraxial field angle of the source
	float output_scale = 1.0f; // SDR/HDR consistency multiplier applied this frame
};

// Decide what the flare pass will draw this frame and publish it, once per scene
// render. Does all the game-state access (sun projection, occlusion raycast,
// gates) and all the deferred work (flushing throttled aperture rebuilds), so
// that everything downstream is a pure read.
//
// Called from stars_draw(), which is the one place that both runs after the view
// and projection matrices are live and runs before the sun sprites and the
// post-processing pass consume the result.
void lens_flare_frame_update();

// Publish an empty frame: nothing flares, so every consumer reads "no".
//
// For a scene render that cannot reach the flare pass at all -- an environment map
// goes straight to a render target, outside the post-processing chain. Publishing
// nothing rather than skipping the publish is deliberate: it keeps the answer in
// one place, so no consumer has to know where it is being called from, and it
// stops the previous frame's published draws from being read by a render that
// isn't going to draw them.
void lens_flare_clear_frame();

// What the last lens_flare_frame_update() published: one entry per light source
// that has something to draw -- suns first, in sun order, then thrusters
// (empty = skip the pass entirely). Every entry is drawn with the textures of
// lens_flare_active_lens().
//
// The single source of truth for the pass -- the render backends draw exactly
// these, the sun renderer asks lens_flare_sun_starburst_drawn() about them, and
// the lab reports on them.
const SCP_vector<lens_flare_draw>& lens_flare_get_frame_draws();

// ---- thruster flares ----
//
// Engines are the other intensely bright thing in a FreeSpace scene, so they
// flare through the same camera lens the suns do. What differs is the source: a
// sun is a point at infinity with a tabled colour and a shadow test, while a
// nozzle is a finite source whose brightness follows the throttle, the
// afterburner, how squarely it faces the camera, and how far away it is.
//
// Every lit nozzle is its own source. Averaging a ship's engines into one was
// tried first and is wrong for the ships it matters on: a capital ship's engines
// are set far enough apart to be separate points in frame, so a single flare at
// their centroid sits where no engine is.
//
// Declared per species in species_defs.tbl ("$Thruster Flare:"), and off unless
// a species asks for it -- so no existing mod gains flares it never tabled, and
// a mission with no camera lens mounted still gets none either way.

struct thruster_flare_info {
	// False until a species_defs.tbl entry declares "$Thruster Flare:". Tables
	// written before this existed have no such block, so their engines keep
	// flaring exactly as much as they used to: not at all.
	bool enabled = false;

	// Brightness at the reference apparent size -- one nozzle of radius r seen
	// from 32r away (see lens_flare_thrusters.cpp) -- scaling linearly from there.
	//
	// The defaults are starting points for tuning in the lab, not derived values.
	// They are this large because at any real combat range a nozzle subtends a
	// small fraction of the reference, so a value near 1.0 puts the whole effect
	// below the level a pixel can show -- which is what made engines look like
	// they only flared under afterburner.
	//
	// The afterburner figure *replaces* the normal one while the burner or a
	// booster is lit rather than multiplying it, so a species can make the two
	// states independently bright without doing division in the table.
	float intensity = 6.0f;
	float afterburner_intensity = 15.0f;

	// Linear rgb the flare is tinted with, multiplying the lens's own tint the
	// same way a sun's colour does.
	vec3d color = {{{1.0f, 1.0f, 1.0f}}};
};

// The lab's live override of every species' thruster-flare settings: unset means
// each species' own table entry stands.
//
// One override for all species rather than one per species, because the lab
// shows one ship at a time -- and, more usefully, because it leaves the tabled
// values untouched, so nothing has to be backed up and restored between missions
// the way a lens's edited aperture does.
//
// Handed out mutably, like lens_flare_get_tuning(). Cleared by
// lens_flare_reset_for_level().
std::optional<thruster_flare_info>& lens_flare_lab_thruster_flare();

// The settings that actually apply to a species this frame: its own, unless the
// lab is overriding. The single resolver, so no caller re-implements the
// precedence (compare lens_flare_active_lens()). An unknown species gets the
// defaults, which are "off".
thruster_flare_info lens_flare_thruster_settings(int species_idx);

// ---- internals exposed for unit testing ----

// The name in "$Default Lens:", or "" when the table declares no default.
//
// Diagnostic only: nothing needs it to *resolve* a default any more, because
// lens_flare_switch_to() does that for every caller (an empty or <default> name
// lands on it). Kept for the load-time log line and so a test can assert what a
// table declared.
const char* lens_flare_default_name();

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
