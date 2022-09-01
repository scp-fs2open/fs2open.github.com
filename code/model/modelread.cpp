/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <cstring>
#include <cctype>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#endif

#define MODEL_LIB

#include "asteroid/asteroid.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "freespace.h"		// For flFrameTime
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "io/key.h"
#include "io/timer.h"
#include "math/fvi.h"
#include "math/vecmat.h"
#include "model/model.h"
#include "model/modelreplace.h"
#include "model/modelsinc.h"
#include "parse/parselo.h"
#include "render/3dinternal.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "weapon/weapon.h"
#include "tracing/tracing.h"

#include <algorithm>
#include <stack>
#include <map>

flag_def_list model_render_flags[] =
{
	{"no lighting",		MR_NO_LIGHTING,     0},
	{"transparent",		MR_ALL_XPARENT,     0},
	{"no Zbuffer",		MR_NO_ZBUFFER,      0},
	{"no cull",			MR_NO_CULL,         0},
	{"no glowmaps",		MR_NO_GLOWMAPS,     0},
	{"force clamp",		MR_FORCE_CLAMP,     0},
};
  	 
int model_render_flags_size = sizeof(model_render_flags)/sizeof(flag_def_list);

#define MAX_SUBMODEL_COLLISION_ANGULAR_VELOCITY		(PI / 6.0f)		// max 30 degrees per frame
#define MAX_SUBMODEL_COLLISION_LINEAR_VELOCITY		100.0f			// max 100 meters per frame

// info for special polygon lists

polymodel *Polygon_models[MAX_POLYGON_MODELS];
SCP_vector<polymodel_instance*> Polygon_model_instances;

SCP_vector<bsp_collision_tree> Bsp_collision_tree_list;

static int model_initted = 0;

#ifndef NDEBUG
CFILE *ss_fp = NULL;			// file pointer used to dump subsystem information
char  model_filename[_MAX_PATH];		// temp used to store filename
char	debug_name[_MAX_PATH];
static bool ss_warning_shown_null = false;		// have we shown the warning dialog concerning the subsystems?
static bool ss_warning_shown_mismatch = false;	// ditto but for a different warning
#endif

static uint Global_checksum = 0;

// Anything less than this is considered incompatible.
#define PM_COMPATIBLE_VERSION 1900

// Anything greater than or equal to PM_COMPATIBLE_VERSION and 
// whose major version is less than or equal to this is considered
// compatible.  
#define PM_OBJFILE_MAJOR_VERSION 30

// 23.01 adds support for submodel translation
// 23.00 adds support for increased subobject vertex limit via TMAP2POLY
// 
// 22.02 adds support for submodel translation
// 22.01 adds support for external weapon model angle offsets
// 22.00 fixes the POF byte alignment and introduces the SLC2 chunk
//
// 21.19 adds support for submodel translation
// 21.18 adds support for external weapon model angle offsets
// 21.17 adds support for engine thruster banks linked to specific engine subsystems
// FreeSpace 2 shipped at POF version 21.17
// Descent: FreeSpace shipped at POF version 20.14
// See also https://wiki.hard-light.net/index.php/POF_data_structure
#define PM_LATEST_VERTLIM_VERSION	2301
#define PM_FIRST_VERTLIM_VERSION	2300

#define PM_LATEST_ALIGNED_VERSION	2202
#define PM_FIRST_ALIGNED_VERSION	2200

#define PM_LATEST_LEGACY_VERSION	2119


static int Model_signature = 0;

void interp_configure_vertex_buffers(polymodel*, int, const model_read_deferred_tasks& deferredTasks);
void interp_pack_vertex_buffers(polymodel* pm, int mn);
void interp_create_detail_index_buffer(polymodel *pm, int detail);
void interp_create_transparency_index_buffer(polymodel *pm, int detail_num);
void model_interp_process_shield_mesh(polymodel * pm);

void model_set_subsys_path_nums(polymodel *pm, int n_subsystems, model_subsystem *subsystems);
void model_set_bay_path_nums(polymodel *pm);

uint align_bsp_data(ubyte* bsp_in, ubyte* bsp_out, uint bsp_size);
uint convert_sldc_to_slc2(ubyte* sldc, ubyte* slc2, uint tree_size);


// Goober5000 - see SUBSYSTEM_X in model.h
// NOTE: Each subsystem must match up with its #define, or there will be problems
const char *Subsystem_types[SUBSYSTEM_MAX] =
{
	"None",
	"Engines",
	"Turrets",
	"Radar",
	"Navigation",
	"Communications",
	"Weapons",
	"Sensors",
	"Solar panels",
	"Gas collection",
	"Activation",
	"Unknown"
};


//WMC - For general compatibility stuff.
//Note that the order of the items in this list
//determine the order that they are tried in ai_goal_fixup_dockpoints
flag_def_list Dock_type_names[] =
{
	{ "cargo",		DOCK_TYPE_CARGO,	0 },
	{ "rearm",		DOCK_TYPE_REARM,	0 },
	{ "generic",	DOCK_TYPE_GENERIC,	0 }
};

int Num_dock_type_names = sizeof(Dock_type_names) / sizeof(flag_def_list);

SCP_vector<glow_point_bank_override> glowpoint_bank_overrides;


// Goober5000 - reimplementation of Bobboau's $dumb_rotation and $look_at features in a way that works with the rest of the model instance system
// note: since these data types are only ever used in this file, they don't need to be in model.h

class intrinsic_motion
{
public:
	bool is_object;
	int model_instance_num;
	SCP_vector<int> submodel_list;

	intrinsic_motion(bool _is_object, int _model_instance_num)
		: is_object(_is_object), model_instance_num(_model_instance_num)
	{}

	void add_submodel(int _submodel_num, submodel_instance *_submodel_instance_1, float _turn_rate)
	{
		submodel_list.push_back(_submodel_num);
		_submodel_instance_1->current_turn_rate = _turn_rate;
		_submodel_instance_1->desired_turn_rate = _turn_rate;
	}
};

SCP_unordered_map<int, intrinsic_motion> Intrinsic_motions;


void model_free(polymodel* pm)
{
	int i, j;
	safe_kill(pm->ship_bay);

	if (pm->paths) {
		for (i = 0; i < pm->n_paths; i++) {
			for (j = 0; j < pm->paths[i].nverts; j++) {
				if (pm->paths[i].verts[j].turret_ids) {
					vm_free(pm->paths[i].verts[j].turret_ids);
				}
			}
			if (pm->paths[i].verts) {
				vm_free(pm->paths[i].verts);
			}
		}
		vm_free(pm->paths);
	}

	if (pm->shield.verts) {
		vm_free(pm->shield.verts);
	}

	if (pm->shield.tris) {
		vm_free(pm->shield.tris);
	}

	if (pm->gun_banks) {	// NOLINT
		delete[] pm->gun_banks;
	}

	if (pm->missile_banks) {	// NOLINT
		delete[] pm->missile_banks;
	}

	if (pm->docking_bays) {
		for (i = 0; i < pm->n_docks; i++) {
			if (pm->docking_bays[i].splines) {
				vm_free(pm->docking_bays[i].splines);
			}
		}
		vm_free(pm->docking_bays);
	}


	if (pm->thrusters) {
		for (i = 0; i < pm->n_thrusters; i++) {
			if (pm->thrusters[i].points)
				vm_free(pm->thrusters[i].points);
		}

		vm_free(pm->thrusters);
	}

	if (pm->glow_point_banks) { // free the glows!!! -Bobboau
		for (i = 0; i < pm->n_glow_point_banks; i++) {
			if (pm->glow_point_banks[i].points)
				vm_free(pm->glow_point_banks[i].points);
		}

		vm_free(pm->glow_point_banks);
	}

#ifndef NDEBUG
	if (pm->debug_info) {
		vm_free(pm->debug_info);
	}
#endif

	model_octant_free(pm);

	if (pm->submodel) {
		for (i = 0; i < pm->n_models; i++) {
			pm->submodel[i].buffer.clear();

			if (pm->submodel[i].bsp_data) {
				vm_free(pm->submodel[i].bsp_data);
			}

			if (pm->submodel[i].collision_tree_index >= 0) {
				model_remove_bsp_collision_tree(pm->submodel[i].collision_tree_index);
			}

			if (pm->submodel[i].outline_buffer != nullptr) {
				vm_free(pm->submodel[i].outline_buffer);
				pm->submodel[i].outline_buffer = nullptr;
			}
		}

		delete[] pm->submodel;
	}

	if (pm->xc) {
		vm_free(pm->xc);
	}

	if (pm->lights) {
		vm_free(pm->lights);
	}

	if (pm->shield_collision_tree) {
		vm_free(pm->shield_collision_tree);
	}

	if (pm->shield.buffer_id.isValid()) {
		gr_delete_buffer(pm->shield.buffer_id);
		pm->shield.buffer_id = gr_buffer_handle::invalid();
		pm->shield.buffer_n_verts = 0;
	}

	if (pm->vert_source.Vbuffer_handle.isValid()) {
		gr_heap_deallocate(GpuHeap::ModelVertex, pm->vert_source.Vertex_offset);
		pm->vert_source.Vbuffer_handle = gr_buffer_handle::invalid();

		pm->vert_source.Vertex_offset = 0;
		pm->vert_source.Base_vertex_offset = 0;
	}

	if (pm->vert_source.Vertex_list != NULL) {
		vm_free(pm->vert_source.Vertex_list);
		pm->vert_source.Vertex_list = NULL;
	}

	if (pm->vert_source.Ibuffer_handle.isValid()) {
		gr_heap_deallocate(GpuHeap::ModelIndex, pm->vert_source.Index_offset);

		pm->vert_source.Ibuffer_handle = gr_buffer_handle::invalid();
		pm->vert_source.Index_offset = 0;
	}

	if (pm->vert_source.Index_list != NULL) {
		vm_free(pm->vert_source.Index_list);
		pm->vert_source.Index_list = NULL;
	}

	pm->vert_source.Vertex_list_size = 0;
	pm->vert_source.Index_list_size = 0;

	for (i = 0; i < MAX_MODEL_DETAIL_LEVELS; ++i) {
		pm->detail_buffers[i].clear();
	}

	pm->id = 0;
	delete pm;
}

// Free up a model, getting rid of all its memory
// With the basic page in system this can be called from outside of modelread.cpp
void model_unload(int modelnum, int force)
{
	int num;

	if ( modelnum >= MAX_POLYGON_MODELS ) {
		num = modelnum % MAX_POLYGON_MODELS;
	} else {
		num = modelnum;
	}

	if ( (num < 0) || (num >= MAX_POLYGON_MODELS))	{
		return;
	}

	polymodel *pm = Polygon_models[num];

	if ( !pm )	{
		return;
	}

	Assert( pm->used_this_mission >= 0 );

	if (!force && (--pm->used_this_mission > 0))
		return;

	mprintf(("Unloading model '%s' from slot '%i'\n", pm->filename, num));

	// so that the textures can be released
	pm->used_this_mission = 0;

	// we want to call bm_release() from here rather than just bm_unload() in order
	// to get the slots back so we set "release" to true.
	model_page_out_textures(pm->id, true);

	// run through Ship_info and if the model has been loaded we'll need to reset the modelnum to -1.
	for (auto& si : Ship_info) {
		if (pm->id == si.model_num) {
			si.model_num = -1;
		}

		if (pm->id == si.cockpit_model_num) {
			si.cockpit_model_num = -1;
		}

		if (pm->id == si.model_num_hud) {
			si.model_num_hud = -1;
		}
	}

	// need to reset weapon models as well
	for (auto& wi : Weapon_info) {
		if (pm->id == wi.model_num) {
			wi.model_num = -1;
		}
		if (pm->id == wi.external_model_num) {
			wi.external_model_num = -1;
		}
	}

	model_free(pm);

	Polygon_models[num] = NULL;	
}

void model_free_all()
{
	int i;

	if ( !model_initted)	{
		model_init();
		return;
	}

	mprintf(( "Freeing all existing models...\n" ));
	model_instance_free_all();

	for (i=0;i<MAX_POLYGON_MODELS;i++) {
		// forcefully unload all loaded models (be careful with this)
		model_unload(i, 1);		
	}
}

void model_instance_free_all()
{
	size_t i;

	// free any outstanding model instances
	for ( i = 0; i < Polygon_model_instances.size(); ++i ) {
		if ( Polygon_model_instances[i] ) {
			model_delete_instance((int)i);
		}
	}

	// clear skybox model instance if we have one; it is not an object and therefore has no <object>_delete function which would remove the instance
	Nmodel_instance_num = -1;

	Polygon_model_instances.clear();
}

void model_page_in_start()
{
	int i;

	if ( !model_initted ) {
		model_init();
		return;
	}

	mprintf(( "Starting model page in...\n" ));

	for (i=0; i<MAX_POLYGON_MODELS; i++) {
		if (Polygon_models[i] != NULL)
			Polygon_models[i]->used_this_mission = 0;
	}
}

void model_page_in_stop()
{
	int i;

	Assert( model_initted );

	mprintf(( "Stopping model page in...\n" ));

	for (i=0; i<MAX_POLYGON_MODELS; i++) {
		if (Polygon_models[i] == NULL)
			continue;

		if (Polygon_models[i]->used_this_mission)
			continue;
	
		model_unload(i);
	}
}

void model_init()
{
	int i;

	if ( model_initted )		{
		Int3();		// Model_init shouldn't be called twice!
		return;
	}

	for (i=0;i<MAX_POLYGON_MODELS;i++) {
		Polygon_models[i] = NULL;
	}

	model_initted = 1;
}

// routine to parse out values from a user property field of an object
void get_user_prop_value(char *buf, char *value)
{
	char *p, *p1, c;

	p = buf;
	while ( isspace(*p) || (*p == '=') )		// skip white space and equal sign
		p++;
	p1 = p;
	while ( !iscntrl(*p1) )
		p1++;
	c = *p1;
	*p1 = '\0';
	strcpy(value, p);
	*p1 = c;
}

// routine to parse out a vec3d from a user property field of an object
bool get_user_vec3d_value(char *buf, vec3d *value, bool require_brackets, const char *submodel_name, const char *filename)
{
	float f1, f2, f3;
	char closing_bracket = '\0';
	bool success = false;

	pause_parse();
	Mp = buf;
	snprintf(Current_filename, sizeof(Current_filename), "submodel %s on %s", submodel_name, filename);

	// Check if there's a missing line break before the next "$".
	char end_separator = '\0';
	char* end_pos = buf;
	while (!iscntrl(*end_pos) && *end_pos != '$')
		end_pos++;

	// We found a $ before the next line break, remember it and replace it with a line break.
	if (*end_pos == '$') {
		end_separator = *end_pos;
		*end_pos = '\n';
	}

	// Note that we can't simply return from within this block
	// because we always need to call unpause_parse before we
	// leave the function.  A one-iteration loop with break
	// statements allows the code to jump to the end.  Alternatively,
	// goto could have been used.
	do {
		// skip white space and equal sign or colon
		while (isspace(*Mp) || (*Mp == '=') || (*Mp == ':'))
			Mp++;

		if (require_brackets)
		{
			// look for vector bracket
			if (*Mp == '{')
				closing_bracket = '}';
			else if (*Mp == '[')
				closing_bracket = ']';
			else
				break;
		}

		// get comma-separated floats
		if (stuff_float(&f1, true) != 2)
			break;
		if (stuff_float(&f2, true) != 2)
			break;
		if (stuff_float(&f3, true) != 2)
			break;

		if (require_brackets)
		{
			ignore_white_space();

			if (*Mp != closing_bracket)
				break;
		}

		value->xyz = { f1, f2, f3 };
		success = true;
	} while (false);

	if (end_separator != '\0') {
		// Revert the character replacement we did at the start
		*end_pos = end_separator;
	}

	unpause_parse();
	return success;
}

// routine to look for one of the specified user properties
// if p is not null, sets p to the next character AFTER the string and a space/equals/colon (not the beginning of the string, as strstr would)
// returns the index of the property found, or -1 if not found
// NB: the first recognized option is returned, so if one option is a substring of another, put it later in the list!
int prop_string(char *props, char **p, int n_args, ...)
{
	char *pos = nullptr;
	va_list args;
	int index = -1;

	va_start(args, n_args);

	for (int i = 0; i < n_args; ++i)
	{
		const char *option = va_arg(args, const char *);

		// look for our option in the props fields
		if ((pos = strstr(props, option)) != nullptr)
		{
			// we found it
			index = i;

			// so advance past the string and its following character
			pos += strlen(option);
			pos++;

			break;
		}
	}

	va_end(args);

	// if we have a p, assign *p
	// (if nothing was found, *p will be nullptr)
	if (p != nullptr)
		*p = pos;

	return index;
}

// syntactic sugar
int prop_string(char *props, char **p, const char *option0)
{
	return prop_string(props, p, 1, option0);
}
int prop_string(char *props, char **p, const char *option0, const char *option1)
{
	return prop_string(props, p, 2, option0, option1);
}
int prop_string(char *props, char **p, const char *option0, const char *option1, const char *option2)
{
	return prop_string(props, p, 3, option0, option1, option2);
}

bool in(const char *str, const char *substr)
{
	return stristr(str, substr) != nullptr;
}

bool in(char *&p, char *str, const char *substr)
{
	p = stristr(str, substr);
	return p != nullptr;
}

const Model::Subsystem_Flags carry_flags[] = { Model::Subsystem_Flags::Crewpoint, Model::Subsystem_Flags::Rotates, Model::Subsystem_Flags::Translates, Model::Subsystem_Flags::Triggered, Model::Subsystem_Flags::Artillery, Model::Subsystem_Flags::Stepped_rotate, Model::Subsystem_Flags::Stepped_translate };
// Function to copy model data from one subsystem set to another subsystem set.  This function
// is called when two ships use the same model data, but since the model only gets read in one time,
// the subsystem data is only present in one location.  The ship code will call this routine to fix
// this situation by copying stuff from the source subsystem set to the dest subsystem set.
void model_copy_subsystems( int n_subsystems, model_subsystem *d_sp, model_subsystem *s_sp )
{
	int i, j;
	model_subsystem *source, *dest;

	for (i = 0; i < n_subsystems; i++ ) {
		source = &s_sp[i];
		for ( j = 0; j < n_subsystems; j++ ) {
			dest = &d_sp[j];
			if ( !subsystem_stricmp( source->subobj_name, dest->subobj_name) ) {
				for (auto const &flag : carry_flags) {
					if (source->flags[flag])
						dest->flags.set(flag);
				}

				dest->subobj_num = source->subobj_num;
				dest->model_num = source->model_num;
				dest->pnt = source->pnt;
				dest->radius = source->radius;
				dest->type = source->type;
				dest->turret_gun_sobj = source->turret_gun_sobj;

                strcpy_s(dest->name, source->name);

				if ( dest->type == SUBSYSTEM_TURRET ) {
					int nfp;

					dest->turret_fov = source->turret_fov;
					dest->turret_num_firing_points = source->turret_num_firing_points;
					dest->turret_norm = source->turret_norm;

					for (nfp = 0; nfp < dest->turret_num_firing_points; nfp++ )
						dest->turret_firing_point[nfp] = source->turret_firing_point[nfp];

					if ( dest->flags[Model::Subsystem_Flags::Crewpoint] )
						strcpy_s(dest->crewspot, source->crewspot);
				}
				break;
			}
		}
		if ( j == n_subsystems )
			Int3();							// get allender -- something is amiss with models

	}
}

// routine to get/set subsystem information
void set_subsystem_info(int model_num, model_subsystem *subsystemp, char *props, const char *dname)
{
	char *p;
	char buf[64];
	char	lcdname[256];
	int		idx;

	if (in(p, props, "$name"))
		get_user_prop_value(p+5, subsystemp->name);
	else
		strcpy_s(subsystemp->name, dname);

	strcpy_s(lcdname, dname);
	strlwr(lcdname);

	auto modelp = model_get(model_num);
	bsp_info* submodelp = nullptr;
	if (subsystemp->subobj_num >= 0) {
		submodelp = &modelp->submodel[subsystemp->subobj_num];
	}

	// check the name for its specific type
	if ( strstr(lcdname, "engine") ) {
		subsystemp->type = SUBSYSTEM_ENGINE;
	} else if ( strstr(lcdname, "radar") ) {
		subsystemp->type = SUBSYSTEM_RADAR;
	} else if ( strstr(lcdname, "turret") ) {
		float angle;

		subsystemp->type = SUBSYSTEM_TURRET;
		if (in(p, props, "$fov"))
			get_user_prop_value(p+4, buf);			// get the value of the fov
		else
			strcpy_s(buf,"180");
		angle = fl_radians(atoi(buf))/2.0f;

		// don't set the turret FOV if it has already been set (e.g. through ships.tbl)
		if (!subsystemp->flags[Model::Subsystem_Flags::Turret_barrel_override_fov])
			subsystemp->turret_fov = cosf(angle);

		subsystemp->turret_num_firing_points = 0;

		if (in(p, props, "$crewspot")) {
			subsystemp->flags.set(Model::Subsystem_Flags::Crewpoint);
			get_user_prop_value(p+9, subsystemp->crewspot);
		}

	} else if ( strstr(lcdname, "navigation") ) {
		subsystemp->type = SUBSYSTEM_NAVIGATION;
	} else if ( strstr(lcdname, "communication") )  {
		subsystemp->type = SUBSYSTEM_COMMUNICATION;
	} else if ( strstr(lcdname, "weapon") ) {
		subsystemp->type = SUBSYSTEM_WEAPONS;
	} else if ( strstr(lcdname, "sensor") ) {
		subsystemp->type = SUBSYSTEM_SENSORS;
	} else if ( strstr(lcdname, "solar") ) {
		subsystemp->type = SUBSYSTEM_SOLAR;
	} else if ( strstr(lcdname, "gas") ) {
		subsystemp->type = SUBSYSTEM_GAS_COLLECT;
	} else if ( strstr(lcdname, "activator") ) {
		subsystemp->type = SUBSYSTEM_ACTIVATION;
	}  else { // If unrecognized type, set to unknown so artist can continue working...
		subsystemp->type = SUBSYSTEM_UNKNOWN;
		mprintf(("Subsystem '%s' on ship %s is not recognized as a common subsystem type\n", dname, modelp->filename));
	}

	if (in(props, "$triggered")) {
		subsystemp->flags.set(Model::Subsystem_Flags::Rotates);
		subsystemp->flags.set(Model::Subsystem_Flags::Translates);
		subsystemp->flags.set(Model::Subsystem_Flags::Triggered);
	}

	// Dumb-Rotating subsystem
	if (prop_string(props, nullptr, "$dumb_rotate") >= 0) {
		// no special subsystem handling needed here, but make sure we didn't specify both methods
		if (prop_string(props, nullptr, "$rotate") >= 0) {
			Warning(LOCATION, "Subsystem '%s' on ship %s cannot have both rotation and dumb-rotation!", dname, modelp->filename);
		}
	}
	// Look-At subsystem
	else if (in(p, props, "$look_at")) {
		// no special subsystem handling needed here, but make sure we didn't specify both methods
		if (prop_string(props, nullptr, "$rotate") >= 0) {
			Warning(LOCATION, "Subsystem '%s' on ship %s cannot have both rotation and look-at!", dname, modelp->filename);
		}
	}
	// Rotating subsystem
	else if ((idx = prop_string(props, &p, "$rotate_time", "$rotate_rate", "$rotate")) >= 0) {
        subsystemp->flags.set(Model::Subsystem_Flags::Rotates);

		// get value for (a) complete rotation (b) step (c) activation
		get_user_prop_value(p, buf);	// note: p points to the value since we used prop_string

		// for retail compatibility, $rotate means $rotate_time
		float turn_rate;
		if (idx == 0 || idx == 2) {
			float turn_time = static_cast<float>(atof(buf));
			if (fl_near_zero(turn_time, 0.01f)) {
				Warning(LOCATION, "Rotation has a turn time of 0 for subsystem '%s' on ship %s!", dname, modelp->filename);
				turn_rate = 1.0f;
			} else {
				turn_rate = PI2 / turn_time;
			}
		} else {
			turn_rate = static_cast<float>(atof(buf));
		}

		// CASE OF WEAPON ROTATION (primary only)
		if (in(p, props, "$pbank")) {
			subsystemp->flags.set(Model::Subsystem_Flags::Artillery);

			// get which pbank should trigger rotation
			get_user_prop_value(p+6, buf);
			subsystemp->weapon_rotation_pbank = atoi(buf);
		} // end of weapon rotation stuff

		
		// *** determine how the subsys rotates ***

		// CASE OF STEPPED ROTATION
		if (in(props, "$stepped")) {

			subsystemp->stepped_rotation.reset(new stepped_rotation);
            subsystemp->flags.set(Model::Subsystem_Flags::Stepped_rotate);

			// get number of steps
			if (in(p, props, "$steps")) {
				get_user_prop_value(p+6, buf);
				int num_steps = atoi(buf);
				if (num_steps <= 0) {
					Warning(LOCATION, "In model %s, subsystem %s, $steps must be greater than 0!", modelp->filename, submodelp->name);
					num_steps = 8;
				}
				subsystemp->stepped_rotation->num_steps = num_steps;
			} else {
				subsystemp->stepped_rotation->num_steps = 8;
			}

			// get pause time
			if (in(p, props, "$t_paused")) {
				get_user_prop_value(p+9, buf);
				float t_pause = (float)atof(buf);
				if (t_pause < 0.0f) {
					Warning(LOCATION, "In model %s, subsystem %s, $t_paused must not be negative!", modelp->filename, submodelp->name);
					t_pause = 2.0f;
				}
				subsystemp->stepped_rotation->t_pause = t_pause;
			} else {
				subsystemp->stepped_rotation->t_pause = 2.0f;
			}

			// get transition time - time to make a complete movement
			if (in(p, props, "$t_transit")) {
				get_user_prop_value(p+10, buf);
				float t_transit = (float)atof(buf);
				if (t_transit < 0.0f) {
					Warning(LOCATION, "In model %s, subsystem %s, $t_transit must not be negative!", modelp->filename, submodelp->name);
					t_transit = 2.0f;
				}
				subsystemp->stepped_rotation->t_transit = t_transit;
			} else {
				subsystemp->stepped_rotation->t_transit = 2.0f;
			}

			// get fraction of time spent in accel
			if (in(p, props, "$fraction_accel")) {
				get_user_prop_value(p+15, buf);
				float fraction = (float)atof(buf);
				if (fraction < 0.0f || fraction > 0.5f) {
					Warning(LOCATION, "In model %s, subsystem %s, $fraction_accel must not be negative and must be less than or equal to 0.5!", modelp->filename, submodelp->name);
					fraction = 0.3f;
				}
				subsystemp->stepped_rotation->fraction = fraction;
			} else {
				subsystemp->stepped_rotation->fraction = 0.3f;
			}

			float step_distance = PI2 / subsystemp->stepped_rotation->num_steps;
			float t_trans = subsystemp->stepped_rotation->t_transit;
			float fraction = subsystemp->stepped_rotation->fraction;

			// reverse the direction if we start out with reverse velocity
			if (turn_rate < 0.0f) {
				subsystemp->stepped_rotation->backwards = true;
			}

			subsystemp->stepped_rotation->max_turn_accel = fl_near_zero(fraction) ? 0.0f : step_distance / (fraction * (1.0f - fraction) * t_trans * t_trans);
			subsystemp->stepped_rotation->max_turn_rate = step_distance / ((1.0f - fraction) * t_trans);
		}

		// CASE OF NORMAL CONTINUOUS ROTATION
		else {
			if (submodelp) {
				submodelp->default_turn_rate = turn_rate;
			}
		}

		float turn_accel = 0.5f;
		if (in(p, props, "$rotate_accel")) {
			get_user_prop_value(p + 13, buf);

			if (!stricmp(buf, "instant")) {
				if (submodelp) {
					submodelp->flags.set(Model::Submodel_flags::Instant_rotate_accel);
				}
				turn_accel = 0.0f;
			} else {
				turn_accel = static_cast<float>(atof(buf));
				if (turn_accel < 0.0f) {
					Warning(LOCATION, "Model %s, submodel %s, $rotate_accel %f cannot be negative!", modelp->filename, dname, turn_accel);
					turn_accel *= -1;
				}
			}
		}
		if (submodelp) {
			submodelp->default_turn_accel = turn_accel;
		}
	}
	// Translating subsystem
	else if ((idx = prop_string(props, &p, "$translate_rate", "$translate")) >= 0) {
        subsystemp->flags.set(Model::Subsystem_Flags::Translates);

		// get value for continuous or stepped translation
		get_user_prop_value(p, buf);	// note: p points to the value since we used prop_string

		// $translate means $translate_rate; there is no $translate_time
		float shift_rate = static_cast<float>(atof(buf));

		// *** determine how the subsys translates ***

		// CASE OF STEPPED TRANSLATION
		if (in(props, "$stepped")) {

			subsystemp->stepped_translation.reset(new stepped_translation);
            subsystemp->flags.set(Model::Subsystem_Flags::Stepped_translate);

			// get whether to reverse after the step
			// (always reverse unless the props say explicitly not to)
			if (in(p, props, "$reverse_after_step")) {
				get_user_prop_value(p+19, buf);
				if (stricmp(buf, "false")) {
					subsystemp->stepped_translation->reverse_after_step = true;
				}
			} else {
				subsystemp->stepped_translation->reverse_after_step = true;
			}

			// get step distance
			if (in(p, props, "$step_distance")) {
				get_user_prop_value(p+14, buf);
				float step_dist = (float)atof(buf);
				if (step_dist < 0.0f) {
					Warning(LOCATION, "In model %s, subsystem %s, $step_distance must not be negative!", modelp->filename, submodelp->name);
					step_dist = 25.0f;
				}
				subsystemp->stepped_translation->step_distance = step_dist;
			} else {
				subsystemp->stepped_translation->step_distance = 25.0f;
			}

			// get pause time
			if (in(p, props, "$t_paused")) {
				get_user_prop_value(p+9, buf);
				float t_pause = (float)atof(buf);
				if (t_pause < 0.0f) {
					Warning(LOCATION, "In model %s, subsystem %s, $t_paused must not be negative!", modelp->filename, submodelp->name);
					t_pause = 2.0f;
				}
				subsystemp->stepped_translation->t_pause = t_pause;
			} else {
				subsystemp->stepped_translation->t_pause = 2.0f;
			}

			// get transition time - time to make a complete movement
			subsystemp->stepped_translation->t_transit = fl_abs(subsystemp->stepped_translation->step_distance / shift_rate);

			// get fraction of time spent in accel
			if (in(p, props, "$fraction_accel")) {
				get_user_prop_value(p+15, buf);
				float fraction = (float)atof(buf);
				if (fraction < 0.0f || fraction > 0.5f) {
					Warning(LOCATION, "In model %s, subsystem %s, $fraction_accel must not be negative and must be less than or equal to 0.5!", modelp->filename, submodelp->name);
					fraction = 0.3f;
				}
				subsystemp->stepped_translation->fraction = fraction;
			} else {
				subsystemp->stepped_translation->fraction = 0.3f;
			}

			float step_distance = subsystemp->stepped_translation->step_distance;
			float t_trans = subsystemp->stepped_translation->t_transit;
			float fraction = subsystemp->stepped_translation->fraction;

			// reverse the direction if we start out with reverse velocity
			if (shift_rate < 0.0f) {
				subsystemp->stepped_translation->backwards = true;
			}

			subsystemp->stepped_translation->max_shift_accel = fl_near_zero(fraction) ? 0.0f : step_distance / (fraction * (1.0f - fraction) * t_trans * t_trans);
			subsystemp->stepped_translation->max_shift_rate = step_distance / ((1.0f - fraction) * t_trans);
		}

		// CASE OF NORMAL CONTINUOUS TRANSLATION
		else {
			if (submodelp) {
				submodelp->default_shift_rate = shift_rate;
			}
		}

		float shift_accel = 0.5f;
		if (in(p, props, "$translate_accel")) {
			get_user_prop_value(p + 16, buf);

			if (!stricmp(buf, "instant")) {
				if (submodelp) {
					submodelp->flags.set(Model::Submodel_flags::Instant_translate_accel);
				}
				shift_accel = 0.0f;
			} else {
				shift_accel = static_cast<float>(atof(buf));
				if (shift_accel < 0.0f) {
					Warning(LOCATION, "Model %s, submodel %s, $translate_accel %f cannot be negative!", modelp->filename, dname, shift_accel);
					shift_accel *= -1;
				}
			}
		}
		if (submodelp) {
			submodelp->default_shift_accel = shift_accel;
		}
	}
}

// used in collision code to check if submodel rotates too far
float get_submodel_delta_angle(const submodel_instance *smi)
{
	// find the angle
	float delta_angle = smi->cur_angle - smi->prev_angle;

	// make sure we get the short way around
	if (delta_angle > PI) {
		delta_angle = (PI2 - delta_angle);
	}

	return delta_angle;
}

float get_submodel_delta_shift(const submodel_instance *smi)
{
	// this is a bit simpler
	return abs(smi->cur_offset - smi->prev_offset);
}

void do_new_subsystem( int n_subsystems, model_subsystem *slist, int subobj_num, float rad, const vec3d *pnt, char *props, const char *subobj_name, int model_num )
{
	int i;
	model_subsystem *subsystemp;

	if ( slist==NULL ) {
#ifndef NDEBUG
		if (!ss_warning_shown_null) {
			mprintf(("No subsystems found for model \"%s\".\n", model_get(model_num)->filename));
			ss_warning_shown_null = true;
		}
#endif
		return;			// For TestCode, POFView, etc don't bother
	}
	
	// try to find the name of the subsystem passed here on the list of subsystems currently on the
	// ship.  Assign the values only when the right subsystem is found

	for (i = 0; i < n_subsystems; i++ ) {
		subsystemp = &slist[i];

#ifndef NDEBUG
		// Goober5000 - notify if there's a mismatch
		if ( stricmp(subobj_name, subsystemp->subobj_name) != 0 && !subsystem_stricmp(subobj_name, subsystemp->subobj_name) )
		{
			nprintf(("Model", "NOTE: Subsystem \"%s\" in model \"%s\" is represented as \"%s\" in ships.tbl.  This works fine in FSO v3.6 and up, "
				"but is not compatible with FS2 retail.\n", subobj_name, model_get(model_num)->filename, subsystemp->subobj_name));

		}
#endif

		if (!subsystem_stricmp(subobj_name, subsystemp->subobj_name))
		{
			if (subobj_num >= 0)
				model_get(model_num)->submodel[subobj_num].subsys_num = i;

			subsystemp->subobj_num = subobj_num;
			subsystemp->turret_gun_sobj = -1;
			subsystemp->model_num = model_num;
			subsystemp->pnt = *pnt;				// use the offset to get the center point of the subsystem
			subsystemp->radius = rad;

			set_subsystem_info(model_num, subsystemp, props, subobj_name);
			strcpy_s(subsystemp->subobj_name, subobj_name);						// copy the object name
			return;
		}
	}
#ifndef NDEBUG
	char bname[_MAX_FNAME];

	if ( !ss_warning_shown_mismatch) {
		_splitpath(model_filename, NULL, NULL, bname, NULL);
		// Lets still give a comment about it and not just erase it
		Warning(LOCATION,"Not all subsystems in model \"%s\" have a record in ships.tbl.\nThis can cause game to crash.\n\nList of subsystems not found from table is in log file.\n", model_get(model_num)->filename );
		mprintf(("Subsystem %s in model %s was not found in ships.tbl!\n", subobj_name, model_get(model_num)->filename));
		ss_warning_shown_mismatch = true;
	} else
#endif
		mprintf(("Subsystem %s in model %s was not found in ships.tbl!\n", subobj_name, model_get(model_num)->filename));

#ifndef NDEBUG
	if ( ss_fp )	{
		_splitpath(model_filename, NULL, NULL, bname, NULL);
		mprintf(("A subsystem was found in model %s that does not have a record in ships.tbl.\nA list of subsystems for this ship will be dumped to:\n\ndata%stables%s%s.subsystems for inclusion\ninto ships.tbl.\n", model_filename, DIR_SEPARATOR_STR, DIR_SEPARATOR_STR, bname));
		char tmp_buffer[128];
		sprintf(tmp_buffer, "$Subsystem:\t\t\t%s,1,0.0\n", subobj_name);
		cfputs(tmp_buffer, ss_fp);
	}
#endif

}

void print_family_tree(polymodel *obj)
{
	mprintf(("PRINTING POLYMODEL TREE\n"));
	mprintf(("%s\n", obj->filename));

	model_iterate_submodel_tree(obj, obj->detail[0], [&](int submodel, int level, bool /*isLeaf*/)
		{
			mprintf(("  "));
			for (int i = 0; i < level; i++)
				mprintf(("  "));

			mprintf(("%s\n", obj->submodel[submodel].name));
		});
}

void create_family_tree(polymodel *obj)
{
	int i;
	for (i=0; i<obj->n_models; i++ )	{
		obj->submodel[i].num_children = 0;
		obj->submodel[i].first_child = -1;
		obj->submodel[i].next_sibling = -1;
	}

	for (i=0; i<obj->n_models; i++ )	{
		int pn;
		pn = obj->submodel[i].parent;
		if ( pn > -1 )	{
			obj->submodel[pn].num_children++;
			int tmp = obj->submodel[pn].first_child;
			obj->submodel[pn].first_child = i;
			obj->submodel[i].next_sibling = tmp;
		}
	}
}

void create_vertex_buffer(polymodel *pm, const model_read_deferred_tasks& deferredTasks)
{
	if (Is_standalone) {
		return;
	}

	TRACE_SCOPE(tracing::ModelCreateVertexBuffers);

	int i;

	// determine the size and configuration of each buffer segment
	for (i = 0; i < pm->n_models; i++) {
		interp_configure_vertex_buffers(pm, i, deferredTasks);
	}

	// figure out which vertices are transparent
	for ( i = 0; i < pm->n_models; i++ ) {
		if ( !pm->submodel[i].flags[Model::Submodel_flags::Is_thruster] ) {
			interp_create_transparency_index_buffer(pm, i);
		}
	}
	clear_bm_lookup_cache();

	size_t stride = 0;
	// Determine the global stride of this model (should be the same for every submodel)
	for ( i = 0; i < pm->n_models; ++i ) {
		if (pm->submodel[i].buffer.model_list != nullptr && pm->submodel[i].buffer.stride != stride) {
			Assertion(stride == 0, "Submodel %d of model %s has a stride of "
				SIZE_T_ARG
				" while the rest of the model has a vertex stride of "
				SIZE_T_ARG
				"!", i, pm->filename, pm->submodel[i].buffer.stride, stride);

			stride = pm->submodel[i].buffer.stride;
		}
	}

	// create another set of indexes for the detail buffers
	for ( i = 0; i < pm->n_detail_levels; i++ )	{
		interp_create_detail_index_buffer(pm, i);
	}

	// now actually fill the buffer with our info ...
	for (i = 0; i < pm->n_models; i++) {
		interp_pack_vertex_buffers(pm, i);

		// release temporary memory
		pm->submodel[i].buffer.release();
		pm->submodel[i].trans_buffer.release();
	}

	// pack the merged index buffers to the vbo.
	for ( i = 0; i < pm->n_detail_levels; ++i ) {
		if ( pm->detail_buffers[i].model_list == NULL ) {
			continue;
		}

		model_interp_pack_buffer(&pm->vert_source, &pm->detail_buffers[i]);
		pm->detail_buffers[i].release();
	}

	pm->flags |= PM_FLAG_BATCHED;

	// ... and then finalize buffer
	model_interp_submit_buffers(&pm->vert_source, stride);

	model_interp_process_shield_mesh(pm);
}

// Goober5000
bool maybe_swap_mins_maxs(vec3d *mins, vec3d *maxs)
{
	float temp;
	bool swap_was_necessary = false;

	if (mins->xyz.x > maxs->xyz.x)
	{
		temp = mins->xyz.x;
		mins->xyz.x = maxs->xyz.x;
		maxs->xyz.x = temp;
		swap_was_necessary = true;
	}
	if (mins->xyz.y > maxs->xyz.y)
	{
		temp = mins->xyz.y;
		mins->xyz.y = maxs->xyz.y;
		maxs->xyz.y = temp;
		swap_was_necessary = true;
	}
	if (mins->xyz.z > maxs->xyz.z)
	{
		temp = mins->xyz.z;
		mins->xyz.z = maxs->xyz.z;
		maxs->xyz.z = temp;
		swap_was_necessary = true;
	}

// This is a mini utility that prints out the proper hex string for the
// mins and maxs so that the POF file can be modified in a hex editor.
// Currently none of the major POF editors allow editing of bounding boxes.
#if 0
	if (swap_was_necessary)
	{
		// use C hackery to convert float values to raw bytes
		const int NUM_BYTES = 24;
		typedef struct converter
		{
			union
			{
				struct
				{
					float min_x, min_y, min_z, max_x, max_y, max_z;
				} _float;
				ubyte _byte[NUM_BYTES];
			};
		} converter;

		// fill in the values
		converter z;
		z._float.min_x = mins->xyz.x;
		z._float.min_y = mins->xyz.y;
		z._float.min_z = mins->xyz.z;
		z._float.max_x = maxs->xyz.x;
		z._float.max_y = maxs->xyz.y;
		z._float.max_z = maxs->xyz.z;

		// prep string
		char hex_str[5];
		char text[100 + (5 * NUM_BYTES)];
		strcpy_s(text, "The following is the correct hex string for the minima and maxima:\n");

		// append hex values to the string
		for (int i = 0; i < NUM_BYTES; i++)
		{
			sprintf(hex_str, "%02X ", z._byte[i]);
			strcat_s(text, hex_str);
		}

		// notify the user
		Warning(LOCATION, text);
	}
#endif

	return swap_was_necessary;
}

void model_calc_bound_box( vec3d *box, vec3d *big_mn, vec3d *big_mx)
{
	box[0].xyz.x = big_mn->xyz.x; box[0].xyz.y = big_mn->xyz.y; box[0].xyz.z = big_mn->xyz.z;
	box[1].xyz.x = big_mx->xyz.x; box[1].xyz.y = big_mn->xyz.y; box[1].xyz.z = big_mn->xyz.z;
	box[2].xyz.x = big_mx->xyz.x; box[2].xyz.y = big_mx->xyz.y; box[2].xyz.z = big_mn->xyz.z;
	box[3].xyz.x = big_mn->xyz.x; box[3].xyz.y = big_mx->xyz.y; box[3].xyz.z = big_mn->xyz.z;


	box[4].xyz.x = big_mn->xyz.x; box[4].xyz.y = big_mn->xyz.y; box[4].xyz.z = big_mx->xyz.z;
	box[5].xyz.x = big_mx->xyz.x; box[5].xyz.y = big_mn->xyz.y; box[5].xyz.z = big_mx->xyz.z;
	box[6].xyz.x = big_mx->xyz.x; box[6].xyz.y = big_mx->xyz.y; box[6].xyz.z = big_mx->xyz.z;
	box[7].xyz.x = big_mn->xyz.x; box[7].xyz.y = big_mx->xyz.y; box[7].xyz.z = big_mx->xyz.z;
}

void extract_movement_info(bsp_info *sm, bool is_rotation, int *&movement_axis_id, vec3d *&movement_axis, int *&movement_type)
{
	if (is_rotation)
	{
		movement_axis_id = &sm->rotation_axis_id;
		movement_axis = &sm->rotation_axis;
		movement_type = &sm->rotation_type;
	}
	else
	{
		movement_axis_id = &sm->translation_axis_id;
		movement_axis = &sm->translation_axis;
		movement_type = &sm->translation_type;
	}
}

void determine_submodel_movement(bool is_rotation, const char *filename, bsp_info *sm, char *props, SCP_vector<SCP_string> &look_at_submodel_names)
{
	int *movement_axis_id, *movement_type;
	vec3d *movement_axis;
	char *p;

	extract_movement_info(sm, is_rotation, movement_axis_id, movement_axis, movement_type);

	// determine movement axis
	// (the axis is a vector from 0,0,0 to the point specified)
	// note: the standard axis point definitions are copied from Volition code originally in model_init_submodel_axis_pt
	if (*movement_axis_id == MOVEMENT_AXIS_X)
		*movement_axis = vmd_x_vector;
	else if (*movement_axis_id == MOVEMENT_AXIS_Y)
		*movement_axis = vmd_y_vector;
	else if (*movement_axis_id == MOVEMENT_AXIS_Z)
		*movement_axis = vmd_z_vector;
	else if (*movement_axis_id == MOVEMENT_AXIS_OTHER)
	{
		auto axis_string = is_rotation ? "$rotation_axis" : "$translation_axis";

		if (in(p, props, axis_string))
		{
			if (get_user_vec3d_value(p + 20, movement_axis, true, sm->name, filename))
				vm_vec_normalize(movement_axis);
			else
			{
				Warning(LOCATION, "Failed to parse %s on subsystem '%s' on ship %s!", axis_string, sm->name, filename);
				*movement_type = MOVEMENT_TYPE_NONE;
			}
		}
		else
		{
			Warning(LOCATION, "A %s was not specified for subsystem '%s' on ship %s!", axis_string, sm->name, filename);
			*movement_type = MOVEMENT_TYPE_NONE;
		}
	}

	if (is_rotation)
	{
		// note, this should come BEFORE do_new_subsystem() for proper error handling (to avoid both rotating and look-at submodel)
		if (in(p, props, "$look_at"))
		{
			sm->rotation_type = MOVEMENT_TYPE_INTRINSIC;

			// we need to work out the correct subobject number later, after all subobjects have been processed
			sm->look_at_submodel = static_cast<int>(look_at_submodel_names.size());

			char submodel_name[MAX_NAME_LEN];
			get_user_prop_value(p + 9, submodel_name);
			look_at_submodel_names.push_back(submodel_name);
		}
		else
			sm->look_at_submodel = -1; // No look_at

		// optional extra property for look_at
		if (in(p, props, "$look_at_offset"))
		{
			auto offset = (float)atof(p + 16);

			// model property is specified in degrees, so convert it
			offset = fl_radians(offset);

			// check range (the angle is now in radians)
			if (offset < -PI2 || offset > PI2)
			{
				Warning(LOCATION, "Submodel '%s' of model '%s' has a look_at_offset that is outside the range of -360 to 360!", sm->name, filename);
				offset = -1.0f;
			}
			// make the angle positive, since negative angles will be set at first look_at call
			else if (offset < 0.0f)
				offset += PI2;

			sm->look_at_offset = offset;
		}
		else
			sm->look_at_offset = -1.0f;
	}

	if (is_rotation)
	{
		// note, this should come BEFORE do_new_subsystem() for proper error handling (to avoid both rotating and dumb-rotating submodel)
		int idx = prop_string(props, &p, "$dumb_rotate_time", "$dumb_rotate_rate", "$dumb_rotate");
		if (idx >= 0)
		{
			sm->rotation_type = MOVEMENT_TYPE_INTRINSIC;

			// do this the same way as regular $rotate
			char buf[64];
			get_user_prop_value(p, buf);

			// for past SCP compatibility, $dumb_rotate means $dumb_rotate_rate
			float turn_rate;
			if (idx == 0)
			{
				auto turn_time = static_cast<float>(atof(buf));
				if (fl_near_zero(turn_time, 0.01f))
				{
					Warning(LOCATION, "Dumb-Rotation has a turn time of 0 for subsystem '%s' on ship %s!", sm->name, filename);
					turn_rate = 1.0f;
				}
				else
					turn_rate = PI2 / turn_time;
			}
			else
				turn_rate = static_cast<float>(atof(buf));

			sm->default_turn_rate = turn_rate;
			sm->flags.set(Model::Submodel_flags::Instant_rotate_accel);
		}
	}
}

void maybe_adjust_movement_axis(bool is_rotation, bsp_info *sm)
{
	int *movement_axis_id, *movement_type;
	vec3d *movement_axis;

	extract_movement_info(sm, is_rotation, movement_axis_id, movement_axis, movement_type);

	// if we have a frame of reference, we need to transform the movement axis and make it a non-standard one
	if (!vm_matrix_equal(sm->frame_of_reference, vmd_identity_matrix) && (*movement_type != MOVEMENT_TYPE_NONE) && (*movement_axis_id != MOVEMENT_AXIS_NONE))
	{
		vec3d new_axis;
		vm_vec_unrotate(&new_axis, movement_axis, &sm->frame_of_reference);
		*movement_axis = new_axis;
		*movement_axis_id = MOVEMENT_AXIS_OTHER;
	}
}

void do_movement_sanity_checks(bool is_rotation, bsp_info *sm, bsp_info *parent_sm, const char *filename)
{
	int *movement_axis_id, *movement_type;
	vec3d *movement_axis;

	extract_movement_info(sm, is_rotation, movement_axis_id, movement_axis, movement_type);

	// make sure this is a validly normalized axis
	if (vm_vec_mag(movement_axis) < 0.999f || vm_vec_mag(movement_axis) > 1.001f)
		*movement_type = MOVEMENT_TYPE_NONE;

	// maybe use the FOR to manipulate the axes
	// (do this before the compatibility check below to prevent doing it twice)
	maybe_adjust_movement_axis(is_rotation, sm);

	if (is_rotation)
	{
		// important compatibility check: if there are multipart turrets without rotation axes defined, define them
		// also, some of the retail models got the axes wrong, so fix those :-/
		// what this boils down to is that we must force turret axes for submodels with frame_of_reference defined
		//     and also for turrets which don't have their axes set to "other"
		if (parent_sm && in(parent_sm->name, "turret"))
		{
			auto base = parent_sm;
			auto gun = sm;

			if (!vm_matrix_equal(base->frame_of_reference, vmd_identity_matrix)
				|| (base->rotation_axis_id != MOVEMENT_AXIS_OTHER))
			{
				base->rotation_axis_id = MOVEMENT_AXIS_Y;
				base->rotation_axis = vmd_y_vector;
				base->rotation_type = MOVEMENT_TYPE_TURRET;
				maybe_adjust_movement_axis(true, base);
			}

			if (!vm_matrix_equal(gun->frame_of_reference, vmd_identity_matrix)
				|| (gun->rotation_axis_id != MOVEMENT_AXIS_OTHER))
			{
				gun->rotation_axis_id = MOVEMENT_AXIS_X;
				gun->rotation_axis = vmd_x_vector;
				gun->rotation_type = MOVEMENT_TYPE_TURRET;
				maybe_adjust_movement_axis(true, gun);
			}
		}
	}

	// add a warning if movement is specified without movement axis.
	if (*movement_axis_id == MOVEMENT_AXIS_NONE)
	{
		auto str = is_rotation ? "rotation" : "translation";

		if (*movement_type == MOVEMENT_TYPE_REGULAR)
			Warning(LOCATION, "%s without %s axis defined on submodel '%s' of model '%s'!", str, str, sm->name, filename);
		else if (*movement_type == MOVEMENT_TYPE_INTRINSIC)
			Warning(LOCATION, "Intrinsic %s (e.g. dumb-rotate or look-at) without %s axis defined on submodel '%s' of model '%s'!", str, str, sm->name, filename);

		*movement_type = MOVEMENT_TYPE_NONE;
	}

	// clear the axis if the submodel doesn't move
	// (don't clear can_move because of gun_rotation)
	if (*movement_type == MOVEMENT_TYPE_NONE)
	{
		*movement_axis_id = MOVEMENT_AXIS_NONE;
		*movement_axis = vmd_zero_vector;
	}

	// Set the can_move field on submodels which are of a moving type or which have such a parent somewhere down the hierarchy
	if (*movement_type != MOVEMENT_TYPE_NONE)
		sm->flags.set(Model::Submodel_flags::Can_move);
	else if (parent_sm && parent_sm->flags[Model::Submodel_flags::Can_move])
		sm->flags.set(Model::Submodel_flags::Can_move);
}

void resolve_submodel_index(const polymodel *pm, const char *requester, const char *field, int &submodel_index, const SCP_vector<SCP_string> &submodel_list)
{
	auto submodel_name = submodel_list[submodel_index].c_str();

	// search for this submodel name among all submodels
	for (int j = 0; j < pm->n_models; j++) {
		if (!stricmp(submodel_name, pm->submodel[j].name)) {
			nprintf(("Model", "NOTE: Matched %s %s %s %s with subobject id %d\n", pm->filename, requester, field, submodel_name, j));

			// set the correct submodel reference, and we're done
			submodel_index = j;
			return;
		}
	}

	// models could specify the submodel number, so let's maintain compatibilty
	if (can_construe_as_integer(submodel_name)) {
		submodel_index = atoi(submodel_name);
		return;
	}

	Warning(LOCATION, "Unable to match %s %s %s %s with a submodel!\n", pm->filename, requester, field, submodel_name);
	submodel_index = -1;
}

int read_model_file_no_subsys(polymodel * pm, const char* filename, int ferror, model_read_deferred_tasks& subsystemParseList)
{
	CFILE *fp;
	int version;
	int id, len, next_chunk;
	int i,j;
	vec3d temp_vec;

	fp = cfopen(filename,"rb");

	if (!fp) {
		if (ferror == 1) {
			Error( LOCATION, "Can't open model file <%s>", filename );
		} else if (ferror == 0) {
			Warning( LOCATION, "Can't open model file <%s>", filename );
		}

		return -1;
	}

	TRACE_SCOPE(tracing::ReadModelFile);

	// generate checksum for the POF
	cfseek(fp, 0, SEEK_SET);	
	cf_chksum_long(fp, &Global_checksum);
	cfseek(fp, 0, SEEK_SET);


	// code to get a filename to write out subsystem information for each model that
	// is read.  This info is essentially debug stuff that is used to help get models
	// into the game quicker
#if 0
	{
		char bname[_MAX_FNAME];

		_splitpath(filename, NULL, NULL, bname, NULL);
		sprintf(debug_name, "%s.subsystems", bname);
		ss_fp = cfopen(debug_name, "wb", CFILE_NORMAL, CF_TYPE_TABLES );
		if ( !ss_fp )	{
			mprintf(( "Can't open debug file for writing subsystems for %s\n", filename));
		} else {
			strcpy_s(model_filename, filename);
			ss_warning_shown_null = false;
			ss_warning_shown_mismatch = false;
		}
	}
#endif

	id = cfread_int(fp);

	if (id != POF_HEADER_ID)
		Error( LOCATION, "Bad ID in model file <%s>",filename);

	// Version is major*100+minor
	// So, major = version / 100;
	//     minor = version % 100;
	version = cfread_int(fp);

	//Warning( LOCATION, "POF Version = %d", version );
	
	if (version < PM_COMPATIBLE_VERSION || (version/100) > PM_OBJFILE_MAJOR_VERSION)	{
		Warning(LOCATION,"Bad version (%d) in model file <%s>",version,filename);
		return 0;
	}
	if (version > PM_LATEST_LEGACY_VERSION && version < PM_FIRST_ALIGNED_VERSION) {
		Warning(LOCATION, "Model file %s is version %d, but the latest supported version on this build of FSO is %d.  The model may not work correctly.", filename, version, PM_LATEST_LEGACY_VERSION);
	} else if (version > PM_LATEST_ALIGNED_VERSION && version < PM_FIRST_VERTLIM_VERSION) {
		Warning(LOCATION, "Model file %s is version %d, but the latest supported version on this build of FSO is %d.  The model may not work correctly.", filename, version, PM_LATEST_ALIGNED_VERSION);
	} else if (version > PM_LATEST_VERTLIM_VERSION) {
		Warning(LOCATION, "Model file %s is version %d, but the latest supported version on this build of FSO is %d.  The model may not work correctly.", filename, version, PM_LATEST_VERTLIM_VERSION);
	}

	pm->version = version;
	Assert(strlen(filename) < FILESPEC_LENGTH );
	strcpy_s(pm->filename, filename);

	memset( &pm->view_positions, 0, sizeof(pm->view_positions) );

	// reset insignia counts
	pm->num_ins = 0;

	// reset glow points!! - Goober5000
	pm->n_glow_point_banks = 0;

	// reset SLDC
	pm->shield_collision_tree = NULL;
	pm->sldc_size = 0;

	id = cfread_int(fp);
	len = cfread_int(fp);
	next_chunk = cftell(fp) + len;

	// keep track of any submodels we might notice
	SCP_vector<SCP_string> look_at_submodel_names;
	SCP_vector<SCP_string> dock_parent_submodel_names;

	while (!cfeof(fp)) {

//		mprintf(("Processing chunk <%c%c%c%c>, len = %d\n",id,id>>8,id>>16,id>>24,len));
//		key_getch();

		switch (id) {

			case ID_OHDR: {		//Object header
				//vector v;

				//mprintf(0,"Got chunk OHDR, len=%d\n",len);

#if defined( FREESPACE1_FORMAT )
				pm->n_models = cfread_int(fp);
//				mprintf(( "Num models = %d\n", pm->n_models ));
				pm->rad = cfread_float(fp);
				pm->flags = cfread_int(fp);	// 1=Allow tiling
#elif defined( FREESPACE2_FORMAT )
				pm->rad = cfread_float(fp);
				pm->flags = cfread_int(fp);	// 1=Allow tiling
				pm->n_models = cfread_int(fp);
//				mprintf(( "Num models = %d\n", pm->n_models ));
#endif
                Assertion(pm->n_models >= 1, "Models without any submodels are not supported!");

				// Check for unrealistic radii
				if ( pm->rad <= 0.1f )
				{
					Warning(LOCATION, "Model <%s> has a radius <= 0.1f\n", filename);
				}

				pm->submodel = new bsp_info[pm->n_models];

				//Assert(pm->n_models <= MAX_SUBMODELS);

				cfread_vector(&pm->mins,fp);
				cfread_vector(&pm->maxs,fp);

				// sanity first!
				if (maybe_swap_mins_maxs(&pm->mins, &pm->maxs)) {
					Warning(LOCATION, "Inverted bounding box on model '%s'!  Swapping values to compensate.", pm->filename);
				}
				model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
				
				pm->n_detail_levels = cfread_int(fp);
			//	mprintf(( "There are %d detail levels\n", pm->n_detail_levels ));
				for (i=0; i<pm->n_detail_levels;i++ )	{
					pm->detail[i] = cfread_int(fp);
					pm->detail_depth[i] = 0.0f;
			///		mprintf(( "Detail level %d is model %d.\n", i, pm->detail[i] ));
				}

				pm->num_debris_objects = cfread_int(fp);
			    if (pm->num_debris_objects > MAX_DEBRIS_OBJECTS) {
				    Error(LOCATION,
				          "Model %s specified that it contains %d debris objects but only %d are supported by the "
				          "engine.",
				          filename, pm->num_debris_objects, MAX_DEBRIS_OBJECTS);
			    }
				// mprintf(( "There are %d debris objects\n", pm->num_debris_objects ));
				for (i=0; i<pm->num_debris_objects;i++ )	{
					pm->debris_objects[i] = cfread_int(fp);
					// mprintf(( "Debris object %d is model %d.\n", i, pm->debris_objects[i] ));
				}

				if ( pm->version >= 1903 )	{
	
					if ( pm->version >= 2009 )	{
																	
						pm->mass = cfread_float(fp);
						cfread_vector( &pm->center_of_mass, fp );
						cfread_vector( &pm->moment_of_inertia.vec.rvec, fp );
						cfread_vector( &pm->moment_of_inertia.vec.uvec, fp );
						cfread_vector( &pm->moment_of_inertia.vec.fvec, fp );

						if(!is_valid_vec(&pm->moment_of_inertia.vec.rvec) || !is_valid_vec(&pm->moment_of_inertia.vec.uvec) || !is_valid_vec(&pm->moment_of_inertia.vec.fvec)) {
							Warning(LOCATION, "Moment of inertia values for model %s are invalid. This has to be fixed.\n", pm->filename);
							Int3();
						}
					} else {
						// old code where mass wasn't based on area, so do the calculation manually

						float vol_mass = cfread_float(fp);
						//	Attn: John Slagel:  The following is better done in bspgen.
						// Convert volume (cubic) to surface area (quadratic) and scale so 100 -> 100
						float area_mass = (float) pow(vol_mass, 0.6667f) * 4.65f;

						pm->mass = area_mass;
						float mass_ratio = vol_mass / area_mass; 
							
						cfread_vector( &pm->center_of_mass, fp );
						cfread_vector( &pm->moment_of_inertia.vec.rvec, fp );
						cfread_vector( &pm->moment_of_inertia.vec.uvec, fp );
						cfread_vector( &pm->moment_of_inertia.vec.fvec, fp );

						if(!is_valid_vec(&pm->moment_of_inertia.vec.rvec) || !is_valid_vec(&pm->moment_of_inertia.vec.uvec) || !is_valid_vec(&pm->moment_of_inertia.vec.fvec)) {
							Warning(LOCATION, "Moment of inertia values for model %s are invalid. This has to be fixed.\n", pm->filename);
							Int3();
						}

						// John remove this with change to bspgen
						vm_vec_scale( &pm->moment_of_inertia.vec.rvec, mass_ratio );
						vm_vec_scale( &pm->moment_of_inertia.vec.uvec, mass_ratio );
						vm_vec_scale( &pm->moment_of_inertia.vec.fvec, mass_ratio );
					}

					// a custom MOI is only used for ships, but we should probably log it anyway
					if ( IS_MOI_VEC_NULL(&pm->moment_of_inertia.vec.rvec)
						&& IS_MOI_VEC_NULL(&pm->moment_of_inertia.vec.uvec)
						&& IS_MOI_VEC_NULL(&pm->moment_of_inertia.vec.fvec) )
					{
						mprintf(("Model %s has a null moment of inertia!  (This is only a problem if the model is a ship.)\n", filename));
					}

				} else {
					pm->mass = 50.0f;
					vm_vec_zero( &pm->center_of_mass );
					vm_set_identity( &pm->moment_of_inertia );
					vm_vec_scale(&pm->moment_of_inertia.vec.rvec, 0.001f);
					vm_vec_scale(&pm->moment_of_inertia.vec.uvec, 0.001f);
					vm_vec_scale(&pm->moment_of_inertia.vec.fvec, 0.001f);
				}

				// read in cross section info
				pm->xc = NULL;
				if ( pm->version >= 2014 ) {
					pm->num_xc = cfread_int(fp);
					if (pm->num_xc > 0) {
						pm->xc = (cross_section*) vm_malloc(pm->num_xc*sizeof(cross_section));
						for (i=0; i<pm->num_xc; i++) {
							pm->xc[i].z = cfread_float(fp);
							pm->xc[i].radius = cfread_float(fp);
						}
					}
				} else {
					pm->num_xc = 0;
				}

				if ( pm->version >= 2007 )	{
					pm->num_lights = cfread_int(fp);
					//mprintf(( "Found %d lights!\n", pm->num_lights ));

					if (pm->num_lights > 0) {
						pm->lights = (bsp_light *)vm_malloc( sizeof(bsp_light)*pm->num_lights );
						for (i=0; i<pm->num_lights; i++ )	{			
							cfread_vector(&pm->lights[i].pos,fp);
							pm->lights[i].type = cfread_int(fp);
						}
					}
				} else {
					pm->num_lights = 0;
					pm->lights = NULL;
				}

				break;
			}
			
			case ID_SOBJ: {		//Subobject header
				int n, parent;
				char *p, props[MAX_PROP_LEN];
//				float d;

				//mprintf(0,"Got chunk SOBJ, len=%d\n",len);

				n = cfread_int(fp);
				//mprintf(("SOBJ IDed itself as %d\n", n));
				Assert(n < pm->n_models );
				auto sm = &pm->submodel[n];

#if defined( FREESPACE2_FORMAT )	
				sm->rad = cfread_float(fp);		//radius
#endif

				parent = cfread_int(fp);
				sm->parent = parent;
				auto parent_sm = parent < 0 ? nullptr : &pm->submodel[parent];
				sm->depth = 1;
				{
					int parent_sm_id = parent;
					while (parent_sm_id >= 0) {
						sm->depth++;
						parent_sm_id = pm->submodel[parent_sm_id].parent;
					}
				}



//				cfread_vector(&sm->norm,fp);
//				d = cfread_float(fp);				
//				cfread_vector(&sm->pnt,fp);
				cfread_vector(&sm->offset,fp);

//			mprintf(( "Subobj %d, offs = %.1f, %.1f, %.1f\n", n, sm->offset.xyz.x, sm->offset.xyz.y, sm->offset.xyz.z ));
	
#if defined ( FREESPACE1_FORMAT )
				sm->rad = cfread_float(fp);		//radius
#endif

//				sm->tree_offset = cfread_int(fp);	//offset
//				sm->data_offset = cfread_int(fp);	//offset

				cfread_vector(&sm->geometric_center,fp);

				cfread_vector(&sm->min,fp);
				cfread_vector(&sm->max,fp);

				sm->name[0] = '\0';

				cfread_string_len(sm->name, MAX_NAME_LEN, fp);		// get the name
				cfread_string_len(props, MAX_PROP_LEN, fp);			// and the user properties

				// Check for unrealistic radii
				if ( sm->rad <= 0.1f ) {
					Warning(LOCATION, "Submodel <%s> in model <%s> has a radius <= 0.1f\n", sm->name, filename);
				}
				
				// sanity first!
				if (maybe_swap_mins_maxs(&sm->min, &sm->max)) {
					Warning(LOCATION, "Inverted bounding box on submodel '%s' of model '%s'!  Swapping values to compensate.", sm->name, pm->filename);
				}
				model_calc_bound_box(sm->bounding_box, &sm->min, &sm->max);

				// ---------- submodel movement ----------

				sm->rotation_type = cfread_int(fp);
				sm->rotation_axis_id = cfread_int(fp);

				// change turret rotation type to MOVEMENT_TYPE_TURRET
				if ( in(sm->name, "turret") || (parent_sm && (parent_sm->rotation_type == MOVEMENT_TYPE_TURRET)) ) {
					sm->rotation_type = MOVEMENT_TYPE_TURRET;
				} else if (sm->rotation_type == MOVEMENT_TYPE_REGULAR) {
					if (in(sm->name, "thruster")) {
						sm->rotation_type = MOVEMENT_TYPE_NONE;
					} else if (in(props, "$triggered")) {
						sm->rotation_type = MOVEMENT_TYPE_TRIGGERED;
					}
				}

				determine_submodel_movement(true, pm->filename, sm, props, look_at_submodel_names);

				// submodel translation is a new POF feature
				if ((pm->version >= 2119 && pm->version < PM_FIRST_ALIGNED_VERSION)
					|| (pm->version >= 2202 && pm->version < PM_FIRST_VERTLIM_VERSION)
					|| (pm->version >= 2301))
				{
					sm->translation_type = cfread_int(fp);
					sm->translation_axis_id = cfread_int(fp);

					if (sm->translation_type == MOVEMENT_TYPE_REGULAR) {
						if (in(props, "$triggered")) {
							sm->translation_type = MOVEMENT_TYPE_TRIGGERED;
						}
					}

					determine_submodel_movement(false, pm->filename, sm, props, look_at_submodel_names);
				} else {
					sm->translation_type = MOVEMENT_TYPE_NONE;
					sm->translation_axis_id = -1;
				}

				if ( sm->name[0] == '\0' ) {
					strcpy_s(sm->name, "unknown object name");
				}

				if (in(p, props, "$special")) {
					char type[64];

					get_user_prop_value(p+9, type);
					if ( !stricmp(type, "subsystem") ) {	// if we have a subsystem, put it into the list!
						subsystemParseList.model_subsystems.emplace(sm->name, model_read_deferred_tasks::model_subsystem_parse{ n, sm->rad, sm->offset, props });
					} else {
						if ( !stricmp(type, "no_rotate") || !stricmp(type, "no_movement") ) {
							// mark those submodels which should not move - i.e., those with no subsystem
							sm->rotation_type = MOVEMENT_TYPE_NONE;
						} else {
							// if submodel rotates (via bspgen), then there is either a subsys or special=no_rotate
							Assert( sm->rotation_type != MOVEMENT_TYPE_REGULAR );
						}
						if ( !stricmp(type, "no_translate") || !stricmp(type, "no_movement") ) {
							// mark those submodels which should not move - i.e., those with no subsystem
							sm->translation_type = MOVEMENT_TYPE_NONE;
						} else {
							// if submodel translates (via bspgen), then there is either a subsys or special=no_translate
							Assert( sm->translation_type != MOVEMENT_TYPE_REGULAR );
						}
					}
				}

				// ---------- done with submodel movement (except for gun_rotation and sanity checks) ----------

				sm->flags.set(Model::Submodel_flags::No_collisions, in(props, "$no_collisions"));

				sm->flags.set(Model::Submodel_flags::Nocollide_this_only, in(props, "$nocollide_this_only"));

				sm->flags.set(Model::Submodel_flags::Collide_invisible, in(props, "$collide_invisible"));

				if (in(props, "$gun_rotation")) {
					sm->flags.set(Model::Submodel_flags::Gun_rotation);
					sm->flags.set(Model::Submodel_flags::Can_move);		// this is something of a special case because it's rotating without "rotating"
				}

				if (in(p, props, "$lod0_name"))
					get_user_prop_value(p+10, sm->lod_name);

				sm->flags.set(Model::Submodel_flags::Attach_thrusters, in(props, "$attach_thrusters"));

				if (in(p, props, "$detail_box:")) {
					p += 12;
					while (*p == ' ') p++;
					sm->use_render_box = atoi(p);

					if (in(p, props, "$box_offset:")) {
						p += 12;
						while (*p == ' ') p++;
						sm->render_box_offset.xyz.x = (float)strtod(p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_box_offset.xyz.y = (float)strtod(++p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_box_offset.xyz.z = (float)strtod(++p, (char **)nullptr);

						sm->flags.set(Model::Submodel_flags::Use_render_box_offset);
					}

					if (in(p, props, "$box_min:")) {
						p += 9;
						while (*p == ' ') p++;
						sm->render_box_min.xyz.x = (float)strtod(p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_box_min.xyz.y = (float)strtod(++p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_box_min.xyz.z = (float)strtod(++p, (char **)nullptr);
					} else {
						sm->render_box_min = sm->min;
					}

					if (in(p, props, "$box_max:")) {
						p += 9;
						while (*p == ' ') p++;
						sm->render_box_max.xyz.x = (float)strtod(p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_box_max.xyz.y = (float)strtod(++p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_box_max.xyz.z = (float)strtod(++p, (char **)nullptr);
					} else {
						sm->render_box_max = sm->max;
					}

					if (in(p, props, "$do_not_scale_distances")) {
						p += 23;
						sm->flags.set(Model::Submodel_flags::Do_not_scale_detail_distances);
					}
				}

				if (in(p, props, "$detail_sphere:")) {
					p += 15;
					while (*p == ' ') p++;
					sm->use_render_sphere = atoi(p);

					if (in(p, props, "$radius:")) {
						p += 8;
						while (*p == ' ') p++;
						sm->render_sphere_radius = (float)strtod(p, (char **)nullptr);
					} else {
						sm->render_sphere_radius = sm->rad;
					}

					if (in(p, props, "$offset:")) {
						p += 8;
						while (*p == ' ') p++;
						sm->render_sphere_offset.xyz.x = (float)strtod(p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_sphere_offset.xyz.y = (float)strtod(++p, (char **)nullptr);
						while (*p != ',') p++;
						sm->render_sphere_offset.xyz.z = (float)strtod(++p, (char **)nullptr);

						sm->flags.set(Model::Submodel_flags::Use_render_sphere_offset);
					} else {
						sm->render_sphere_offset = vmd_zero_vector;
					}

					if (in(p, props, "$do_not_scale_distances")) {
						p += 23;
						sm->flags.set(Model::Submodel_flags::Do_not_scale_detail_distances);
					}
				}

				// KeldorKatarn, with modifications
				if (in(p, props, "$uvec")) {
					matrix submodel_orient;

					if (get_user_vec3d_value(p + 5, &submodel_orient.vec.uvec, false, sm->name, pm->filename)) {

						if (in(p, props, "$fvec")) {

							if (get_user_vec3d_value(p + 5, &submodel_orient.vec.fvec, false, sm->name, pm->filename)) {

								vm_vec_normalize(&submodel_orient.vec.uvec);
								vm_vec_normalize(&submodel_orient.vec.fvec);

								vm_vec_cross(&submodel_orient.vec.rvec, &submodel_orient.vec.uvec, &submodel_orient.vec.fvec);
								vm_vec_cross(&submodel_orient.vec.fvec, &submodel_orient.vec.rvec, &submodel_orient.vec.uvec);

								vm_vec_normalize(&submodel_orient.vec.fvec);
								vm_vec_normalize(&submodel_orient.vec.rvec);

								vm_orthogonalize_matrix(&submodel_orient);

								sm->frame_of_reference = submodel_orient;

							} else {
								Warning(LOCATION,
									"Submodel '%s' of model '%s' has an improperly formatted $fvec declaration in its properties."
									"\n\n$fvec should be followed by 3 numbers separated with commas.",
									sm->name, filename);
							}
						} else {
							Warning(LOCATION, "Improper custom orientation matrix for subsystem %s; you must define both an up vector and a forward vector", sm->name);
						}
					} else {
						Warning(LOCATION,
							"Submodel '%s' of model '%s' has an improperly formatted $uvec declaration in its properties."
							"\n\n$uvec should be followed by 3 numbers separated with commas.",
							sm->name, filename);
					}
				} else {
					sm->frame_of_reference = parent_sm ? parent_sm->frame_of_reference : vmd_identity_matrix;
				}

				// ---------- submodel movement sanity checks ----------

				do_movement_sanity_checks(true, sm, parent_sm, pm->filename);
				do_movement_sanity_checks(false, sm, parent_sm, pm->filename);

				// ---------- done submodel movement sanity checks ----------

				{
					int nchunks = cfread_int( fp );		// Throw away nchunks
					if ( nchunks > 0 )	{
						Error( LOCATION, "Model '%s' is chunked.  See John or Adam!\n", pm->filename );
					}
				}

				//ShivanSpS - if pof version is 2200 or higher load bsp_data as it is, otherwise, align it
				if (pm->version >= 2200)
				{
					sm->bsp_data_size = cfread_int(fp);
					if (sm->bsp_data_size > 0) {
						sm->bsp_data = (ubyte*)vm_malloc(sm->bsp_data_size);
						cfread(sm->bsp_data, 1, sm->bsp_data_size, fp);
						swap_bsp_data(pm, sm->bsp_data);
					}
					else {
						sm->bsp_data = nullptr;
					}
				}
				else
				{
					sm->bsp_data_size = cfread_int(fp);

					if (sm->bsp_data_size > 0) {
						auto bsp_data = reinterpret_cast<ubyte *>(vm_malloc(sm->bsp_data_size));

						cfread(bsp_data, 1, sm->bsp_data_size, fp);

						// byte swap first thing
						swap_bsp_data(pm, bsp_data);

						auto bsp_data_size_aligned = align_bsp_data(bsp_data, nullptr, sm->bsp_data_size);

						if (bsp_data_size_aligned != static_cast<uint>(sm->bsp_data_size)) {
							auto bsp_data_aligned = reinterpret_cast<ubyte *>(vm_malloc(bsp_data_size_aligned));

							align_bsp_data(bsp_data, bsp_data_aligned, sm->bsp_data_size);

							// release unaligned data
							vm_free(bsp_data);
							bsp_data = nullptr;

							nprintf(("Model", "BSP ALIGN => %s:%s resized by %d bytes (%d total)\n", pm->filename, sm->name, bsp_data_size_aligned-sm->bsp_data_size, bsp_data_size_aligned));

							sm->bsp_data = bsp_data_aligned;
							sm->bsp_data_size = bsp_data_size_aligned;
						} else {
							sm->bsp_data = bsp_data;
						}
					}
					else {
						sm->bsp_data = nullptr;
					}
				}

				sm->flags.set(Model::Submodel_flags::Is_thruster, in(sm->name, "thruster"));

				// Genghis: if we have a thruster and none of the collision 
				// properties were provided, then set "nocollide_this_only".
				if (sm->flags[Model::Submodel_flags::Is_thruster] && !(sm->flags[Model::Submodel_flags::No_collisions, Model::Submodel_flags::Nocollide_this_only, Model::Submodel_flags::Collide_invisible]) )
				{
					sm->flags.set(Model::Submodel_flags::Nocollide_this_only);
				}

				sm->flags.set(Model::Submodel_flags::Is_damaged, in(sm->name, "-destroyed"));

				break;
			}

			case ID_SLDC: // kazan - Shield Collision tree
			{   //ShivanSpS - if pof version is 2200 or higher ignore SLDC, otherwise convert it to slc2.
				if (pm->version < 2200) {
					//mprintf(("SLDC data is being converted to SLC2.\n"));
					pm->sldc_size = cfread_int(fp);

					std::unique_ptr<ubyte[]> sldc_tree(new ubyte[pm->sldc_size]);
					std::unique_ptr<ubyte[]> slc2_tree(new ubyte[pm->sldc_size * 2]);

					cfread(sldc_tree.get(), 1, pm->sldc_size, fp);
					//mprintf(("SLDC Shield Collision Tree was %d bytes in size\n", pm->sldc_size));
					pm->sldc_size = convert_sldc_to_slc2(sldc_tree.get(), slc2_tree.get(), pm->sldc_size);
					//mprintf(("SLC2 Shield Collision Tree is %d bytes in size\n", pm->sldc_size));
					pm->shield_collision_tree = (ubyte*)vm_malloc(pm->sldc_size); //sldc_size is slc2 size, reused variable
					memcpy(pm->shield_collision_tree, slc2_tree.get(), pm->sldc_size);
					swap_sldc_data(pm->shield_collision_tree);
				}
			}
			break;

			case ID_SLC2: // ShivanSpS -Newer version of the SLDC Shield Collision tree, only pof version 2200.
			{
				if (pm->version >= 2200) {
					pm->sldc_size = cfread_int(fp);
					pm->shield_collision_tree = (ubyte*)vm_malloc(pm->sldc_size);
					cfread(pm->shield_collision_tree, 1, pm->sldc_size, fp);
					swap_sldc_data(pm->shield_collision_tree);
					//mprintf(( "SLC2 Shield Collision Tree, %d bytes in size\n", pm->sldc_size));
				}
			}
			break;

			case ID_SHLD:
				{
					pm->shield.nverts = cfread_int( fp );		// get the number of vertices in the list

					if (pm->shield.nverts > 0) {
						pm->shield.verts = (shield_vertex *)vm_malloc(pm->shield.nverts * sizeof(shield_vertex) );
						Assert( pm->shield.verts );
						for ( i = 0; i < pm->shield.nverts; i++ ) {						// read in the vertex list
							cfread_vector( &(pm->shield.verts[i].pos), fp );
						}
					}

					pm->shield.ntris = cfread_int( fp );		// get the number of triangles that compose the shield

					if (pm->shield.ntris > 0) {
						pm->shield.tris = (shield_tri *)vm_malloc(pm->shield.ntris * sizeof(shield_tri) );
						Assert( pm->shield.tris );
						for ( i = 0; i < pm->shield.ntris; i++ ) {
							cfread_vector( &temp_vec, fp );
							vm_vec_normalize_safe(&temp_vec);
							pm->shield.tris[i].norm = temp_vec;
							for ( j = 0; j < 3; j++ ) {
								pm->shield.tris[i].verts[j] = cfread_int( fp );		// read in the indices into the shield_vertex list
#ifndef NDEBUG
								if (pm->shield.tris[i].verts[j] >= pm->shield.nverts) {
									Error(LOCATION, "Ship %s has a bogus shield mesh.\nOnly %i vertices, index %i found.\n", filename, pm->shield.nverts, pm->shield.tris[i].verts[j]);
								}
#endif
							}
							
							for ( j = 0; j < 3; j++ ) {
								pm->shield.tris[i].neighbors[j] = cfread_int( fp );	// read in the neighbor indices -- indexes into tri list
#ifndef NDEBUG
								if (pm->shield.tris[i].neighbors[j] >= pm->shield.ntris) {
									Error(LOCATION, "Ship %s has a bogus shield mesh.\nOnly %i triangles, index %i found.\n", filename, pm->shield.ntris, pm->shield.tris[i].neighbors[j]);
								}
#endif
							}
						}
					}
				}
				break;

			// guns and missiles use almost exactly the same code
			case ID_GPNT:
			case ID_MPNT:
			{
				int n_weps = cfread_int(fp);
				w_bank *wep_banks = nullptr;

				if (n_weps > 0)
				{
					wep_banks = new w_bank[n_weps];
					for (i = 0; i < n_weps; ++i)
					{
						w_bank *bank = &wep_banks[i];

						bank->num_slots = cfread_int(fp);
						if (bank->num_slots > 0)
						{
							bank->pnt = new vec3d[bank->num_slots];
							bank->norm = new vec3d[bank->num_slots];
							bank->external_model_angle_offset = new float[bank->num_slots];

							for (j = 0; j < bank->num_slots; ++j)
							{
								cfread_vector(&(bank->pnt[j]), fp);
								cfread_vector(&temp_vec, fp);
								vm_vec_normalize_safe(&temp_vec);
								bank->norm[j] = temp_vec;

								// angle offsets are a new POF feature
								// (note that any version >= 2201 supports them, including all vertlim versions)
								if ((pm->version >= 2118 && pm->version < PM_FIRST_ALIGNED_VERSION) || (pm->version >= 2201))
									bank->external_model_angle_offset[j] = fl_radians(cfread_float(fp));
								else
									bank->external_model_angle_offset[j] = 0.0f;
							}
						}
					}
				}

				if (id == ID_GPNT)
				{
					pm->n_guns = n_weps;
					pm->gun_banks = wep_banks;
				}
				else
				{
					pm->n_missiles = n_weps;
					pm->missile_banks = wep_banks;
				}
				break;
			}

			case ID_DOCK: {
				char props[MAX_PROP_LEN];

				pm->n_docks = cfread_int(fp);

				if (pm->n_docks > 0) {
					pm->docking_bays = (dock_bay *)vm_malloc(sizeof(dock_bay) * pm->n_docks);
					Assert( pm->docking_bays != NULL );

					for (i = 0; i < pm->n_docks; i++ ) {
						char *p;
						dock_bay *bay = &pm->docking_bays[i];

						cfread_string_len( props, MAX_PROP_LEN, fp );
						if (in(p, props, "$name")) {
							get_user_prop_value(p+5, bay->name);

							auto length = strlen(bay->name);
							if ((length > 0) && is_white_space(bay->name[length-1])) {
								nprintf(("Model", "model '%s' has trailing whitespace on bay name '%s'; this will be trimmed\n", pm->filename, bay->name));
								drop_trailing_white_space(bay->name);
							}
							if (strlen(bay->name) == 0) {
								nprintf(("Model", "model '%s' has an empty name specified for docking point %d\n", pm->filename, i));
							}
						} else {
							nprintf(("Model", "model '%s' has no name specified for docking point %d\n", pm->filename, i));
							sprintf(bay->name, "<unnamed bay %c>", 'A' + i);
						}

#ifndef NDEBUG
						// check for duplicates
						// (we just warn here and take no action, because even some retail models have duplicate dockpoints)
						for (j = 0; j < i; j++) {
							if (stricmp(bay->name, pm->docking_bays[j].name) == 0) {
								Warning(LOCATION, "Duplicate docking bay name '%s' found on model '%s'!", bay->name, pm->filename);
							}
						}
#endif

						bay->num_spline_paths = cfread_int( fp );
						if ( bay->num_spline_paths > 0 ) {
							bay->splines = (int *)vm_malloc(sizeof(int) * bay->num_spline_paths);
							for ( j = 0; j < bay->num_spline_paths; j++ )
								bay->splines[j] = cfread_int(fp);
						} else {
							bay->splines = NULL;
						}

						// see if this dockpoint should be anchored to a submodel
						if (in(p, props, "$parent_submodel")) {
							// we need to work out the correct subobject number later, after all subobjects have been processed
							bay->parent_submodel = static_cast<int>(dock_parent_submodel_names.size());

							char submodel_name[MAX_NAME_LEN];
							get_user_prop_value(p + 16, submodel_name);
							dock_parent_submodel_names.push_back(submodel_name);
						} else {
							bay->parent_submodel = -1; // No submodel
						}

						// determine what this docking bay can be used for
						if ( !strnicmp(bay->name, "cargo", 5) )
							bay->type_flags = DOCK_TYPE_CARGO;
						else
							bay->type_flags = (DOCK_TYPE_REARM | DOCK_TYPE_GENERIC);

						bay->num_slots = cfread_int(fp);

						if(bay->num_slots != 2) {
							Warning(LOCATION, "Model '%s' has %d slots in dock point '%s'; models must have exactly %d slots per dock point.", filename, bay->num_slots, bay->name, 2);
						}

						for (j = 0; j < bay->num_slots; j++) {
							cfread_vector( &(bay->pnt[j]), fp );
							cfread_vector( &(bay->norm[j]), fp );
							if(vm_vec_mag(&(bay->norm[j])) <= 0.0f) {
								Warning(LOCATION, "Model '%s' dock point '%s' has a null normal. ", filename, bay->name);
							}
						}

						if(vm_vec_same(&bay->pnt[0], &bay->pnt[1])) {
							Warning(LOCATION, "Model '%s' has two identical docking slot positions on docking port '%s'. This is not allowed.  A new second slot position will be generated.", filename, bay->name);

							// just move the second point over by some amount
							bay->pnt[1].xyz.z += 10.0f;
						}

						vec3d diff;
						vm_vec_normalized_dir(&diff, &bay->pnt[0], &bay->pnt[1]);
						float dot = vm_vec_dot(&diff, &bay->norm[0]);
						if(fl_abs(dot) > 0.99f) {
							Warning(LOCATION, "Model '%s', docking port '%s' has docking slot positions that lie on the same axis as the docking normal.  This will cause a NULL VEC crash when docked to another ship.  A new docking normal will be generated.", filename, bay->name);

							// generate a simple rotation matrix in all three dimensions (though bank is probably not needed)
							angles a = { PI_2, PI_2, PI_2 };
							matrix m;
							vm_angles_2_matrix(&m, &a);

							// rotate the docking normal
							vec3d temp = bay->norm[0];
							vm_vec_rotate(&bay->norm[0], &temp, &m);
						}
					}
				}
				break;
			}

			case ID_GLOW:					//start glow point reading -Bobboau
			{
				char props[MAX_PROP_LEN];

				int gpb_num = cfread_int(fp);

				pm->n_glow_point_banks = gpb_num;
				pm->glow_point_banks = NULL;

				if (gpb_num > 0)
				{
					pm->glow_point_banks = (glow_point_bank *) vm_malloc(sizeof(glow_point_bank) * gpb_num);
					Assert(pm->glow_point_banks != NULL);
				}

				for (int gpb = 0; gpb < gpb_num; gpb++)
				{
					glow_point_bank *bank = &pm->glow_point_banks[gpb];

					bank->is_on = true;
					bank->glow_timestamp = 0;
					bank->disp_time = cfread_int(fp);
					bank->on_time = cfread_int(fp);
					bank->off_time = cfread_int(fp);
					bank->submodel_parent = cfread_int(fp);
					bank->LOD = cfread_int(fp);
					bank->type = cfread_int(fp);
					bank->num_points = cfread_int(fp);
					bank->points = NULL;
					bank->glow_bitmap = -1;
					bank->glow_neb_bitmap = -1;

					if (bank->num_points > 0)
						bank->points = (glow_point *) vm_malloc(sizeof(glow_point) * bank->num_points);

					//if((bank->off_time > 0) && (bank->disp_time > 0))
						//bank->is_on = false;
	
					cfread_string_len(props, MAX_PROP_LEN, fp);
					// look for $glow_texture=xxx
					auto length = strlen(props);

					if (length > 0)
					{
						auto base_length = strlen("$glow_texture=");
						char *glow_texture_start = strstr(props, "$glow_texture=");
						if ( (glow_texture_start != nullptr) && (strlen(glow_texture_start + base_length) > 0) ) {
							char *glow_texture_name = glow_texture_start + base_length;
							
							if (glow_texture_name[0] == '$')
								glow_texture_name++;

							bank->glow_bitmap = bm_load(glow_texture_name);

							if (bank->glow_bitmap < 0)
							{
								Warning( LOCATION, "Couldn't open glowpoint texture '%s'\nreferenced by model '%s'\n", glow_texture_name, pm->filename);
							}
							else
							{
								nprintf(( "Model", "Glow point bank %i texture num is %d for '%s'\n", gpb, bank->glow_bitmap, pm->filename));
							}

							strcat(glow_texture_name, "-neb");
							bank->glow_neb_bitmap = bm_load(glow_texture_name);

							if (bank->glow_neb_bitmap < 0)
							{
								bank->glow_neb_bitmap = bank->glow_bitmap;
								nprintf(( "Model", "Glow point bank nebula texture not found for '%s', using normal glowpoint texture instead\n", pm->filename));
							//	Error( LOCATION, "Couldn't open texture '%s'\nreferenced by model '%s'\n", glow_texture_name, pm->filename );
							}
							else
							{
								nprintf(( "Model", "Glow point bank %i nebula texture num is %d for '%s'\n", gpb, bank->glow_neb_bitmap, pm->filename));
							}
						} else {
							Warning( LOCATION, "No glow point texture for bank '%d' referenced by model '%s'\n", gpb, pm->filename);
						}
					} 
					else 
					{
						Warning( LOCATION, "No glow point texture for bank '%d' referenced by model '%s'\n", gpb, pm->filename);
					}

					for (j = 0; j < bank->num_points; j++)
					{
						glow_point *p = &bank->points[j];

						cfread_vector(&(p->pnt), fp);
						cfread_vector( &temp_vec, fp );
						if (!IS_VEC_NULL_SQ_SAFE(&temp_vec))
							vm_vec_normalize(&temp_vec);
						else
							vm_vec_zero(&temp_vec);
						p->norm = temp_vec;
						p->radius = cfread_float( fp);
					}
				}
				break;					
			 }

			case ID_FUEL:
				char props[MAX_PROP_LEN];
				pm->n_thrusters = cfread_int(fp);

				if (pm->n_thrusters > 0) {
					pm->thrusters = (thruster_bank *)vm_malloc(sizeof(thruster_bank) * pm->n_thrusters);
					Assert( pm->thrusters != NULL );

					for (i = 0; i < pm->n_thrusters; i++ ) {
						thruster_bank *bank = &pm->thrusters[i];

						bank->num_points = cfread_int(fp);
						bank->points = NULL;

						if (bank->num_points > 0)
							bank->points = (glow_point *) vm_malloc(sizeof(glow_point) * bank->num_points);

						bank->obj_num = -1;
						bank->submodel_num = -1;
						bank->wash_info_pointer = nullptr;

						if (pm->version >= 2117) {
							cfread_string_len( props, MAX_PROP_LEN, fp );
							// look for $engine_subsystem=xxx
							auto length = strlen(props);
							if (length > 0) {
								auto base_length = strlen("$engine_subsystem=");
								char *engine_subsys_start;
								if (in(engine_subsys_start, props, "$engine_subsystem=") && (strlen(engine_subsys_start + base_length) > 0)) {
									char *engine_subsys_name = engine_subsys_start + base_length;
									if (engine_subsys_name[0] == '$') {
										engine_subsys_name++;
									}

									nprintf(("wash", "Ship %s with engine wash associated with subsys %s\n", filename, engine_subsys_name));

									subsystemParseList.engine_subsystems.emplace(engine_subsys_name, model_read_deferred_tasks::engine_subsystem_parse{ i });
								}
							}
						}

						for (j = 0; j < bank->num_points; j++) {
							glow_point *p = &bank->points[j];

							cfread_vector( &(p->pnt), fp );
							cfread_vector( &temp_vec, fp );
							vm_vec_normalize_safe(&temp_vec);
							p->norm = temp_vec;

							if ( pm->version > 2004 )	{
								p->radius = cfread_float( fp );
								//mprintf(( "Rad = %.2f\n", rad ));
							} else {
								p->radius = 1.0f;
							}
						}
						//mprintf(( "Num slots = %d\n", bank->num_slots ));
					}
				}
				break;

			case ID_TGUN:
			case ID_TMIS: {
				int n_banks = cfread_int(fp);			// Number of turrets

				for ( i = 0; i < n_banks; i++ ) {
					int n_slots;						// How many firepoints the turret has

					int base_obj = cfread_int(fp);		// The parent subobj of the turret (the gun base)
					int gun_obj = cfread_int(fp);       // The subobj that the firepoints are physically attached to (the gun barrel)
					
					if (base_obj != gun_obj && pm->submodel[gun_obj].parent != base_obj) {
						Warning(LOCATION, "Model %s turret %s has a gun submodel that is not an immediate child object of the base", pm->filename, pm->submodel[base_obj].name);
						gun_obj = base_obj; // fall back to singlepart handling
					}

					cfread_vector(&temp_vec, fp);
					vm_vec_normalize_safe(&temp_vec);
					n_slots = cfread_int(fp);
					SCP_vector<vec3d> firingpoints;
					for (j = 0; j < n_slots; j++) {
						if (j < MAX_TFP) {
							vec3d firepoint;
							cfread_vector(&firepoint, fp);
							firingpoints.emplace_back(std::move(firepoint));
						}
						else
						{
							vec3d bogus;
							cfread_vector(&bogus, fp);
						}
					}
					Assertion(n_slots > 0, "Turret %s in model %s has no firing points.\n", pm->submodel[gun_obj].name, pm->filename);

					subsystemParseList.weapons_subsystems.emplace(base_obj, model_read_deferred_tasks::weapon_subsystem_parse{ i, gun_obj, temp_vec, n_slots, std::move(firingpoints) });
				}
				break;
			}

			case ID_SPCL: {
				char name[MAX_NAME_LEN], props_spcl[MAX_PROP_LEN], *p;
				int n_specials;
				float radius;
				vec3d pnt;

				n_specials = cfread_int(fp);		// get the number of special subobjects we have
				for (i = 0; i < n_specials; i++) {

					// get the next free object of the subobject list.  Flag error if no more room

					cfread_string_len(name, MAX_NAME_LEN, fp);			// get the name of this special polygon

					cfread_string_len(props_spcl, MAX_PROP_LEN, fp);		// will definately have properties as well!
					cfread_vector( &pnt, fp );
					radius = cfread_float( fp );

					// check if $Split
					if (in(name, "$split")) {
						pm->split_plane[pm->num_split_plane] = pnt.xyz.z;
						pm->num_split_plane++;
						Assert(pm->num_split_plane <= MAX_SPLIT_PLANE);
					} else if (in(p, props_spcl, "$special")) {
						char type[64];

						get_user_prop_value(p+9, type);
						if ( !stricmp(type, "subsystem") ) {	// if we have a subsystem, put it into the list!
							subsystemParseList.model_subsystems.emplace(&name[1], model_read_deferred_tasks::model_subsystem_parse{ -1, radius, pnt, props_spcl }); // skip the first '$' character of the name
						} else if ( !stricmp(type, "shieldpoint") ) {
							pm->shield_points.push_back(pnt);
						}
					} else if (in(name, "$enginelarge") || in(name, "$enginehuge")) 
					{
						subsystemParseList.model_subsystems.emplace(&name[1], model_read_deferred_tasks::model_subsystem_parse{ -1, radius, pnt, props_spcl }); // skip the first '$' character of the name	
					} else {
						nprintf(("Warning", "Unknown special object type %s while reading model %s\n", name, pm->filename));
					}					
				}
				break;
			}
			
			case ID_TXTR: {		//Texture filename list
				int n;
//				char name_buf[128];

				//mprintf(0,"Got chunk TXTR, len=%d\n",len);


				n = cfread_int(fp);
				pm->n_textures = n;
				// Don't overwrite memory!!
				Verify(pm->n_textures <= MAX_MODEL_TEXTURES);
				//mprintf(0,"  num textures = %d\n",n);
				for (i=0; i<n; i++ )
				{
					char tmp_name[256];
					cfread_string_len(tmp_name,127,fp);
					model_load_texture(pm, i, tmp_name);
					//mprintf(0,"<%s>\n",name_buf);
				}


				break;
			}
			
/*			case ID_IDTA:		//Interpreter data
				//mprintf(0,"Got chunk IDTA, len=%d\n",len);

				pm->model_data = (ubyte *)vm_malloc(len);
				pm->model_data_size = len;
				Assert(pm->model_data != NULL );
			
				cfread(pm->model_data,1,len,fp);
			
				break;
*/

			case ID_INFO:		// don't need to do anything with info stuff

				#ifndef NDEBUG
					pm->debug_info_size = len;
					pm->debug_info = (char *)vm_malloc(pm->debug_info_size+1);
					Assert(pm->debug_info!=NULL);
					memset(pm->debug_info,0,len+1);
					cfread( pm->debug_info, 1, len, fp );
				#endif
				break;

			case ID_GRID:
				break;

			case ID_PATH:
				pm->n_paths = cfread_int( fp );

				if (pm->n_paths <= 0) {
					break;
				}

				pm->paths = (model_path *)vm_malloc(sizeof(model_path)*pm->n_paths);
				Assert( pm->paths != NULL );

				memset( pm->paths, 0, sizeof(model_path) * pm->n_paths );
					
				for (i=0; i<pm->n_paths; i++ )	{
					cfread_string_len(pm->paths[i].name, MAX_NAME_LEN-1, fp);

					// check for reused path names... not fatal, but maybe problematic
					for (j = 0; j < i; j++) {
						if (!stricmp(pm->paths[i].name, pm->paths[j].name)) {
							Warning(LOCATION, "Path '%s' in model %s has a name that is not unique!", pm->paths[i].name, pm->filename);
						}
					}

					if ( pm->version >= 2002 ) {
						// store the sub_model name number of the parent
						cfread_string_len(pm->paths[i].parent_name , MAX_NAME_LEN-1, fp);
						// get rid of leading '$' char in name
						if ( pm->paths[i].parent_name[0] == '$' ) {
							char tmpbuf[MAX_NAME_LEN];
							strcpy_s(tmpbuf, pm->paths[i].parent_name+1);
							strcpy_s(pm->paths[i].parent_name, tmpbuf);
						}
						// store the sub_model index (ie index into pm->submodel) of the parent
						pm->paths[i].parent_submodel = -1;
						for ( j = 0; j < pm->n_models; j++ ) {
							if ( !stricmp( pm->submodel[j].name, pm->paths[i].parent_name) ) {
								pm->paths[i].parent_submodel = j;
							}
						}
					} else {
						pm->paths[i].parent_name[0] = 0;
						pm->paths[i].parent_submodel = -1;
					}

					pm->paths[i].nverts = cfread_int( fp );
					pm->paths[i].verts = (mp_vert *)vm_malloc( sizeof(mp_vert) * pm->paths[i].nverts );
					pm->paths[i].goal = pm->paths[i].nverts - 1;
					pm->paths[i].type = MP_TYPE_UNUSED;
					pm->paths[i].value = 0;
					Assert(pm->paths[i].verts!=NULL);
					memset( pm->paths[i].verts, 0, sizeof(mp_vert) * pm->paths[i].nverts );

					for (j=0; j<pm->paths[i].nverts; j++ )	{
						cfread_vector(&pm->paths[i].verts[j].pos,fp );
						pm->paths[i].verts[j].radius = cfread_float( fp );
						
						{					// version 1802 added turret stuff
							int nturrets, k;

							nturrets = cfread_int( fp );
							pm->paths[i].verts[j].nturrets = nturrets;

							if (nturrets > 0) {
								pm->paths[i].verts[j].turret_ids = (int *)vm_malloc( sizeof(int) * nturrets );
								for ( k = 0; k < nturrets; k++ )
									pm->paths[i].verts[j].turret_ids[k] = cfread_int( fp );
							}
						} 
						
					}
				}
				break;

			case ID_EYE:					// an eye position(s)
				{
					int num_eyes;

					// all eyes points are stored simply as vectors and their normals.
					// 0th element is used as usual player view position.

					num_eyes = cfread_int( fp );
					pm->n_view_positions = num_eyes;
					Assert ( num_eyes < MAX_EYES );
					for (i = 0; i < num_eyes; i++ ) {
						pm->view_positions[i].parent = cfread_int( fp );
						cfread_vector( &pm->view_positions[i].pnt, fp );
						cfread_vector( &pm->view_positions[i].norm, fp );
					}
				}
				break;			

			case ID_INSG:				
				int num_ins, num_verts, num_faces, idx, idx2, idx3;			
				
				// get the # of insignias
				num_ins = cfread_int(fp);
				pm->num_ins = num_ins;
				
				// read in the insignias
				for(idx=0; idx<num_ins; idx++){
					// get the detail level
					pm->ins[idx].detail_level = cfread_int(fp);
					if (pm->ins[idx].detail_level < 0) {
						Warning(LOCATION, "Model '%s': insignia uses an invalid LOD (%i)\n", pm->filename, pm->ins[idx].detail_level);
					}

					// # of faces
					num_faces = cfread_int(fp);
					pm->ins[idx].num_faces = num_faces;
					Assert(num_faces <= MAX_INS_FACES);

					// # of vertices
					num_verts = cfread_int(fp);
					Assert(num_verts <= MAX_INS_VECS);

					// read in all the vertices
					for(idx2=0; idx2<num_verts; idx2++){
						cfread_vector(&pm->ins[idx].vecs[idx2], fp);
					}

					// read in world offset
					cfread_vector(&pm->ins[idx].offset, fp);

					// read in all the faces
					for(idx2=0; idx2<pm->ins[idx].num_faces; idx2++){						
						// read in 3 vertices
						for(idx3=0; idx3<3; idx3++){
							pm->ins[idx].faces[idx2][idx3] = cfread_int(fp);
							pm->ins[idx].u[idx2][idx3] = cfread_float(fp);
							pm->ins[idx].v[idx2][idx3] = cfread_float(fp);
						}
						vec3d tempv;

						//get three points (rotated) and compute normal

						vm_vec_perp(&tempv, 
							&pm->ins[idx].vecs[pm->ins[idx].faces[idx2][0]], 
							&pm->ins[idx].vecs[pm->ins[idx].faces[idx2][1]], 
							&pm->ins[idx].vecs[pm->ins[idx].faces[idx2][2]]);

						vm_vec_normalize_safe(&tempv);

						pm->ins[idx].norm[idx2] = tempv;
//						mprintf(("insignorm %.2f %.2f %.2f\n",pm->ins[idx].norm[idx2].xyz.x, pm->ins[idx].norm[idx2].xyz.y, pm->ins[idx].norm[idx2].xyz.z));

					}
				}					
				break;

			// autocentering info
			case ID_ACEN:
				cfread_vector(&pm->autocenter, fp);
				pm->flags |= PM_FLAG_AUTOCEN;
				break;

			default:
				mprintf(("Unknown chunk <%c%c%c%c>, len = %d\n",id,id>>8,id>>16,id>>24,len));
				cfseek(fp,len,SEEK_CUR);
				break;

		}
		cfseek(fp,next_chunk,SEEK_SET);

		id = cfread_int(fp);
		len = cfread_int(fp);
		next_chunk = cftell(fp) + len;
	}

	// Now that we've processed all the chunks, resolve the submodel indexes if we have any...

	// handle look_at
	for (i = 0; i < pm->n_models; i++) {
		auto sm = &pm->submodel[i];

		if (sm->look_at_submodel >= 0) {
			resolve_submodel_index(pm, sm->name, "$look_at target", sm->look_at_submodel, look_at_submodel_names);

			// if we couldn't find it, we shouldn't move
			if (sm->look_at_submodel < 0) {
				sm->rotation_type = MOVEMENT_TYPE_NONE;
			}
			// are we navel-gazing?
			else if (sm->look_at_submodel == i) {
				Warning(LOCATION, "Matched %s %s $look_at: target with its own submodel!  Submodel cannot look at itself!\n", pm->filename, sm->name);
				sm->look_at_submodel = -1;
				sm->rotation_type = MOVEMENT_TYPE_NONE;
			}
		}
	}

	// handle dockpoint parent_submodels
	for (i = 0; i < pm->n_docks; i++) {
		auto dock = &pm->docking_bays[i];

		if (dock->parent_submodel >= 0) {
			resolve_submodel_index(pm, dock->name, "$parent_submodel", dock->parent_submodel, dock_parent_submodel_names);
		}
	}

	// And now look through all the submodels and set the model flag if any are intrinsic-moving
	for (i = 0; i < pm->n_models; i++) {
		if (pm->submodel[i].rotation_type == MOVEMENT_TYPE_INTRINSIC || pm->submodel[i].translation_type == MOVEMENT_TYPE_INTRINSIC) {
			pm->flags |= PM_FLAG_HAS_INTRINSIC_MOTION;
			break;
		}
	}

#ifndef NDEBUG
	if ( ss_fp) {
		int size;
		
		cfclose(ss_fp);
		ss_fp = cfopen(debug_name, "rb");
		if ( ss_fp )	{
			size = cfilelength(ss_fp);
			cfclose(ss_fp);
			if ( size <= 0 )	{
				_unlink(debug_name);
			}
		}
	}
#endif

	cfclose(fp);

	// mprintf(("Done processing chunks\n"));
	return 1;
}

int read_model_file(polymodel* pm, const char* filename, int ferror, model_read_deferred_tasks& deferredTasks, model_parse_depth depth = {})
{
	int status = 0;

	//See if this is a modular, virtual pof, and if so, parse it from there
	if (read_virtual_model_file(pm, filename, depth, ferror, deferredTasks)) {
		status = 1;
	}
	else {
		status = read_model_file_no_subsys(pm, filename, ferror, deferredTasks);
	}

	return status;
}

//reads a binary file containing a 3d model
int read_and_process_model_file(polymodel* pm, const char* filename, int n_subsystems, model_subsystem* subsystems, int ferror, model_read_deferred_tasks& deferredTasks)
{
	int status = read_model_file(pm, filename, ferror, deferredTasks);

	for (const auto& subsystem : deferredTasks.model_subsystems) {
		auto propBuffer = make_unique<char[]>(subsystem.second.props.size() + 1);
		strncpy(propBuffer.get(), subsystem.second.props.c_str(), subsystem.second.props.size() + 1);

		do_new_subsystem(n_subsystems, subsystems, subsystem.second.subobj_nr, subsystem.second.rad, &subsystem.second.pnt, propBuffer.get(), subsystem.first.c_str(), pm->id);		
	}

	for (const auto& subsystem : deferredTasks.engine_subsystems) {
		// start off assuming the subsys is invalid
		int table_error = 1;
		auto bank = &pm->thrusters[subsystem.second.thruster_nr];

		for (int k = 0; k < n_subsystems; k++) {
			if (!subsystem_stricmp(subsystems[k].subobj_name, subsystem.first.c_str())) {
				bank->submodel_num = subsystems[k].subobj_num;

				bank->wash_info_pointer = subsystems[k].engine_wash_pointer;
				if (bank->wash_info_pointer != nullptr) {
					table_error = 0;
				}
				// also set what subsystem this is attached to but not if we only have one thruster bank
				// do this so that original :V: models still work like they used to
				if (pm->n_thrusters > 1) {
					bank->obj_num = k;
				}
				break;
			}
		}

		if ((bank->wash_info_pointer == nullptr) && (n_subsystems > 0)) {
			if (table_error) {
				//	Warning(LOCATION, "No engine wash table entry in ships.tbl for ship model %s", filename);
			}
			else {
				Warning(LOCATION, "Inconsistent model: Engine wash engine subsystem does not match any ship subsytem names for ship model %s", filename);
			}
		}
	}

	for (const auto& subsystem : deferredTasks.weapons_subsystems) {
		model_subsystem* subsystemp;
		if (subsystems) {
			int snum = 0;
			for (snum = 0; snum < n_subsystems; snum++) {
				subsystemp = &subsystems[snum];

				if (subsystem.first == subsystemp->subobj_num) {
					subsystemp->turret_norm = subsystem.second.turretNorm;
					subsystemp->turret_gun_sobj = subsystem.second.gun_subobj_nr;

					if (subsystem.second.n_slots > MAX_TFP) {
						Warning(LOCATION, "Model %s has %i turret firing points on subsystem %s, maximum is %i", pm->filename, subsystem.second.n_slots, subsystemp->name, MAX_TFP);
					}

					for (int j = 0; j < subsystem.second.n_slots; j++) {
						if (j < MAX_TFP)
							subsystemp->turret_firing_point[j] = subsystem.second.firingpoints[j];
					}
					Assertion(subsystem.second.n_slots > 0, "Turret %s in model %s has no firing points.\n", subsystemp->name, pm->filename);

					subsystemp->turret_num_firing_points = subsystem.second.n_slots;

					// copy the subsystem index that the gun base submodel should have at this point
					Assertion(pm->submodel[subsystem.first].subsys_num >= 0, "Turret gun base should have a subsystem index!");
					pm->submodel[subsystem.second.gun_subobj_nr].subsys_num = pm->submodel[subsystem.first].subsys_num;

					break;
				}
			}
			if (snum == n_subsystems) {
				nprintf(("Warning", "Turret submodel %i not found for turret %i in model %s\n", subsystem.first, subsystem.second.turret_nr, pm->filename));
			}
		}
	}

	return status;
}


//Goober
void model_load_texture(polymodel *pm, int i, char *file)
{
	// NOTE: it doesn't help to use more than MAX_FILENAME_LEN here as bmpman will use that restriction
	//       we also have to make sure there is always a trailing NUL since overflow doesn't add it
	char tmp_name[MAX_FILENAME_LEN];
	strcpy_s(tmp_name, file);
	strlwr(tmp_name);

	texture_map *tmap = &pm->maps[i];
	tmap->Clear();

	//WMC - IMPORTANT!!
	//The Fred_running checks are there so that FRED will see those textures and put them in the
	//texture replacement box.

	// base maps ---------------------------------------------------------------
	texture_info *tbase = &tmap->textures[TM_BASE_TYPE];

	if (strstr(tmp_name, "thruster") || strstr(tmp_name, "invisible") || strstr(tmp_name, "warpmap"))
	{
		// Don't load textures for thruster animations or invisible textures
		// or warp models!-Bobboau
		tbase->clear();
	}
	else
	{
		// check if we should be transparent, include "-trans" but make sure to skip anything that might be "-transport"
		if ( (strstr(tmp_name, "-trans") && !strstr(tmp_name, "-transpo")) || strstr(tmp_name, "shockwave") || !strcmp(tmp_name, "nameplate") ) {
			tmap->is_transparent = true;
		}

		if (strstr(tmp_name, "-amb")) {
			tmap->is_ambient = true;
		}

		tbase->LoadTexture(tmp_name, pm->filename);
		
		if ( tbase->GetTexture() < 0 ) {
			Warning(LOCATION, "Couldn't open texture '%s'\nreferenced by model '%s'\n", tmp_name, pm->filename);
		}
	}
	// -------------------------------------------------------------------------

	// glow maps ---------------------------------------------------------------
	texture_info *tglow = &tmap->textures[TM_GLOW_TYPE];
	if ( (!Cmdline_glow && !Fred_running) || (tbase->GetTexture() < 0))
	{
		tglow->clear();
	}
	else
	{
		strcpy_s(tmp_name, file);
		strcat_s(tmp_name, "-glow" );
		strlwr(tmp_name);

		tglow->LoadTexture(tmp_name, pm->filename);
	}
	// -------------------------------------------------------------------------

	// specular maps -----------------------------------------------------------
	texture_info *tspec = &tmap->textures[TM_SPECULAR_TYPE];
	texture_info *tspecgloss = &tmap->textures[TM_SPEC_GLOSS_TYPE];
	if ( (!Cmdline_spec && !Fred_running) || (tbase->GetTexture() < 0))
	{
		tspec->clear();
		tspecgloss->clear();
	}
	else
	{
		// look for reflectance map
		strcpy_s(tmp_name, file);
		strcat_s(tmp_name, "-reflect");
		strlwr(tmp_name);

		tspecgloss->LoadTexture(tmp_name, pm->filename);

		// look for a legacy shine map as well
		strcpy_s(tmp_name, file);
		strcat_s(tmp_name, "-shine");
		strlwr(tmp_name);

		tspec->LoadTexture(tmp_name, pm->filename);
	}
	//tmap->spec_map.original_texture = tmap->spec_map.texture;
	// -------------------------------------------------------------------------

	// bump maps ---------------------------------------------------------------
	texture_info *tnorm = &tmap->textures[TM_NORMAL_TYPE];
	if ( (!Cmdline_normal && !Fred_running) || (tbase->GetTexture() < 0) ) {
		tnorm->clear();
	} else {
		strcpy_s(tmp_name, file);
		strcat_s(tmp_name, "-normal");
		strlwr(tmp_name);

		tnorm->LoadTexture(tmp_name, pm->filename);
	}

	// try to get a height map too
	texture_info *theight = &tmap->textures[TM_HEIGHT_TYPE];
	if ((!Cmdline_height && !Fred_running) || (tbase->GetTexture() < 0)) {
		theight->clear();
	} else {
		strcpy_s(tmp_name, file);
		strcat_s(tmp_name, "-height");
		strlwr(tmp_name);

		theight->LoadTexture(tmp_name, pm->filename);
	}

	// ambient occlusion maps
	texture_info *tambient = &tmap->textures[TM_AMBIENT_TYPE];

	strcpy_s(tmp_name, file);
	strcat_s(tmp_name, "-ao");
	strlwr(tmp_name);

	tambient->LoadTexture(tmp_name, pm->filename);

	// Utility map -------------------------------------------------------------
	texture_info *tmisc = &tmap->textures[TM_MISC_TYPE];

	strcpy_s(tmp_name, file);
	strcat_s(tmp_name, "-misc");
	strlwr(tmp_name);

	tmisc->LoadTexture(tmp_name, pm->filename);

	// -------------------------------------------------------------------------

	// See if we need to compile a new shader for this material
	int shader_flags = 0;

	if (tbase->GetTexture() > 0)
		shader_flags |= SDR_FLAG_MODEL_DIFFUSE_MAP;
	if (tglow->GetTexture() > 0 && Cmdline_glow)
		shader_flags |= SDR_FLAG_MODEL_GLOW_MAP;
	if ((tspec->GetTexture() > 0 || tspecgloss->GetTexture() > 0) && Cmdline_spec)
		shader_flags |= SDR_FLAG_MODEL_SPEC_MAP;
	if (tnorm->GetTexture() > 0 && Cmdline_normal)
		shader_flags |= SDR_FLAG_MODEL_NORMAL_MAP;
	if (theight->GetTexture() > 0 && Cmdline_height)
		shader_flags |= SDR_FLAG_MODEL_HEIGHT_MAP;
	if ((tspec->GetTexture() > 0 || tspecgloss->GetTexture() > 0) && Cmdline_env && Cmdline_spec) // No env maps without spec map
		shader_flags |= SDR_FLAG_MODEL_ENV_MAP;
	if (tmisc->GetTexture() > 0)
		shader_flags |= SDR_FLAG_MODEL_MISC_MAP;
	if (tambient->GetTexture() >0)
		shader_flags |= SDR_FLAG_MODEL_AMBIENT_MAP;
	
	gr_maybe_create_shader(SDR_TYPE_MODEL, SDR_FLAG_MODEL_SHADOW_MAP);

	shader_flags |= SDR_FLAG_MODEL_CLIP;

	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_ANIMATED);
	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_ANIMATED | SDR_FLAG_MODEL_FOG);

	shader_flags |= SDR_FLAG_MODEL_DEFERRED;

	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT);
	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_FOG);

	shader_flags &= ~SDR_FLAG_MODEL_DEFERRED;
	shader_flags |= SDR_FLAG_MODEL_TRANSFORM;

	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_ANIMATED);
	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_ANIMATED | SDR_FLAG_MODEL_FOG);

	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT);
	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_FOG);

	shader_flags |= SDR_FLAG_MODEL_DEFERRED;

	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_ANIMATED);
	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_ANIMATED | SDR_FLAG_MODEL_FOG);

	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT);
	gr_maybe_create_shader(SDR_TYPE_MODEL, shader_flags | SDR_FLAG_MODEL_LIGHT | SDR_FLAG_MODEL_FOG);
}

//returns the number of this model
int model_load(const  char* filename, int n_subsystems, model_subsystem* subsystems, int ferror, int duplicate)
{
	int i, num;
	polymodel *pm = NULL;

	if ( !model_initted )
		model_init();

	num = -1;

	for (i=0; i< MAX_POLYGON_MODELS; i++)	{
		if ( Polygon_models[i] )	{
			if (!stricmp(filename , Polygon_models[i]->filename) && !duplicate) {
				// Model already loaded; just return.
				Polygon_models[i]->used_this_mission++;
				return Polygon_models[i]->id;
			}
		} else if ( num == -1 )	{
			// This is the first empty slot
			num = i;
		}
	}

	// No empty slot
	if ( num == -1 )	{
		Error( LOCATION, "Too many models" );
		return -1;
	}

	TRACE_SCOPE(tracing::LoadModelFile);

	mprintf(( "Loading model '%s' into slot '%i'\n", filename, num ));

	pm = new polymodel;	
	Polygon_models[num] = pm;

	pm->n_paths = 0;
	pm->paths = NULL;

	uint org_sig = static_cast<uint>(Model_signature);
	if ( org_sig + MAX_POLYGON_MODELS > INT_MAX || org_sig + MAX_POLYGON_MODELS < org_sig )	{
		Model_signature = 0; // Overflow
	} else {
		Model_signature+=MAX_POLYGON_MODELS; // No overflow
	}
	Assert( (Model_signature % MAX_POLYGON_MODELS) == 0 );
	pm->id = Model_signature + num;
	Assert( (pm->id % MAX_POLYGON_MODELS) == num );

	extern int Parse_normal_problem_count;
	Parse_normal_problem_count = 0;

	pm->used_this_mission = 0;

#ifndef NDEBUG
	char busy_text[60] = { '\0' };

	strcat_s( busy_text, "** ModelLoad: " );
	strcat_s( busy_text, filename );
	strcat_s( busy_text, " **" );

	game_busy(busy_text);
#endif

	model_read_deferred_tasks deferredTasks;

	if (read_and_process_model_file(pm, filename, n_subsystems, subsystems, ferror, deferredTasks) < 0)	{
		if (pm != NULL) {
			delete pm;
		}

		Polygon_models[num] = NULL;
		return -1;
	}

	pm->used_this_mission++;

#ifdef _DEBUG
	if(Fred_running && Parse_normal_problem_count > 0)
	{
		char buffer[100];
		sprintf(buffer,"Serious problem loading model %s, %d normals capped to zero",
			filename, Parse_normal_problem_count);
		os::dialogs::Message(os::dialogs::MESSAGEBOX_ERROR, buffer);
	}
#endif

	//=============================
	// Find the destroyed replacement models

	// Set up the default values
	for (i=0; i<pm->n_models; i++ )	{
		pm->submodel[i].my_replacement = -1;	// assume nothing replaces this
		pm->submodel[i].i_replace = -1;		// assume this doesn't replaces anything
	}

	// Search for models that have destroyed versions
	for (i=0; i<pm->n_models; i++ )	{
		int j;
		char destroyed_name[128];

		strcpy_s( destroyed_name, pm->submodel[i].name );
		strcat_s( destroyed_name, "-destroyed" );
		for (j=0; j<pm->n_models; j++ )	{
			if ( !stricmp( pm->submodel[j].name, destroyed_name ))	{
				pm->submodel[i].my_replacement = j;
				pm->submodel[j].i_replace = i;
			}
		}

		// Search for models with live debris
		// This debris comes from a destroyed subsystem when ship is still alive
		char live_debris_name[128];

		strcpy_s( live_debris_name, "debris-" );
		strcat_s( live_debris_name, pm->submodel[i].name );

		pm->submodel[i].num_live_debris = 0;
		for (j=0; j<pm->n_models; j++ ) {
			// check if current model name is substring of destroyed
			if ( strstr( pm->submodel[j].name, live_debris_name ))	{
				mprintf(( "Found live debris model for '%s'\n", pm->submodel[i].name ));
				Assert(pm->submodel[i].num_live_debris < MAX_LIVE_DEBRIS);
				pm->submodel[i].live_debris[pm->submodel[i].num_live_debris++] = j;
				pm->submodel[j].flags.set(Model::Submodel_flags::Is_live_debris);

				// make sure live debris doesn't have a parent
				pm->submodel[j].parent = -1;
			}
		}

	}

	create_family_tree(pm);

	// maybe generate vertex buffers
	create_vertex_buffer(pm, deferredTasks);

	//==============================
	// Find all the lower detail versions of the hires model
	for (i=0; i<pm->n_models; i++ )	{
		int j;
		size_t l1;
		bsp_info * sm1 = &pm->submodel[i];

		sm1->num_details = 0;
		// If a backward compatibility LOD name is declared use it
		if (sm1->lod_name[0] != '\0') {
			l1=strlen(sm1->lod_name);
		}
		// otherwise use the name for LOD comparision
		else {
			l1 = strlen(sm1->name);
		}

		for (j=0; j<pm->num_debris_objects;j++ )	{
			if ( i == pm->debris_objects[j] )	{
				sm1->flags.set(Model::Submodel_flags::Is_damaged);
			} 
		}


		for (j=0; j<MAX_MODEL_DETAIL_LEVELS; j++ )	{
			sm1->details[j] = -1;
		}

		for (j=0; j<pm->n_models; j++ )	{
			bsp_info * sm2 = &pm->submodel[j];

			if ( i==j ) continue;
			
			// if sm2 is a detail of sm1 and sm1 is a high detail, then add it to sm1's list
			if (strlen(sm2->name)!=l1) continue; 
	
			int ndiff = 0;
			size_t first_diff = 0;
			for ( size_t k=0; k<l1; k++)	{
				// If a backward compatibility LOD name is declared use it
				if (sm1->lod_name[0] != '\0') {
					if (sm1->lod_name[k] != sm2->name[k] )	{
						if (ndiff==0) first_diff = k;
						ndiff++;
					}
				}
				// otherwise do the standard LOD comparision
				else {
					if (sm1->name[k] != sm2->name[k] )	{
						if (ndiff==0) first_diff = k;
						ndiff++;
					}
				}
			}
			if (ndiff==1)	{		// They only differ by one character!
				int dl1, dl2;
				// If a backward compatibility LOD name is declared use it
				if (sm1->lod_name[0] != '\0') {
					dl1 = SCP_tolower(sm1->lod_name[first_diff]) - 'a';
				}
				// otherwise do the standard LOD comparision
				else {
					dl1 = SCP_tolower(sm1->name[first_diff]) - 'a';
				}
				dl2 = SCP_tolower(sm2->name[first_diff]) - 'a';

				// Handle LODs named "detail0/1/2/etc" too (as opposed to "detaila/b/c/etc")
				if (sm1->parent == -1 && sm2->parent == -1 && !sm1->flags[Model::Submodel_flags::Is_damaged, Model::Submodel_flags::Is_live_debris] && !sm2->flags[Model::Submodel_flags::Is_damaged, Model::Submodel_flags::Is_live_debris]) {
					dl2 = dl2 - dl1;
					dl1 = 0;
				}

				if ( (dl1<0) || (dl2<0) || (dl1>=MAX_MODEL_DETAIL_LEVELS) || (dl2>=MAX_MODEL_DETAIL_LEVELS) ) continue;	// invalid detail levels

				if ( dl1 == 0 )	{
					dl2--;	// Start from 1 up...
					if (dl2 >= sm1->num_details ) sm1->num_details = dl2+1;
					sm1->details[dl2] = j;
  				    mprintf(( "Submodel '%s' is detail level %d of '%s'\n", sm2->name, dl2 + 1, sm1->name ));
				}
			}
		}

		for (j=0; j<sm1->num_details; j++ )	{
			if ( sm1->details[j] == -1 )	{
				sm1->num_details = 0;
			}
		}

	}


	model_octant_create( pm );

	TRACE_SCOPE(tracing::ModelParseAllBSPTrees);

	for (i = 0; i < pm->n_models; ++i) {
		if (!pm->submodel[i].flags[Model::Submodel_flags::Nocollide_this_only, Model::Submodel_flags::No_collisions]) {
			pm->submodel[i].collision_tree_index = model_create_bsp_collision_tree();
			bsp_collision_tree* tree             = model_get_bsp_collision_tree(pm->submodel[i].collision_tree_index);
			model_collide_parse_bsp(tree, pm->submodel[i].bsp_data, pm->version);
		}
	}

	// Find the core_radius... the minimum of 
	float rx, ry, rz;
	rx = fl_abs( pm->submodel[pm->detail[0]].max.xyz.x - pm->submodel[pm->detail[0]].min.xyz.x );
	ry = fl_abs( pm->submodel[pm->detail[0]].max.xyz.y - pm->submodel[pm->detail[0]].min.xyz.y );
	rz = fl_abs( pm->submodel[pm->detail[0]].max.xyz.z - pm->submodel[pm->detail[0]].min.xyz.z );

	pm->core_radius = MIN( rx, MIN(ry, rz) ) / 2.0f;

	for (i=0; i<pm->n_view_positions; i++ )	{
		if ( pm->view_positions[i].parent == pm->detail[0] )	{
			float d = vm_vec_mag( &pm->view_positions[i].pnt );

			d += 0.1f;		// Make the eye 1/10th of a meter inside the sphere.

			if ( d > pm->core_radius )	{
				pm->core_radius = d;
			}		
		}
	}

	// Goober5000 - originally done in ship_create for no apparent reason
	model_set_subsys_path_nums(pm, n_subsystems, subsystems);
	model_set_bay_path_nums(pm);

	return pm->id;
}

int model_create_instance(int objnum, int model_num)
{
	Assertion(objnum >= -1 && objnum < MAX_OBJECTS, "objnum must be -1 or a valid object index!");

	// this will also run a bunch of Assertions
	auto pm = model_get(model_num);

	// go through model instances and find an empty slot
	int open_slot = -1;
	for (int i = 0; i < (int)Polygon_model_instances.size(); i++) {
		if ( !Polygon_model_instances[i] ) {
			open_slot = i;
		}
	}

	auto pmi = new polymodel_instance;
	pmi->model_num = model_num;
	pmi->objnum = objnum;

	// if not found, create a slot
	if ( open_slot < 0 ) {
		Polygon_model_instances.push_back( pmi );
		open_slot = (int)(Polygon_model_instances.size() - 1);
	} else {
		Polygon_model_instances[open_slot] = pmi;
	}
	pmi->id = open_slot;

	if (pm->n_models > 0)
		pmi->submodel = new submodel_instance[pm->n_models];

	// add intrinsic_motion instances if this model is intrinsic-moving
	if (pm->flags & PM_FLAG_HAS_INTRINSIC_MOTION) {
		intrinsic_motion motion(objnum >= 0, open_slot);

		for (int i = 0; i < pm->n_models; i++) {
			if (pm->submodel[i].rotation_type == MOVEMENT_TYPE_INTRINSIC) {
				// note: dumb_turn_rate will be 0.0f for look_at
				motion.add_submodel(i, &pmi->submodel[i], pm->submodel[i].default_turn_rate);
			}
		}

		if (motion.submodel_list.empty()) {
			Assertion(!motion.submodel_list.empty(), "This model has the PM_FLAG_HAS_INTRINSIC_MOTION flag; why doesn't it have an intrinsic-moving submodel?");
		} else {
			Intrinsic_motions.insert(std::make_pair(pmi->id, std::move(motion)));
		}
	}

	return open_slot;
}

void model_delete_instance(int model_instance_num)
{
	Assert(model_instance_num >= 0);
	Assert(model_instance_num < (int)Polygon_model_instances.size());
	Assert(Polygon_model_instances[model_instance_num] != nullptr);

	polymodel_instance *pmi = Polygon_model_instances[model_instance_num];

	animation::ModelAnimationSet::stopAnimations(pmi);

	if ( pmi->submodel ) {
		delete[] pmi->submodel;
		pmi->submodel = nullptr;
	}

	delete pmi;

	Polygon_model_instances[model_instance_num] = nullptr;

	// delete intrinsic motions associated with this instance
	Intrinsic_motions.erase(model_instance_num);
}

// ensure that the subsys path is at least SUBSYS_PATH_DIST from the 
// second last to last point.
void model_maybe_fixup_subsys_path(polymodel *pm, int path_num)
{
	vec3d	*v1, *v2, dir;
	float	dist;
	int		index_1, index_2;

	Assert( (path_num >= 0) && (path_num < pm->n_paths) );

	model_path *mp;
	mp = &pm->paths[path_num];

	Assert(mp != NULL);
	if (mp->nverts <= 1 ) {
		Error(LOCATION, "Subsystem Path (%s) Parent (%s) in model (%s) has less than 2 vertices/points!", mp->name, mp->parent_name, pm->filename);
	}
	
	index_1 = 1;
	index_2 = 0;

	v1 = &mp->verts[index_1].pos;
	v2 = &mp->verts[index_2].pos;
	
	dist = vm_vec_dist(v1, v2);
	if (dist < (SUBSYS_PATH_DIST - 10))
	{
		vm_vec_normalized_dir(&dir, v2, v1);
		vm_vec_scale_add(v2, v1, &dir, SUBSYS_PATH_DIST);
	}
}

// fill in the path_num field inside the model_subsystem struct.  This is an index into
// the pm->paths[] array, which is a path that provides a frontal approach to a subsystem
// (used for attacking purposes)
//
// NOTE: path_num in model_subsystem has the follows the following convention:
//			> 0	=> index into pm->paths[] for model that subsystem sits on
//			-1		=> path is not yet determined (may or may not exist)
//			-2		=> path doesn't yet exist for this subsystem
void model_set_subsys_path_nums(polymodel *pm, int n_subsystems, model_subsystem *subsystems)
{
	int i, j;

	for (i = 0; i < n_subsystems; i++)
		subsystems[i].path_num = -1;

	for (i = 0; i < n_subsystems; i++)
	{
		for (j = 0; j < pm->n_paths; j++)
		{
			if ( ((subsystems[i].subobj_num != -1) && (subsystems[i].subobj_num == pm->paths[j].parent_submodel)) ||
				(!subsystem_stricmp(subsystems[i].subobj_name, pm->paths[j].parent_name)) )
			{
				if (pm->n_paths > j)
				{
					subsystems[i].path_num = j;
					model_maybe_fixup_subsys_path(pm, j);

					break;
				}
			}
		}

		// If a path num wasn't located, then set value to -2
		if (subsystems[i].path_num == -1)
			subsystems[i].path_num = -2;
	}
}

// Determine the path indices (indicies into pm->paths[]) for the paths used for approaching/departing
// a fighter bay on a capital ship.
void model_set_bay_path_nums(polymodel *pm)
{
	int i;

	if (pm->ship_bay != NULL)
	{
		vm_free(pm->ship_bay);
		pm->ship_bay = NULL;
	}

	/*
	// currently only capital ships have fighter bays
	if ( !(sip->is_big_or_huge()) ) {
		return;
	}
	*/

	// malloc out storage for the path information
	pm->ship_bay = (ship_bay *) vm_malloc(sizeof(ship_bay));
	Assert(pm->ship_bay != NULL);

	pm->ship_bay->num_paths = 0;
	// TODO: determine if zeroing out here is affecting any earlier initializations
	pm->ship_bay->arrive_flags = 0;	// bitfield, set to 1 when that path number is reserved for an arrival
	pm->ship_bay->depart_flags = 0;	// bitfield, set to 1 when that path number is reserved for a departure

	// sanity part 1
	memset(pm->ship_bay->path_indexes, -1, MAX_SHIP_BAY_PATHS * sizeof(int));

	// iterate through the paths that exist in the polymodel, searching for $bayN pathnames
	bool too_many_paths = false;
	for (i = 0; i < pm->n_paths; i++)
	{
		if (!strnicmp(pm->paths[i].name, NOX("$bay"), 4))
		{
			int bay_num;
			char temp[3];

			strncpy(temp, pm->paths[i].name + 4, 2);
			temp[2] = 0;
			bay_num = atoi(temp);

			if (bay_num < 1 || bay_num > MAX_SHIP_BAY_PATHS)
			{
				if(bay_num > MAX_SHIP_BAY_PATHS)
				{
					too_many_paths = true;
				}
				if(bay_num < 1)
				{
					Warning(LOCATION, "Model '%s' bay path '%s' index '%d' has an invalid bay number of %d", pm->filename, pm->paths[i].name, i, bay_num);
				}
				continue;
			}

			pm->ship_bay->path_indexes[bay_num - 1] = i;
			pm->ship_bay->num_paths++;
		}
	}
	if(too_many_paths)
	{
		Warning(LOCATION, "Model '%s' has too many bay paths - max is %d", pm->filename, MAX_SHIP_BAY_PATHS);
	}

	// sanity part 2
	for (i = 0; i < pm->ship_bay->num_paths; i++)
	{
		if (pm->ship_bay->path_indexes[i] < 0)
		{
			Warning(LOCATION, "Model '%s' does not have a '$bay%.2d' path specified!  A total of %d bay paths were counted.  Either there is a gap in the path sequence, or a path has a duplicate name.", pm->filename, i + 1, pm->ship_bay->num_paths);
			pm->ship_bay->path_indexes[i] = 0;	// avoid crashes
		}
	}
}

// Get "parent" submodel for live debris submodel
int model_get_parent_submodel_for_live_debris( int model_num, int live_debris_model_num )
{
	polymodel *pm = model_get(model_num);

	Assert(pm->submodel[live_debris_model_num].flags[Model::Submodel_flags::Is_live_debris]);

	int mn;
	bsp_info *child;

	// Start with the high level of detail hull 
	// Check all its children until we find the submodel to which the live debris belongs
	child = &pm->submodel[pm->detail[0]];
	mn = child->first_child;

	while (mn > 0) {
		child = &pm->submodel[mn];

		if (child->num_live_debris > 0) {
			// check all live debris submodels for the current child
			for (int idx=0; idx<child->num_live_debris; idx++) {
				if (child->live_debris[idx] == live_debris_model_num) {
					return mn;
				}
			}
			// DKA 5/26/99: can multiple live debris subsystems with each ship
			// NO LONGER TRUE Can only be 1 submodel with live debris
			// Error( LOCATION, "Could not find parent submodel for live debris.  Possible model error");
		}

		// get next child
		mn = child->next_sibling;
	}
	Error( LOCATION, "Could not find parent submodel for live debris");
	return -1;
}


float model_get_radius( int modelnum )
{
	polymodel *pm;

	pm = model_get(modelnum);

	return pm->rad;
}

float model_get_core_radius( int modelnum )
{
	polymodel *pm;

	pm = model_get(modelnum);

	return pm->core_radius;
}

float submodel_get_radius( int modelnum, int submodelnum )
{
	polymodel *pm;

	pm = model_get(modelnum);

	return pm->submodel[submodelnum].rad;
}



polymodel * model_get(int model_num)
{
	if ( model_num < 0 ) {
		Warning(LOCATION, "Invalid model number %d requested. Please post the call stack where an SCP coder can see it.\n", model_num);
		return NULL;
	}

	int num = model_num % MAX_POLYGON_MODELS;
	
	Assertion( num >= 0, "Model id %d is invalid. Please backtrace and investigate.\n", num);
	Assertion( num < MAX_POLYGON_MODELS, "Model id %d is larger than MAX_POLYGON_MODELS (%d). This is impossible, thus we have to conclude that math as we know it has ceased to work.\n", num, MAX_POLYGON_MODELS );
	Assertion( Polygon_models[num], "No model with id %d found. Please backtrace and investigate.\n", num );
	Assertion( Polygon_models[num]->id == model_num, "Index collision between model %s and requested model %d. Please backtrace and investigate.\n", Polygon_models[num]->filename, model_num );

	if (num < 0 || num > MAX_POLYGON_MODELS || !Polygon_models[num] || Polygon_models[num]->id != model_num)
		return NULL;

	return Polygon_models[num];
}

polymodel_instance* model_get_instance(int model_instance_num)
{
	Assert( model_instance_num >= 0 );
	Assert( model_instance_num < (int)Polygon_model_instances.size() );
	if ( model_instance_num < 0 || model_instance_num >= (int)Polygon_model_instances.size() ) {
		return NULL;
	} 

	return Polygon_model_instances[model_instance_num];
}

// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
int model_find_2d_bound_min(int model_num,matrix *orient, vec3d * pos,int *x1, int *y1, int *x2, int *y2 )
{
	int n_valid_pts;
	int i, x,y,min_x, min_y, max_x, max_y;
	int rval = 0;

	polymodel* pm = model_get(model_num);

	g3_start_instance_matrix(pos,orient,false);
	
	n_valid_pts = 0;

	min_x = min_y = max_x = max_y = 0;

	for (i=0; i<8; i++ )	{
		vertex pt;
		ubyte flags;

		flags = g3_rotate_vertex(&pt,&pm->bounding_box[i]);
		if ( !(flags&CC_BEHIND) ) {
			g3_project_vertex(&pt);

			if (!(pt.flags & PF_OVERFLOW)) {
				x = fl2i(pt.screen.xyw.x);
				y = fl2i(pt.screen.xyw.y);
				if ( n_valid_pts == 0 )	{
					min_x = x;
					min_y = y;
					max_x = x;
					max_y = y;
				} else {
					if ( x < min_x ) min_x = x;
					if ( y < min_y ) min_y = y;

					if ( x > max_x ) max_x = x;
					if ( y > max_y ) max_y = y;
				}
				n_valid_pts++;
			}
		}
	}

	if ( n_valid_pts < 8 )	{
		rval = 2;
	}

	if (x1) *x1 = min_x;
	if (y1) *y1 = min_y;

	if (x2) *x2 = max_x;
	if (y2) *y2 = max_y;

	g3_done_instance(false);

	return rval;
}


// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
int submodel_find_2d_bound_min(int model_num,int submodel, matrix *orient, vec3d * pos,int *x1, int *y1, int *x2, int *y2 )
{
	polymodel * po;
	int n_valid_pts;
	int i, x,y,min_x, min_y, max_x, max_y;
	bsp_info * sm;

	po = model_get(model_num);
	if ( (submodel < 0) || (submodel >= po->n_models ) ) return 1;
	sm = &po->submodel[submodel];
	
	g3_start_instance_matrix(pos,orient,false);
	
	n_valid_pts = 0;

	min_x = min_y = max_x = max_y = 0;

	for (i=0; i<8; i++ )	{
		vertex pt;
		ubyte flags;

		flags = g3_rotate_vertex(&pt,&sm->bounding_box[i]);
		if ( !(flags&CC_BEHIND) ) {
			g3_project_vertex(&pt);

			if (!(pt.flags & PF_OVERFLOW)) {
				x = fl2i(pt.screen.xyw.x);
				y = fl2i(pt.screen.xyw.y);
				if ( n_valid_pts == 0 )	{
					min_x = x;
					min_y = y;
					max_x = x;
					max_y = y;
				} else {
					if ( x < min_x ) min_x = x;
					if ( y < min_y ) min_y = y;

					if ( x > max_x ) max_x = x;
					if ( y > max_y ) max_y = y;
				}
				n_valid_pts++;
			}
		}
	}

	if ( n_valid_pts == 0 )	{
		return 2;
	}

	if (x1) *x1 = min_x;
	if (y1) *y1 = min_y;

	if (x2) *x2 = max_x;
	if (y2) *y2 = max_y;

	g3_done_instance(false);

	return 0;
}

/**
 * Find 2D bound for sub object
 *
 * Note that x1,y1,x2,y2 aren't clipped to 2D screen coordinates.
 *
 * Calculates the focal length of the camera, and uses the law of similar
 * triangles to project the subsystem's radius to the screen.
 *
 * @return zero if x1,y1,x2,y2 are valid
 * @return 2 for point offscreen
 */
int subobj_find_2d_bound(float radius ,matrix * /*orient*/, vec3d * pos,int *x1, int *y1, int *x2, int *y2 )
{
	float w,h,focal_length;
	vertex pnt;

	g3_rotate_vertex(&pnt,pos);

	if ( pnt.flags & CC_BEHIND ) 
		return 2;

	if (!(pnt.flags&PF_PROJECTED))
		g3_project_vertex(&pnt);

	if (pnt.flags & PF_OVERFLOW)
		return 2;

	focal_length = Canv_h2 * Matrix_scale.xyz.y;
	h = radius * focal_length / pnt.world.xyz.z;

	w = h;

	if (x1) *x1 = fl2i(pnt.screen.xyw.x - w);
	if (y1) *y1 = fl2i(pnt.screen.xyw.y - h);

	if (x2) *x2 = fl2i(pnt.screen.xyw.x + w);
	if (y2) *y2 = fl2i(pnt.screen.xyw.y + h);

	return 0;
}


// Given a rotating submodel, find the local and world axes of rotation.
void model_get_rotating_submodel_axis(vec3d *model_axis, vec3d *world_axis, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, matrix *objorient)
{
	Assert(pm->id == pmi->model_num);
	bsp_info *sm = &pm->submodel[submodel_num];
	Assert(sm->rotation_type == MOVEMENT_TYPE_REGULAR || sm->rotation_type == MOVEMENT_TYPE_INTRINSIC || sm->rotation_type == MOVEMENT_TYPE_TRIGGERED);

	*model_axis = sm->rotation_type == MOVEMENT_TYPE_TRIGGERED ? pmi->submodel[submodel_num].rotation_axis : sm->rotation_axis;
	model_instance_local_to_global_dir(world_axis, model_axis, pm, pmi, submodel_num, objorient);
}

// Normalize the submodel angle and convert float angle to angles struct
void submodel_canonicalize_rotation(bsp_info *sm, submodel_instance *smi, bool clamp)
{
	smi->canonical_prev_orient = smi->canonical_orient;

	if (clamp)
	{
		// normalize the angle so that we are within a valid range:
		//  greater than or equal to 0
		//  less than PI2
		while (smi->cur_angle > PI2)
			smi->cur_angle -= PI2;
		while (smi->cur_angle < 0.0f)
			smi->cur_angle += PI2;
	}

	// get the matrix and the angles
	switch (sm->rotation_axis_id)
	{
		case MOVEMENT_AXIS_X:
		{
			angles angs = vmd_zero_angles;
			angs.p = smi->cur_angle;
			vm_angles_2_matrix(&smi->canonical_orient, &angs);
			break;
		}

		case MOVEMENT_AXIS_Y:
		{
			angles angs = vmd_zero_angles;
			angs.h = smi->cur_angle;
			vm_angles_2_matrix(&smi->canonical_orient, &angs);
			break;
		}

		case MOVEMENT_AXIS_Z:
		{
			angles angs = vmd_zero_angles;
			angs.b = smi->cur_angle;
			vm_angles_2_matrix(&smi->canonical_orient, &angs);
			break;
		}

		default:
			vm_quaternion_rotate(&smi->canonical_orient, smi->cur_angle, &sm->rotation_axis);
			break;
	}
}

// Convert float displacement to vector, but no normalization (clamping) is needed
void submodel_canonicalize_translation(bsp_info *sm, submodel_instance *smi)
{
	smi->canonical_prev_offset = smi->canonical_offset;

	// get the vector
	switch (sm->rotation_axis_id)
	{
		case MOVEMENT_AXIS_X:
			vm_vec_copy_scale(&smi->canonical_offset, &vmd_x_vector, smi->cur_offset);
			break;

		case MOVEMENT_AXIS_Y:
			vm_vec_copy_scale(&smi->canonical_offset, &vmd_y_vector, smi->cur_offset);
			break;

		case MOVEMENT_AXIS_Z:
			vm_vec_copy_scale(&smi->canonical_offset, &vmd_z_vector, smi->cur_offset);
			break;

		default:
			vm_vec_copy_scale(&smi->canonical_offset, &sm->translation_axis, smi->cur_offset);
			break;
	}
}

// Does stepped rotation of a submodel
void submodel_stepped_rotate(model_subsystem *psub, submodel_instance *smi)
{
	Assert(psub->flags[Model::Subsystem_Flags::Stepped_rotate]);

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	bsp_info *sm = &pm->submodel[psub->subobj_num];

	if ( sm->rotation_type != MOVEMENT_TYPE_REGULAR ) return;

	if (!smi->stepped_rotation_started.isValid())
		smi->stepped_rotation_started = _timestamp();

	float elapsed_time = timestamp_since(smi->stepped_rotation_started) / static_cast<float>(MILLISECONDS_PER_SECOND);

	// save last angles
	smi->prev_angle = smi->cur_angle;

	// angular displacement of one step
	float step_size = (PI2 / psub->stepped_rotation->num_steps);

	// get time to complete one step, including pause
	float step_time = psub->stepped_rotation->t_transit + psub->stepped_rotation->t_pause;

	// step_offset_time is TIME into current step
	float step_offset_time = static_cast<float>(fmod(elapsed_time, step_time));

	// get step we are on (round down)
	int cur_step = static_cast<int>(elapsed_time / step_time);

	// get base angle
	smi->cur_angle = (cur_step % psub->stepped_rotation->num_steps) * step_size;

	// determine which phase of rotation we're in
	float coast_start_time = psub->stepped_rotation->fraction * psub->stepped_rotation->t_transit;
	float decel_start_time = psub->stepped_rotation->t_transit * (1.0f - psub->stepped_rotation->fraction);
	float pause_start_time = psub->stepped_rotation->t_transit;

	float start_coast_angle = 0.5f * psub->stepped_rotation->max_turn_accel * coast_start_time * coast_start_time;

	if (step_offset_time < coast_start_time) {
		// do accel
		float accel_time = step_offset_time;
		smi->cur_angle += 0.5f * psub->stepped_rotation->max_turn_accel * accel_time * accel_time;
		smi->current_turn_rate = psub->stepped_rotation->max_turn_accel * accel_time;
	} else if (step_offset_time < decel_start_time) {
		// do coast
		float coast_time = step_offset_time - coast_start_time;
		smi->cur_angle += start_coast_angle + psub->stepped_rotation->max_turn_rate * coast_time;
		smi->current_turn_rate = psub->stepped_rotation->max_turn_rate;
	} else if (step_offset_time < pause_start_time) {
		// do decel
		float time_to_pause = psub->stepped_rotation->t_transit - step_offset_time;
		smi->cur_angle += (step_size - 0.5f * psub->stepped_rotation->max_turn_accel * time_to_pause * time_to_pause);
		smi->current_turn_rate = psub->stepped_rotation->max_turn_rate * time_to_pause;
	} else {
		// do pause
		smi->cur_angle += step_size;
		smi->current_turn_rate = 0.0f;
	}

	// if we're going backwards, flip the whole thing
	if (psub->stepped_rotation->backwards) {
		smi->cur_angle *= -1.0f;
	}

	submodel_canonicalize_rotation(sm, smi, true);
}

// Does stepped translation of a submodel
void submodel_stepped_translate(model_subsystem *psub, submodel_instance *smi)
{
	Assert(psub->flags[Model::Subsystem_Flags::Stepped_translate]);

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	bsp_info *sm = &pm->submodel[psub->subobj_num];

	if ( sm->translation_type != MOVEMENT_TYPE_REGULAR ) return;

	if (!smi->stepped_translation_started.isValid())
		smi->stepped_translation_started = _timestamp();

	float elapsed_time = timestamp_since(smi->stepped_translation_started) / static_cast<float>(MILLISECONDS_PER_SECOND);

	// save last offset
	smi->prev_offset = smi->cur_offset;

	// linear displacement of one step
	float step_size = psub->stepped_translation->step_distance;

	// get time to complete one step, including pause
	float step_time = psub->stepped_translation->t_transit + psub->stepped_translation->t_pause;

	// step_offset_time is TIME into current step
	float step_offset_time = static_cast<float>(fmod(elapsed_time, step_time));

	// get step we are on (round down)
	int cur_step = static_cast<int>(elapsed_time / step_time);

	// set base displacement to 0 for now
	smi->cur_offset = 0.0f;

	// determine which phase of translation we're in
	float coast_start_time = psub->stepped_translation->fraction * psub->stepped_translation->t_transit;
	float decel_start_time = psub->stepped_translation->t_transit * (1.0f - psub->stepped_translation->fraction);
	float pause_start_time = psub->stepped_translation->t_transit;

	float start_coast_dist = 0.5f * psub->stepped_translation->max_shift_accel * coast_start_time * coast_start_time;

	if (step_offset_time < coast_start_time) {
		// do accel
		float accel_time = step_offset_time;
		smi->cur_offset += 0.5f * psub->stepped_translation->max_shift_accel * accel_time * accel_time;
		smi->current_shift_rate = psub->stepped_translation->max_shift_accel * accel_time;
	} else if (step_offset_time < decel_start_time) {
		// do coast
		float coast_time = step_offset_time - coast_start_time;
		smi->cur_offset += start_coast_dist + psub->stepped_translation->max_shift_rate * coast_time;
		smi->current_shift_rate = psub->stepped_translation->max_shift_rate;
	} else if (step_offset_time < pause_start_time) {
		// do decel
		float time_to_pause = psub->stepped_translation->t_transit - step_offset_time;
		smi->cur_offset += (step_size - 0.5f * psub->stepped_translation->max_shift_accel * time_to_pause * time_to_pause);
		smi->current_shift_rate = psub->stepped_translation->max_shift_rate * time_to_pause;
	} else {
		// do pause
		smi->cur_offset += step_size;
		smi->current_shift_rate = 0.0f;
	}

	// set correct displacement depending on whether we are alternating or moving continuously
	if (psub->stepped_translation->reverse_after_step) {
		if (cur_step % 2 == 1) {
			smi->cur_offset = step_size - smi->cur_offset;
		}
	} else {
		smi->cur_offset += cur_step * step_size;
	}

	// if we're going backwards, flip the whole thing
	if (psub->stepped_translation->backwards) {
		smi->cur_offset *= -1.0f;
	}

	submodel_canonicalize_translation(sm, smi);
}

// Instantly rotate a submodel (around its axis of rotation) so that it is oriented toward its look_at_submodel.
// Uses the same pointing logic as in model_rotate_gun
void submodel_look_at(polymodel *pm, polymodel_instance *pmi, int submodel_num)
{
	vec3d world_axis, world_pos, dst, planar_dst, dir, rotated_vec;

	auto sm = &pm->submodel[submodel_num];
	auto smi = &pmi->submodel[submodel_num];

	Assert(sm->rotation_type == MOVEMENT_TYPE_INTRINSIC);
	Assert(sm->look_at_submodel >= 0);

	// save last angles
	smi->prev_angle = smi->cur_angle;
	smi->canonical_prev_orient = smi->canonical_orient;

	//------------
	// Calculate the destination point in world coordinates
	model_instance_local_to_global_point(&dst, &vmd_zero_vector, pm, pmi, sm->look_at_submodel, &vmd_identity_matrix, &vmd_zero_vector);

	//------------
	// Project the destination point onto the submodel base plane
	model_instance_local_to_global_dir(&world_axis, &sm->rotation_axis, pm, pmi, sm->parent, &vmd_identity_matrix);
	model_instance_local_to_global_point(&world_pos, &vmd_zero_vector, pm, pmi, submodel_num, &vmd_identity_matrix, &vmd_zero_vector);

	vm_project_point_onto_plane(&planar_dst, &dst, &world_axis, &world_pos);

	//------------
	// Calculate angle to rotate towards projected point
	model_instance_local_to_global_dir(&rotated_vec, &sm->frame_of_reference.vec.fvec, pm, pmi, sm->parent, &vmd_identity_matrix);
	vm_vec_sub(&dir, &planar_dst, &world_pos);
	vm_vec_normalize(&dir);
	smi->cur_angle = vm_vec_delta_ang_norm(&rotated_vec, &dir, &world_axis);

	// apply an offset to the angle, since the direction we look at may be different than the default orientation!
	// if we have not specified an offset in the POF, assume that the very first time we call submodel_look_at, the submodel is pointing in the correct direction
	if (sm->look_at_offset < 0.0f)
	{
		sm->look_at_offset = -(smi->cur_angle);

		// ensure the offset is in the proper range (see submodel_canonicalize_rotation)
		while (sm->look_at_offset > PI2)
			sm->look_at_offset -= PI2;
		while (sm->look_at_offset < 0.0f)
			sm->look_at_offset += PI2;
	}
	smi->cur_angle += sm->look_at_offset;

	// calculate turn rate
	// (try to avoid a one-frame dramatic spike in the turn rate if the angle passes 0.0 or PI2)
	if (abs(smi->cur_angle - smi->prev_angle) < PI)
		smi->current_turn_rate = smi->desired_turn_rate = (smi->cur_angle - smi->prev_angle) / flFrametime;

	// and now set the other submodel fields
	submodel_canonicalize_rotation(sm, smi, true);
}

// Rotates the angle of a submodel, when the submodel has a subsystem (which is almost always the case)
void submodel_rotate(model_subsystem *psub, submodel_instance *smi)
{
	bsp_info * sm;

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	sm = &pm->submodel[psub->subobj_num];

	if ( sm->rotation_type != MOVEMENT_TYPE_REGULAR ) return;

	submodel_rotate(sm, smi);
}

// Translates the offset of a submodel, when the submodel has a subsystem
void submodel_translate(model_subsystem *psub, submodel_instance *smi)
{
	bsp_info * sm;

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	sm = &pm->submodel[psub->subobj_num];

	if ( sm->translation_type != MOVEMENT_TYPE_REGULAR ) return;

	submodel_translate(sm, smi);
}

// Helper function for both rotation and translation
void submodel_movement_calc(float &prev_value, float &cur_value, float &current_rate, float desired_rate, float accel, bool instant_accel)
{
	// save last value
	prev_value = cur_value;

	float delta;

	if (instant_accel) {
		delta = desired_rate * flFrametime;
		current_rate = desired_rate;
	} else {
		// probably send in a calculated desired rate
		float diff = desired_rate - current_rate;

		float final_rate;
		if (diff > 0) {
			final_rate = current_rate + accel * flFrametime;
			if (final_rate > desired_rate) {
				final_rate = desired_rate;
			}
		} else if (diff < 0) {
			final_rate = current_rate - accel * flFrametime;
			if (final_rate < desired_rate) {
				final_rate = desired_rate;
			}
		} else {
			final_rate = desired_rate;
		}

		delta = (current_rate + final_rate) * 0.5f * flFrametime;
		current_rate = final_rate;
	}

	// Apply movement
	cur_value += delta;
}
void submodel_rotate(bsp_info *sm, submodel_instance *smi)
{
	submodel_movement_calc(smi->prev_angle, smi->cur_angle, smi->current_turn_rate, smi->desired_turn_rate, smi->turn_accel, sm->flags[Model::Submodel_flags::Instant_rotate_accel]);
	submodel_canonicalize_rotation(sm, smi, true);
}

void submodel_translate(bsp_info *sm, submodel_instance *smi)
{
	submodel_movement_calc(smi->prev_offset, smi->cur_offset, smi->current_shift_rate, smi->desired_shift_rate, smi->shift_accel, sm->flags[Model::Submodel_flags::Instant_translate_accel]);
	submodel_canonicalize_translation(sm, smi);
}

// Tries to move joints so that the turret points to the point dst.
// turret1 is the angles of the turret, turret2 is the angles of the gun from turret
//	Returns 1 if rotated gun, 0 if no gun to rotate (rotation handled by AI)
int model_rotate_gun(object *objp, polymodel *pm, polymodel_instance *pmi, ship_subsys *ss, vec3d *dst, bool reset)
{
	model_subsystem *turret = ss->system_info;

	// This should not happen
	if ( turret->turret_gun_sobj < 0 || turret->subobj_num == turret->turret_gun_sobj ) {
		return 0;
	}

	auto base_sm = &pm->submodel[turret->subobj_num];
	auto gun_sm = &pm->submodel[turret->turret_gun_sobj];

	auto base_smi = &pmi->submodel[turret->subobj_num];
	auto gun_smi = &pmi->submodel[turret->turret_gun_sobj];

	bool limited_base_rotation = false;

	// Check for a valid turret
	Assert( turret->turret_num_firing_points > 0 );
	// Check for a valid subsystem
	Assert( ss != NULL );

	// Find the heading and pitch that the gun needs to turn to
	float desired_base_angle, desired_gun_angle;

	if (!reset) {
		vec3d world_axis, world_pos, planar_dst, dir, rotated_vec;
		matrix save_base_orient;

		// NOTE: this code assumes that the turret's fvec is where the base should point and the uvec is where the gun should point

		//------------
		// Project the destination point onto the turret base plane
		model_instance_local_to_global_dir(&world_axis, &base_sm->rotation_axis, pm, pmi, base_sm->parent, &objp->orient);
		model_instance_local_to_global_point(&world_pos, &vmd_zero_vector, pm, pmi, turret->subobj_num, &objp->orient, &objp->pos);

		vm_project_point_onto_plane(&planar_dst, dst, &world_axis, &world_pos);

		//------------
		// Calculate base angle to rotate towards projected point
		model_instance_local_to_global_dir(&rotated_vec, &base_sm->frame_of_reference.vec.fvec, pm, pmi, base_sm->parent, &objp->orient);
		vm_vec_sub(&dir, &planar_dst, &world_pos);
		vm_vec_normalize(&dir);
		desired_base_angle = vm_vec_delta_ang_norm(&rotated_vec, &dir, &world_axis);

		//------------
		// Pretend the base is pointing directly at the target
		save_base_orient = base_smi->canonical_orient;
		vm_quaternion_rotate(&base_smi->canonical_orient, desired_base_angle, &base_sm->rotation_axis);

		//------------
		// Project the destination point onto the turret gun plane with the base in the desired orientation
		// NOTE: the rotation axis is given in the model's reference frame, so it needs to be rotated when the base is rotated
		model_instance_local_to_global_dir(&world_axis, &gun_sm->rotation_axis, pm, pmi, gun_sm->parent, &objp->orient);
		model_instance_local_to_global_point(&world_pos, &vmd_zero_vector, pm, pmi, turret->turret_gun_sobj, &objp->orient, &objp->pos);

		vm_project_point_onto_plane(&planar_dst, dst, &world_axis, &world_pos);

		//------------
		// Calculate gun angle to rotate towards projected point
		model_instance_local_to_global_dir(&rotated_vec, &gun_sm->frame_of_reference.vec.uvec, pm, pmi, gun_sm->parent, &objp->orient);
		vm_vec_sub(&dir, &planar_dst, &world_pos);
		vm_vec_normalize(&dir);
		desired_gun_angle = vm_vec_delta_ang_norm(&rotated_vec, &dir, &world_axis);
		// for ventral turrets without custom matrixes
		if (vm_vec_dot(&gun_sm->frame_of_reference.vec.uvec, &turret->turret_norm) < 0.0f) {
			desired_gun_angle = PI + desired_gun_angle;
		}

		//------------
		// Restore the base
		base_smi->canonical_orient = save_base_orient;

	} else {
		desired_base_angle = base_smi->turret_idle_angle;
		desired_gun_angle = 0.0f;

		if ((turret->subobj_num != turret->turret_gun_sobj)) {
			desired_gun_angle = gun_smi->turret_idle_angle;
		}
	}

	if (turret->flags[Model::Subsystem_Flags::Turret_base_restricted_fov])
		limited_base_rotation = true;

	//------------
	// Gradually turn the turret towards the desired angles
	float step_size = turret->turret_turning_rate * flFrametime;
	float base_delta, gun_delta;

	if (reset)
		step_size /= 3.0f;
	else
		ss->rotation_timestamp = timestamp(turret->turret_reset_delay);

	base_delta = vm_interp_angle(&base_smi->cur_angle, desired_base_angle, step_size, limited_base_rotation);
	gun_delta = vm_interp_angle(&gun_smi->cur_angle, desired_gun_angle, step_size);

	submodel_canonicalize_rotation(base_sm, base_smi, true);
	submodel_canonicalize_rotation(gun_sm, gun_smi, true);

	//------------
	// Set fields for turret rotation sounds

	ss->base_rotation_rate_pct = 0.0f;
	ss->gun_rotation_rate_pct = 0.0f;

	if (turret->turret_base_rotation_snd.isValid())
	{
		if (step_size > 0)
		{
			base_delta = (float) (fabs(base_delta)) / step_size;
			if (base_delta > 1.0f)
				base_delta = 1.0f;
			ss->base_rotation_rate_pct = base_delta;
		}
	}

	if (turret->turret_gun_rotation_snd.isValid())
	{
		if (step_size > 0)
		{
			gun_delta = (float) (fabs(gun_delta)) / step_size;
			if (gun_delta > 1.0f)
				gun_delta = 1.0f;
			ss->gun_rotation_rate_pct = gun_delta;
		}
	}

//	base_angles->h -= step_size*(key_down_timef(KEY_1)-key_down_timef(KEY_2) );
//	gun_angles->p += step_size*(key_down_timef(KEY_3)-key_down_timef(KEY_4) );

	if (turret->flags[Model::Subsystem_Flags::Fire_on_target])
	{
		base_delta = vm_delta_from_interp_angle(base_smi->cur_angle, desired_base_angle);
		gun_delta = vm_delta_from_interp_angle(gun_smi->cur_angle, desired_gun_angle);
		ss->points_to_target = sqrt((base_delta*base_delta) + (gun_delta*gun_delta));
	}

	return 1;
}


// Goober5000
// For a submodel, return its overall offset from the main model.
void model_find_submodel_offset(vec3d *outpnt, const polymodel *pm, int submodel_num)
{
	model_local_to_global_point(outpnt, &vmd_zero_vector, pm, submodel_num);
}

void model_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, int model_num, int submodel_num, const matrix *objorient, const vec3d *objpos)
{
	return model_local_to_global_point(outpnt, mpnt, model_get(model_num), submodel_num, objorient, objpos);
}

void model_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, const polymodel *pm, int submodel_num, const matrix *objorient, const vec3d *objpos)
{
	vec3d pnt;
	int mn;

	pnt = *mpnt;
	mn = submodel_num;

	//instance up the tree for this point
	while ( (mn >= 0) && (pm->submodel[mn].parent >= 0) ) {
		// the angles in non-instanced models are always zero, so no need to rotate
		// and no need to translate, for the same reason
		vm_vec_add2(&pnt, &pm->submodel[mn].offset);

		mn = pm->submodel[mn].parent;
	}

	//now instance for the entire object
	if (objorient && objpos) {
		vm_vec_unrotate(outpnt, &pnt, objorient);
		vm_vec_add2(outpnt, objpos);
	} else {
		*outpnt = pnt;
	}
}

void model_instance_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, int model_instance_num, int submodel_num, const matrix *objorient, const vec3d *objpos, bool use_last_frame)
{
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);
	return model_instance_local_to_global_point(outpnt, mpnt, pm, pmi, submodel_num, objorient, objpos, use_last_frame);
}

void model_instance_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient, const vec3d *objpos, bool use_last_frame)
{
	vec3d pnt;
	vec3d tpnt;
	int mn;
	Assert(pm->id == pmi->model_num);

	pnt = *mpnt;
	mn = submodel_num;

	//instance up the tree for this point
	while ( (mn >= 0) && (pm->submodel[mn].parent >= 0) ) {
		vm_vec_unrotate(&tpnt, &pnt, use_last_frame ? &pmi->submodel[mn].canonical_prev_orient : &pmi->submodel[mn].canonical_orient);
		vm_vec_add(&pnt, &tpnt, use_last_frame ? &pmi->submodel[mn].canonical_prev_offset : &pmi->submodel[mn].canonical_offset);
		vm_vec_add2(&pnt, &pm->submodel[mn].offset);

		mn = pm->submodel[mn].parent;
	}

	//now instance for the entire object
	if (objorient && objpos) {
		vm_vec_unrotate(outpnt, &pnt, objorient);
		vm_vec_add2(outpnt, objpos);
	} else {
		*outpnt = pnt;
	}
}

void model_instance_local_to_global_point_dir(vec3d *out_pnt, vec3d *out_dir, const vec3d *in_pnt, const vec3d *in_dir, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient, const vec3d *objpos)
{
	vec3d pnt, tpnt, dir, tdir;
	int mn;
	Assert(pm->id == pmi->model_num);

	pnt = *in_pnt;
	dir = *in_dir;
	mn = submodel_num;

	// instance up the tree for this point
	while ( (mn >= 0) && (pm->submodel[mn].parent >= 0) ) {
		vm_vec_unrotate(&tpnt, &pnt, &pmi->submodel[mn].canonical_orient);
		vm_vec_add(&pnt, &tpnt, &pmi->submodel[mn].canonical_offset);
		vm_vec_add2(&pnt, &pm->submodel[mn].offset);

		vm_vec_unrotate(&tdir, &dir, &pmi->submodel[mn].canonical_orient);
		dir = tdir;

		mn = pm->submodel[mn].parent;
	}

	// now instance for the entire object
	if (objorient && objpos) {
		vm_vec_unrotate(out_pnt, &pnt, objorient);
		vm_vec_add2(out_pnt, objpos);

		vm_vec_unrotate(out_dir, &dir, objorient);
	} else {
		*out_pnt = pnt;
		*out_dir = dir;
	}
}

void model_instance_local_to_global_point_orient(vec3d *outpnt, matrix *outorient, const vec3d *submodel_pnt, const matrix *submodel_orient, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient, const vec3d *objpos)
{
	vec3d pnt, tpnt;
	matrix orient;
	int mn;
	Assert(pm->id == pmi->model_num);

	pnt = *submodel_pnt;
	orient = *submodel_orient;
	mn = submodel_num;

	// instance up the tree for this point
	while ( (mn >= 0) && (pm->submodel[mn].parent >= 0) ) {
		vm_vec_unrotate(&tpnt, &pnt, &pmi->submodel[mn].canonical_orient);
		vm_vec_add(&pnt, &tpnt, &pmi->submodel[mn].canonical_offset);
		vm_vec_add2(&pnt, &pm->submodel[mn].offset);

		orient = orient * pmi->submodel[mn].canonical_orient;

		mn = pm->submodel[mn].parent;
	}

	// now instance for the entire object
	if (objorient && objpos) {
		vm_vec_unrotate(outpnt, &pnt, objorient);
		vm_vec_add2(outpnt, objpos);

		*outorient = orient * *objorient;
	} else {
		*outpnt = pnt;
		*outorient = orient;
	}
}

void model_instance_global_to_local_point(vec3d* outpnt, const vec3d* mpnt, int model_instance_num, int submodel_num, const matrix* objorient, const vec3d* objpos, bool use_last_frame) {
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);
	return model_instance_global_to_local_point(outpnt, mpnt, pm, pmi, submodel_num, objorient, objpos, use_last_frame);
}

void model_instance_global_to_local_point(vec3d* outpnt, const vec3d* mpnt, const polymodel* pm, const polymodel_instance* pmi, int submodel_num, const matrix* objorient, const vec3d* objpos, bool use_last_frame) {
	Assert(pm->id == pmi->model_num);

	constexpr int preallocatedStackDepth = 5;
	std::tuple<const matrix*, const vec3d*, const vec3d*> preallocatedStack[preallocatedStackDepth];

	auto submodelStack = pm->submodel[submodel_num].depth <= preallocatedStackDepth ? preallocatedStack : new std::tuple<const matrix*, const vec3d*, const vec3d*>[pm->submodel[submodel_num].depth];
	int stackCounter = 0;

	int mn = submodel_num;

	//Go up the chain of parents to build a stack of transformations from parent -> child
	while ((mn >= 0) && (pm->submodel[mn].parent >= 0)) {
		if(use_last_frame) {
			std::get<0>(submodelStack[stackCounter]) = &pmi->submodel[mn].canonical_prev_orient;
			std::get<1>(submodelStack[stackCounter]) = &pmi->submodel[mn].canonical_prev_offset;
		} else {
			std::get<0>(submodelStack[stackCounter]) = &pmi->submodel[mn].canonical_orient;
			std::get<1>(submodelStack[stackCounter]) = &pmi->submodel[mn].canonical_offset;
		}
		std::get<2>(submodelStack[stackCounter++]) = &pm->submodel[mn].offset;
		mn = pm->submodel[mn].parent;
	}

	if (objorient != nullptr && objpos != nullptr) {
		std::get<0>(submodelStack[stackCounter]) = objorient;
		std::get<1>(submodelStack[stackCounter]) = &vmd_zero_vector;
		std::get<2>(submodelStack[stackCounter++]) = objpos;
	}
	stackCounter--;
		
	vec3d resultPnt = *mpnt;

	while (stackCounter >= 0) {
		const auto& transform = submodelStack[stackCounter--];

		vm_vec_sub2(&resultPnt, std::get<2>(transform));
		vm_vec_sub2(&resultPnt, std::get<1>(transform));
		vm_vec_rotate(&resultPnt, &resultPnt, std::get<0>(transform));
	}

	*outpnt = resultPnt;

	if (pm->submodel[submodel_num].depth > preallocatedStackDepth)
		delete[] submodelStack;
}

void model_instance_global_to_local_dir(vec3d* out_dir, const vec3d* in_dir, int model_instance_num, int submodel_num, const matrix* objorient, bool use_submodel_parent, bool use_last_frame) {
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);
	model_instance_global_to_local_dir(out_dir, in_dir, pm, pmi, use_submodel_parent ? pm->submodel[submodel_num].parent : submodel_num, objorient, use_last_frame);
}

void model_instance_global_to_local_dir(vec3d* out_dir, const vec3d* in_dir, const polymodel* pm, const polymodel_instance* pmi, int submodel_num, const matrix* objorient, bool use_last_frame) {
	Assert(pm->id == pmi->model_num);

	constexpr int preallocatedStackDepth = 5;
	const matrix* preallocatedStack[preallocatedStackDepth];

	auto submodelStack = pm->submodel[submodel_num].depth <= preallocatedStackDepth ? preallocatedStack : new const matrix*[pm->submodel[submodel_num].depth];
	int stackCounter = 0;

	int mn = submodel_num;

	//Go up the chain of parents to build a stack of transformations from parent -> child
	while ((mn >= 0) && (pm->submodel[mn].parent >= 0)) {
		if (use_last_frame)
			submodelStack[stackCounter++] = &pmi->submodel[mn].canonical_prev_orient;
		else
			submodelStack[stackCounter++] = &pmi->submodel[mn].canonical_orient;
		mn = pm->submodel[mn].parent;
	}

	if (objorient != nullptr)
		submodelStack[stackCounter++] = objorient;

	stackCounter--;

	vec3d resultDir = *in_dir;

	while (stackCounter >= 0) {
		const auto& transform = submodelStack[stackCounter--];

		vm_vec_rotate(&resultDir, &resultDir, transform);
	}

	*out_dir = resultDir;

	if (pm->submodel[submodel_num].depth > preallocatedStackDepth)
		delete[] submodelStack;
}

/*
 * Get all submodel indexes that satisfy the following:
 * 1) Have the rotating or intrinsic-rotating movement type
 * 2) Are currently rotating (i.e. actually moving and not part of the superstructure due to being destroyed or replaced)
 * 3) Are not rotating too far for collision detection (c.f. MAX_SUBMODEL_COLLISION_ANGULAR_VELOCITY)
 * And check the translating equivalent as well
 */
void model_get_moving_submodel_list(SCP_vector<int> &submodel_vector, const object *objp)
{
	Assert(objp->type == OBJ_SHIP || objp->type == OBJ_WEAPON || objp->type == OBJ_ASTEROID);
	
	int model_instance_num;
	int model_num;
	if (objp->type == OBJ_SHIP) {
		model_instance_num = Ships[objp->instance].model_instance_num;
		model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num;
	}
	else if (objp->type == OBJ_WEAPON) {
		model_instance_num = Weapons[objp->instance].model_instance_num;
		if (model_instance_num < 0) {
			return;
		}
		model_num = Weapon_info[Weapons[objp->instance].weapon_info_index].model_num;
	}
	else if (objp->type == OBJ_ASTEROID) {
		model_instance_num = Asteroids[objp->instance].model_instance_num;
		if (model_instance_num < 0) {
			return;
		}
		model_num = Asteroid_info[Asteroids[objp->instance].asteroid_type].model_num[Asteroids[objp->instance].asteroid_subtype];
	}
	else {
		return;
	}

	polymodel *pm = model_get(model_num);
	polymodel_instance *pmi = model_get_instance(model_instance_num);
	
	
	model_iterate_submodel_tree(pm, pm->detail[0], [pm, pmi, &submodel_vector](int submodel, int /*currentLevel*/, bool /*isLeaf*/, bool& isMoving, bool& skipChildren) {
		if (skipChildren)
			return;

		const auto& child_submodel = pm->submodel[submodel];
		const auto& child_submodel_instance = pmi->submodel[submodel];

		// Don't check it or its children if it is destroyed or it is a replacement (non-moving)
		if (child_submodel.flags[Model::Submodel_flags::No_collisions] || child_submodel_instance.blown_off || child_submodel.i_replace != -1) {
			skipChildren = true;
			return;
		}

		if (child_submodel.rotation_type == MOVEMENT_TYPE_REGULAR || child_submodel.rotation_type == MOVEMENT_TYPE_INTRINSIC) {
			float delta_angle = get_submodel_delta_angle(&child_submodel_instance);
			isMoving |= delta_angle < MAX_SUBMODEL_COLLISION_ANGULAR_VELOCITY;
		} else if (child_submodel.translation_type == MOVEMENT_TYPE_REGULAR || child_submodel.translation_type == MOVEMENT_TYPE_INTRINSIC) {
			float delta_shift = get_submodel_delta_shift(&child_submodel_instance);
			isMoving |= delta_shift < MAX_SUBMODEL_COLLISION_LINEAR_VELOCITY;
		} else if (child_submodel.flags[Model::Submodel_flags::Can_move]) {
			isMoving = true;
		}

		if (isMoving && !child_submodel.flags[Model::Submodel_flags::Nocollide_this_only])
			submodel_vector.push_back(submodel);
	}, 0, false, false);
}

void model_get_submodel_tree_list(SCP_vector<int> &submodel_vector, const polymodel *pm, int mn)
{
	if ( pm->submodel[mn].buffer.model_list != NULL ) {
		submodel_vector.push_back(mn);
	}

	int i = pm->submodel[mn].first_child;

	while ( i >= 0 ) {
		model_get_submodel_tree_list(submodel_vector, pm, i);

		i = pm->submodel[i].next_sibling;
	}
}

void model_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, int model_num, int submodel_num, const matrix *objorient)
{
	model_local_to_global_dir(out_dir, in_dir, model_get(model_num), submodel_num, objorient);
}

void model_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, const polymodel *pm, int submodel_num, const matrix *objorient)
{
	SCP_UNUSED(pm);
	SCP_UNUSED(submodel_num);

	//now instance for the entire object
	if (objorient) {
		vm_vec_unrotate(out_dir, in_dir, objorient);
	} else {
		*out_dir = *in_dir;
	}
}

void model_instance_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, int model_instance_num, int submodel_num, const matrix *objorient, bool use_submodel_parent)
{
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);
	model_instance_local_to_global_dir(out_dir, in_dir, pm, pmi, use_submodel_parent ? pm->submodel[submodel_num].parent : submodel_num, objorient);
}

void model_instance_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient)
{
	vec3d pnt;
	vec3d tpnt;
	int mn;
	Assert(pm->id == pmi->model_num);

	pnt = *in_dir;
	mn = submodel_num;

	// instance up the tree for this point
	while ( (mn >= 0) && (pm->submodel[mn].parent >= 0) ) {
		vm_vec_unrotate(&tpnt, &pnt, &pmi->submodel[mn].canonical_orient);
		pnt = tpnt;

		mn = pm->submodel[mn].parent;
	}

	// now instance for the entire object
	if (objorient) {
		vm_vec_unrotate(out_dir, &pnt, objorient);
	} else {
		*out_dir = pnt;
	}
}


// Clears all the submodel instances stored in a model to their defaults.
void model_clear_instance(int model_num)
{
	// ---- stuff that should be moved into model instances at some point
	int i;
	auto pm = model_get(model_num);

	// reset textures to original ones
	for (i=0; i<pm->n_textures; i++ )	{
		pm->maps[i].ResetToOriginal();
	}
	// ---- end of stuff that should be moved into model instances at some point

	interp_clear_instance();
}

void model_set_submodel_instance_motion_info(bsp_info *sm, submodel_instance *smi)
{
	smi->current_turn_rate = 0.0f;
	smi->desired_turn_rate = sm->default_turn_rate;
	smi->turn_accel = sm->default_turn_accel;

	smi->current_shift_rate = 0.0f;
	smi->desired_shift_rate = sm->default_shift_rate;
	smi->shift_accel = sm->default_shift_accel;
}

// Sets the submodel instance data when a tech room model instance is created.
// This only needs to be done at creation, not every frame.
void model_set_up_techroom_instance(ship_info *sip, int model_instance_num)
{
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);
	flagset<Ship::Subsystem_Flags> empty;

	sip->animations.clearShipData(pmi);
	sip->animations.getAll(pmi, animation::ModelAnimationTriggerType::Initial).start(animation::ModelAnimationDirection::FWD, true, true);

	model_iterate_submodel_tree(pm, pm->detail[0], [&](int submodel, int /*level*/, bool /*isLeaf*/)
		{
			model_replicate_submodel_instance(pm, pmi, submodel, empty);
		});
}

/*
 * This function handles copying submodel instance information to other submodel instances as appropriate.  The copy_from parameter is used for
 * copying data to other LODs, and is only specified from within this function itself.  The "public" function header omits this parameter.
 */
void model_replicate_submodel_instance_sub(polymodel *pm, polymodel_instance *pmi, const submodel_instance *copy_from, int submodel_num, flagset<Ship::Subsystem_Flags>& flags)
{
	Assert(pm->id == pmi->model_num);
	
	Assertion(submodel_num >= 0 && submodel_num < pm->n_models,
		"Submodel number (%d) which should be updated is out of range! Must be between 0 and %d. This happened on model %s.",
		submodel_num, pm->n_models - 1, pm->filename);

	if ( submodel_num < 0 ) return;
	if ( submodel_num >= pm->n_models ) return;

	submodel_instance *smi = &pmi->submodel[submodel_num];
	bsp_info *sm = &pm->submodel[submodel_num];
	
	// Set the "blown out" flags.
	if ( flags[Ship::Subsystem_Flags::No_disappear] ) {
		smi->blown_off = false;
	} else if ( copy_from ) {
		smi->blown_off = copy_from->blown_off;
	}

	if ( smi->blown_off )	{
		if ( sm->my_replacement >= 0 && !(flags[Ship::Subsystem_Flags::No_replace]) ) {
			auto r_smi = &pmi->submodel[sm->my_replacement];
			r_smi->blown_off = false;
			if ( copy_from ) {
				r_smi->cur_angle = copy_from->cur_angle;
				r_smi->canonical_orient = copy_from->canonical_orient;
				r_smi->canonical_prev_orient = copy_from->canonical_prev_orient;

				r_smi->cur_offset = copy_from->cur_offset;
				r_smi->canonical_offset = copy_from->canonical_offset;
				r_smi->canonical_prev_offset = copy_from->canonical_prev_offset;
			} else {
				r_smi->cur_angle = smi->cur_angle;
				r_smi->canonical_orient = smi->canonical_orient;
				r_smi->canonical_prev_orient = smi->canonical_prev_orient;

				r_smi->cur_offset = smi->cur_offset;
				r_smi->canonical_offset = smi->canonical_offset;
				r_smi->canonical_prev_offset = smi->canonical_prev_offset;
			}
		}
	} else {
		// If submodel isn't yet blown off and has a -destroyed replacement model, we prevent
		// the replacement model from being drawn by marking it as having been blown off
		if ( sm->my_replacement >= 0 && sm->my_replacement != submodel_num)	{
			auto r_smi = &pmi->submodel[sm->my_replacement];
			r_smi->blown_off = true;
		}
	}

	// Set the angles and offset.
	if ( copy_from ) {
		smi->cur_angle = copy_from->cur_angle;
		smi->canonical_orient = copy_from->canonical_orient;
		smi->canonical_prev_orient = copy_from->canonical_prev_orient;

		smi->cur_offset = copy_from->cur_offset;
		smi->canonical_offset = copy_from->canonical_offset;
		smi->canonical_prev_offset = copy_from->canonical_prev_offset;
	}

	// For all the detail levels of this submodel, set them also.
	for ( int i=0; i<sm->num_details; i++ )	{
		model_replicate_submodel_instance_sub( pm, pmi, smi, sm->details[i], flags );
	}
}

void model_replicate_submodel_instance(polymodel *pm, polymodel_instance *pmi, int submodel_num, flagset<Ship::Subsystem_Flags>& flags)
{
	model_replicate_submodel_instance_sub(pm, pmi, nullptr, submodel_num, flags);
}

void model_do_intrinsic_motions_sub(intrinsic_motion *im)
{
	polymodel_instance *pmi = model_get_instance(im->model_instance_num);
	Assert(pmi != nullptr);
	polymodel *pm = model_get(pmi->model_num);
	Assert(pm != nullptr);
	flagset<Ship::Subsystem_Flags> empty;

	// Handle all submodels which have intrinsic motion
	for (auto submodel_num: im->submodel_list)
	{
		if (pm->submodel[submodel_num].look_at_submodel >= 0)
			submodel_look_at(pm, pmi, submodel_num);
		else
			submodel_rotate(&pm->submodel[submodel_num], &pmi->submodel[submodel_num]);
	}
}

// Handle the intrinsic motions for either a) a single object model; or b) all non-object models.
//
// This function called as part of object movement.  All types of object movement, including intrinsic rotations and translations,
// should be handled at the same time - unless you want inconsistent collisions or damage sparks that aren't attached to models.
//
// -- Goober5000
void model_do_intrinsic_motions(object *objp)
{
	// we are handling a specific object
	if (objp)
	{
		int model_instance_num = object_get_model_instance(objp);
		if (model_instance_num >= 0)
		{
			auto obj_it = Intrinsic_motions.find(model_instance_num);
			if (obj_it != Intrinsic_motions.end())
			{
				Assertion(obj_it->second.is_object, "Inconsistent intrinsic motion: an object's motion is not flagged as belonging to an object!");

				// update the submodels
				model_do_intrinsic_motions_sub(&obj_it->second);
			}
		}
	}
	// we are handling all non-objects (so basically just skyboxes)
	else
	{
		for (auto &pair: Intrinsic_motions)
		{
			if (!pair.second.is_object)
			{
				// update the submodels
				model_do_intrinsic_motions_sub(&pair.second);
			}
		}
	}
}

void model_instance_clear_arcs(polymodel *pm, polymodel_instance *pmi)
{
	Assert(pm->id == pmi->model_num);

	for (int i = 0; i < pm->n_models; ++i) {
		pmi->submodel[i].num_arcs = 0;		// Turn off any electric arcing effects
	}
}

// Adds an electrical arcing effect to a submodel
void model_instance_add_arc(polymodel *pm, polymodel_instance *pmi, int sub_model_num, vec3d *v1, vec3d *v2, int arc_type )
{
	Assert(pm->id == pmi->model_num);

	if ( sub_model_num == -1 )	{
		sub_model_num = pm->detail[0];
	}

	Assert( sub_model_num >= 0 );
	Assert( sub_model_num < pm->n_models );

	if ( sub_model_num < 0 ) return;
	if ( sub_model_num >= pm->n_models ) return;
	auto smi = &pmi->submodel[sub_model_num];

	if ( smi->num_arcs < MAX_ARC_EFFECTS )	{
		smi->arc_type[smi->num_arcs] = (ubyte)arc_type;
		smi->arc_pts[smi->num_arcs][0] = *v1;
		smi->arc_pts[smi->num_arcs][1] = *v2;
		smi->num_arcs++;
	}
}

int model_find_submodel_index(const polymodel& pm, const char* name) {
	for (int i = 0; i < pm.n_models; i++)
	{
		if (!stricmp(pm.submodel[i].name, name))
			return i;
	}

	return -1;
}

int model_find_submodel_index(int modelnum, const char *name)
{
	auto pm = model_get(modelnum);

	return model_find_submodel_index(*pm, name);
}

// function to return an index into the docking_bays array which matches the criteria passed
// to this function.  dock_type is one of the DOCK_TYPE_XXX defines in model.h
// Goober5000 - now finds more than one dockpoint of this type
int model_find_dock_index(int modelnum, int dock_type, int index_to_start_at)
{
	int i;
	polymodel *pm;

	// get model and make sure it has dockpoints
	pm = model_get(modelnum);
	if ( pm->n_docks <= 0 )
		return -1;

	// look for a dockpoint of this type
	for (i = index_to_start_at; i < pm->n_docks; i++ )
	{
		if ( dock_type & pm->docking_bays[i].type_flags )
			return i;
	}

	// if we get here, type wasn't found -- return -1 and hope for the best
	return -1;
}

// function to return an index into the docking_bays array which matches the string passed
// Fred uses strings to identify docking positions.  This function also accepts generic strings
// so that a desginer doesn't have to know exact names if building a mission from hand.
int model_find_dock_name_index(int modelnum, const char* name)
{
	int i;
	polymodel *pm;

	// get model and make sure it has dockpoints
	pm = model_get(modelnum);
	if ( pm->n_docks <= 0 )
		return -1;

	// check the generic names and call previous function to find first dock point of
	// the specified type
	for(i = 0; i < Num_dock_type_names; i++)
	{
		if(!stricmp(name, Dock_type_names[i].name)) {
			return model_find_dock_index(modelnum, Dock_type_names[i].def);
		}
	}
	/*
	if ( !stricmp(name, "cargo") )
		return model_find_dock_index( modelnum, DOCK_TYPE_CARGO );
	else if (!stricmp( name, "rearm") )
		return model_find_dock_index( modelnum, DOCK_TYPE_REARM );
	else if (!stricmp( name, "generic") )
		return model_find_dock_index( modelnum, DOCK_TYPE_GENERIC );
	*/

	// look for a dockpoint with this name
	for (i = 0; i < pm->n_docks; i++ )
	{
		if ( !stricmp(pm->docking_bays[i].name, name) )
			return i;
	}

	// if the bay does not have a name in the model, the model loading code
	// will assign it a default name... check for that here
	if (!strnicmp(name, "<unnamed bay ", 13))
	{
		int index = (name[13] - 'A');
		if (index >= 0 && index < pm->n_docks)
			return index;
	}

	// if we get here, name wasn't found -- return -1 and hope for the best
	return -1;
}

// returns the actual name of a docking point on a model, needed by Fred.
char *model_get_dock_name(int modelnum, int index)
{
	polymodel *pm;

	pm = model_get(modelnum);
	Assert((index >= 0) && (index < pm->n_docks));
	return pm->docking_bays[index].name;
}

int model_get_num_dock_points(int modelnum)
{
	polymodel *pm;

	pm = model_get(modelnum);
	return pm->n_docks;
}

int model_get_dock_index_type(int modelnum, int index)
{
	polymodel *pm = model_get(modelnum);				

	return pm->docking_bays[index].type_flags;
}

// get all the different docking point types on a model
int model_get_dock_types(int modelnum)
{
	int i, type = 0;
	polymodel *pm;

	pm = model_get(modelnum);
	for (i=0; i<pm->n_docks; i++)
		type |= pm->docking_bays[i].type_flags;

	return type;
}

// Goober5000
// returns index in [0, MAX_SHIP_BAY_PATHS)
int model_find_bay_path(int modelnum, char *bay_path_name)
{
	int i;
	polymodel *pm = model_get(modelnum);

	if (pm->ship_bay == NULL)
		return -1;

	if (pm->ship_bay->num_paths <= 0)
		return -1;

	for (i = 0; i < pm->ship_bay->num_paths; i++)
	{
		if (!stricmp(pm->paths[pm->ship_bay->path_indexes[i]].name, bay_path_name))
			return i;
	}

	return -1;
}

int model_create_bsp_collision_tree()
{
	// first find an open slot
	size_t i;
	bool slot_found = false;

	for ( i = 0; i < Bsp_collision_tree_list.size(); ++i ) {
		if ( !Bsp_collision_tree_list[i].used ) {
			slot_found = true;
			break;
		}
	}

	if ( slot_found ) {
		Bsp_collision_tree_list[i].used = true;

		return (int)i;
	}

	bsp_collision_tree tree;

	tree.used = true;
	Bsp_collision_tree_list.push_back(tree);

	return (int)(Bsp_collision_tree_list.size() - 1);
}

bsp_collision_tree *model_get_bsp_collision_tree(int tree_index)
{
	Assert(tree_index >= 0);
	Assert((uint) tree_index < Bsp_collision_tree_list.size());

	return &Bsp_collision_tree_list[tree_index];
}

void model_remove_bsp_collision_tree(int tree_index)
{
	Bsp_collision_tree_list[tree_index].used = false;

	if ( Bsp_collision_tree_list[tree_index].node_list ) {
		vm_free(Bsp_collision_tree_list[tree_index].node_list);
	}

	if ( Bsp_collision_tree_list[tree_index].leaf_list ) {
		vm_free(Bsp_collision_tree_list[tree_index].leaf_list);
	}
	
	if ( Bsp_collision_tree_list[tree_index].point_list ) {
		vm_free( Bsp_collision_tree_list[tree_index].point_list );
	}
	
	if ( Bsp_collision_tree_list[tree_index].vert_list ) {
		vm_free( Bsp_collision_tree_list[tree_index].vert_list);
	}
}

#if BYTE_ORDER == BIG_ENDIAN

// tigital -
void swap_bsp_defpoints(ubyte * p)
{
	int n, i;
	int nverts = INTEL_INT( w(p+8) );		//tigital
	int offset = INTEL_INT( w(p+16) );
	int n_norms = INTEL_INT( w(p+12) );

	w(p+8) = nverts;
	w(p+16) = offset;
	w(p+12) = n_norms;

	ubyte * normcount = p+20;
	vec3d *src = vp(p+offset);

	model_allocate_interp_data(nverts, n_norms);

	for (n=0; n<nverts; n++ )	{
		src->xyz.x = INTEL_FLOAT( &src->xyz.x );		//tigital
		src->xyz.y = INTEL_FLOAT( &src->xyz.y );
		src->xyz.z = INTEL_FLOAT( &src->xyz.z );

		Interp_verts[n] = src;
		src++;	//tigital

		for (i=0; i<normcount[n]; i++){
			src->xyz.x = INTEL_FLOAT( &src->xyz.x );		//tigital
			src->xyz.y = INTEL_FLOAT( &src->xyz.y );
			src->xyz.z = INTEL_FLOAT( &src->xyz.z );
			src++;
		}
	}
}

void swap_bsp_tmappoly( polymodel * pm, ubyte * p )
{
	uint i, nv;
	vec3d * normal = vp(p+8);	//tigital
	vec3d * center = vp(p+20);
	float radius = INTEL_FLOAT( &fl(p+32) );

	fl(p+32) = radius;

	normal->xyz.x = INTEL_FLOAT( &normal->xyz.x );
	normal->xyz.y = INTEL_FLOAT( &normal->xyz.y );
	normal->xyz.z = INTEL_FLOAT( &normal->xyz.z );
	center->xyz.x = INTEL_FLOAT( &center->xyz.x );
	center->xyz.y = INTEL_FLOAT( &center->xyz.y );
	center->xyz.z = INTEL_FLOAT( &center->xyz.z );

	nv = INTEL_INT( uw(p+36));		//tigital
		uw(p+36) = nv;

	int tmap_num = INTEL_INT( w(p+40) );	//tigital
		w(p+40) = tmap_num;

	auto verts = reinterpret_cast<model_tmap_vert_old*>(&p[TMAP_VERTS]);
	for (i = 0; i < nv; i++) {
		verts[i].vertnum = INTEL_SHORT(verts[i].vertnum);	//tigital
		verts[i].normnum = INTEL_SHORT(verts[i].normnum);	
		verts[i].u = INTEL_FLOAT(&verts[i].u);
		verts[i].v = INTEL_FLOAT(&verts[i].v);
	}
}

void swap_bsp_tmap2poly(polymodel* pm, ubyte* p)
{
	uint i, nv;
	model_tmap_vert* verts;

	nv = INTEL_INT(uw(p + TMAP2_NVERTS)); // tigital
	uw(p + TMAP2_NVERTS) = nv;

	int tmap_num = INTEL_INT(w(p + TMAP2_TEXNUM)); // tigital
	w(p + TMAP2_TEXNUM) = tmap_num;

	verts = (model_tmap_vert*)(p + TMAP2_VERTS);
	for (i = 0; i < nv; i++) {
		verts[i].vertnum = INTEL_INT(verts[i].vertnum);
		verts[i].normnum = INTEL_INT(verts[i].normnum);
		verts[i].u = INTEL_FLOAT(&verts[i].u);
		verts[i].v = INTEL_FLOAT(&verts[i].v);
	}
}

void swap_bsp_flatpoly( polymodel * pm, ubyte * p )
{
	uint i, nv;
	short *verts;
	vec3d * normal = vp(p+8);	//tigital
	vec3d * center = vp(p+20);

	float radius = INTEL_FLOAT( &fl(p+32) );

	fl(p+32) = radius; 

	normal->xyz.x = INTEL_FLOAT( &normal->xyz.x );
	normal->xyz.y = INTEL_FLOAT( &normal->xyz.y );
	normal->xyz.z = INTEL_FLOAT( &normal->xyz.z );
	center->xyz.x = INTEL_FLOAT( &center->xyz.x );
	center->xyz.y = INTEL_FLOAT( &center->xyz.y );
	center->xyz.z = INTEL_FLOAT( &center->xyz.z );

	nv = INTEL_INT( uw(p+36));		//tigital
		uw(p+36) = nv;
        
	if ( nv < 0 ) return;

	verts = (short *)(p+44);
	for (i=0; i<nv*2; i++){
		verts[i] = INTEL_SHORT( verts[i] );
	}

	if ( pm->version < 2003 )	{
		// Set the "normal_point" part of field to be the center of the polygon
		vec3d center_point;
		vm_vec_zero( &center_point );

		for (i=0;i<nv;i++)	{
			vm_vec_add2( &center_point, Interp_verts[verts[i*2]] );
		}

		center_point.xyz.x /= nv;
		center_point.xyz.y /= nv;
		center_point.xyz.z /= nv;

		*vp(p+20) = center_point;

		float rad = 0.0f;

		for (i=0;i<nv;i++)	{
			float dist = vm_vec_dist( &center_point, Interp_verts[verts[i*2]] );
			if ( dist > rad )	{
				rad = dist;
			}
		}
		fl(p+32) = rad;
	}
}

void swap_bsp_sortnorm2(polymodel* pm, ubyte* p)
{
	int frontlist = INTEL_INT(w(p + 8));	//tigital
	int backlist = INTEL_INT(w(p + 12));

	w(p + 8) = frontlist;
	w(p + 12) = backlist;

	vec3d* bmin = vp(p + 8);	//tigital
	vec3d* bmax = vp(p + 20);

	bmin->xyz.x = INTEL_FLOAT(&bmin->xyz.x);
	bmin->xyz.y = INTEL_FLOAT(&bmin->xyz.y);
	bmin->xyz.z = INTEL_FLOAT(&bmin->xyz.z);
	bmax->xyz.x = INTEL_FLOAT(&bmax->xyz.x);
	bmax->xyz.y = INTEL_FLOAT(&bmax->xyz.y);
	bmax->xyz.z = INTEL_FLOAT(&bmax->xyz.z);

	if (backlist) swap_bsp_data(pm, p + backlist);
	if (frontlist) swap_bsp_data(pm, p + frontlist);
}

void swap_bsp_sortnorm( polymodel * pm, ubyte * p )
{
	int frontlist = INTEL_INT( w(p+36) );	//tigital
	int backlist = INTEL_INT( w(p+40) );
	int prelist = INTEL_INT( w(p+44) );
	int postlist = INTEL_INT( w(p+48) );
	int onlist = INTEL_INT( w(p+52) );

	w(p+36) = frontlist;
	w(p+40) = backlist;
	w(p+44) = prelist;
	w(p+48) = postlist;
	w(p+52) = onlist;

	vec3d * normal = vp(p+8);	//tigital
	vec3d * center = vp(p+20);
	int  tmp = INTEL_INT( w(p+32) );
	
	w(p+32) = tmp;

	normal->xyz.x = INTEL_FLOAT( &normal->xyz.x );
	normal->xyz.y = INTEL_FLOAT( &normal->xyz.y );
	normal->xyz.z = INTEL_FLOAT( &normal->xyz.z );
	center->xyz.x = INTEL_FLOAT( &center->xyz.x );
	center->xyz.y = INTEL_FLOAT( &center->xyz.y );
	center->xyz.z = INTEL_FLOAT( &center->xyz.z );

	vec3d * bmin = vp(p+56);	//tigital
	vec3d * bmax = vp(p+68);

	bmin->xyz.x = INTEL_FLOAT( &bmin->xyz.x );
	bmin->xyz.y = INTEL_FLOAT( &bmin->xyz.y );
	bmin->xyz.z = INTEL_FLOAT( &bmin->xyz.z );
	bmax->xyz.x = INTEL_FLOAT( &bmax->xyz.x );
	bmax->xyz.y = INTEL_FLOAT( &bmax->xyz.y );
	bmax->xyz.z = INTEL_FLOAT( &bmax->xyz.z );

	if (prelist) swap_bsp_data(pm,p+prelist);
	if (backlist) swap_bsp_data(pm,p+backlist);
	if (onlist) swap_bsp_data(pm,p+onlist);
	if (frontlist) swap_bsp_data(pm,p+frontlist);
	if (postlist) swap_bsp_data(pm,p+postlist);
}
#endif // BIG_ENDIAN

void swap_bsp_data( polymodel * pm, void * model_ptr )
{
#if BYTE_ORDER == BIG_ENDIAN
	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;
	vec3d * min;
	vec3d * max;

	chunk_type = INTEL_INT( w(p) );	//tigital
	chunk_size = INTEL_INT( w(p+4) );
	w(p) = chunk_type;
	w(p+4) = chunk_size;

	bool end = chunk_type == OP_EOF;
	while (!end) {
		switch (chunk_type) {
			case OP_EOF:
				return;
			case OP_DEFPOINTS:
				swap_bsp_defpoints(p); 
				break;
			case OP_FLATPOLY:
				swap_bsp_flatpoly(pm, p);
				break;
			case OP_TMAPPOLY:
				swap_bsp_tmappoly(pm, p);
				break;
			case OP_SORTNORM:	
				swap_bsp_sortnorm(pm, p);
				break;
			case OP_SORTNORM2:
				swap_bsp_sortnorm2(pm, p);
				end = true; // should not continue after this chunk
				break;
			case OP_BOUNDBOX:
				min = vp(p+8);
				max = vp(p+20);
				min->xyz.x = INTEL_FLOAT( &min->xyz.x );
				min->xyz.y = INTEL_FLOAT( &min->xyz.y );
				min->xyz.z = INTEL_FLOAT( &min->xyz.z );
				max->xyz.x = INTEL_FLOAT( &max->xyz.x );
				max->xyz.y = INTEL_FLOAT( &max->xyz.y );
				max->xyz.z = INTEL_FLOAT( &max->xyz.z );
				break;
			case OP_TMAP2POLY:
				swap_bsp_tmap2poly(pm, p);
				end = true; // should not continue after this chunk
				break;
			default:
				mprintf(( "Bad chunk type %d, len=%d in modelread:swap_bsp_data\n", chunk_type, chunk_size ));
				Int3();		// Bad chunk type!
			return;
		}

		p += chunk_size;
		chunk_type = INTEL_INT( w(p));	//tigital
		chunk_size = INTEL_INT( w(p+4) );
		w(p) = chunk_type;
		w(p+4) = chunk_size;

		if (chunk_type == OP_EOF)
			end = true;
	}

	return;
#else
(void)pm;
(void)model_ptr;
#endif
}

void swap_sldc_data(ubyte* buffer)
{
	//ShivanSpS - Changed type char for a type int for SLC2
#if BYTE_ORDER == BIG_ENDIAN
	int* type_p = (int*)(buffer);
	int* size_p = (int*)(buffer + 4);
	*size_p = INTEL_INT(*size_p);
	*type_p = INTEL_INT(*type_p);

	// split and polygons
	vec3d* minbox_p = (vec3d*)(buffer + 8);
	vec3d* maxbox_p = (vec3d*)(buffer + 20);

	minbox_p->xyz.x = INTEL_FLOAT(&minbox_p->xyz.x);
	minbox_p->xyz.y = INTEL_FLOAT(&minbox_p->xyz.y);
	minbox_p->xyz.z = INTEL_FLOAT(&minbox_p->xyz.z);

	maxbox_p->xyz.x = INTEL_FLOAT(&maxbox_p->xyz.x);
	maxbox_p->xyz.y = INTEL_FLOAT(&maxbox_p->xyz.y);
	maxbox_p->xyz.z = INTEL_FLOAT(&maxbox_p->xyz.z);


	// split
	unsigned int* front_offset_p = (unsigned int*)(buffer + 32);
	unsigned int* back_offset_p = (unsigned int*)(buffer + 36);

	// polygons
	unsigned int* num_polygons_p = (unsigned int*)(buffer + 32);

	unsigned int* shld_polys = (unsigned int*)(buffer + 36);

	if (*type_p == 0) // SPLIT
	{
		*front_offset_p = INTEL_INT(*front_offset_p);
		*back_offset_p = INTEL_INT(*back_offset_p);
	}
	else
	{
		*num_polygons_p = INTEL_INT(*num_polygons_p);
		for (unsigned int i = 0; i < *num_polygons_p; i++)
		{
			shld_polys[i] = INTEL_INT(shld_polys[i]);
		}
	}
#else
	(void)buffer;
#endif
}

void glowpoint_override_defaults(glow_point_bank_override *gpo)
{
	gpo->name[0] = 0;
	gpo->type = 0;
	gpo->on_time = 0;
	gpo->off_time = 0;
	gpo->disp_time = 0;
	gpo->glow_bitmap = -1;
	gpo->glow_neb_bitmap = -1;
	gpo->is_on = true;
	gpo->type_override = false;
	gpo->on_time_override = false;
	gpo->off_time_override = false;
	gpo->disp_time_override = false;
	gpo->glow_bitmap_override = false;
	gpo->pulse_period_override = false;
	gpo->pulse_type = 0;
	gpo->pulse_period = 0;
	gpo->pulse_amplitude = 1.0f;
	gpo->pulse_bias = 0.0f;
	gpo->pulse_exponent = 1.0f;
	gpo->is_lightsource = false;
	gpo->radius_multi = 15.0f;
	gpo->light_color = vmd_zero_vector;
	gpo->light_mix_color = vmd_zero_vector;
	gpo->lightcone = false;
	gpo->cone_angle = 90.0f;
	gpo->cone_direction = vmd_zero_vector;
	gpo->dualcone = false;
	gpo->rotating = false;
	gpo->rotation_axis = vmd_zero_vector;
	gpo->rotation_speed = 0.0f;
}

SCP_vector<glow_point_bank_override>::iterator get_glowpoint_bank_override_by_name(const char* name)
{
	SCP_vector<glow_point_bank_override>::iterator gpo = glowpoint_bank_overrides.begin();
	for(;gpo != glowpoint_bank_overrides.end(); ++gpo)	{
		if(!strcmp(gpo->name, name))	{
			return gpo;
		}
	}
	return glowpoint_bank_overrides.end();
}

void parse_glowpoint_table(const char *filename)
{
	try {
		if (cf_exists_full(filename, CF_TYPE_TABLES))
			read_file_text(filename, CF_TYPE_TABLES);
		else
			return;

		reset_parse();

		if (!optional_string("#Glowpoint overrides")) {
			return;
		}

		while (!required_string_either("$Name:", "#End")) {
			glow_point_bank_override gpo;
			glowpoint_override_defaults(&gpo);

			bool replace = false;
			bool skip = false;

			required_string("$Name:");
			stuff_string(gpo.name, F_NAME, NAME_LENGTH);

			if (optional_string("+nocreate")) {
				if (Parsing_modular_table) {
					replace = true;
				}
				else {
					mprintf(("+nocreate specified in non-modular glowpoint table.\n"));
				}
			}

			if (optional_string("$On:")) {
				stuff_boolean(&gpo.is_on);
			}

			if (optional_string("$Displacement time:")) {
				stuff_int(&gpo.disp_time);
				gpo.disp_time_override = true;
			}

			if (optional_string("$On time:")) {
				stuff_int(&gpo.on_time);
				gpo.on_time_override = true;
			}

			if (optional_string("$Off time:")) {
				stuff_int(&gpo.off_time);
				gpo.off_time_override = true;
			}

			if (optional_string("$Texture:")) {
				char glow_texture_name[32];
				stuff_string(glow_texture_name, F_NAME, NAME_LENGTH);

				gpo.glow_bitmap_override = true;

				if (stricmp(glow_texture_name, "none") != 0) {
					gpo.glow_bitmap = bm_load(glow_texture_name);

					if (gpo.glow_bitmap < 0)
					{
						Warning(LOCATION, "Couldn't open texture '%s'\nreferenced by glowpoint preset '%s'\n", glow_texture_name, gpo.name);
					}
					else
					{
						nprintf(("Model", "Glowpoint preset %s texture num is %d\n", gpo.name, gpo.glow_bitmap));
					}

					char glow_texture_neb_name[256];
					strncpy(glow_texture_neb_name, glow_texture_name, 256);
					strcat(glow_texture_neb_name, "-neb");
					gpo.glow_neb_bitmap = bm_load(glow_texture_neb_name);

					if (gpo.glow_neb_bitmap < 0)
					{
						gpo.glow_neb_bitmap = gpo.glow_bitmap;
						nprintf(("Model", "Glowpoint preset nebula texture not found for '%s', using normal glowpoint texture instead\n", gpo.name));
					}
					else
					{
						nprintf(("Model", "Glowpoint preset %s nebula texture num is %d\n", gpo.name, gpo.glow_neb_bitmap));
					}
				}
				else {
					gpo.glow_bitmap_override = true;
				}
			}

			if (optional_string("$Type:")) {
				stuff_int(&gpo.type);
				gpo.type_override = true;
			}

			if (optional_string("$Pulse type:")) {
				char pulsetype[33];
				stuff_string(pulsetype, F_NAME, NAME_LENGTH);
				if (!stricmp(pulsetype, "sine")) {
					gpo.pulse_type = PULSE_SIN;
				}
				else if (!stricmp(pulsetype, "cosine")) {
					gpo.pulse_type = PULSE_COS;
				}
				else if (!stricmp(pulsetype, "triangle")) {
					gpo.pulse_type = PULSE_TRI;
				}
				else if (!stricmp(pulsetype, "shiftedtriangle")) {
					gpo.pulse_type = PULSE_SHIFTTRI;
				}
			}

			if (optional_string("$Pulse period:")) {
				stuff_int(&gpo.pulse_period);
				gpo.pulse_period_override = true;
			}

			if (optional_string("$Pulse amplitude:")) {
				stuff_float(&gpo.pulse_amplitude);
			}

			if (optional_string("$Pulse bias:")) {
				stuff_float(&gpo.pulse_bias);
			}

			if (optional_string("$Pulse exponent:")) {
				stuff_float(&gpo.pulse_exponent);
			}

			if (optional_string("+light")) {
				gpo.is_lightsource = true;

				if (optional_string("$Light radius multiplier:")) {
					stuff_float(&gpo.radius_multi);
				}

				required_string("$Light color:");
				int temp;
				stuff_int(&temp);
				gpo.light_color.xyz.x = temp / 255.0f;
				stuff_int(&temp);
				gpo.light_color.xyz.y = temp / 255.0f;
				stuff_int(&temp);
				gpo.light_color.xyz.z = temp / 255.0f;

				if (optional_string("$Light mix color:")) {
					stuff_int(&temp);
					gpo.light_mix_color.xyz.x = temp / 255.0f;
					stuff_int(&temp);
					gpo.light_mix_color.xyz.y = temp / 255.0f;
					stuff_int(&temp);
					gpo.light_mix_color.xyz.z = temp / 255.0f;
				}

				if (optional_string("+lightcone")) {
					gpo.lightcone = true;

					if (optional_string("$Cone angle:")) {
						stuff_float(&gpo.cone_angle);
						gpo.cone_inner_angle = cosf((gpo.cone_angle - ((gpo.cone_angle < 20.0f) ? gpo.cone_angle*0.5f : 20.0f)) / 180.0f * PI);
						gpo.cone_angle = cosf(gpo.cone_angle / 180.0f * PI);
					}

					required_string("$Cone direction:");
					stuff_float_list(gpo.cone_direction.a1d, 3);
					if (vm_vec_mag_quick(&gpo.cone_direction) != 0.0f) {
						vm_vec_normalize(&gpo.cone_direction);
					}
					else {
						Warning(LOCATION, "Null vector specified in cone direction for glowpoint override %s. Discarding preset.", gpo.name);
						skip = true;
					}
					if (optional_string("+dualcone")) {
						gpo.dualcone = true;
					}

					if (optional_string("+rotating")) {
						gpo.rotating = true;
						required_string("$Rotation axis:");
						stuff_float_list(gpo.rotation_axis.a1d, 3);
						if (vm_vec_mag_quick(&gpo.rotation_axis) != 0.0f) {
							vm_vec_normalize(&gpo.rotation_axis);
						}
						else {
							Warning(LOCATION, "Null vector specified in rotation axis for glowpoint override %s. Discarding preset.", gpo.name);
							skip = true;
						}
						required_string("$Rotation speed:");
						stuff_float(&gpo.rotation_speed);
					}
				}
			}
			if (!skip) {
				SCP_vector<glow_point_bank_override>::iterator gpoi = get_glowpoint_bank_override_by_name(gpo.name);
				if (gpoi == glowpoint_bank_overrides.end()) {
					if (!replace) {
						glowpoint_bank_overrides.push_back(gpo);
					}
				}
				else {
					if (!replace) {
						Warning(LOCATION, "+nocreate not specified for glowpoint override that already exists. Discarding duplicate entry: %s", gpo.name);
					}
					else {
						glowpoint_bank_overrides.erase(gpoi);
						glowpoint_bank_overrides.push_back(gpo);
					}
				}
			}

		}
		required_string("#End");
	} catch (const parse::ParseException& e) {
		mprintf(("Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void glowpoint_init()
{
	glowpoint_bank_overrides.clear();
	parse_glowpoint_table("glowpoints.tbl");
	parse_modular_table(NOX("*-gpo.tbm"), parse_glowpoint_table);
}

void model_subsystem::reset()
{
    flags.reset();
    memset(name, 0, sizeof(name));
    memset(subobj_name, 0, sizeof(alt_dmg_sub_name));
    memset(alt_sub_name, 0, sizeof(alt_sub_name));
    memset(alt_dmg_sub_name, 0, sizeof(alt_dmg_sub_name));
    subobj_num = 0;
    model_num = 0;
    type = 0;
    pnt.xyz.x = pnt.xyz.y = pnt.xyz.z = 0.0f;
    radius = 0;

    max_subsys_strength = 0;
    armor_type_idx = 0;

    memset(crewspot, 0, sizeof(crewspot));
    turret_norm.xyz.x = turret_norm.xyz.y = turret_norm.xyz.z = 0.0f;
    
    turret_fov = 0;
    turret_max_fov = 0;
    turret_base_fov = 0;
    turret_num_firing_points = 0;
    for (auto it = std::begin(turret_firing_point); it != std::end(turret_firing_point); ++it)
        it->xyz.x = it->xyz.y = it->xyz.z = 0.0f;
    turret_gun_sobj = 0;
    turret_turning_rate = 0;
    turret_base_rotation_snd = gamesnd_id();
    turret_base_rotation_snd_mult = 0;
    turret_gun_rotation_snd = gamesnd_id();
    turret_gun_rotation_snd_mult = 0;

    alive_snd = gamesnd_id();
    dead_snd = gamesnd_id();
    rotation_snd = gamesnd_id();

    engine_wash_pointer = NULL;

    weapon_rotation_pbank = 0;
    stepped_rotation.reset();
    stepped_translation.reset();

    awacs_intensity = 0.0f;
    awacs_radius = 0.0f;
    scan_time = -1;

    for (auto it = std::begin(primary_banks); it != std::end(primary_banks); ++it)
        *it = 0;
    for (auto it = std::begin(primary_bank_capacity); it != std::end(primary_bank_capacity); ++it)
        *it = 0;
    for (auto it = std::begin(secondary_banks); it != std::end(secondary_banks); ++it)
        *it = 0;
    for (auto it = std::begin(secondary_bank_capacity); it != std::end(secondary_bank_capacity); ++it)
        *it = 0;

    path_num = 0;

    turret_reset_delay = 0;

    for (auto it = std::begin(target_priority); it != std::end(target_priority); ++it)
        *it = 0;

    num_target_priorities = 0;

    optimum_range = 0;
    favor_current_facing = 0;

    turret_rof_scaler = 0;

    turret_max_bomb_ownage = 0;
    turret_max_target_ownage = 0;

	beam_warmdown_program = actions::ProgramSet();
}

model_subsystem::model_subsystem() {
	reset();
}

uint convert_sldc_to_slc2(ubyte* sldc, ubyte* slc2, uint tree_size)
{
	//ShivanSpS SLDC must be converted to SLC2 in order to be used by shield collision system
	//Convert SLDC to SLC2
	uint node_size, node_type_int, new_tree_size = 0, count = 0;
	char node_type_char;

	//Process the SLDC tree to the end
	while (count < tree_size) {
		//Save Node type and size
		memcpy(&node_type_char, sldc, 1);
		memcpy(&node_size, sldc + 1, 4);

		//Convert Node type to int
		node_type_int = (int)node_type_char;

		//Copy the node type and new node size, move pointers
		memcpy(slc2, &node_type_int, 4);
		node_size += 3;
		memcpy(slc2 + 4, &node_size, 4);
		node_size -= 3;
		slc2 += 8;
		sldc += 5;


		//Copy Vectors
		memcpy(slc2, sldc, 24);
		slc2 += 24;
		sldc += 24;

		if (node_type_char == 0) {
			//Front and back offsets must be adjusted
			uint front, back, newback = 0;
			ubyte* p;

			p = sldc - 29;
			memcpy(&back, p + 33, 4);

			//I need to find the new distance to back.
			while (p < sldc + back - 29) {
				uint ns;
				memcpy(&ns, p + 1, 4);
				p += ns;
				newback += ns + 3;

			}
			//Copy offsets
			front = node_size + 3;
			memcpy(slc2, &front, 4); //Front is always this node size+3;
			memcpy(slc2 + 4, &newback, 4);

			slc2 += 8;
			sldc += 8;
		}
		else {
			//Copy the remaining data on the node
			memcpy(slc2, sldc, node_size - 29);

			//Move pointers
			slc2 += node_size - 29;
			sldc += node_size - 29;
		}
		//Count the new tree size and move the counter
		count += node_size;
		new_tree_size += node_size + 3;
	}

	//return the SLC2 tree size
	return new_tree_size;
}

// if bsp_out is NULL then we just calculate new size
uint align_bsp_data(ubyte* bsp_in, ubyte* bsp_out, uint bsp_size)
{
	//ShivanSpS 
	ubyte* end;
	uint copied = 0;
	end = bsp_in + bsp_size;

	uint bsp_chunk_type, bsp_chunk_size;
	do {
		//Read Chunk type and size
		memcpy(&bsp_chunk_type, bsp_in, 4);
		
		//Chunk type 0 is EOF, but the size is read as 0, it needs to be adjusted
		if (bsp_chunk_type == 0) {
			bsp_chunk_size = 4;
		}
		else {
			memcpy(&bsp_chunk_size, bsp_in + 4, 4);
		}

		//mprintf(("|%d | %d|\n",bsp_chunk_type,bsp_chunk_size));

		//DEFPOINTS is the only bsp data chunk that could be unaligned
		if (bsp_chunk_type == 1) {
			//if the size is not divisible by 4 align it, otherwise copy it.
			if ((bsp_chunk_size % 4) != 0) {
				//mprintf(("BSP DEFPOINTS DATA ALIGNED.\n"));
				//Get the new size
				uint newsize = bsp_chunk_size + 4 - (bsp_chunk_size % 4);

				if (bsp_out) {
					//Copy the entire chunk to dest
					memcpy(bsp_out, bsp_in, bsp_chunk_size);
					//Write the new chunk size on dest
					memcpy(bsp_out + 4, &newsize, 4);
					//The the position of vertex data
					uint vertex_offset;
					memcpy(&vertex_offset, bsp_in + 16, 4);
					//Move vertex data to the back of the chunk
					memmove(bsp_out + vertex_offset + (newsize - bsp_chunk_size), bsp_out + vertex_offset, bsp_chunk_size - vertex_offset);
					vertex_offset += (newsize - bsp_chunk_size);
					//Write new vertex offset
					memcpy(bsp_out + 16, &vertex_offset, 4);
					//Move pointers
					bsp_out += newsize;
				}

				//Move pointers
				bsp_in += bsp_chunk_size;
				copied += newsize;
			}
			else {
				//if aligned just copy it
				if (bsp_out) {
					memcpy(bsp_out, bsp_in, bsp_chunk_size);
					bsp_out += bsp_chunk_size;
				}

				bsp_in += bsp_chunk_size;
				copied += bsp_chunk_size;
			}
		}
		else {
			//If the chunk is not a defpoint just copy it
			if (bsp_out) {
				memcpy(bsp_out, bsp_in, bsp_chunk_size);
				bsp_out += bsp_chunk_size;
			}

			bsp_in += bsp_chunk_size;
			copied += bsp_chunk_size;
		}
	} while (bsp_in < end);

	//Returns the size of the aligned bsp_data
	return copied;
}