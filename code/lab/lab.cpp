/*
* lab.cpp
* created by WMCoolmon
*
* You may not sell or otherwise commercially exploit the source or things you
* create based on the source.
*
*/



#include "cmdline/cmdline.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "graphics/light.h"
#include "hud/hudshield.h"
#include "io/key.h"
#include "io/timer.h"
#include "lab/wmcgui.h"
#include "lighting/lighting.h"
#include "mission/missionparse.h"
#include "missionui/missionscreencommon.h"
#include "model/model.h"
#include "object/objectsnd.h"
#include "playerman/managepilot.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "species_defs/species_defs.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"
#include "starfield/starfield.h"
#include "starfield/nebula.h"
#include "nebula/neb.h"
#include "model/modelrender.h"
#include "tracing/tracing.h"

// flags
#define LAB_FLAG_NORMAL				(0)		// default
#define LAB_FLAG_NO_ROTATION		(1<<0)	// don't rotate models
#define LAB_FLAG_SHOW_INSIGNIA		(1<<1)	// show ships with insignia applied
#define LAB_FLAG_SHOW_DEBRIS		(1<<2)	// render debris instead of normal LOD
#define LAB_FLAG_SUBMODEL_ROTATE	(1<<3)	// do rotation for any rotating ship subobjects
#define LAB_FLAG_LIGHTNING_ARCS		(1<<4)	// show damage lightning
#define LAB_FLAG_UNUSED				(1<<5)	// feel free to change that to something you need - Valathil
#define LAB_FLAG_SHOW_WEAPONS		(1<<6)	// determines if external weapons models are displayed
#define LAB_FLAG_INITIAL_ROTATION	(1<<7)	// initial rotation setting
#define LAB_FLAG_DESTROYED_SUBSYSTEMS	(1<<8)	// render model as if all subsystems are destroyed

// modes
#define LAB_MODE_NONE		0	// not showing anything
#define LAB_MODE_SHIP		1	// dealing with ships
#define LAB_MODE_WEAPON		2	// dealing with weapons


// variables
static GUIScreen *Lab_screen = NULL;
static Window *Lab_toolbar = NULL;
static Window *Lab_class_window = NULL;
static Window *Lab_class_toolbar = NULL;
static Window *Lab_flags_window = NULL;
static Window *Lab_render_options_window = NULL;
static Window *Lab_material_override_window = NULL;
static Window *Lab_variables_window = NULL;
static Window *Lab_description_window = NULL;
static Window *Lab_background_window = NULL;
static Text *Lab_description_text = NULL;
static TreeItem **Lab_species_nodes = NULL;

static int Lab_insignia_bitmap = -1;
static int Lab_insignia_index = -1;

static ubyte Lab_mode = LAB_MODE_NONE;
static int Lab_selected_index = -1;
static int Lab_last_selected_ship = -1;
static int Lab_selected_object = -1;
static int Lab_last_selected_weapon = -1;

static int Lab_model_num = -1;
static int Lab_weaponmodel_num[MAX_SHIP_WEAPONS];
static int Lab_model_LOD = 0;
static char Lab_model_filename[MAX_FILENAME_LEN];
static char Lab_weaponmodel_filename[MAX_SHIP_WEAPONS][MAX_FILENAME_LEN];

static vec3d Lab_model_pos = ZERO_VECTOR;

static matrix Lab_model_orient = vmd_identity_matrix;
static int Lab_viewer_flags = LAB_MODE_NONE;

static int Lab_detail_texture_save = 0;

SCP_string Lab_selected_mission = "None";
matrix Lab_skybox_orientation = vmd_identity_matrix;

static int anim_timer_start = 0;

static float Lab_thrust_len = 1.0f;
static bool Lab_thrust_afterburn = false;

// Trackball_mode:
//   1  ==  rotate	(left-mouse)
//   2  ==  pan		(shift-left-mouse)
//   3  ==  zoom	(right-mouse)
static int Trackball_mode = 1;
static int Trackball_active = 0;

SCP_string Lab_team_color = "<none>";

camid Lab_cam;
float lab_cam_distance = 100.0f;
float lab_cam_phi = 1.24f; // Values chosen to approximate the initial rotation used previously
float lab_cam_theta = 2.25f;
bool Lab_render_wireframe = false;
bool Lab_render_without_light = false;
bool Lab_render_show_thrusters = false;
bool Lab_render_show_detail = false;
bool Lab_render_show_shields = false;
bool Lab_rotate_subobjects = true;

bool Lab_Basemap_override    = false;
bool Lab_Glowmap_override	 = false;
bool Lab_Specmap_override	 = false;
bool Lab_Envmap_override	 = false;
bool Lab_Normalmap_override	 = false;
bool Lab_Heightmap_override	 = false;
bool Lab_Miscmap_override    = false;
bool Lab_emissive_light_override = !Cmdline_no_emissive;

fix Lab_Save_Missiontime;

// functions
void labviewer_change_ship_lod(Tree *caller);
void labviewer_change_ship(Tree *caller);
void labviewer_make_desc_window(Button *caller);
void labviewer_update_flags_window();


// ---------------------- General/Utility Functions ----------------------------

void rotate_view(int dx, int dy)
{
	if (dx == 0 && dy == 0) return;

	lab_cam_theta += dx / 100.0f;
	lab_cam_phi += dy / 100.0f;
	
	CLAMP(lab_cam_phi, 0.01f, PI - 0.01f);
}

void labviewer_change_model(char *model_fname, int lod = 0, int sel_index = -1)
{
	bool change_model = true;
	int j, l;
	ship_info *sip = NULL;

	anim_timer_start = timer_get_milliseconds();

	if ((model_fname != NULL) && (Lab_mode == LAB_MODE_SHIP)) {
		if ((Lab_selected_index >= 0) && (Lab_selected_index == sel_index)) {
			change_model = false;
		}
	}

	Lab_selected_index = sel_index;

	if (change_model) {
		bool valid_ship = ((Lab_mode == LAB_MODE_SHIP) && (sel_index >= 0));

		if (Lab_model_num != -1) {
			model_page_out_textures(Lab_model_num, true);
			model_unload(Lab_model_num);
			Lab_model_num = -1;

			for (j = 0; j < MAX_SHIP_WEAPONS; j++) {
				if (Lab_weaponmodel_num[j] >= 0) {
					model_page_out_textures(Lab_weaponmodel_num[j], true);
					model_unload(Lab_weaponmodel_num[j]);
					Lab_weaponmodel_num[j] = -1;
				}
			}

			if (Lab_last_selected_ship >= 0) {
				ship_page_out_textures(Lab_last_selected_ship, true);
			}
		}

		// only load a new model if we are supposed to (so that we can use this function to reset states)
		if (model_fname != NULL) {
			model_subsystem *subsystems = NULL;
			
			Lab_model_num = model_load(model_fname, (valid_ship) ? Ship_info[sel_index].n_subsystems : 0, subsystems, 0);

			if (Lab_model_num >= 0) {
				strcpy_s(Lab_model_filename, model_fname);
			}
			else {
				memset(Lab_model_filename, 0, sizeof(Lab_model_filename));
			}

			if (valid_ship) {
				if (Lab_model_num >= 0) {
					model_page_in_textures(Lab_model_num, sel_index);
				}
			}

			// do the same for weapon models if necessary
			if (Lab_mode == LAB_MODE_SHIP) {
				sip = &Ship_info[Lab_selected_index];
				l = 0;
				for (j = 0; j < sip->num_primary_banks; j++) {
					weapon_info *wip = &Weapon_info[sip->primary_bank_weapons[j]];
					if (!sip->draw_primary_models[j])
						continue;
					Lab_weaponmodel_num[l] = -1;
					if (strlen(wip->external_model_name)) {
						Lab_weaponmodel_num[l] = model_load(wip->external_model_name, 0, NULL);
					}
					if (Lab_weaponmodel_num[l] >= 0) {
						strcpy_s(Lab_weaponmodel_filename[l], wip->external_model_name);
					}
					else {
						memset(Lab_weaponmodel_filename[l], 0, sizeof(Lab_weaponmodel_filename[l]));
					}
					l++;
				}

				for (j = 0; j < sip->num_secondary_banks; j++) {
					weapon_info *wip = &Weapon_info[sip->secondary_bank_weapons[j]];
					if (!sip->draw_secondary_models[j])
						continue;
					Lab_weaponmodel_num[l] = -1;
					if (strlen(wip->external_model_name)) {
						Lab_weaponmodel_num[l] = model_load(wip->external_model_name, 0, NULL);
					}
					if (Lab_weaponmodel_num[l] >= 0) {
						strcpy_s(Lab_weaponmodel_filename[l], wip->external_model_name);
					}
					else {
						memset(Lab_weaponmodel_filename[l], 0, sizeof(Lab_weaponmodel_filename[l]));
					}
					l++;
				}
			}
		}
		else {
			// clear out the model filename
			memset(Lab_model_filename, 0, sizeof(Lab_model_filename));
			if (Lab_weaponmodel_num[0] >= 0) {
				for (j = 0; j < MAX_SHIP_WEAPONS; j++) {
					memset(Lab_weaponmodel_filename[j], 0, sizeof(Lab_weaponmodel_filename[j]));
				}
			}
		}
	}

	if (lod == 99) {
		Lab_model_LOD = 0;
		Lab_viewer_flags |= LAB_FLAG_SHOW_DEBRIS;
	}
	else {
		Lab_model_LOD = lod;
		Lab_viewer_flags &= ~LAB_FLAG_SHOW_DEBRIS;
	}

	if (model_fname == NULL) {
		Lab_last_selected_ship = -1;
	}

	labviewer_update_flags_window();
}

void labviewer_recalc_camera()
{
	auto cam = Lab_cam.getCamera();

	if (Lab_selected_object != -1) {
		vec3d new_position;
		new_position.xyz.x = sinf(lab_cam_phi) * cosf(lab_cam_theta);
		new_position.xyz.y = cosf(lab_cam_phi);
		new_position.xyz.z = sinf(lab_cam_phi) * sinf(lab_cam_theta);

		vm_vec_scale(&new_position, lab_cam_distance);

		cam->set_position(&new_position);
		cam->set_rotation_facing(&Lab_model_pos);
	}
}

void labviewer_render_model(float frametime)
{
	angles rot_angles;
	float rev_rate;

	ship_info *sip = NULL;

	if (Lab_selected_object != -1 && (Objects[Lab_selected_object].type == OBJ_SHIP) && (Lab_selected_index >= 0)) {
		sip = &Ship_info[Lab_selected_index];
	}

	light_reset();

	// get correct revolution rate
	rev_rate = REVOLUTION_RATE;

	if (sip != NULL) {
		if (sip->is_big_ship()) {
			rev_rate *= 1.7f;
		}
		else if (sip->is_huge_ship()) {
			rev_rate *= 3.0f;
		}
	}

	// rotate/pan/zoom the model as much as required for this frame
	if (Trackball_active) {
		int dx, dy;
		matrix mat1, mat2;

		mouse_get_delta(&dx, &dy);

		if (dx || dy) {
			// Rotate the ship
			if (Trackball_mode == 1)
			{
				vm_trackball(-dx, -dy, &mat1);
				vm_matrix_x_matrix(&mat2, &mat1, &Lab_model_orient);

				Lab_model_orient = mat2;

			}
			// zoom in/out
			else if (Trackball_mode == 2)
			{
				lab_cam_distance *= 1.0f + (dy / 20.0f);
				if (lab_cam_distance < 1.0f)
					lab_cam_distance = 1.0f;
			}
			// rotate background
			else if (Trackball_mode == 3)
			{
				rotate_view(dx, dy);
			}

			labviewer_recalc_camera();
		}
	}
	// otherwise do orient/rotation calculation, if we are supposed to
	else if (!(Lab_viewer_flags & LAB_FLAG_NO_ROTATION))
	{
		rot_angles.p = 0.0f;
		rot_angles.b = 0.0f;
		rot_angles.h = PI2 * frametime / rev_rate;
		vm_rotate_matrix_by_angles(&Lab_model_orient, &rot_angles);
	}

	if (Lab_selected_object != -1)
	{
		auto lab_render_light_save = Lab_render_without_light;
		auto lab_debris_override_save = Cmdline_nomotiondebris;
		auto lab_envmap_override_save = Envmap_override;
		auto lab_emissive_light_save = Cmdline_no_emissive;

		if (Lab_selected_mission.compare("None") == 0) {
			Lab_render_without_light = true;
			Cmdline_nomotiondebris = 1;
		}

		Cmdline_no_emissive = !Lab_emissive_light_override;

		object* obj = &Objects[Lab_selected_object];

		obj->pos = Lab_model_pos;
		obj->orient = Lab_model_orient;

		Envmap_override = Lab_Envmap_override;

		if (obj->type == OBJ_SHIP) {
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Rotators_locked, !Lab_rotate_subobjects);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Draw_as_wireframe, Lab_render_wireframe);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_full_detail, Lab_render_show_detail);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_light, Lab_render_without_light);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_diffuse, Lab_Basemap_override);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_glowmap, Lab_Glowmap_override);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_normalmap, Lab_Normalmap_override);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_specmap, Lab_Specmap_override);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_heightmap, Lab_Heightmap_override);
			Ships[obj->instance].flags.set(Ship::Ship_Flags::Render_without_miscmap, Lab_Miscmap_override);

			ship_process_post(obj, frametime);
			ship_model_update_instance(obj);

			Ships[obj->instance].team_name = Lab_team_color;
		}
		else if (obj->type == OBJ_WEAPON) 
		{
		}

		if (Lab_render_wireframe)
			model_render_set_wireframe_color(&Color_white);

		if (Lab_render_show_thrusters && obj->type == OBJ_SHIP) {
			obj->phys_info.forward_thrust = 1.0f;
			Ships[obj->instance].flags.remove(Ship::Ship_Flags::No_thrusters);

			if (Lab_thrust_afterburn)
				obj->phys_info.flags |= PF_AFTERBURNER_ON;
			else
				obj->phys_info.flags &= ~PF_AFTERBURNER_ON;
		}
		else {
			obj->phys_info.forward_thrust = 0.0f;

			if (obj->type == OBJ_SHIP)
				Ships[obj->instance].flags.set(Ship::Ship_Flags::No_thrusters);
		}

		Trail_render_override = true;
		game_render_frame(Lab_cam);
		Trail_render_override = false;

		Lab_render_without_light = lab_render_light_save;
		Cmdline_nomotiondebris = lab_debris_override_save;
		Envmap_override = lab_envmap_override_save;
		Cmdline_no_emissive = lab_emissive_light_save;

		gr_reset_clip();
		gr_set_color_fast(&HUD_color_debug);
		if (Cmdline_frame_profile) {
			tracing::frame_profile_process_frame();
			gr_string(gr_screen.center_offset_x + 20, gr_screen.center_offset_y + 100 + gr_get_font_height() + 1,
				tracing::get_frame_profile_output().c_str(), GR_RESIZE_NONE);
		}
	}
}

void labviewer_do_render(float frametime)
{
	GR_DEBUG_SCOPE("Lab Render");

	int w, h;

	// render our particular thing
	if (Lab_selected_object >= 0) {
		labviewer_render_model(frametime);

		// print out the current pof filename, to help with... something
		if (strlen(Lab_model_filename)) {
			gr_get_string_size(&w, &h, Lab_model_filename);
			gr_set_color_fast(&Color_white);
			gr_string(gr_screen.center_offset_x + gr_screen.center_w - w, gr_screen.center_offset_y + gr_screen.center_h - h, Lab_model_filename, GR_RESIZE_NONE);
		}
	}

	// print FPS at bottom left, might be helpful
	extern void game_get_framerate();
	extern float frametotal;
	extern float Framerate;

	game_get_framerate();

	gr_set_color_fast(&Color_white);

	if (frametotal != 0.0f) {
		gr_printf_no_resize(gr_screen.center_offset_x + 2, gr_screen.center_offset_y + gr_screen.center_h - gr_get_font_height(), "FPS: %3i Camera Distance: %4f", fl2i(Framerate + 0.5f), lab_cam_distance);
	}
	else {
		gr_string(gr_screen.center_offset_x + 10, gr_screen.center_offset_y + gr_screen.center_h - gr_get_font_height(), "FPS: ?", GR_RESIZE_NONE);
	}

	//Print FXAA preset
	if (Cmdline_fxaa && !PostProcessing_override)
		gr_printf_no_resize(gr_screen.center_offset_x + 2, gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 2) - 3, "FXAA Preset: %i", Cmdline_fxaa_preset);
	
	//Print current Team Color setting, if any
	if (Lab_team_color != "<none>")
		gr_printf_no_resize(gr_screen.center_offset_x + 2, gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 3) - 3, "Use T and Y to cycle through available Team Color settings. Current: %s", Lab_team_color.c_str());

	//Camera usage info
	gr_printf_no_resize(gr_screen.center_offset_x + 2, gr_screen.center_offset_y + gr_screen.center_h - (gr_get_font_height() * 4) - 3, "Hold LMB to rotate the ship or weapon. Hold RMB to rotate the Camera. Hold Shift + LMB to zoom in or out. Use number keys to switch between FXAA presets.");
}

void labviewer_exit(Button *caller)
{
	if (Lab_selected_object != -1) {
		obj_delete(Lab_selected_object);
	}
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

// ----------------------------  Class Window ----------------------------------
void labviewer_close_class_window(GUIObject *caller)
{
	if (Lab_class_toolbar) {
		Lab_class_toolbar->DeleteChildren();
	}

	Lab_class_window = NULL;

	Lab_mode = LAB_MODE_NONE;

	// reset any existing model/bitmap that is showing
	labviewer_change_model(NULL);
}

void labviewer_set_class_window(int mode)
{
	if (Lab_class_window == NULL) {
		Lab_class_window = (Window*)Lab_screen->Add(new Window("Class Window", gr_screen.center_offset_x + 50, gr_screen.center_offset_y + 50));
		Lab_class_window->SetCloseFunction(labviewer_close_class_window);
	}

	if (Lab_class_toolbar == NULL) {
		Lab_class_toolbar = (Window*)Lab_screen->Add(new Window("Class Toolbar", gr_screen.center_offset_x + 0, gr_screen.center_offset_y + Lab_toolbar->GetHeight(), -1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));
	}

	// clear out all existing children
	Lab_class_window->ClearContent();
	Lab_class_toolbar->ClearContent();

	// set our new title
	if (mode == LAB_MODE_SHIP) {
		Lab_class_window->SetCaption("Ship Classes");
	}
	else if (mode == LAB_MODE_WEAPON) {
		Lab_class_window->SetCaption("Weapon Classes");
	}

	// reset any existing model/bitmap that is showing
	labviewer_change_model(NULL);
}


// ------------------------------  Flags Window --------------------------------
template<class T>
struct lab_flag {
	Checkbox* cb;
	T flag;
};

static SCP_vector<lab_flag<Ship::Info_Flags>> Ship_Class_Flags;
static SCP_vector<lab_flag<Weapon::Info_Flags>> Weapon_Class_Flags;

void labviewer_flags_clear()
{
	if (Lab_flags_window != NULL) {
		Lab_flags_window->DeleteChildren();
	}

	Ship_Class_Flags.clear();
	Weapon_Class_Flags.clear();
}

template <class T>
void labviewer_flags_add(int* X, int* Y, const char *flag_name, T flag, SCP_vector<lab_flag<T>>& flag_list)
{
	int x = 0, y = 0;

	Assert((Lab_flags_window != NULL) && (flag_name != NULL));

	lab_flag<T> new_flag;

	if (X) {
		x = *X;
	}

	if (Y) {
		y = *Y;
	}

	new_flag.cb = (Checkbox*)Lab_flags_window->AddChild(new Checkbox(flag_name, x, y));
	new_flag.flag = flag;
	flag_list.push_back(new_flag);

	if (X) {
		*X += new_flag.cb->GetWidth() + 2;
	}

	if (Y) {
		*Y += new_flag.cb->GetHeight() + 1;
	}
}

void labviewer_populate_flags_window()
{
	if (Lab_mode == LAB_MODE_NONE) {
		return;
	}

	if (Lab_flags_window == NULL) {
		return;
	}

	// clear out anything that already exists
	labviewer_flags_clear();

	int y = 0;

	// ship flags ...
	if (Lab_mode == LAB_MODE_SHIP) {
		for (size_t i = 0; i < Num_ship_flags; ++i)
		{
			labviewer_flags_add<Ship::Info_Flags>(nullptr, &y, Ship_flags[i].name, Ship_flags[i].def, Ship_Class_Flags);
		}
	}
	// weapon flags ...
	else if (Lab_mode == LAB_MODE_WEAPON) {
		for (size_t i = 0; i < num_weapon_info_flags; ++i)
		{
			labviewer_flags_add<Weapon::Info_Flags>(nullptr, &y, Weapon_Info_Flags[i].name, Weapon_Info_Flags[i].def, Weapon_Class_Flags);
		}
	}
}

void labviewer_update_flags_window()
{
	if ((Lab_selected_index < 0) || (Lab_mode == LAB_MODE_NONE)) {
		return;
	}

	if (Lab_mode == LAB_MODE_SHIP) {
		auto sip = &Ship_info[Lab_selected_index];

		for (auto flag_def : Ship_Class_Flags)
		{
			if (flag_def.flag == Ship::Info_Flags::NUM_VALUES) continue;
			flag_def.cb->SetFlag(sip->flags, flag_def.flag, sip);
		}
	}
	else if (Lab_mode == LAB_MODE_WEAPON) {
		auto wip = &Weapon_info[Lab_selected_index];

		for (auto flag_def : Weapon_Class_Flags)
		{
			if (flag_def.flag == Weapon::Info_Flags::NUM_VALUES) continue;
			flag_def.cb->SetFlag(wip->wi_flags, flag_def.flag, wip);
		}
	}
}

void labviewer_close_flags_window(GUIObject *caller)
{
	Lab_flags_window = NULL;

	Ship_Class_Flags.empty();
	Weapon_Class_Flags.empty();
}

void labviewer_make_flags_window(Button *caller)
{
	if (Lab_flags_window == NULL) {
		Lab_flags_window = (Window*)Lab_screen->Add(new Window("Flags Window", gr_screen.center_offset_x + gr_screen.center_w - 205, gr_screen.center_offset_y + 200));
		Lab_flags_window->SetCloseFunction(labviewer_close_flags_window);
	}

	// set our new title
	if (Lab_mode == LAB_MODE_SHIP) {
		Lab_flags_window->SetCaption("Ship Flags");
	}
	else if (Lab_mode == LAB_MODE_WEAPON) {
		Lab_flags_window->SetCaption("Weapon Flags");
	}

	// populate the window with our flags (controls both ships and weapons flags)
	labviewer_populate_flags_window();

	// update content, if we need to
	labviewer_update_flags_window();
}

// -----------------------   Variables Window   --------------------------------
#define VAR_POS_LEFTWIDTH		150
#define VAR_POS_RIGHTWIDTH		100
#define VAR_POS_RIGHTX			160

#define VAR_ADD_HEADER(name) {	\
	ntp = (Text*)Lab_variables_window->AddChild(new Text((name), (name), VAR_POS_RIGHTX/2, y + 8, VAR_POS_RIGHTWIDTH));	\
	y += ntp->GetHeight() + 10;	\
}

static SCP_vector<Text*> Lab_variables;

void labviewer_close_variables_window(GUIObject *caller)
{
	Lab_variables_window = NULL;

	Lab_variables.clear();
}

void labviewer_variables_clear()
{
	if (Lab_variables_window != NULL) {
		Lab_variables_window->DeleteChildren();
	}

	Lab_variables.clear();
}

void labviewer_variables_add(int *Y, const char *var_name)
{
	int y = 0;
	Text *new_text;

	Assert((Lab_variables_window != NULL) && (var_name != NULL));

	if (Y) {
		y = *Y;
	}

	// variable
	Lab_variables_window->AddChild(new Text((var_name), (var_name), 0, y, VAR_POS_LEFTWIDTH));
	// edit box
	new_text = (Text*)Lab_variables_window->AddChild(new Text(SCP_string((var_name)) + SCP_string("Editbox"), "", VAR_POS_RIGHTX, y, VAR_POS_RIGHTWIDTH, 12, T_EDITTABLE));

	if (Y) {
		*Y += new_text->GetHeight() + 2;
	}

	Lab_variables.push_back(new_text);
}

void labviewer_populate_variables_window()
{
	Text *ntp;
	int y;

	if (Lab_mode == LAB_MODE_NONE) {
		return;
	}

	if (Lab_variables_window == NULL) {
		return;
	}

	// clear out anything that already exists
	labviewer_variables_clear();

	y = 0;

	// IMPORTANT NOTE: If you add something here, make sure you add it to labviewer_update_variables_window() as well!
	// ship vFEWfe<ariables ...
	if (Lab_mode == LAB_MODE_SHIP) {
		labviewer_variables_add(&y, "Name");
		labviewer_variables_add(&y, "Species");
		labviewer_variables_add(&y, "Type");
		labviewer_variables_add(&y, "Default Team Color");

		// physics
		VAR_ADD_HEADER("Physics");
		labviewer_variables_add(&y, "Density");
		labviewer_variables_add(&y, "Damp");
		labviewer_variables_add(&y, "Rotdamp");
		labviewer_variables_add(&y, "Max vel (x)");
		labviewer_variables_add(&y, "Max vel (y)");
		labviewer_variables_add(&y, "Max vel (z)");
		labviewer_variables_add(&y, "Warp in speed");
		labviewer_variables_add(&y, "Warp out speed");

		// other
		VAR_ADD_HEADER("Stats");
		labviewer_variables_add(&y, "Shields");
		labviewer_variables_add(&y, "Hull");
		labviewer_variables_add(&y, "Subsys repair rate");
		labviewer_variables_add(&y, "Hull repair rate");
		labviewer_variables_add(&y, "Countermeasures");
		labviewer_variables_add(&y, "HUD Icon");

		VAR_ADD_HEADER("Power");
		labviewer_variables_add(&y, "Power output");
		labviewer_variables_add(&y, "Max oclk speed");
		labviewer_variables_add(&y, "Max weapon reserve");

		VAR_ADD_HEADER("Afterburner");
		labviewer_variables_add(&y, "Fuel");
		labviewer_variables_add(&y, "Burn rate");
		labviewer_variables_add(&y, "Recharge rate");

		VAR_ADD_HEADER("Explosion");
		labviewer_variables_add(&y, "Inner radius");
		labviewer_variables_add(&y, "Outer radius");
		labviewer_variables_add(&y, "Damage");
		labviewer_variables_add(&y, "Blast");
		labviewer_variables_add(&y, "Propagates");
		labviewer_variables_add(&y, "Shockwave speed");
		labviewer_variables_add(&y, "Shockwave count");

		// techroom
		VAR_ADD_HEADER("Techroom");
		labviewer_variables_add(&y, "Closeup zoom");
		labviewer_variables_add(&y, "Closeup pos (x)");
		labviewer_variables_add(&y, "Closeup pos (y)");
		labviewer_variables_add(&y, "Closeup pos (z)");
	}
	// weapon variables ...
	else if (Lab_mode == LAB_MODE_WEAPON) {
		labviewer_variables_add(&y, "Name");
		labviewer_variables_add(&y, "Subtype");

		// physics
		VAR_ADD_HEADER("Physics");
		labviewer_variables_add(&y, "Mass");
		labviewer_variables_add(&y, "Max speed");
		labviewer_variables_add(&y, "Lifetime");
		labviewer_variables_add(&y, "Range");
		labviewer_variables_add(&y, "Min Range");

		VAR_ADD_HEADER("Damage");
		labviewer_variables_add(&y, "Fire wait");
		labviewer_variables_add(&y, "Damage");
		labviewer_variables_add(&y, "Armor factor");
		labviewer_variables_add(&y, "Shield factor");
		labviewer_variables_add(&y, "Subsys factor");

		VAR_ADD_HEADER("Armor");
		labviewer_variables_add(&y, "Damage type");

		VAR_ADD_HEADER("Shockwave");
		labviewer_variables_add(&y, "Speed");

		VAR_ADD_HEADER("Missiles");
		labviewer_variables_add(&y, "Turn time");
		labviewer_variables_add(&y, "FOV");
		labviewer_variables_add(&y, "Min locktime");
		labviewer_variables_add(&y, "Pixels/sec");
		labviewer_variables_add(&y, "Catchup pixels/sec");
		labviewer_variables_add(&y, "Catchup pixel pen.");
		labviewer_variables_add(&y, "Swarm count");
		labviewer_variables_add(&y, "Swarm wait");
	}
}

#define VAR_SET_VALUE(value) {	\
	Assert( i < Lab_variables.size() );	\
	Lab_variables[i]->SetText((value));	\
	i++;	\
}

#define VAR_SET_VALUE_SAVE(value, max_size) {	\
	Assert( i < Lab_variables.size() );	\
	Lab_variables[i]->SetText((value));	\
	if ((max_size) < 1) {	\
		Assert( (max_size) == 0 );	\
		Lab_variables[i]->SetSaveLoc(&(value), T_ST_ONENTER);	\
	} else {	\
		Lab_variables[i]->SetSaveLoc(&(value), T_ST_ONENTER, (max_size), 0);	\
	}	\
	i++;	\
}
extern SCP_vector<SCP_string> Hud_shield_filenames;

void labviewer_update_variables_window()
{
	uint i = 0;

	if (Lab_mode == LAB_MODE_NONE) {
		return;
	}

	if (Lab_variables_window == NULL) {
		return;
	}

	if (Lab_selected_index < 0) {
		return;
	}

	// IMPORTANT NOTE: If you add something here, make sure you add it to labviewer_populate_variables_window() as well!
	// ship variables ...
	if (Lab_mode == LAB_MODE_SHIP) {
		Assert(Lab_selected_index < static_cast<int>(Ship_info.size()));
		ship_info *sip = &Ship_info[Lab_selected_index];

		VAR_SET_VALUE(sip->name);
		VAR_SET_VALUE_SAVE(sip->species, (int)(Species_info.size() - 1));
		VAR_SET_VALUE_SAVE(sip->class_type, (int)(Ship_types.size() - 1));
		VAR_SET_VALUE(sip->default_team_name);

		VAR_SET_VALUE_SAVE(sip->density, 0);
		VAR_SET_VALUE_SAVE(sip->damp, 0);
		VAR_SET_VALUE_SAVE(sip->rotdamp, 0);
		VAR_SET_VALUE_SAVE(sip->max_vel.xyz.x, 0);
		VAR_SET_VALUE_SAVE(sip->max_vel.xyz.y, 0);
		VAR_SET_VALUE_SAVE(sip->max_vel.xyz.z, 0);
		VAR_SET_VALUE_SAVE(sip->warpin_speed, 0);
		VAR_SET_VALUE_SAVE(sip->warpout_speed, 0);

		VAR_SET_VALUE_SAVE(sip->max_shield_strength, 0);
		VAR_SET_VALUE_SAVE(sip->max_hull_strength, 0);
		VAR_SET_VALUE_SAVE(sip->subsys_repair_rate, 0);
		VAR_SET_VALUE_SAVE(sip->hull_repair_rate, 0);
		VAR_SET_VALUE_SAVE(sip->cmeasure_max, 0);
		VAR_SET_VALUE_SAVE(sip->shield_icon_index, (int)(Hud_shield_filenames.size() - 1));

		VAR_SET_VALUE_SAVE(sip->power_output, 0);
		VAR_SET_VALUE_SAVE(sip->max_overclocked_speed, 0);
		VAR_SET_VALUE_SAVE(sip->max_weapon_reserve, 0);
		//	VAR_SET_VALUE_SAVE(sip->max_shield_regen_per_second, 0);
		//	VAR_SET_VALUE_SAVE(sip->max_weapon_regen_per_second, 0);

		VAR_SET_VALUE_SAVE(sip->afterburner_fuel_capacity, 0);
		VAR_SET_VALUE_SAVE(sip->afterburner_burn_rate, 0);
		VAR_SET_VALUE_SAVE(sip->afterburner_recover_rate, 0);

		VAR_SET_VALUE_SAVE(sip->shockwave.inner_rad, 0);
		VAR_SET_VALUE_SAVE(sip->shockwave.outer_rad, 0);
		VAR_SET_VALUE_SAVE(sip->shockwave.damage, 0);
		VAR_SET_VALUE_SAVE(sip->shockwave.blast, 0);
		VAR_SET_VALUE_SAVE(sip->explosion_propagates, 0);
		VAR_SET_VALUE_SAVE(sip->shockwave.speed, 0);
		VAR_SET_VALUE_SAVE(sip->shockwave_count, 0);

		VAR_SET_VALUE_SAVE(sip->closeup_zoom, 0);
		VAR_SET_VALUE_SAVE(sip->closeup_pos.xyz.x, 0);
		VAR_SET_VALUE_SAVE(sip->closeup_pos.xyz.y, 0);
		VAR_SET_VALUE_SAVE(sip->closeup_pos.xyz.z, 0);
	}
	// weapon variables ...
	else if (Lab_mode == LAB_MODE_WEAPON) {
		Assert(Lab_selected_index < Num_weapon_types);
		weapon_info *wip = &Weapon_info[Lab_selected_index];

		VAR_SET_VALUE(wip->name);
		VAR_SET_VALUE_SAVE(wip->subtype, Num_weapon_subtypes - 1);

		VAR_SET_VALUE_SAVE(wip->mass, 0);
		VAR_SET_VALUE_SAVE(wip->max_speed, 0);
		VAR_SET_VALUE_SAVE(wip->lifetime, 0);
		VAR_SET_VALUE_SAVE(wip->weapon_range, 0);
		VAR_SET_VALUE_SAVE(wip->WeaponMinRange, 0);

		VAR_SET_VALUE_SAVE(wip->fire_wait, 0);
		VAR_SET_VALUE_SAVE(wip->damage, 0);
		VAR_SET_VALUE_SAVE(wip->armor_factor, 0);
		VAR_SET_VALUE_SAVE(wip->shield_factor, 0);
		VAR_SET_VALUE_SAVE(wip->subsystem_factor, 0);

		VAR_SET_VALUE_SAVE(wip->damage_type_idx, 0);

		VAR_SET_VALUE_SAVE(wip->shockwave.speed, 0);

		VAR_SET_VALUE_SAVE(wip->turn_time, 0);
		VAR_SET_VALUE_SAVE(wip->fov, 0);
		VAR_SET_VALUE_SAVE(wip->min_lock_time, 0);
		VAR_SET_VALUE_SAVE(wip->lock_pixels_per_sec, 0);
		VAR_SET_VALUE_SAVE(wip->catchup_pixels_per_sec, 0);
		VAR_SET_VALUE_SAVE(wip->catchup_pixel_penalty, 0);
		VAR_SET_VALUE_SAVE(wip->swarm_count, 0);
		VAR_SET_VALUE_SAVE(wip->SwarmWait, 0);
	}
}

void labviewer_make_variables_window(Button *caller)
{
	if (Lab_variables_window != NULL) {
		return;
	}

	Lab_variables_window = (Window*)Lab_screen->Add(new Window("Class Variables", gr_screen.center_offset_x + gr_screen.center_w - (VAR_POS_RIGHTX + VAR_POS_RIGHTWIDTH + 25), gr_screen.center_offset_y + 200));

	// set our new title
	if (Lab_mode == LAB_MODE_SHIP) {
		Lab_variables_window->SetCaption("Ship Variables");
	}
	else if (Lab_mode == LAB_MODE_WEAPON) {
		Lab_variables_window->SetCaption("Weapon Variables");
	}

	// populate the window with our flags (controls both ships and weapons flags)
	labviewer_populate_variables_window();

	// update content, if we need to
	labviewer_update_variables_window();

	Lab_variables_window->SetCloseFunction(labviewer_close_variables_window);
}

// --------------------   Render Options Window   ------------------------------
void labviewer_change_detail_texture(Tree *caller)
{
	int slider_pos = (int)(caller->GetSelectedItem()->GetData());
	Assert((slider_pos >= 0) && (slider_pos <= MAX_DETAIL_LEVEL));

	Detail.hardware_textures = slider_pos;
}

void labviewer_close_render_options_window(GUIObject *caller)
{
	Lab_render_options_window = NULL;
}

#define ADD_RENDER_FLAG(text, flag, var) {	\
	cbp = (Checkbox*)Lab_render_options_window->AddChild(new Checkbox((text), 2, y));	\
	cbp->SetFlag(&(flag), (var));	\
	y += cbp->GetHeight() + 2;	\
}

#define ADD_RENDER_BOOL(text, flag) {	\
	cbp = (Checkbox*)Lab_render_options_window->AddChild(new Checkbox((text), 2, y));	\
	cbp->SetBool(&(flag));	\
	y += cbp->GetHeight() + 1;	\
}

void labviewer_render_options_set_ambient_factor(Slider *caller) {
	gr_calculate_ambient_factor(fl2i( caller->GetSliderValue()));
}

void labviewer_render_options_set_static_light_factor(Slider *caller) {
	static_light_factor = caller->GetSliderValue();
}

void labviewer_render_options_set_bloom(Slider *caller) {
	Cmdline_bloom_intensity = fl2i(caller->GetSliderValue());
}

void labviewer_make_render_options_window(Button *caller)
{
	Checkbox *cbp;
	Tree *cmp;
	TreeItem *ctip;
	int y = 0;

	if (Lab_render_options_window != NULL) {
		return;
	}

	Lab_render_options_window = (Window*)Lab_screen->Add(new Window("Render Options", gr_screen.center_offset_x + gr_screen.center_w - 300, gr_screen.center_offset_y + 200));
	Assert(Lab_render_options_window != NULL);

	// add all of the flags that we want/need...

	// viewer flags
	ADD_RENDER_FLAG("Disable Model Rotation", Lab_viewer_flags, LAB_FLAG_NO_ROTATION);
	ADD_RENDER_FLAG("Show Insignia", Lab_viewer_flags, LAB_FLAG_SHOW_INSIGNIA);
	ADD_RENDER_FLAG("Show Damage Lightning", Lab_viewer_flags, LAB_FLAG_LIGHTNING_ARCS);
	ADD_RENDER_BOOL("Rotate Subsystems", Lab_rotate_subobjects);

	if (Cmdline_postprocess) {
		ADD_RENDER_BOOL("Hide Post Processing", PostProcessing_override);
	}

	// map related flags
	ADD_RENDER_BOOL("No Diffuse Map", Lab_Basemap_override);
	if (Cmdline_glow) {
		ADD_RENDER_BOOL("No Glow Map", Lab_Glowmap_override);
	}
	if (Cmdline_spec) {
		ADD_RENDER_BOOL("No Specular Map", Lab_Specmap_override);
	}
	if (Cmdline_env) {
		ADD_RENDER_BOOL("No Environment Map", Lab_Envmap_override);
	}
	if (Cmdline_normal) {
		ADD_RENDER_BOOL("No Normal Map", Lab_Normalmap_override);
	}
	if (Cmdline_height) {
		ADD_RENDER_BOOL("No Height Map", Lab_Heightmap_override);
	}
	ADD_RENDER_BOOL("No Team Colors", Lab_Miscmap_override);
	ADD_RENDER_BOOL("No Glow Points", Glowpoint_override);
	// model flags
	ADD_RENDER_BOOL("Wireframe", Lab_render_wireframe);
	ADD_RENDER_BOOL("No Lighting", Lab_render_without_light);
	ADD_RENDER_BOOL("Show Full Detail", Lab_render_show_detail);
	ADD_RENDER_BOOL("Show Thrusters", Lab_render_show_thrusters);
	ADD_RENDER_FLAG("Show Ship Weapons", Lab_viewer_flags, LAB_FLAG_SHOW_WEAPONS);
	ADD_RENDER_FLAG("Initial Rotation", Lab_viewer_flags, LAB_FLAG_INITIAL_ROTATION);
	ADD_RENDER_FLAG("Show Destroyed Subsystems", Lab_viewer_flags, LAB_FLAG_DESTROYED_SUBSYSTEMS);

	ADD_RENDER_BOOL("Emissive Lighting", Lab_emissive_light_override);
	Slider* sldr = (Slider*)Lab_render_options_window->AddChild(new Slider("Ambient Factor", 0, 128, 0, y + 2, labviewer_render_options_set_ambient_factor, Lab_render_options_window->GetWidth()));
	y += sldr->GetHeight() + 1;
	sldr = (Slider*)Lab_render_options_window->AddChild(new Slider("Direct. Lights", 0.0f, 2.0f, 0, y + 2, labviewer_render_options_set_static_light_factor, Lab_render_options_window->GetWidth()));
	y += sldr->GetHeight() + 1;
	sldr = (Slider*)Lab_render_options_window->AddChild(new Slider("Bloom", 0, 200, 0, y + 2, labviewer_render_options_set_bloom, Lab_render_options_window->GetWidth()));
	y += sldr->GetHeight() + 1;

	// start tree
	cmp = (Tree*)Lab_render_options_window->AddChild(new Tree("Detail Options Tree", 0, y + 2, NULL, Lab_render_options_window->GetWidth()));

	// 3d hardware texture slider options
	ctip = cmp->AddItem(NULL, "3D Hardware Textures", 0, false);

	cmp->AddItem(ctip, "Minimum", 0, false, labviewer_change_detail_texture);
	cmp->AddItem(ctip, "Low", 1, false, labviewer_change_detail_texture);
	cmp->AddItem(ctip, "Medium", 2, false, labviewer_change_detail_texture);
	cmp->AddItem(ctip, "High", 3, false, labviewer_change_detail_texture);
	cmp->AddItem(ctip, "Maximum", 4, false, labviewer_change_detail_texture);

	// set close function
	Lab_render_options_window->SetCloseFunction(labviewer_close_render_options_window);
}
// -------------------------  Material Override Window  ------------------------------

void labviewer_close_material_override_window(GUIObject *caller)
{
	Lab_material_override_window = NULL;

	Basemap_color_override_set = false;
	Basemap_color_override[0] = 0.0f;
	Basemap_color_override[1] = 0.0f;
	Basemap_color_override[2] = 0.0f;
	Basemap_color_override[3] = 1.0f;

	Glowmap_color_override_set = false;
	Glowmap_color_override[0] = 0.0f;
	Glowmap_color_override[1] = 0.0f;
	Glowmap_color_override[2] = 0.0f;

	Specmap_color_override_set = false;
	Specmap_color_override[0] = 0.0f;
	Specmap_color_override[1] = 0.0f;
	Specmap_color_override[2] = 0.0f;
}

void labviewer_set_material_override_diffuse_red(Slider *caller)
{
	Basemap_color_override[0] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_diffuse_green(Slider *caller)
{
	Basemap_color_override[1] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_diffuse_blue(Slider *caller)
{
	Basemap_color_override[2] = caller->GetSliderValue() / 255.0f;
}
void labviewer_set_material_override_glow_red(Slider *caller)
{
	Glowmap_color_override[0] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_glow_green(Slider *caller)
{
	Glowmap_color_override[1] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_glow_blue(Slider *caller)
{
	Glowmap_color_override[2] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_specular_red(Slider *caller)
{
	Specmap_color_override[0] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_specular_green(Slider *caller)
{
	Specmap_color_override[1] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_specular_blue(Slider *caller)
{
	Specmap_color_override[2] = caller->GetSliderValue() / 255.0f;
}

void labviewer_set_material_override_specular_gloss(Slider *caller)
{
	Gloss_override = caller->GetSliderValue() / 255.0f;
}

void labviewer_make_material_override_window(Button *caller)
{
	Checkbox *cbp;
	Slider *sldr;
	int y = 0;

	if (Lab_material_override_window != NULL) {
		return;
	}

	Lab_material_override_window = (Window*)Lab_screen->Add(new Window("Material Override", gr_screen.max_w - 300, 200));
	Assert(Lab_material_override_window != NULL);

	// add all of the flags that we want/need...

	// map related flags

	cbp = (Checkbox*)Lab_material_override_window->AddChild(new Checkbox("Override Diffuse Color", 2, y));
	cbp->SetBool(&Basemap_color_override_set);
	y += cbp->GetHeight() + 1;

	sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Red", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_diffuse_red, 275));
	y += sldr->GetHeight() + 1;

	sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Green", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_diffuse_green, 275));
	y += sldr->GetHeight() + 1;

	sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Blue", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_diffuse_blue, 275));
	y += sldr->GetHeight() + 1;

	if (Cmdline_glow) {
		cbp = (Checkbox*)Lab_material_override_window->AddChild(new Checkbox("Override Glow Color", 2, y));
		cbp->SetBool(&Glowmap_color_override_set);
		y += cbp->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Red", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_glow_red, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Green", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_glow_green, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Blue", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_glow_blue, 275));
		y += sldr->GetHeight() + 1;
	}
	if (Cmdline_spec) {
		cbp = (Checkbox*)Lab_material_override_window->AddChild(new Checkbox("Override Spec Color", 2, y));
		cbp->SetBool(&Specmap_color_override_set);
		y += cbp->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Red", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_red, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Green", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_green, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Blue", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_blue, 275));
		y += sldr->GetHeight() + 1;

		cbp = (Checkbox*)Lab_material_override_window->AddChild(new Checkbox("Override Gloss", 2, y));
		cbp->SetBool(&Gloss_override_set);
		y += cbp->GetHeight() + 1;

		sldr = (Slider*)Lab_material_override_window->AddChild(new Slider("Gloss", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_gloss, 275));
		y += sldr->GetHeight() + 1;
	}

	// set close function
	Lab_material_override_window->SetCloseFunction(labviewer_close_material_override_window);
}

// -------------------------  Description Window  ------------------------------
void labviewer_close_desc_window(GUIObject *caller)
{
	Lab_description_text = NULL;
	Lab_description_window = NULL;
}

void labviewer_update_desc_window()
{
	if ((Lab_description_window == NULL) || (Lab_description_text == NULL)) {
		return;
	}

	if (Lab_selected_index != -1) {
		if (Lab_mode == LAB_MODE_SHIP) {
			Lab_description_window->SetCaption(Ship_info[Lab_selected_index].name);

			if (Ship_info[Lab_selected_index].tech_desc != NULL) {
				Lab_description_text->SetText(Ship_info[Lab_selected_index].tech_desc);
			}
			else {
				Lab_description_text->SetText("No description available.");
			}
		}
		else if (Lab_mode == LAB_MODE_WEAPON) {
			Lab_description_window->SetCaption(Weapon_info[Lab_selected_index].name);

			if (Weapon_info[Lab_selected_index].tech_desc != NULL) {
				Lab_description_text->SetText(Weapon_info[Lab_selected_index].tech_desc);
			}
			else {
				Lab_description_text->SetText("No description available.");
			}
		}
	}
}

void labviewer_make_desc_window(Button *caller)
{
	if (Lab_description_window != NULL) {
		return;
	}

	Lab_description_window = (Window*)Lab_screen->Add(new Window("Description", gr_screen.center_offset_x + gr_screen.center_w - gr_screen.center_w / 3 - 50,
		gr_screen.center_offset_y + gr_screen.center_h - gr_screen.center_h / 6 - 50, gr_screen.center_w / 3,
		gr_screen.center_h / 6));
	Lab_description_text = (Text*)Lab_description_window->AddChild(new Text("Description Text", "No ship selected.", 0, 0));

	labviewer_update_desc_window();

	Lab_description_window->SetCloseFunction(labviewer_close_desc_window);
}

// ------------------------   Ships Window   -----------------------------------
void labviewer_make_ship_window(Button *caller)
{
	GUIObject *cbp;
	TreeItem *ctip, *stip;
	int x, idx;

	if (Lab_mode == LAB_MODE_SHIP) {
		return;
	}


	// switch the class window to ship mode
	labviewer_set_class_window(LAB_MODE_SHIP);

	if ((Lab_class_window == NULL) || (Lab_class_toolbar == NULL)) {
		Int3();
		Lab_mode = LAB_MODE_SHIP;
		return;
	}


	// populate the class toolbar
	x = 0;
	cbp = Lab_class_toolbar->AddChild(new Button("Class Description", x, 0, labviewer_make_desc_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_class_toolbar->AddChild(new Button("Class Options", x, 0, labviewer_make_flags_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_class_toolbar->AddChild(new Button("Class Variables", x, 0, labviewer_make_variables_window));


	// populate ship class window
	Tree *cmp = (Tree*)Lab_class_window->AddChild(new Tree("Ship Tree", 0, 0));

	if (Lab_species_nodes != NULL) {
		for (idx = 0; idx < (int)Species_info.size(); idx++) {
			Lab_species_nodes[idx]->ClearAllItems();
		}

		delete[] Lab_species_nodes;
		Lab_species_nodes = NULL;
	}

	Lab_species_nodes = new TreeItem*[Species_info.size() + 1];

	// Add species nodes
	for (idx = 0; idx < (int)Species_info.size(); idx++) {
		Lab_species_nodes[idx] = cmp->AddItem(NULL, Species_info[idx].species_name, 0, false);
	}

	// Just in case. I don't actually think this is possible though.
	Lab_species_nodes[Species_info.size()] = cmp->AddItem(NULL, "Other", 0, false);

	// Now add the ships
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if ((it->species >= 0) && (it->species < (int)Species_info.size())) {
			stip = Lab_species_nodes[it->species];
		}
		else {
			stip = Lab_species_nodes[Species_info.size()];
		}

		ctip = cmp->AddItem(stip, it->name, (int)std::distance(Ship_info.cbegin(), it), false, labviewer_change_ship);
		cmp->AddItem(ctip, "Model", 0, false, labviewer_change_ship_lod);
		//cmp->AddItem(ctip, "Debris", 99, false, labviewer_change_ship_lod);
	}

	// if any nodes are empty, just add a single "<none>" entry so we know that species doesn't have anything yet
	// No the <= is not a mistake :)
	for (idx = 0; idx < (int)Species_info.size(); idx++) {
		if (!Lab_species_nodes[idx]->HasChildren()) {
			cmp->AddItem(Lab_species_nodes[idx], "<none>", 0, false, NULL);
		}
	}

	// if the "Other" entry doesn't contain anything then just delete it
	if (!Lab_species_nodes[Species_info.size()]->HasChildren()) {
		delete Lab_species_nodes[Species_info.size()];
	}
	
	// and... we're done!
	Lab_mode = LAB_MODE_SHIP;

	// populate the flags window, if it exists
	// (NOTE: must be done *after* Lab_mode is set properly)
	if (Lab_flags_window != NULL) {
		Lab_flags_window->SetCaption("Ship Flags");

		labviewer_populate_flags_window();
		labviewer_update_flags_window();
	}

	if (Lab_description_window != NULL) {
		labviewer_update_desc_window();
	}

	if (Lab_variables_window != NULL) {
		Lab_variables_window->SetCaption("Ship Variables");

		labviewer_populate_variables_window();
		labviewer_update_variables_window();
	}
}

void labviewer_change_ship_lod(Tree* caller)
{
	int ship_index = (int)(caller->GetSelectedItem()->GetParentItem()->GetData());
	Assert(ship_index >= 0);

	if (Lab_selected_object == -1)
	{
		// Goober5000 - The lab loads subsystems into its special lab-specific vector, but normally subsystems are loaded into the Ship_info
		// entry.  Note also that models are only loaded once each.  If a lab model was previously loaded using the "lightweight" method,
		// ship_create will not find any of the subsystems it is looking for.  So we have to make sure the model is only loaded for the purposes
		// of error-checking and then immediately cleared so that it can be subsequently loaded the lab way.

		// reset any existing model/bitmap that is showing
		labviewer_change_model(NULL);
	}
	else
	{
		obj_delete(Lab_selected_object);
	}

	Lab_selected_object = ship_create(&vmd_identity_matrix, &Lab_model_pos, ship_index);
	Objects[Lab_selected_object].flags.set(Object::Object_Flags::Player_ship);

	lab_cam_distance = Objects[Lab_selected_object].radius * 1.6f;

	Lab_last_selected_ship = Lab_selected_index;
	labviewer_change_model(Ship_info[ship_index].pof_file, caller->GetSelectedItem()->GetData(), ship_index);
	if (Ship_info[ship_index].uses_team_colors) {
		Lab_team_color = Ship_info[ship_index].default_team_name;
	}
	else {
		Lab_team_color = "<none>";
	}

	labviewer_update_desc_window();
	labviewer_update_flags_window();
	labviewer_update_variables_window();
	labviewer_recalc_camera();
}

void labviewer_change_ship(Tree *caller)
{
	Lab_selected_index = (int)(caller->GetSelectedItem()->GetData());

	labviewer_update_desc_window();
	labviewer_update_flags_window();
	labviewer_update_variables_window();
}

// ---------------------------  Weapons Window  --------------------------------
void labviewer_show_tech_model(Tree *caller)
{
	int weap_index = (int)(caller->GetSelectedItem()->GetParentItem()->GetData());
	Assert(weap_index >= 0);

	labviewer_change_model(Weapon_info[weap_index].tech_model, caller->GetSelectedItem()->GetData(), weap_index);
}

void labviewer_show_external_model(Tree *caller)
{
	int weap_index = (int)(caller->GetSelectedItem()->GetParentItem()->GetData());
	Assert(weap_index >= 0);

	labviewer_change_model(Weapon_info[weap_index].external_model_name, caller->GetSelectedItem()->GetData(), weap_index);
}

extern void weapon_load_bitmaps(int weapon_index);
void labviewer_change_weapon(Tree *caller)
{
	int weap_index = (int)(caller->GetSelectedItem()->GetData());
	Assert(weap_index >= 0);

	if (Lab_selected_object != -1)
		obj_delete(Lab_selected_object);

	if (!(Weapon_info[weap_index].wi_flags[Weapon::Info_Flags::Beam])) {
		Lab_selected_object = weapon_create(&vmd_zero_vector, &vmd_identity_matrix, weap_index, -1);
	}

	lab_cam_distance = Objects[Lab_selected_object].radius * 20.0f;

	Lab_selected_index = weap_index;
	Lab_last_selected_weapon = Lab_selected_index;

	labviewer_update_desc_window();
	labviewer_update_flags_window();
	labviewer_update_variables_window();
	labviewer_recalc_camera();
}

// weapon window create function
void labviewer_make_weap_window(Button* caller)
{
	GUIObject *cbp;
	TreeItem *stip;
	int x;

	if (Lab_mode == LAB_MODE_WEAPON) {
		return;
	}


	// switch the class window to weapon mode
	labviewer_set_class_window(LAB_MODE_WEAPON);

	if ((Lab_class_window == NULL) || (Lab_class_toolbar == NULL)) {
		Int3();
		Lab_mode = LAB_MODE_WEAPON;
		return;
	}


	// populate the weapons toolbar
	x = 0;
	cbp = Lab_class_toolbar->AddChild(new Button("Class Description", x, 0, labviewer_make_desc_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_class_toolbar->AddChild(new Button("Class Options", x, 0, labviewer_make_flags_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_class_toolbar->AddChild(new Button("Class Variables", x, 0, labviewer_make_variables_window));


	// populate the weapons window
	Tree *cmp = (Tree*)Lab_class_window->AddChild(new Tree("Weapon Tree", 0, 0));

	// Unfortunately these are hardcoded
	TreeItem **type_nodes = new TreeItem*[Num_weapon_subtypes];
	int i;

	// Add type nodes
	for (i = 0; i < Num_weapon_subtypes; i++) {
		type_nodes[i] = cmp->AddItem(NULL, Weapon_subtype_names[i], 0, false);
	}

	// Now add the weapons
	for (i = 0; i < Num_weapon_types; i++) {
		if (Weapon_info[i].subtype == WP_UNUSED) {
			continue;
		}
		else if (Weapon_info[i].subtype >= Num_weapon_subtypes) {
			Warning(LOCATION, "Invalid weapon subtype found on weapon %s", Weapon_info[i].name);
			continue;
		}

		if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Beam]) {
			stip = type_nodes[WP_BEAM];
		}
		else {
			stip = type_nodes[Weapon_info[i].subtype];
		}

		cmp->AddItem(stip, Weapon_info[i].name, i, false, labviewer_change_weapon);

		//if (Weapon_info[i].tech_model[0] != '\0') {
		//	cmp->AddItem(cwip, "Tech Model", 0, false, labviewer_show_tech_model);
		//}
		//if (Weapon_info[i].external_model_name[0] != '\0') {
		//	cmp->AddItem(cwip, "External Model", 0, false, labviewer_show_external_model);
		//}
	}

	// Get rid of any empty nodes
	for (i = 0; i < Num_weapon_subtypes; i++) {
		if (!type_nodes[i]->HasChildren()) {
			delete type_nodes[i];
		}
	}
	delete[] type_nodes;

	Lab_mode = LAB_MODE_WEAPON;

	// populate the flags window, if it exists
	// (NOTE: must be done *after* Lab_mode is set properly)
	if (Lab_flags_window != NULL) {
		Lab_flags_window->SetCaption("Weapon Flags");

		labviewer_populate_flags_window();
		labviewer_update_flags_window();
	}

	if (Lab_description_window != NULL) {
		labviewer_update_desc_window();
	}

	if (Lab_variables_window != NULL) {
		Lab_variables_window->SetCaption("Weapon Variables");

		labviewer_populate_variables_window();
		labviewer_update_variables_window();
	}
}

// ----------------------------- Backgrounds -----------------------------------

static TreeItem** Mission_directories = NULL;
size_t Num_mission_directories = 0;

char skybox_model[MAX_FILENAME_LEN];
int skybox_flags;

int ambient_light_level;
extern const char *Neb2_filenames[];

char envmap_name[MAX_FILENAME_LEN];

const char* mission_ext_list[] = { ".fs2" };

SCP_string get_directory_or_vp(char* path)
{
	SCP_string result(path);

	// Is this a mission in a directory?
	size_t found = result.find("data" DIR_SEPARATOR_STR "missions");

	if (found == std::string::npos)  // Guess not
	{
		found = result.find(".vp");
	}

	auto directory_name_pos = result.rfind(DIR_SEPARATOR_STR, found - strlen(DIR_SEPARATOR_STR) - 1);

	result = result.substr(directory_name_pos, found - directory_name_pos);

	found = result.find(DIR_SEPARATOR_STR);
	//Strip directory separators
	while (found != std::string::npos)
	{
		result.erase(found, strlen(DIR_SEPARATOR_STR));
		found = result.find(DIR_SEPARATOR_STR);
	}

	return result;
}

void labviewer_change_background_actual()
{
	matrix skybox_orientation;

	stars_pre_level_init(true);
	vm_set_identity(&skybox_orientation);

	// (DahBlount) - Remember to load the debris anims
	stars_load_debris(false);

	if (Lab_selected_mission.compare("None") != 0)
	{
		read_file_text((Lab_selected_mission + ".fs2").c_str(), CF_TYPE_MISSIONS);
		reset_parse();

		flagset<Mission::Mission_Flags> flags;
		skip_to_start_of_string("+Flags");
		if (optional_string("+Flags:"))
			stuff_flagset(&flags);

		// Are we using a skybox?
		skip_to_start_of_string_either("$Skybox Model:", "#Background bitmaps");

		strcpy_s(skybox_model, "");
		if (optional_string("$Skybox Model:"))
		{
			stuff_string(skybox_model, F_NAME, MAX_FILENAME_LEN);

			if (optional_string("+Skybox Orientation:"))
			{
				stuff_matrix(&skybox_orientation);
			}

			if (optional_string("+Skybox Flags:")) {
				skybox_flags = 0;
				stuff_int(&skybox_flags);
			}
			else {
				skybox_flags = DEFAULT_NMODEL_FLAGS;
			}

			stars_set_background_model(skybox_model, NULL, skybox_flags);
			stars_set_background_orientation(&skybox_orientation);

			skip_to_start_of_string("#Background bitmaps");
		}

		if (optional_string("#Background bitmaps"))
		{
			required_string("$Num stars:");
			stuff_int(&Num_stars);
			if (Num_stars >= MAX_STARS)
				Num_stars = MAX_STARS;

			required_string("$Ambient light level:");
			stuff_int(&ambient_light_level);

			if (ambient_light_level == 0)
			{
				ambient_light_level = DEFAULT_AMBIENT_LIGHT_LEVEL;
			}

			gr_set_ambient_light(ambient_light_level & 0xff,
				(ambient_light_level >> 8) & 0xff,
				(ambient_light_level >> 16) & 0xff);

			strcpy_s(Neb2_texture_name, "Eraseme3");
			Neb2_poof_flags = ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
			if (optional_string("+Neb2:")) {
				stuff_string(Neb2_texture_name, F_NAME, MAX_FILENAME_LEN);

				required_string("+Neb2Flags:");
				stuff_int(&Neb2_poof_flags);

				if (flags[Mission::Mission_Flags::Fullneb]) {
					neb2_post_level_init();
				}
			}

			if (flags[Mission::Mission_Flags::Fullneb]) {
				// no regular nebula stuff
				nebula_close();
			}
			else
			{
				Nebula_index = -1;
				if (optional_string("+Nebula:")) {
					char str[MAX_FILENAME_LEN];
					int z;
					stuff_string(str, F_NAME, MAX_FILENAME_LEN);

					// parse the proper nebula type (full or not)	
					for (z = 0; z < NUM_NEBULAS; z++) {
						if (flags[Mission::Mission_Flags::Fullneb]) {
							if (!stricmp(str, Neb2_filenames[z])) {
								Nebula_index = z;
								break;
							}
						}
						else {
							if (!stricmp(str, Nebula_filenames[z])) {
								Nebula_index = z;
								break;
							}
						}
					}

					if (z == NUM_NEBULAS)
						WarningEx(LOCATION, "Unknown nebula %s!", str);

					if (optional_string("+Color:")) {
						stuff_string(str, F_NAME, MAX_FILENAME_LEN);
						for (z = 0; z < NUM_NEBULA_COLORS; z++) {
							if (!stricmp(str, Nebula_colors[z])) {
								Mission_palette = z;
								break;
							}
						}
					}

					if (z == NUM_NEBULA_COLORS)
						WarningEx(LOCATION, "Unknown nebula color %s!", str);

					if (optional_string("+Pitch:")) {
						stuff_int(&Nebula_pitch);
					}
					else {
						Nebula_pitch = 0;
					}

					if (optional_string("+Bank:")) {
						stuff_int(&Nebula_bank);
					}
					else {
						Nebula_bank = 0;
					}

					if (optional_string("+Heading:")) {
						stuff_int(&Nebula_heading);
					}
					else {
						Nebula_heading = 0;
					}
				}

				if (Nebula_index >= 0) {
					nebula_init(Nebula_filenames[Nebula_index], Nebula_pitch, Nebula_bank, Nebula_heading);
				}
				else {
					nebula_close();
				}
			}

			stars_load_debris(flags[Mission::Mission_Flags::Fullneb]);

			Num_backgrounds = 0;
			extern void parse_one_background(background_t* background);
			while (optional_string("$Bitmap List:") || check_for_string("$Sun:") || check_for_string("$Starbitmap:"))
			{
				// don't allow overflow; just make sure the last background is the last read
				if (Num_backgrounds >= MAX_BACKGROUNDS)
				{
					Warning(LOCATION, "Too many backgrounds in mission!  Max is %d.", MAX_BACKGROUNDS);
					Num_backgrounds = MAX_BACKGROUNDS - 1;
				}

				parse_one_background(&Backgrounds[Num_backgrounds]);
				Num_backgrounds++;
			}

			stars_load_first_valid_background();

			if (optional_string("$Environment Map:")) {
				stuff_string(envmap_name, F_NAME, MAX_FILENAME_LEN);
			}
		}
	}
	else {
		// (DahBlount) - This spot should be used to disable rendering features that only apply to missions.
		Motion_debris_override = true;
	}
}

void labviewer_change_background(Tree* caller)
{
	Lab_selected_mission = caller->GetSelectedItem()->Name;

	labviewer_change_background_actual();
}

void lab_background_window_close(GUIObject* caller) 
{
	Lab_background_window = NULL;
}

void labviewer_make_background_window(Button* caller)
{
	if (Lab_background_window != NULL) return;

	Lab_background_window = (Window*)Lab_screen->Add(new Window("Mission Backgrounds", gr_screen.center_offset_x + 250, gr_screen.center_offset_y + 50));
	Lab_background_window->SetCloseFunction(lab_background_window_close);
	SCP_vector<SCP_string> missions;

	cf_get_file_list(missions, CF_TYPE_MISSIONS, NOX("*.fs2"));

	SCP_map<SCP_string, SCP_vector<SCP_string>> directories;
	char fullpath[MAX_PATH_LEN];

	for (auto filename : missions)
	{
		cf_find_file_location((filename + ".fs2").c_str(), CF_TYPE_MISSIONS, sizeof(fullpath) - 1, fullpath, NULL, NULL);
		auto location = get_directory_or_vp(fullpath);

		directories[location].push_back(filename);
	}

	Num_mission_directories = directories.size();
	Mission_directories = new TreeItem*[Num_mission_directories];

	Tree* missiontree = (Tree*)Lab_background_window->AddChild(new Tree("Missions", 0, 0));
	missiontree->AddItem(NULL, "None", 0, true, labviewer_change_background);

	int i = 0;
	for (auto directory : directories)
	{
		auto directoryItem = Mission_directories[i];
		directoryItem = missiontree->AddItem(NULL, directory.first);

		for (auto Lab_mission : directory.second)
		{
			missiontree->AddItem(directoryItem, Lab_mission, 0, true, labviewer_change_background);
		}
	}
}

// ----------------------------- Lab functions ---------------------------------

void lab_init()
{
	GUIObject *cbp;
	int x, i;

	weapon_pause_sounds();

	gr_set_clear_color(0, 0, 0);


	//We start by creating the screen/toolbar
	Lab_screen = GUI_system.PushScreen(new GUIScreen("Lab"));

	Lab_toolbar = (Window*)Lab_screen->Add(new Window("Toolbar", gr_screen.center_offset_x, gr_screen.center_offset_y, -1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));

	// start filling the main toolbar
	x = 0;
	cbp = Lab_toolbar->AddChild(new Button("Ships", x, 0, labviewer_make_ship_window));
	
	x += cbp->GetWidth() + 10;
	cbp = Lab_toolbar->AddChild(new Button("Weapons", x, 0, labviewer_make_weap_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_toolbar->AddChild(new Button("Render Options", x, 0, labviewer_make_render_options_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_toolbar->AddChild(new Button("Material Overrides", x, 0, labviewer_make_material_override_window));

	x += cbp->GetWidth() + 10;
	cbp = Lab_toolbar->AddChild(new Button("Backgrounds", x, 0, labviewer_make_background_window));

	x += cbp->GetWidth() + 20;
	cbp = Lab_toolbar->AddChild(new Button("Exit", x, 0, labviewer_exit));


	// reset some defaults, just to be sure
	Lab_model_pos = vmd_zero_vector;
	Lab_mode = LAB_MODE_NONE;
	Lab_thrust_len = 1.0f;
	Lab_thrust_afterburn = false;
	for (i = 0; i < MAX_SHIP_WEAPONS; i++) {
		Lab_weaponmodel_num[i] = -1;
	}

	// save detail options
	Lab_detail_texture_save = Detail.hardware_textures;
	// load up the list of insignia that we might use on the ships
	pilot_load_squad_pic_list();

	// the default insignia bitmap
	Lab_insignia_index = 0;
	Assert((Lab_insignia_index < Num_pilot_squad_images));

	Lab_insignia_bitmap = bm_load_duplicate(Pilot_squad_image_names[Lab_insignia_index]);

	// enable post-processing by default in the lab
	PostProcessing_override = false;
	// disable model rotation by default in the lab
	Lab_viewer_flags |= LAB_FLAG_NO_ROTATION;
	Lab_viewer_flags |= LAB_FLAG_INITIAL_ROTATION;


	flagset<Object::Object_Flags> obs_flags;
	auto obj_index = obj_create(OBJ_OBSERVER, -1, 0, &vmd_identity_matrix, &vmd_zero_vector, 0, obs_flags);

	Player_obj = &Objects[obj_index];
	Viewer_obj = Player_obj;

	Viewer_mode = VM_EXTERNAL;

	if (!Lab_cam.isValid())
		Lab_cam = cam_create("Lab camera");

	Lab_Save_Missiontime = Missiontime;
	gr_init_alphacolor(&HUD_color_debug, 128, 255, 128, HUD_color_alpha * 16);

	if (The_mission.ai_profile == nullptr)
		The_mission.ai_profile = &Ai_profiles[Default_ai_profile];
}

#include "controlconfig/controlsconfig.h"
#include "lab.h"
void lab_do_frame(float frametime)
{
	GR_DEBUG_SCOPE("Lab Frame");

	gr_reset_clip();
	gr_clear();

	Missiontime += Frametime;

	labviewer_do_render(frametime);

	bool test1 = (GUI_system.OnFrame(frametime, !(Trackball_active) ? true : false, false) == GSOF_NOTHINGPRESSED);

	if (test1) {
		int key = GUI_system.GetKeyPressed();
		int status = GUI_system.GetStatus();

		// set trackball modes
		if (status & GST_MOUSE_LEFT_BUTTON) {
			Trackball_active = 1;
			Trackball_mode = 1;	// rotate viewed object

			if (key_get_shift_status() & KEY_SHIFTED) {
				Trackball_mode = 2;	// zoom
			}
		}
		else if (status & GST_MOUSE_RIGHT_BUTTON) {
			Trackball_active = 1;
			Trackball_mode = 3;	// rotate camera
		}
		else if (!mouse_down(MOUSE_LEFT_BUTTON | MOUSE_RIGHT_BUTTON)) {
			// reset trackball modes
			Trackball_active = 0;
			Trackball_mode = 0;
		}

		//Due to switch scoping rules, this has to be declared here
		SCP_map<SCP_string, team_color>::iterator color_itr = Team_Colors.find(Lab_team_color);
		// handle any key presses
		switch (key) {
			// switch between the current insignia bitmap to render with
		case KEY_DIVIDE: {
			if (!(Lab_viewer_flags & LAB_FLAG_SHOW_INSIGNIA)) {
				break;
			}

			Lab_insignia_index = (Lab_insignia_index + 1) % Num_pilot_squad_images;
			Assert((Lab_insignia_index >= 0) && (Lab_insignia_index < Num_pilot_squad_images));

			if (Lab_insignia_bitmap >= 0) {
				bm_release(Lab_insignia_bitmap);
				Lab_insignia_bitmap = -1;
			}

			Lab_insignia_bitmap = bm_load_duplicate(Pilot_squad_image_names[Lab_insignia_index]);

			break;
		}

						 // change between damage lightning effects
		case KEY_L:
			// REIMPLEMENT
			break;

			// Adjust FXAA presets
		case KEY_0:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 0;
			break;
		case KEY_1:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 1;
			break;
		case KEY_2:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 2;
			break;
		case KEY_3:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 3;
			break;
		case KEY_4:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 4;
			break;
		case KEY_5:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 5;
			break;
		case KEY_6:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 6;
			break;
		case KEY_7:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 7;
			break;
		case KEY_8:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 8;
			break;
		case KEY_9:
			if (!PostProcessing_override)
				Cmdline_fxaa_preset = 9;
			break;

		case KEY_T:
			if (color_itr == Team_Colors.begin()) {
				color_itr = --Team_Colors.end();
				Lab_team_color = color_itr->first;
			}
			else {
				--color_itr;
				Lab_team_color = color_itr->first;
			}
			break;

		case KEY_Y:
			++color_itr;
			if (color_itr == Team_Colors.end())
				color_itr = Team_Colors.begin();
			Lab_team_color = color_itr->first;
			break;

			// bail...
		case KEY_ESC:
			labviewer_exit(NULL);
			break;

		default: {
			// check for game-specific controls
			if (Lab_mode == LAB_MODE_SHIP) {
				if (check_control(PLUS_5_PERCENT_THROTTLE, key))
					Lab_thrust_len += 0.05f;
				else if (check_control(MINUS_5_PERCENT_THROTTLE, key))
					Lab_thrust_len -= 0.05f;

				CLAMP(Lab_thrust_len, 0.0f, 1.0f);

				if (check_control(AFTERBURNER, key))
					Lab_thrust_afterburn = !Lab_thrust_afterburn;
			}

			break;
		}
		}
	}

	gr_flip();
}

void lab_close()
{
	int i;

	Lab_toolbar = NULL;
	Lab_class_toolbar = NULL;
	Lab_class_window = NULL;
	Lab_flags_window = NULL;
	Lab_render_options_window = NULL;
	Lab_background_window = NULL;

	delete Lab_screen;

	Lab_screen = NULL;


	if (Lab_species_nodes != NULL) {
		for (i = 0; i < (int)Species_info.size(); i++) {
			delete Lab_species_nodes[i];
		}

		delete[] Lab_species_nodes;
		Lab_species_nodes = NULL;
	}

	if (Lab_insignia_bitmap >= 0) {
		bm_release(Lab_insignia_bitmap);
		Lab_insignia_bitmap = -1;
	}

	Lab_insignia_index = -1;

	if (Lab_model_num != -1) {
		model_page_out_textures(Lab_model_num, true);
		model_unload(Lab_model_num);
		Lab_model_num = -1;
	}

	for (i = 0; i < MAX_SHIP_WEAPONS; i++) {
		if (Lab_weaponmodel_num[i] >= 0) {
			model_page_out_textures(Lab_weaponmodel_num[i], true);
			model_unload(Lab_weaponmodel_num[i]);
			Lab_weaponmodel_num[i] = -1;
		}
	}

	Lab_selected_mission = "None";
	stars_pre_level_init(true);

	memset(Lab_model_filename, 0, sizeof(Lab_model_filename));

	Basemap_override = false;
	Envmap_override = false;
	Specmap_override = false;
	Normalmap_override = false;
	Heightmap_override = false;
	Glowpoint_override = false;
	PostProcessing_override = false;

	Lab_Basemap_override = false;
	Lab_Glowmap_override = false;
	Lab_Specmap_override = false;
	Lab_Envmap_override = false;
	Lab_Normalmap_override = false;
	Lab_Heightmap_override = false;

	// reset detail levels to default
	Detail.hardware_textures = Lab_detail_texture_save;

	//Reset ambient factor
	gr_calculate_ambient_factor(Cmdline_ambient_factor);

	weapon_unpause_sounds();
	//audiostream_unpause_all();
	game_flush();

	cam_delete(Lab_cam);

	Missiontime = Lab_Save_Missiontime;
}
