#ifndef PROP_FLAGS_H
#define PROP_FLAGS_H

#include "globalincs/flagset.h"

namespace Prop {

FLAG_LIST(Prop_Flags){
	Glowmaps_disabled,                      // No glowmaps for this weapon instance
	Draw_as_wireframe,                      // Render wireframe for this weapon instance
	Render_full_detail,                     // Render full detail for this weapon instance
	Render_without_light,                   // Render without light for this weapon instance
	Render_without_diffuse,                 // Render without diffuse for this weapon instance
	Render_without_glowmap,                 // Render without glowmap for this weapon instance
	Render_without_normalmap,               // Render without normal map for this weapon instance
	Render_without_heightmap,               // Render without height map for this weapon instance
	Render_without_ambientmap,              // Render without ambient for this weapon instance
	Render_without_specmap,                 // Render without spec for this weapon instance
	Render_without_reflectmap,              // Render without reflect for this weapon instance
	Render_with_alpha_mult,                 // Render with an alpha multiplier

	NUM_VALUES};

FLAG_LIST(Info_Flags){
	No_collide = 0,      // No collisions
	No_fred,             // not available in fred
	No_impact_debris,    // wookieejedi - Don't spawn the small debris on impact
	No_lighting,

	NUM_VALUES};

} // namespace Prop
#endif
