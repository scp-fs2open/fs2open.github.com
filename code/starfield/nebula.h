/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#ifndef _NEBULA_H
#define _NEBULA_H

#include "globalincs/pstypes.h"

// mainly only needed by Fred
extern int Nebula_pitch;
extern int Nebula_bank;
extern int Nebula_heading;

struct angles;

// A placed cloud in a generated nebula pattern: a soft ellipse on the nebula's flat map, the same
// map the FS1 nebula meshes were drawn on.  Longitude wraps, so a cloud can cross the seam.
//   +Cloud: lon lat width height angle brightness
struct generated_nebula_cloud {
	float lon = 0.0f;          // center longitude in degrees, 0..360
	float lat = 0.0f;          // center latitude in degrees, -90..90
	float width = 30.0f;       // size across, in degrees of longitude (full width at half brightness)
	float height = 10.0f;      // size along the map's other axis, on the same scale as width
	float angle = 0.0f;        // rotation of the ellipse on the map, in degrees
	float brightness = 0.5f;   // peak brightness, 0..1 (overlapping clouds add up, capped at 1)
};

// A generated (FS1-style) background nebula pattern, made of two layers:
//   - a procedural background from noise (the density..seed fields), unless +Clouds Only is set
//   - placed clouds (+Cloud), broken up by the same noise according to +Cloud Detail
// The layers combine with a screen blend, so clouds brighten the background without clipping.
// Patterns and colors come from generated_nebula.tbl (#Generated Nebula Patterns / #Generated
// Nebula Colors); the built-in one holds the FS1 set (Nebula01..03 as clouds-only patterns laid out
// like the FS1 originals, and FS1's nine colors).  *-gneb.tbm files add entries or override them by
// name; an entry that lists any +Cloud replaces the pattern's whole cloud list.
struct generated_nebula_pattern {
	SCP_string name;
	SCP_string mesh;           // +Mesh: an FS1 .neb mesh drawn instead of the generated clouds when the file exists
	SCP_string palette_prefix; // +Palette Prefix: with a color's +Palette Suffix: names the FS1 palette file (<prefix>-<suffix>.pcx)
	SCP_vector<generated_nebula_cloud> clouds;
	bool  clouds_only  = false;    // no procedural background, only the placed clouds
	float cloud_detail = 0.35f;    // how much the noise breaks up the clouds: 0 = smooth, 1 = ragged
	float density  = 0.30f;    // fraction of bright "knots" (mostly-black otherwise)
	float freq_u   = 2.0f;     // noise frequency along longitude (unequal u/v => streaky)
	float freq_v   = 5.0f;     // noise frequency along latitude
	int   octaves  = 4;        // fbm octaves; fewer = smoother/broader masses (1-2 = FS1-like), more = busier
	float warp     = 0.4f;     // directional warp strength (stretches the wisps)
	float contrast = 1.5f;     // falloff curve applied to bright knots
	float intensity = 1.0f;    // overall brightness multiplier (channels are clamped to max)
	int   seed     = 0;        // noise seed (gives each pattern a distinct look)
	int   res_lon  = 24;       // longitude grid resolution (coarse = retro chunky gouraud)
	int   res_lat  = 12;       // latitude grid resolution
	float band_min = 0.0f;     // latitude extent in 0..1; full sphere by default (poles closed)
	float band_max = 1.0f;
};

// A named tint color for the generated nebula (generated_nebula.tbl #Generated Nebula Colors).
struct generated_nebula_color {
	SCP_string name;
	SCP_string palette_suffix; // +Palette Suffix: see generated_nebula_pattern::palette_prefix; the file's 20-entry ramp replaces the RGB fade when it exists
	ubyte r = 255;
	ubyte g = 255;
	ubyte b = 255;
};

extern SCP_vector<generated_nebula_pattern> Generated_nebula_patterns;
extern SCP_vector<generated_nebula_color>   Generated_nebula_colors;

// Clear the registries above and fill them from generated_nebula.tbl and any *-gneb.tbm.  Called
// once from neb2_init().
void generated_nebula_init();

// Look up a pattern/color by name; returns the registry index or -1 if not found.
int generated_nebula_pattern_lookup(const char *name);
int generated_nebula_color_lookup(const char *name);

// Safe name accessors for the registries (return "" when the index is out of range).
const char *generated_nebula_pattern_name(int index);
const char *generated_nebula_color_name(int index);

extern int Nebula_mission_switch;
// true when the background nebula should be drawn: the mission line when present, else the mod switch
bool nebula_backgrounds_enabled();

// +Nebula: / +Color: names that matched no table entry, kept so saving writes them back unchanged
// instead of dropping the pattern or switching the color
extern SCP_string Nebula_unknown_pattern;
extern SCP_string Nebula_unknown_color;

// The default color index: "Blue" (FS1's default) if the tables define it, else the first color,
// else -1 when there are no colors at all
int generated_nebula_default_color();

// PBH = Pitch, Bank, Heading (in degrees).  index < 0 disables the nebula.
void nebula_init( int index, int pitch, int bank, int heading );
void nebula_close();
// Renders the background nebula (no-op if none is active).
void nebula_render();

#endif	//_NEBULA_H
