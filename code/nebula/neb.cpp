/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "debugconsole/console.h"
#include "freespace.h"
#include "jpgutils/jpgutils.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "object/object.h"
#include "options/Option.h"
#include "parse/parselo.h"
#include "pcxutils/pcxutils.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "tgautils/tgautils.h"
#include "tracing/tracing.h"
#include "graphics/light.h"


// --------------------------------------------------------------------------------------------------------
// NEBULA DEFINES/VARS
//

bool Nebula_sexp_used = false;

ubyte Neb2_fog_color[3] = { 0,0,0 };

// #define NEB2_THUMBNAIL

/*
3D CARDS THAT FOG PROPERLY
Voodoo1
Voodoo2
G200
TNT

3D CARDS THAT DON'T FOG PROPERLY
Permedia2
AccelStar II
*/

// if nebula rendering is active (DCF stuff - not mission specific)
int Neb2_render_mode = NEB2_RENDER_NONE;

SCP_vector<poof_info> Poof_info;

float Poof_dist_threshold;
vec3d Poof_last_gen_pos;
float Poof_accum[MAX_NEB2_POOFS];
float Poof_density_multiplier;

const float UPKEEP_DIST_MULT = 1.2f;

const float PROBABLY_TOO_MANY_POOFS = 100000.0f;

// array of neb2 poofs
int32_t Neb2_poof_flags = 0;

// array of neb2 bitmaps
char Neb2_bitmap_filenames[MAX_NEB2_BITMAPS][MAX_FILENAME_LEN] = {
	"", "", "", "", "", ""
};
int Neb2_bitmap[MAX_NEB2_BITMAPS] = { -1, -1, -1, -1, -1, -1 };
int Neb2_bitmap_count = 0;

// texture to use for this level
char Neb2_texture_name[MAX_FILENAME_LEN] = "";

float max_rotation = 3.75f;
float neb2_flash_fade = 0.3f;

//WMC - these were originally indexed to SHIP_TYPE_FIGHTER_BOMBER
const static float Default_fog_near = 10.0f;
const static float Default_fog_far = 750.0f;

// fog near and far values for rendering the background nebula
#define NEB_BACKG_FOG_NEAR_GLIDE		2.5f
#define NEB_BACKG_FOG_NEAR_D3D			4.5f
#define NEB_BACKG_FOG_FAR_GLIDE			10.0f
#define NEB_BACKG_FOG_FAR_D3D			10.0f
float Neb_backg_fog_near = NEB_BACKG_FOG_NEAR_GLIDE;
float Neb_backg_fog_far = NEB_BACKG_FOG_FAR_GLIDE;

// stats
int pneb_tried = 0;				// total pnebs tried to render
int pneb_tossed_alpha = 0;		// pnebs tossed because of alpha optimization
int pneb_tossed_dot = 0;		// pnebs tossed because of dot product
int pneb_tossed_off = 0;		// pnebs tossed because of being offscree
int neb_tried = 0;				// total nebs tried
int neb_tossed_alpha = 0;		// nebs tossed because of alpha
int neb_tossed_dot = 0;			// nebs tossed because of dot product
int neb_tossed_count = 0;		// nebs tossed because of max render count 

// the AWACS suppression level for the nebula
float Neb2_awacs = -1.0f;

// The visual render distance multipliers for the nebula
float Neb2_fog_near_mult = 1.0f;
float Neb2_fog_far_mult = 1.0f;

// this is the percent of visibility at the fog far distance
const float NEB_FOG_FAR_PCT = 0.1f;

float Neb2_fog_visibility_trail = 1.0f;
float Neb2_fog_visibility_thruster = 1.5f;
float Neb2_fog_visibility_weapon = 1.3f;
float Neb2_fog_visibility_shield = 1.2f;
float Neb2_fog_visibility_glowpoint = 1.2f;
float Neb2_fog_visibility_beam_const = 4.0f;
float Neb2_fog_visibility_beam_scaled_factor = 0.1f;
float Neb2_fog_visibility_particle_const = 1.0f;
float Neb2_fog_visibility_particle_scaled_factor = 1.0f / 12.0f;
float Neb2_fog_visibility_shockwave = 2.5f;
float Neb2_fog_visibility_fireball_const = 1.2f;
float Neb2_fog_visibility_fireball_scaled_factor = 1.0f / 12.0f;


SCP_vector<poof> Neb2_poofs;

int Neb2_background_color[3] = {0, 0, 255};			// rgb background color (used for lame rendering)

const SCP_vector<std::pair<int, SCP_string>> DetailLevelValues = {{ 0, "Minimum" },
                                                                  { 1, "Low" },
                                                                  { 2, "Medium" },
                                                                  { 3, "High" },
                                                                  { 4, "Ultra" }, };

const auto ModelDetailOption = options::OptionBuilder<int>("Graphics.NebulaDetail",
                                                           "Nebula Detail",
                                                           "Detail level of nebulas").category("Graphics").values(
	DetailLevelValues).default_val(MAX_DETAIL_LEVEL).importance(7).change_listener([](int val, bool) {
	Detail.nebula_detail = val;
	return true;
}).finish();

// --------------------------------------------------------------------------------------------------------
// NEBULA FORWARD DECLARATIONS
//

// do a pre-render of the background nebula
void neb2_pre_render(camid cid);

// fill in the position of the eye for this frame
void neb2_get_eye_pos(vec3d *eye_vector);

// fill in the eye orient for this frame
void neb2_get_eye_orient(matrix *eye_matrix);

// --------------------------------------------------------------------------------------------------------
// NEBULA FUNCTIONS
//

static poof_info* get_nebula_poof_pointer(char* nebula_name)
{
	for (int i = 0; i < (int)Poof_info.size(); i++) {
		if (!stricmp(nebula_name, Poof_info[i].name)) {
			return &Poof_info[i];
		}
	}

	// Didn't find anything.
	return nullptr;
}

void parse_nebula_table(const char* filename)
{
	char name[MAX_FILENAME_LEN];

	try
	{
		// read in the nebula.tbl
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// background bitmaps
		while (!optional_string("#end")) {
			// nebula
			optional_string("+Nebula:");
			stuff_string(name, F_NAME, MAX_FILENAME_LEN);

			if (Neb2_bitmap_count < MAX_NEB2_BITMAPS) {
				strcpy_s(Neb2_bitmap_filenames[Neb2_bitmap_count++], name);
			}
			else {
				WarningEx(LOCATION, "nebula.tbl\nExceeded maximum number of nebulas (%d)!\nSkipping %s.", MAX_NEB2_BITMAPS, name);
			}
		}

		// poofs
		while (required_string_one_of(3, "#end", "+Poof:", "$Name:")) {
			bool create_if_not_found = true;
			poof_info pooft;
			poof_info* poofp;
			bool poof_new = true;

			if (optional_string("+Poof:")) { // retail style
				stuff_string(name, F_NAME, MAX_FILENAME_LEN);
				strcpy_s(pooft.bitmap_filename, name);

				strcpy_s(pooft.name, name);

				generic_anim_init(&pooft.bitmap, name);

				if (Poof_info.size() < MAX_NEB2_POOFS) {
					Poof_info.push_back(pooft);
				} else {
					Warning(LOCATION, "More than 32 poofs are defined! Skipping poof %s\n", pooft.name);
				}
			} else if (optional_string("$Name:")) { // new style
				stuff_string(pooft.name, F_NAME, NAME_LENGTH);

				if (optional_string("+nocreate")) {
					if (!Parsing_modular_table) {
						Warning(LOCATION, "+nocreate flag used for nebula poof in non-modular table\n");
					}
					create_if_not_found = false;
				}

				// Does this poof exist already?
				// If so, load this new info into it
				poofp = get_nebula_poof_pointer(pooft.name);
				if (poofp != nullptr) {
					if (!Parsing_modular_table) {
						error_display(1,
							"Error:  Nebula Poof %s already exists.  All nebula poof names must be unique.",
							pooft.name);
					}
					poof_new = false;
				} else {
					// Don't create poof if it has +nocreate and is in a modular table.
					if (!create_if_not_found && Parsing_modular_table) {
						if (!skip_to_start_of_string_either("$Name:", "#end")) {
							error_display(1, "Missing [#end] or [$Name] after nebula poof %s", pooft.name);
						}
						continue;
					}
					// Check if we're at max poofs. If so, then log and continue.
					if (Poof_info.size() < MAX_NEB2_POOFS) {
						Poof_info.push_back(pooft);
						poofp = &Poof_info[Poof_info.size() - 1];
					} else {
						Warning(LOCATION, "More than 32 poofs are defined! Skipping poof %s\n", pooft.name);
						if (!skip_to_start_of_string_either("$Name:", "#end")) {
							error_display(1, "Missing [#end] or [$Name] after nebula poof %s", pooft.name);
						}
						continue;
					}
				}

				if (poof_new) {
					required_string("$Bitmap:");
					stuff_string(name, F_NAME, MAX_FILENAME_LEN);
					strcpy_s(poofp->bitmap_filename, name);
					generic_anim_init(&poofp->bitmap, name);
				} else {
					if (optional_string("$Bitmap:")) {
						stuff_string(name, F_NAME, MAX_FILENAME_LEN);
						strcpy_s(poofp->bitmap_filename, name);
						generic_anim_init(&poofp->bitmap, name);
					}
				}

				if (optional_string("$Scale:"))
					poofp->scale = ::util::parseUniformRange<float>(0.01f, 100000.0f);

				if (optional_string("$Density:")) {
					stuff_float(&poofp->density);
					if (poofp->density <= 0.0f) {
						Warning(LOCATION, "Poof %s must have a density greater than 0.", poofp->name);
						poofp->density = 150.0f;
					}
					poofp->density = 1 / (poofp->density * poofp->density * poofp->density);
				}

				if (optional_string("$Rotation:"))
					poofp->rotation = ::util::parseUniformRange<float>(-1000.0f, 1000.0f);

				if (optional_string("$View Distance:")) {
					stuff_float(&poofp->view_dist);
					if (poofp->view_dist < 0.0f) {
						Warning(LOCATION, "Poof %s must have a positive view distance.", poofp->name);
						poofp->view_dist = 360.f;
					}

					float volume = PI * 4 / 3 * (poofp->view_dist * poofp->view_dist * poofp->view_dist);
					if (volume * poofp->density > PROBABLY_TOO_MANY_POOFS) {
						Warning(LOCATION, "Poof %s will have over 100,000 poofs on the field at once, and could cause serious performance issues. "
							"Remember that as $Density decreases and $View Distance increases, the total number of "
							"poofs increases exponentially.",
							poofp->name);
					}
				}

				if (optional_string("$Alpha:")) {
					poofp->alpha = ::util::parseUniformRange<float>(0.0f, 1.0f);
				}
			}
			else {
				WarningEx(LOCATION, "nebula.tbl\nExceeded maximum number of nebula poofs (%d)!\nSkipping %s.", (int)MAX_NEB2_POOFS, name);
			}
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", "nebula.tbl", e.what()));
		return;
	}
}

// initialize neb2 stuff at game startup
void neb2_init()
{

	Neb2_bitmap_count = 0;
	// first parse the default table
	parse_nebula_table("nebula.tbl");

	// parse any modular tables
	parse_modular_table("*-neb.tbm", parse_nebula_table);
}

bool poof_is_used(size_t idx) {
	return (Neb2_poof_flags & (1 << idx)) != 0;
}

void neb2_get_fog_color(ubyte *r, ubyte *g, ubyte *b)
{
	if (r) *r = Neb2_fog_color[0];
	if (g) *g = Neb2_fog_color[1];
	if (b) *b = Neb2_fog_color[2];
}

void neb2_level_init()
{
	Nebula_sexp_used = false;
}

float nNf_near, nNf_density;

void neb2_poof_setup() {
	if (!Neb2_poof_flags)
		return;

	// make the total density of poofs be the average of all poofs, and each poofs density is its relative proportion compared to others
	// this way we maintain the retail way of not affecting total density by having more poof types
	float Poof_density_sum_square = 0.0f;
	float Poof_density_sum = 0.0f;

	// also determine the minimum distance before re-triggering a poof upkeep
	Poof_dist_threshold = 9999.0f;
	for (size_t i = 0; i < Poof_info.size(); i++) {
		if (poof_is_used(i)) {
			Poof_density_sum_square += Poof_info[i].density * Poof_info[i].density;
			Poof_density_sum += Poof_info[i].density;

			float dist_threshold = Poof_info[i].view_dist * (UPKEEP_DIST_MULT - 1.0f);
			if (dist_threshold < Poof_dist_threshold)
				Poof_dist_threshold = dist_threshold;
		}
	}
	Poof_density_multiplier = Poof_density_sum_square / (Poof_density_sum * Poof_density_sum);
	Poof_density_multiplier *= (Detail.nebula_detail + 0.5f) / (MAX_DETAIL_LEVEL + 0.5f); // scale the poofs down based on detail level
}

void neb2_generate_fog_color(const char *fog_color_palette, ubyte fog_color[])
{
	// Set a default colour just in case something goes wrong
	fog_color[0] = 30;
	fog_color[1] = 52;
	fog_color[2] = 157;

	if (!fog_color_palette || !strlen(fog_color_palette))
		return;

	auto fog_data = new ubyte[768];

	if (pcx_read_header(fog_color_palette, nullptr, nullptr, nullptr, nullptr, fog_data) == PCX_ERROR_NONE) {
		// based on the palette, get an average color value (this doesn't really account for actual pixel usage though)
		ushort r = 0, g = 0, b = 0, pcount = 0;
		for (int idx = 0; idx < 768; idx += 3) {
			if (fog_data[idx] || fog_data[idx+1] || fog_data[idx+2]) {
				r = r + fog_data[idx];
				g = g + fog_data[idx+1];
				b = b + fog_data[idx+2];
				pcount++;
			}
		}

		if (pcount > 0) {
			fog_color[0] = (ubyte)(r / pcount);
			fog_color[1] = (ubyte)(g / pcount);
			fog_color[2] = (ubyte)(b / pcount);
		} else {
			// it's just black
			fog_color[0] = fog_color[1] = fog_color[2] = 0;
		}
	}

	// done, now free up the palette data
	delete[] fog_data;
}

// initialize nebula stuff - call from game_post_level_init(), so the mission has been loaded
void neb2_post_level_init(bool fog_color_override)
{
	// standalone servers can bail here
	if (Game_mode & GM_STANDALONE_SERVER) {
		return;
	}

	// Skip actual rendering if we're in FRED.
	if (Fred_running)
	{
		Neb2_render_mode = NEB2_RENDER_NONE;
		return;
	}

	// if the mission is not a fullneb mission, skip
	if ( !((The_mission.flags[Mission::Mission_Flags::Fullneb]) || Nebula_sexp_used) ) {
		Neb2_render_mode = NEB2_RENDER_NONE;
		Neb2_awacs = -1.0f;
		return;
	}

	// OK, lets try something a bit more interesting
	if (!fog_color_override && strlen(Neb2_texture_name)) {
		neb2_generate_fog_color(Neb2_texture_name, Neb2_fog_color);
	}

	Neb2_render_mode = NEB2_RENDER_HTL;

	// load in all nebula bitmaps
	for (poof_info &pinfo : Poof_info) {
		if (pinfo.bitmap.first_frame < 0) {
			pinfo.bitmap.first_frame = bm_load(pinfo.bitmap_filename);

			if (pinfo.bitmap.first_frame >= 0) {
				pinfo.bitmap.num_frames = 1;
				pinfo.bitmap.total_time = 1.0f;
			}		// fall back to an animated type
			else if (generic_anim_load(&pinfo.bitmap)) {
				mprintf(("Could not find a usable bitmap for nebula poof '%s'!\n", pinfo.name));
				Warning(LOCATION, "Could not find a usable bitmap (%s) for nebula poof '%s'!\n", pinfo.bitmap_filename, pinfo.name);
			}
		}
	}

	pneb_tried = 0;
	pneb_tossed_alpha = 0;
	pneb_tossed_dot = 0;
	neb_tried = 0;
	neb_tossed_alpha = 0;
	neb_tossed_dot = 0;
	neb_tossed_count = 0;

	// setup proper fogging values
	Neb_backg_fog_near = NEB_BACKG_FOG_NEAR_D3D;
	Neb_backg_fog_far = NEB_BACKG_FOG_FAR_D3D;

	// if we are going to use fullneb, but aren't fullneb yet, then be sure to reset our mode
	if ( !(The_mission.flags[Mission::Mission_Flags::Fullneb]) ) {
		Neb2_render_mode = NEB2_RENDER_NONE;
		Neb2_awacs = -1.0f;
	}

	// truncate the poof flags down to the poofs we have
	if (Poof_info.size() < MAX_NEB2_POOFS) {
		int available_poofs_mask = (1 << Poof_info.size()) - 1;

		// check for negative here too, because if we're not at max, 32, then we're gauranteed not to have the sign bit
		if (Neb2_poof_flags > available_poofs_mask || Neb2_poof_flags < 0)
			Warning(LOCATION, "One or more invalid nebula poofs detected!");

		Neb2_poof_flags = Neb2_poof_flags & available_poofs_mask;
	}

	// set the mission fog near dist and density
	float fog_far;
	neb2_get_adjusted_fog_values(&nNf_near, &fog_far, &nNf_density, nullptr);

	for (float& accum : Poof_accum)
		accum = 0.0f;

	// a bit awkward but this will force a full sphere gen
	vm_vec_make(&Poof_last_gen_pos, 999999.0f, 999999.0f, 999999.0f);

	neb2_poof_setup();
}

// shutdown nebula stuff
void neb2_level_close()
{
	// standalone servers can bail here
	if (Game_mode & GM_STANDALONE_SERVER) {
		return;
	}

	// if the mission is not a fullneb mission, skip
	if ( !((The_mission.flags[Mission::Mission_Flags::Fullneb]) || Nebula_sexp_used) ) {
		return;
	}

	// unload all nebula bitmaps
	for (poof_info& pinfo : Poof_info) {
		if (pinfo.bitmap.first_frame >= 0) {
			bm_release(pinfo.bitmap.first_frame);
			pinfo.bitmap.first_frame = -1;
		}
	}

	// clear da poofs
	Neb2_poofs.clear();

	// unflag the mission as being fullneb so stuff doesn't fog in the techdata room :D
    The_mission.flags.remove(Mission::Mission_Flags::Fullneb);
}

// call before beginning all rendering
void neb2_render_setup(camid cid)
{
	GR_DEBUG_SCOPE("Nebula Setup");
	TRACE_SCOPE(tracing::SetupNebula);

	// standalone servers can bail here
	if (Game_mode & GM_STANDALONE_SERVER) {
		return;
	}

	// if the mission is not a fullneb mission, skip
	if ( !(The_mission.flags[Mission::Mission_Flags::Fullneb]) ) {
		return;
	}

	if (Neb2_render_mode == NEB2_RENDER_HTL) {
		// RT The background needs to be the same colour as the fog and this seems
		// to be the ideal place to do it
		ubyte tr = gr_screen.current_clear_color.red;
		ubyte tg = gr_screen.current_clear_color.green;
		ubyte tb = gr_screen.current_clear_color.blue;

		neb2_get_fog_color(
			&gr_screen.current_clear_color.red,
			&gr_screen.current_clear_color.green,
			&gr_screen.current_clear_color.blue);

		gr_clear();

		gr_screen.current_clear_color.red   = tr;
		gr_screen.current_clear_color.green = tg;
		gr_screen.current_clear_color.blue  = tb;

		return;
	}

	// pre-render the real background nebula
	neb2_pre_render(cid);
}

// level paging code
void neb2_page_in()
{
	// load in all nebula bitmaps
	if ( (The_mission.flags[Mission::Mission_Flags::Fullneb]) || Nebula_sexp_used ) {
		for (size_t idx = 0; idx < Poof_info.size(); idx++) {
			if (Poof_info[idx].bitmap.first_frame >= 0 && poof_is_used(idx)) {
				bm_page_in_texture(Poof_info[idx].bitmap.first_frame);
			}
		}
	}
}

// should we not render this object because its obscured by the nebula?
int neb_skip_opt = 0;
DCF(neb_skip, "Toggles culling of objects obscured by nebula")
{
	neb_skip_opt = !neb_skip_opt;
	if (neb_skip_opt) {
		dc_printf("Using neb object skipping!\n");
	} else {
		dc_printf("Not using neb object skipping!\n");
	}
}
int neb2_skip_render(object *objp, float z_depth)
{
	float fog_near, fog_far, fog_density;

	// if we're never skipping
	if (!neb_skip_opt) {
		return 0;
	}
	
	// get near and far fog values based upon object type and rendering mode
	neb2_get_adjusted_fog_values(&fog_near, &fog_far, &fog_density);
	float fog = pow(fog_density, z_depth - fog_near + objp->radius);

	// by object type
	switch( objp->type ) {
	// some objects we always render
		case OBJ_SHOCKWAVE:
		case OBJ_JUMP_NODE:
		case OBJ_NONE:
		case OBJ_GHOST:
		case OBJ_BEAM:
		case OBJ_WAYPOINT:
		return 0;

		// any weapon over 500 meters away
		// Use the "far" distance multiplier here
		case OBJ_WEAPON:
			if (fog < 0.05f) {
				return 1;
			}
			break;

		// any ship less than 3% visible at their closest point
		case OBJ_SHIP:
			if (fog < 0.03f)
				return 1;
			break;

		// any fireball over the fog limit for small ships
		case OBJ_FIREBALL:
			return 0;
			break;

		// any debris over the fog limit for small ships
		case OBJ_DEBRIS:
			return 0;
			break;

		// any asteroid less than 3% visible at their closest point
		case OBJ_ASTEROID:
			if (fog < 0.03f)
				return 1;
			break;

		// hmmm. unknown object type - should probably let it through
		default:
			Int3();
		return 0;
	}
	return 0;
}

// extend LOD 
float neb2_get_lod_scale(int objnum)
{
	ship *shipp;
	ship_info *sip;

	// bogus
	if ( (objnum < 0) 
		|| (objnum >= MAX_OBJECTS) 
		|| (Objects[objnum].type != OBJ_SHIP) 
		|| (Objects[objnum].instance < 0) 
		|| (Objects[objnum].instance >= MAX_SHIPS)) {
		return 1.0f;
	}
	shipp = &Ships[Objects[objnum].instance];
	sip = &Ship_info[shipp->ship_info_index];

	// small ship?
	if (sip->is_small_ship()) {
		return 1.8f;
	} else if (sip->is_big_ship()) {
		return 1.4f;
	}

	// hmm
	return 1.0f;
}


// --------------------------------------------------------------------------------------------------------
// NEBULA FORWARD DEFINITIONS
//

// return the alpha the passed poof should be rendered with, for a 2 shell nebula
float neb2_get_alpha_2shell(float alpha, float inner_radius, float outer_radius, float magic_num, vec3d *v)
{
	float dist;
	vec3d eye_pos;

	// get the eye position
	neb2_get_eye_pos(&eye_pos);

	// determine what alpha to draw this bitmap with
	// higher alpha the closer the bitmap gets to the eye
	dist = vm_vec_dist_quick(&eye_pos, v);

	// if the point is inside the inner radius, alpha is based on distance to the player's eye, 
	// becoming more transparent as it gets close
	if (dist <= inner_radius) {
		// alpha per meter between the magic # and the inner radius
		alpha = alpha / (inner_radius - magic_num);

		// above value times the # of meters away we are
		alpha *= (dist - magic_num);
		return alpha < 0.0f ? 0.0f : alpha;
	}
	// if the point is outside the inner radius, it starts out as completely transparent at max
	// outer radius, and becomes more opaque as it moves towards inner radius
	else if (dist <= outer_radius) {
		// alpha per meter between the outer radius and the inner radius
		alpha = alpha / (outer_radius - inner_radius);

		// above value times the range between the outer radius and the poof
		return alpha < 0.0f ? 0.0f : alpha * (outer_radius - dist);
	}

	// otherwise transparent
	return 0.0f;
}

// -------------------------------------------------------------------------------------------------
// WACKY LOCAL PLAYER NEBULA STUFF
//

void neb2_toggle_poof(int poof_idx, bool enabling) {

	if (enabling) Neb2_poof_flags |= (1 << poof_idx);
	else Neb2_poof_flags &= ~(1 << poof_idx);

	Neb2_poofs.clear();

	// a bit awkward but this will force a full sphere gen
	vm_vec_make(&Poof_last_gen_pos, 999999.0f, 999999.0f, 999999.0f);

	neb2_poof_setup();
}

void new_poof(size_t poof_info_idx, vec3d* pos) {
	poof new_poof;
	poof_info* pinfo = &Poof_info[poof_info_idx];

	new_poof.poof_info_index = poof_info_idx;
	new_poof.flash = 0;
	new_poof.radius = pinfo->scale.next();
	new_poof.pt = *pos;
	new_poof.rot_speed = fl_radians(pinfo->rotation.next());
	new_poof.alpha = pinfo->alpha.next();
	new_poof.anim_time = frand_range(0.0f, pinfo->bitmap.total_time);
	vm_vec_rand_vec(&new_poof.up_vec);

	Neb2_poofs.push_back(new_poof);
}

static uint neb_rand_seed = 0;

void upkeep_poofs()
{
	vec3d eye_pos;
	neb2_get_eye_pos(&eye_pos);

	// cull distant poofs
	if (!Neb2_poofs.empty()) {
		for (size_t i = 0; i < Neb2_poofs.size();) {
			if (vm_vec_dist(&Neb2_poofs[i].pt, &eye_pos) > Poof_info[Neb2_poofs[i].poof_info_index].view_dist * UPKEEP_DIST_MULT) {
				Neb2_poofs[i] = Neb2_poofs.back();
				Neb2_poofs.pop_back();
			}
			else // if we needed to cull we should not advance because we just moved a new poof into this spot
				i++;
		}
	}

	neb_rand_seed = 0;

	// make new poofs
	for (size_t i = 0; i < Poof_info.size(); i++) {
		if (!poof_is_used(i))
			continue;
		poof_info* pinfo = &Poof_info[i];
		
		float gen_side_length = (pinfo->view_dist * UPKEEP_DIST_MULT) * 2;
		float gen_density = pinfo->density * Poof_density_multiplier;

		float poofs_to_gen = gen_side_length * gen_side_length * gen_side_length * gen_density;

		// store the fractional part, take the integer part
		Poof_accum[i] = modff(Poof_accum[i] + (poofs_to_gen), &poofs_to_gen);
		for (int j = 0; j < poofs_to_gen; j++) {
			vec3d pos = eye_pos;
			vec3d offset = eye_pos / gen_side_length;
			vec3d rand_pos = vm_well_distributed_rand_vec(neb_rand_seed, &offset);
			neb_rand_seed++;

			rand_pos.xyz.x *= gen_side_length / 2;
			rand_pos.xyz.y *= gen_side_length / 2;
			rand_pos.xyz.z *= gen_side_length / 2;

			pos += rand_pos;

			// we generated poofs in a cube, now keep only those that are within the view sphere, and weren't within the last view sphere
			// not terribly efficient but very simple
			if (vm_vec_dist(&eye_pos, &pos) <= (gen_side_length / 2) &&
				vm_vec_dist(&Poof_last_gen_pos, &pos) > (gen_side_length / 2))
				new_poof(i, &pos);
		}
	}
}

void neb2_render_poofs()
{
	GR_DEBUG_SCOPE("Nebula render player");
	TRACE_SCOPE(tracing::DrawPoofs);

	vertex p, ptemp;
	float alpha;
	vec3d eye_pos;
	matrix eye_orient;

	// standalone servers can bail here
	if (Game_mode & GM_STANDALONE_SERVER) {
		return;
	}

	// if the mission is not a fullneb mission, skip
	if (!(The_mission.flags[Mission::Mission_Flags::Fullneb])) {
		return;
	}
    
    memset(&p, 0, sizeof(p));
	memset(&ptemp, 0, sizeof(ptemp));

	// get eye position and orientation
	neb2_get_eye_pos(&eye_pos);
	neb2_get_eye_orient(&eye_orient);

	// maybe swap stuff around if the player crossed the dist threshold
	if (vm_vec_dist(&eye_pos, &Poof_last_gen_pos) > Poof_dist_threshold) {
		upkeep_poofs();
		Poof_last_gen_pos = eye_pos;
	}

	// if we've switched nebula rendering off
	if (Neb2_render_mode == NEB2_RENDER_NONE) {
		return;
	}

	// render the nebula
	for (poof &pf : Neb2_poofs) {
		poof_info* pinfo = &Poof_info[pf.poof_info_index];

		// Miss this one out if the id is -1
		if (pinfo->bitmap.first_frame < 0)
			continue;

		// do animation upkeep
		int framenum = 0;
		if (pinfo->bitmap.num_frames > 1) {
			pf.anim_time += flFrametime;

			framenum = bm_get_anim_frame(pinfo->bitmap.first_frame, pf.anim_time, pinfo->bitmap.total_time, true);
		}

		// generate the bitmap orient
		// If the bitmap is large and distant and points in the view fvec direction (like retail poofs do) this looks good when moving,
		// but bad when you rotate (since the bitmaps rotate in place with you, which looks weird).
		// Conversely, if the bitmap is close and small (like retail size-ish) and points at the view position, it looks good
		// when rotating (since the bitmaps remain static) but bad when moving (as they rotate in place to continue pointing at you)
		// 
		// So blend between the two styles based on promixity and size, since distant (relative to their size) bitmaps 
		// are more affected by rotating than moving and close bitmaps are vice versa
		// We will scale the bitmap direction from the view position to "off to infinity" in the negative view fvec direction - Asteroth
		matrix orient;
		vec3d view_pos;
		{
			float scalar = -1 / powf((vm_vec_dist(&eye_pos, &pf.pt) / (10 * pf.radius)), 3.f);

			vm_vec_scale_add(&view_pos, &eye_pos, &eye_orient.vec.fvec, scalar);

			view_pos -= pf.pt;
			vm_vec_normalize(&view_pos);

			vm_vector_2_matrix(&orient, &view_pos, &pf.up_vec, nullptr);
		}

		// update the poof's up vector to be perpindicular to the camera and also rotated by however much its rotating
		vec3d poof_direction;
		vm_vec_normalized_dir(&poof_direction, &pf.pt, &eye_pos);
		vm_project_point_onto_plane(&pf.up_vec, &pf.up_vec, &view_pos, &vmd_zero_vector);
		vm_vec_normalize(&pf.up_vec);
		vm_rot_point_around_line(&pf.up_vec, &pf.up_vec, pf.rot_speed * flFrametime, &vmd_zero_vector, &view_pos);

		// optimization 1 - don't draw backfacing poly's
		if (vm_vec_dot_to_point(&eye_orient.vec.fvec, &eye_pos, &pf.pt) <= 0.0f)
			continue;

		// get the proper alpha value
		alpha = neb2_get_alpha_2shell(pf.alpha, pf.radius, pinfo->view_dist, pf.radius/4, &pf.pt);

		// optimization 2 - don't draw 0.0f or less poly's
		// this amounts to big savings
		if (alpha <= 0.0f)
			continue;

		// render!
		batching_add_polygon(pinfo->bitmap.first_frame + framenum, &pf.pt, &orient, pf.radius, pf.radius, alpha);
	}

	// gr_set_color_fast(&Color_bright_red);
	// gr_printf(30, 100, "Area %.3f", total_area);
#ifdef NEB2_THUMBNAIL
	extern int tbmap;
	if (tbmap != -1) {
		gr_set_bitmap(tbmap);
		gr_bitmap(0, 0);
	}
#endif
}

/*
//Object types
#define OBJ_NONE		0	//unused object
#define OBJ_SHIP		1	//a ship
#define OBJ_WEAPON		2	//a laser, missile, etc
#define OBJ_FIREBALL	3	//an explosion
#define OBJ_START		4	//a starting point marker (player start, etc)
#define OBJ_WAYPOINT	5	//a waypoint object, maybe only ever used by Fred
#define OBJ_DEBRIS		6	//a flying piece of ship debris
#define OBJ_CMEASURE	7	//a countermeasure, such as chaff
#define OBJ_GHOST		8	//so far, just a placeholder for when a player dies.
#define OBJ_POINT		9	//generic object type to display a point in Fred.
#define OBJ_SHOCKWAVE	10	// a shockwave
#define OBJ_WING		11	// not really a type used anywhere, but I need it for Fred.
#define OBJ_OBSERVER	12	// used for multiplayer observers (possibly single player later)
#define OBJ_ASTEROID	13	// An asteroid, you know, a big rock, like debris, sort of.
#define OBJ_JUMP_NODE	14	// A jump node object, used only in Fred.
#define OBJ_BEAM		15	// beam weapons. we have to roll them into the object system to get the benefits of the collision pairs
*/
// get near and far fog values based upon object type and rendering mode
void neb2_get_fog_values(float *fnear, float *ffar, object *objp)
{
	int type_index = -1;

	//use defaults
	*fnear = Default_fog_near;
	*ffar = Default_fog_far;

	if (objp == NULL) {
		return;
	}

	// determine what fog index to use
	if(objp->type == OBJ_SHIP) {
		Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
		if((objp->instance >= 0) && (objp->instance < MAX_SHIPS)) {
			type_index = ship_query_general_type(objp->instance);
			if(type_index > 0) {
				*fnear = Ship_types[type_index].fog_start_dist;
				*ffar = Ship_types[type_index].fog_complete_dist;
			}
		}
	} else if (objp->type == OBJ_FIREBALL) { //mostly here for the warp effect
		*fnear = objp->radius*2;
		*ffar = (objp->radius*objp->radius*200)+objp->radius*200;
		return;
	}
}

// This version of the function allows for global adjustment to fog values
void neb2_get_adjusted_fog_values(float *fnear, float *ffar, float *fdensity, object *objp)
{
	neb2_get_fog_values(fnear, ffar, objp);

	// Multiply fog distances by mission multipliers
	*fnear *= Neb2_fog_near_mult;
	*ffar *= Neb2_fog_far_mult;

	// Avoide divide-by-zero
	if ((*fnear - *ffar) == 0)
		*ffar = *fnear + 1.0f;

	if (fdensity != nullptr)
		*fdensity = powf(NEB_FOG_FAR_PCT, 1 / (*ffar - *fnear));
}

// given a position, returns 0 - 1 the fog visibility of that position, 0 = completely obscured
// distance_mult will multiply the result, use for things that can be obscured but can 'shine through' the nebula more than normal
float neb2_get_fog_visibility(vec3d *pos, float distance_mult)
{
	float pct;

	// get the fog pct
	pct = powf(nNf_density, (vm_vec_dist(&Eye_position, pos) - nNf_near) / distance_mult);
	
    CLAMP(pct, 0.0f, 1.0f);

	return pct;
}

// fogging stuff --------------------------------------------------------------------

// do a pre-render of the background nebula
#define ESIZE		32
ubyte tpixels[ESIZE * ESIZE * 4];		// for 32 bits
int last_esize = -1;
int this_esize = ESIZE;
float ex_scale, ey_scale;
int tbmap = -1;
// UnknownPlayer : Contained herein, the origins of the nebula rendering bug!
// I am really not entirely sure what this code achieves, but the old
// D3D calls were the cause of the nebula bug - they have been commented out.
// If you want to save some rendering time, I would suggest maybe kill this off.
// It doesn't use much, but it APPEARS to be fairly useless unless someone wants
// to enlighten me.
//
void neb2_pre_render(camid cid)
{
	// if the mission is not a fullneb mission, skip
	if (!(The_mission.flags[Mission::Mission_Flags::Fullneb])) {
		return;
	}

	// bail early in lame and poly modes
	if (Neb2_render_mode != NEB2_RENDER_POF) {
		return;
	}

	// set the view clip
	gr_screen.clip_width = this_esize;
	gr_screen.clip_height = this_esize;
	g3_start_frame(1);						// Turn on zbuffering
	g3_set_view(cid.getCamera());
	gr_set_clip(0, 0, this_esize, this_esize);

	// render the background properly
	// hack - turn off nebula stuff
	int neb_save = Neb2_render_mode;
	Neb2_render_mode = NEB2_RENDER_NONE;

	// draw background stuff nebula
	stars_draw_background();

	Neb2_render_mode = neb_save;
	
	// grab the region
	gr_get_region(0, this_esize, this_esize, (ubyte*)tpixels);

#ifdef NEB2_THUMBNAIL
	if (tbmap == -1) {
		tbmap = bm_create(16, this_esize, this_esize, tpixels, 0);
		bm_lock(tbmap, 16, 0);
		bm_unlock(tbmap);
	}
#endif

	// maybe do some swizzling
	
	// end the frame
	g3_end_frame();
	
	gr_clear();	

	// if the size has changed between frames, make a new bitmap
	if (this_esize != last_esize) {
		last_esize = this_esize;

		// recalculate ex_scale and ey_scale values for looking up color values
		ex_scale = (float)this_esize / (float)gr_screen.max_w;
		ey_scale = (float)this_esize / (float)gr_screen.max_h;
	}
}

// fill in the position of the eye for this frame
void neb2_get_eye_pos(vec3d *eye_vector)
{
	*eye_vector = Eye_position;
}

// fill in the eye orient for this frame
void neb2_get_eye_orient(matrix *eye_matrix)
{
	*eye_matrix = Eye_matrix;
}

// nebula DCF functions ------------------------------------------------------
// TODO: With the new debug parser in place, most of these sub-commands can now be handled by neb2. This should clear up the DCF list a bit
DCF(neb2, "list nebula console commands")
{		
//	dc_printf("neb2_fog <X> <float> <float>  : set near and far fog planes for ship type X\n");
//	dc_printf("where X is an integer from 1 - 11\n");
//	dc_printf("1 = cargo containers, 2 = fighters/bombers, 3 = cruisers\n");
//	dc_printf("4 = freighters, 5 = capital ships, 6 = transports, 7 = support ships\n");
//	dc_printf("8 = navbuoys, 9 = sentryguns, 10 = escape pods, 11 = background nebula polygons\n\n");
	
	dc_printf("neb2_select      : <int> <int>  where the first # is the bitmap to be adjusting (0 through 5), and the second int is a 0 or 1, to turn off and on\n");
	dc_printf("neb2_mode        : switch between no nebula, polygon background, pof background, lame, or HTL rendering (0, 1, 2, 3 and 4 respectively)\n\n");	
	dc_printf("neb2_ff          : flash fade/sec\n");
	dc_printf("neb2_background	: rgb background color\n");
	dc_printf("neb2_fog_color   : rgb fog color\n");

//	dc_printf("neb2_fog_vals    : display all the current settings for all above values\n");	
}

DCF(neb2_select, "Enables/disables a poof bitmap")
{
	int bmap;
	bool val_b;

	dc_stuff_int(&bmap);

	if ( (bmap >= 0) && (bmap < (int)Poof_info.size()) ) {
		dc_stuff_boolean(&val_b);

		val_b ? (Neb2_poof_flags |= (1<<bmap)) : (Neb2_poof_flags &= ~(1<<bmap));
	}
}

DCF(neb2_ff, "flash fade/sec")
{
	dc_stuff_float(&neb2_flash_fade);
}

DCF(neb2_mode, "Switches nebula render modes")
{
	int mode;
	dc_stuff_int(&mode);

	switch (mode) {
		case NEB2_RENDER_NONE:
			Neb2_render_mode = NEB2_RENDER_NONE;
		break;

		case NEB2_RENDER_POF:
			Neb2_render_mode = NEB2_RENDER_POF;
			stars_set_background_model(BACKGROUND_MODEL_FILENAME, "Eraseme3");
			stars_set_background_orientation();
		break;

		case NEB2_RENDER_HTL:
			Neb2_render_mode = NEB2_RENDER_HTL;
		break;
	}
}

DCF(neb2_background, "Sets the RGB background color (lame rendering)")
{
	int r, g, b;

	dc_stuff_int(&r);
	dc_stuff_int(&g);
	dc_stuff_int(&b);

	Neb2_background_color[0] = r;
	Neb2_background_color[1] = g;
	Neb2_background_color[2] = b;
}

DCF(neb2_fog_color, "Sets the RGB fog color (HTL)")
{
	ubyte r, g, b;

	dc_stuff_ubyte(&r);
	dc_stuff_ubyte(&g);
	dc_stuff_ubyte(&b);

	Neb2_fog_color[0] = r;
	Neb2_fog_color[1] = g;
	Neb2_fog_color[2] = b;
}
