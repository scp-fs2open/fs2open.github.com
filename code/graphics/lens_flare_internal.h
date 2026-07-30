#pragma once

#include "graphics/lens_flare.h"

// Private interface between the four lens-flare translation units. Nothing here
// is part of the module's API -- see graphics/lens_flare.h for that.
//
//   lens_flare.cpp           module state, the camera lens, texture cache, and
//                            the per-frame build the render backends consume
//   lens_flare_optics.cpp    the paraxial model: ray-transfer matrices, ghost
//                            enumeration, coated-Fresnel reflectance
//   lens_flare_aperture.cpp  image synthesis: the iris mask and the starburst
//                            that is its Fraunhofer transform
//   lens_flare_table.cpp     lens_flares.tbl / *-lens.tbm parsing
//
// The optics and image-synthesis halves touch no engine state at all; they are
// pure functions of a lens_system / lens_aperture.

namespace graphics {

// Fraunhofer C / d / F lines (red / green / blue), in micrometers. The three
// wavelengths everything chromatic in the flare is evaluated at: dispersion and
// coating reflectance in the optics, diffraction scaling in the starburst.
constexpr float Wavelengths_um[3] = {0.65627f, 0.58756f, 0.48613f};

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
