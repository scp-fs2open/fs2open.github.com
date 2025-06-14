#include "prop.h"

#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "freespace.h"
#include "model/model.h"
#include "parse/parselo.h"
#include "render/3d.h"
#include "ship/shipfx.h"
#include "object/objcollide.h"

#include "tracing/Monitor.h"

MONITOR(NumPropsRend)

bool Props_inited = false;

SCP_vector<prop_info> Prop_info;

SCP_vector<std::optional<prop>> Props;

static SCP_vector<SCP_string> Removed_props;

/**
 * Return the index of Prop_info[].name that is *token.
 */
int prop_info_lookup(const char* token)
{
	Assertion(token != nullptr, "NULL token passed to prop_info_lookup");

	for (auto it = Prop_info.cbegin(); it != Prop_info.cend(); ++it)
		if (!stricmp(token, it->name))
			return (int)std::distance(Prop_info.cbegin(), it);

	return -1;
}

int prop_name_lookup(const char *name)
{
	Assertion(name != nullptr, "NULL name passed to prop_name_lookup");

	for (int i=0; i<static_cast<int>(Props.size()); i++){
		auto prop = Props[i] ? &Props[i].value() : nullptr;
		if (prop == nullptr) {
			continue;
		}
		if (prop->objnum >= 0){
			if (Objects[prop->objnum].type == OBJ_PROP) {
				if (!stricmp(name, prop->prop_name)) {
					return i;
				}
			}
		}
	}
	
	// couldn't find it
	return -1;
}

prop* prop_id_lookup(int id)
{
	if (id < 0 || id >= static_cast<int>(Props.size()) || !Props[id].has_value()) {
		Assertion(false, "Could not find prop for id %d", id);
		return nullptr;
	}
	return &Props[id].value();
}

void parse_prop_table(const char* filename)
{
	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();

	required_string("#PROPS");

	while (optional_string("$Name:")) {

		char fname[NAME_LENGTH];
		prop_info* pip = nullptr;
		bool first_time = false;
		bool create_if_not_found = true;
		bool remove_prop = false;

		stuff_string(fname, F_NAME, NAME_LENGTH);

		if (optional_string("+nocreate")) {
			if (!Parsing_modular_table) {
				Warning(LOCATION, "+nocreate flag used for prop in non-modular table");
			}
			create_if_not_found = false;
		}

		// check first if this is on the remove blacklist
		auto it = std::find(Removed_props.begin(), Removed_props.end(), fname);
		if (it != Removed_props.end()) {
			remove_prop = true;
		}

		// we might have a remove tag
		if (optional_string("+remove")) {
			if (!Parsing_modular_table) {
				Warning(LOCATION, "+remove flag used for prop in non-modular table");
			}
			if (!remove_prop) {
				Removed_props.push_back(fname);
				remove_prop = true;
			}
		}

		// Check if prop exists already
		int prop_id = prop_info_lookup(fname);

		// maybe remove it
		if (remove_prop) {
			if (prop_id >= 0) {
				mprintf(("Removing previously parsed prop '%s'\n", fname));
				Prop_info.erase(Prop_info.begin() + prop_id);
			}

			if (!skip_to_start_of_string_either("$Name:", "#End")) {
				error_display(1, "Missing [#End] or [$Name] after prop class %s", fname);
			}
			continue;
		}

		// an entry for this prop exists
		if (prop_id >= 0) {
			pip = &Prop_info[prop_id];
			if (!Parsing_modular_table) {
				Warning(LOCATION,
					"Error:  Prop name %s already exists in %s.  All prop class names must be unique.",
					fname,
					filename);
				if (!skip_to_start_of_string_either("$Name:", "#End")) {
					error_display(1, "Missing [#End] or [$Name] after duplicate prop class %s", fname);
				}
				continue;
			}
		}
		// an entry does not exist
		else {
			// Don't create ship if it has +nocreate and is in a modular table.
			if (!create_if_not_found && Parsing_modular_table) {
				if (!skip_to_start_of_string_either("$Name:", "#End")) {
					error_display(1, "Missing [#End] or [$Name] after prop class %s", fname);
				}
				continue;
			}

			// Check if there are too many ship classes
			//if (Prop_info.size() >= MAX_SHIP_CLASSES) {
				//Error(LOCATION, "Too many prop classes before '%s'; maximum is %d.\n", fname, MAX_SHIP_CLASSES);
			//}

			Prop_info.push_back(prop_info());
			pip = &Prop_info.back();
			first_time = true;

			strcpy_s(pip->name, fname);
		}

		required_string("+POF file:");
		stuff_string(pip->pof_file, F_NAME, MAX_FILENAME_LEN);

		if (optional_string("$Closeup_pos:")) {
			stuff_vec3d(&pip->closeup_pos);
		} else if (first_time && VALID_FNAME(pip->pof_file)) {
			// Calculate from the model file. This is inefficient, but whatever
			int model_idx = model_load(pip->pof_file);
			polymodel* pm = model_get(model_idx);

			// Go through, find best
			pip->closeup_pos.xyz.z = fabsf(pm->maxs.xyz.z);

			float temp = fabsf(pm->mins.xyz.z);
			if (temp > pip->closeup_pos.xyz.z)
				pip->closeup_pos.xyz.z = temp;

			// Now multiply by 2
			pip->closeup_pos.xyz.z *= -2.0f;

			// We're done with the model.
			model_unload(model_idx);
		}

		if (optional_string("$Closeup_zoom:")) {
			stuff_float(&pip->closeup_zoom);

			if (pip->closeup_zoom <= 0.0f) {
				mprintf(("Warning!  Prop '%s' has a $Closeup_zoom value that is less than or equal to 0 (%f). Setting "
						 "to default value.\n",
					pip->name,
					pip->closeup_zoom));
				pip->closeup_zoom = 0.5f;
			}
		}

		if(optional_string("$Detail distance:")) {
			pip->num_detail_levels = (int)stuff_int_list(pip->detail_distance, MAX_PROP_DETAIL_LEVELS, RAW_INTEGER_TYPE);
		}

		if (optional_string("$Custom data:")) {
			parse_string_map(pip->custom_data, "$end_custom_data", "+Val:");
		}

		if (optional_string("$Custom Strings")) {
			while (optional_string("$Name:")) {
				custom_string cs;

				// The name of the string
				stuff_string(cs.name, F_NAME);

				// Arbitrary string value used for grouping strings together
				required_string("+Value:");
				stuff_string(cs.value, F_NAME);

				// The string text itself
				required_string("+String:");
				stuff_string(cs.text, F_MULTITEXT);

				pip->custom_strings.push_back(cs);
			}

			required_string("$end_custom_strings");
		}
	}

	required_string("#END");
}

void prop_init()
{
	
	// first parse the default table
	if (cf_exists_full("props.tbl", CF_TYPE_TABLES)) {
		parse_prop_table("props.tbl");
	}

	// parse any modular tables
	parse_modular_table("*-prp.tbm", parse_prop_table);

	Props_inited = true;
}

/**
 * Returns object index of ship.
 * @return -1 means failed.
 */
int prop_create(matrix* orient, vec3d* pos, int prop_type, const char* name)
{
	prop_info* pip;
	prop* propp;

	if (Props.size() >= MAX_PROPS) {
		if (!Fred_running) {
			Error(LOCATION,
				"There is a limit of %d props in the mission at once.  Please be sure that you do not have more "
				"than %d props present in the mission at the same time.",
				MAX_PROPS,
				MAX_PROPS);
		}
		return -1;
	}

	Assertion((prop_type >= 0) && (prop_type < static_cast<int>(Prop_info.size())),
		"Invalid prop_type %d passed to prop_create() (expected value in the range 0-%d)\n",
		prop_type,
		static_cast<int>(Prop_info.size()) - 1);

	pip = &(Prop_info[prop_type]);

	Props.emplace_back(prop());
	int new_id = static_cast<int>(Props.size()) - 1;
	propp = prop_id_lookup(new_id);
	Assertion(propp != nullptr, "Could not create prop!");

	propp->prop_info_index = prop_type;

	if ((name == nullptr) || (prop_name_lookup(name) >= 0)) {
		// regular name, regular suffix
		char base_name[NAME_LENGTH];
		char suffix[NAME_LENGTH];
		strcpy_s(base_name, Prop_info[prop_type].name);
		sprintf(suffix, NOX(" %d"), static_cast<int>(Props.size()));

		// start building name
		strcpy_s(propp->prop_name, base_name);

		// if generated name will be longer than allowable name, truncate the class section of the name by the overflow
		int char_overflow = static_cast<int>(strlen(base_name) + strlen(suffix)) - (NAME_LENGTH - 1);
		if (char_overflow > 0) {
			propp->prop_name[strlen(base_name) - static_cast<size_t>(char_overflow)] = '\0';
		}

		// complete building the name by adding suffix number
		strcat_s(propp->prop_name, suffix);

	} else {
		strcpy_s(propp->prop_name, name);
	}	

	if (!VALID_FNAME(pip->pof_file)) {
		Error(LOCATION, "Cannot create prop %s; pof file is not valid", pip->name);
		return -1;
	}
	pip->model_num = model_load(pip->pof_file);

	polymodel* pm = model_get(pip->model_num);

	if (pip->num_detail_levels != pm->n_detail_levels) {
		if (!Is_standalone) {
			// just log to file for standalone servers
			Warning(LOCATION,
				"For prop '%s', detail level\nmismatch. Table has %d,\nPOF has %d.",
				pip->name,
				pip->num_detail_levels,
				pm->n_detail_levels);
		} else {
			nprintf(("Warning",
				"For prop '%s', detail level mismatch. Table has %d, POF has %d.",
				pip->name,
				pip->num_detail_levels,
				pm->n_detail_levels));
		}
	}
	for (int i = 0; i < pm->n_detail_levels; i++)
		pm->detail_depth[i] = (i < pip->num_detail_levels) ? i2fl(pip->detail_distance[i]) : 0.0f;

	flagset<Object::Object_Flags> default_ship_object_flags;
	default_ship_object_flags.set(Object::Object_Flags::Renders);
	default_ship_object_flags.set(Object::Object_Flags::Physics);
	default_ship_object_flags.set(Object::Object_Flags::Immobile);
	// JAS: Nav buoys don't need to do collisions!
	// G5K: Corrected to apply specifically for ships with the no-collide flag.  (In retail, navbuoys already have this
	// flag, so this doesn't break anything.)
	default_ship_object_flags.set(Object::Object_Flags::Collides, !pip->flags[Prop::Info_Flags::No_collide]);

	int objnum = obj_create(OBJ_PROP,
		-1,
		new_id,
		orient,
		pos,
		model_get_radius(pip->model_num),
		default_ship_object_flags);
	Assert(objnum >= 0);

	propp->objnum = objnum;

	propp->model_instance_num = model_create_instance(objnum, pip->model_num);

	object* objp = &Objects[objnum];
	objp->hull_strength = 0.0f;
	objp->phys_info.max_vel = vmd_zero_vector;
	objp->phys_info.desired_vel = vmd_zero_vector;
	objp->phys_info.forward_accel_time_const = 1.0;
	objp->phys_info.flags = PF_CONST_VEL;

	// this is a little 'dangerous', values like this can cause the physics to do wierd things
	// but they are the most 'honest', and will help to indicate when the physics is trying to use these values in ways they shouldn't
	objp->phys_info.mass = INFINITY;
	objp->phys_info.I_body_inv = vmd_zero_matrix;


	// allocate memory for keeping glow point bank status (enabled/disabled)
	{
		bool val = true; // default value, enabled

		if (pm->n_glow_point_banks)
			propp->glow_point_bank_active.resize(pm->n_glow_point_banks, val);

		// set any default off banks to off
		for (int bank = 0; bank < pm->n_glow_point_banks; bank++) {
			glow_point_bank_override* gpo = nullptr;

			SCP_unordered_map<int, void*>::iterator gpoi = pip->glowpoint_bank_override_map.find(bank);
			if (gpoi != pip->glowpoint_bank_override_map.end()) {
				gpo = (glow_point_bank_override*)pip->glowpoint_bank_override_map[bank];
			}

			if (gpo) {
				if (gpo->default_off) {
					propp->glow_point_bank_active[bank] = false;
				}
			}
		}
	}

	//animation::anim_set_initial_states(propp);

	// Start up stracking for this ship in multi.
	//if (Game_mode & (GM_MULTIPLAYER)) {
		//multi_rollback_ship_record_add_ship(objnum);
	//}

	// Set time when ship is created
	propp->create_time = timer_get_milliseconds();

	//ship_make_create_time_unique(shipp);

	propp->time_created = Missiontime;

	return objnum;
}

void prop_delete(object* obj)
{
	int num = obj->instance;

	int objnum = OBJ_INDEX(obj);
	Assertion(Props[num].has_value(), "Props[%d] is already nullopt!", num);

	prop& propp = *Props[num];
	Assertion(propp.objnum == objnum, "Prop object id does not match passed object!");

	propp.objnum = -1;

	//animation::ModelAnimationSet::stopAnimations(model_get_instance(propp->model_instance_num));

	// glow point banks
	propp.glow_point_bank_active.clear();

	model_delete_instance(propp.model_instance_num);

	// Leave the slot empty for the duration of the scene
	// The Props array will be compacted at the end of the level
	Props[num] = std::nullopt;
}

/**
 * Change the prop model for a prop to that for prop class 'prop_type'
 *
 * @param n			index of prop in ::Props[] array
 * @param prop_type	prop class (index into ::Prop_info vector)
 */
static void prop_model_change(int n, int prop_type)
{
	prop* sp = prop_id_lookup(n);
	prop_info* sip = &(Prop_info[prop_type]);
	object* objp = &Objects[sp->objnum];
	polymodel_instance* pmi = model_get_instance(sp->model_instance_num);

	// get new model
	if (sip->model_num == -1) {
		sip->model_num = model_load(sip->pof_file);
	}

	polymodel* pm = model_get(sip->model_num);
	Objects[sp->objnum].radius = model_get_radius(pm->id);

	// page in nondims in game
	if ( !Fred_running )
		model_page_in_textures(sip->model_num, prop_type);

	// allocate memory for keeping glow point bank status (enabled/disabled)
	{
		bool val = true; // default value, enabled

		// clear out any old gpb's first, then add new ones if needed
		sp->glow_point_bank_active.clear();

		if (pm->n_glow_point_banks)
			sp->glow_point_bank_active.resize( pm->n_glow_point_banks, val );
		
		// set any default off banks to off
		for (int bank = 0; bank < pm->n_glow_point_banks; bank++) {
			glow_point_bank_override* gpo = nullptr;

			SCP_unordered_map<int, void*>::iterator gpoi = sip->glowpoint_bank_override_map.find(bank);
			if (gpoi != sip->glowpoint_bank_override_map.end()) {
				gpo = (glow_point_bank_override*)sip->glowpoint_bank_override_map[bank];
			}

			if (gpo) {
				if (gpo->default_off) {
					sp->glow_point_bank_active[bank] = false;
				}
			}
		}
	}

	if ( sip->num_detail_levels != pm->n_detail_levels )
	{
		if ( !Is_standalone )
		{
			// just log to file for standalone servers
			Warning(LOCATION, "For prop '%s', detail level\nmismatch. Table has %d,\nPOF has %d.", sip->name, sip->num_detail_levels, pm->n_detail_levels );
		}
		else
		{
			nprintf(("Warning",  "For prop '%s', detail level mismatch. Table has %d, POF has %d.", sip->name, sip->num_detail_levels, pm->n_detail_levels ));
		}
	}	
	for (int i=0; i<pm->n_detail_levels; i++ )
		pm->detail_depth[i] = (i < sip->num_detail_levels) ? i2fl(sip->detail_distance[i]) : 0.0f;

	// reset texture animations
	//sp->base_texture_anim_timestamp = _timestamp();

	model_delete_instance(sp->model_instance_num);

	// create new model instance data
	// note: this is needed for both subsystem stuff and submodel animation stuff
	sp->model_instance_num = model_create_instance(OBJ_INDEX(objp), sip->model_num);
	pmi = model_get_instance(sp->model_instance_num);
}

/**
 * Change the prop class on a prop, and changing all required information
 * for consistency
 *
 * @param n			index of prop in ::Props[] array
 * @param prop_type	prop class (index into ::Prop_info vector)
 * @param by_sexp	SEXP reference
 */
void change_prop_type(int n, int prop_type)
{
	prop* sp = prop_id_lookup(n);

	// do a quick out if we're already using the new ship class
	if (sp->prop_info_index == prop_type)
		return;

	int objnum = sp->objnum;
	//auto prop_entry = ship_registry_get(sp->prop_name);

	prop_info* sip = &(Prop_info[prop_type]);
	prop_info* sip_orig = &Prop_info[sp->prop_info_index];
	object* objp = &Objects[objnum];

	// point to new ship data
	prop_model_change(n, prop_type);
	sp->prop_info_index = prop_type;

	// check class-specific flags

	if (sip->flags[Prop::Info_Flags::No_collide])								// changing TO a no-collision ship class
		obj_set_flags(objp, objp->flags - Object::Object_Flags::Collides);
	else if (sip_orig->flags[Prop::Info_Flags::No_collide])						// changing FROM a no-collision ship class
		obj_set_flags(objp, objp->flags + Object::Object_Flags::Collides);
}

void prop_render(object* obj, model_draw_list* scene)
{
	int num = obj->instance;

	prop* propp = prop_id_lookup(num);

	prop_info* pip = &Prop_info[propp->prop_info_index];

	MONITOR_INC(NumPropsRend, 1);

	model_render_params render_info;

	auto pmi = model_get_instance(propp->model_instance_num);
	auto pm = model_get(pmi->model_num);

	model_clear_instance(pip->model_num);
	model_instance_clear_arcs(pm, pmi);

	uint64_t render_flags = MR_NORMAL;

	if (propp->flags[Prop::Prop_Flags::Render_with_alpha_mult]) {
		render_info.set_alpha_mult(propp->alpha_mult);
	}

	// Valathil - maybe do a scripting hook here to do some scriptable effects?
	//ship_render_set_animated_effect(&render_info, shipp, &render_flags);

	if (pip->flags[Prop::Info_Flags::No_lighting]) {
		render_flags |= MR_NO_LIGHTING;
	}

	if (Rendering_to_shadow_map) {
		render_flags = MR_NO_TEXTURING | MR_NO_LIGHTING;
	}

	if (propp->flags[Prop::Prop_Flags::Glowmaps_disabled]) {
		render_flags |= MR_NO_GLOWMAPS;
	}

	if (propp->flags[Prop::Prop_Flags::Draw_as_wireframe]) {
		render_flags |= MR_SHOW_OUTLINE_HTL | MR_NO_POLYS | MR_NO_TEXTURING;
		render_info.set_color(Wireframe_color);
	}

	if (propp->flags[Prop::Prop_Flags::Render_full_detail]) {
		render_flags |= MR_FULL_DETAIL;
	}

	if (propp->flags[Prop::Prop_Flags::Render_without_light]) {
		render_flags |= MR_NO_LIGHTING;
	}

	uint debug_flags = render_info.get_debug_flags();

	if (propp->flags[Prop::Prop_Flags::Render_without_diffuse]) {
		debug_flags |= MR_DEBUG_NO_DIFFUSE;
	}

	if (propp->flags[Prop::Prop_Flags::Render_without_glowmap]) {
		debug_flags |= MR_DEBUG_NO_GLOW;
	}

	if (propp->flags[Prop::Prop_Flags::Render_without_normalmap]) {
		debug_flags |= MR_DEBUG_NO_NORMAL;
	}

	if (propp->flags[Prop::Prop_Flags::Render_without_ambientmap]) {
		debug_flags |= MR_DEBUG_NO_AMBIENT;
	}

	if (propp->flags[Prop::Prop_Flags::Render_without_specmap]) {
		debug_flags |= MR_DEBUG_NO_SPEC;
	}

	if (propp->flags[Prop::Prop_Flags::Render_without_reflectmap]) {
		debug_flags |= MR_DEBUG_NO_REFLECT;
	}

	render_info.set_flags(render_flags);
	render_info.set_debug_flags(debug_flags);

	render_info.set_object_number(OBJ_INDEX(obj));
	render_info.set_replacement_textures(pmi->texture_replace);

	model_render_queue(&render_info, scene, pip->model_num, &obj->orient, &obj->pos);
}

void spawn_test_prop()
{
	int prop_idx = prop_info_lookup("TestProp"); // whatever’s in your props.tbl
	if (prop_idx < 0) {
		mprintf(("TEST: Prop not found!\n"));
		return;
	}

	matrix mtx = vmd_identity_matrix;
	vec3d pos = ZERO_VECTOR;
	pos.xyz.z = -2000.0f;

	int objnum = prop_create(&mtx, &pos, prop_idx, "Test Prop");

	if (objnum >= 0) {
		mprintf(("TEST: Spawned prop '%s' at objnum %d\n", Prop_info[prop_idx].name, objnum));
	} else {
		mprintf(("TEST: Failed to create prop\n"));
	}
}

void props_level_close()
{
	for (auto& opt_prop : Props) {
		if (opt_prop.has_value()) {
			prop_delete(&Objects[opt_prop->objnum]);
		}
	}

	// Clear all props and empty prop slots
	Props.clear();
}


// handles prop-ship/debris/asteroid/weapon collisions
int prop_check_collision(object* prop_obj, object* other_obj, vec3d* hitpos, collision_info_struct* prop_hit_info)
{
	mc_info	mc;

	Assertion(prop_obj->type == OBJ_PROP, "Object is not a prop!");

	int num = prop_obj->instance;

	prop* propp = prop_id_lookup(num);
	Assertion(propp->objnum == OBJ_INDEX(prop_obj), "Prop object id does not match passed object!");
	
	prop_info* prinfo = &Prop_info[propp->prop_info_index];

	// debris_hit_info NULL - so debris-weapon collision
	if (prop_hit_info == NULL) {
		// debris weapon collision
		Assert(other_obj->type == OBJ_WEAPON);
		mc.model_instance_num = propp->model_instance_num;
		mc.model_num = prinfo->model_num;	// Fill in the model to check
		mc.orient = &prop_obj->orient;					// The object's orient
		mc.pos = &prop_obj->pos;							// The object's position
		mc.p0 = &other_obj->last_pos;				// Point 1 of ray to check
		mc.p1 = &other_obj->pos;					// Point 2 of ray to check
		mc.flags = MC_CHECK_MODEL;

		if (model_collide(&mc)) {
			*hitpos = mc.hit_point_world;
		}

		weapon* wp = &Weapons[other_obj->instance];
		wp->collisionInfo = new mc_info;	// The weapon will free this memory later
		*wp->collisionInfo = mc;

		return mc.num_hits;
	}

	object* heavy_obj = prop_hit_info->heavy;
	object* light_obj = prop_hit_info->light;

	vec3d zero;
	vm_vec_zero(&zero);
	vec3d p0 = light_obj->last_pos - heavy_obj->last_pos;
	vec3d p1 = light_obj->pos - heavy_obj->pos;

	mc.pos = &zero;								// The object's position
	mc.p0 = &p0;									// Point 1 of ray to check
	mc.p1 = &p1;									// Point 2 of ray to check

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;

	// Collision detection from rotation enabled if at rotation is less than 30 degree in frame
	// This should account for all ships
	if (heavy_obj->type == OBJ_SHIP && (vm_vec_mag_squared(&heavy_obj->phys_info.rotvel) * flFrametime * flFrametime) < (PI * PI / 36)) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		prop_hit_info->collide_rotate = true;
		vm_vec_rotate(&p0_temp, &p0, &heavy_obj->last_orient);
		vm_vec_unrotate(&p0_rotated, &p0_temp, &heavy_obj->orient);
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub(&prop_hit_info->light_rel_vel, &p1, &p0_rotated);
		vm_vec_scale(&prop_hit_info->light_rel_vel, 1 / flFrametime);
	}
	else {
		prop_hit_info->collide_rotate = false;
		vm_vec_sub(&prop_hit_info->light_rel_vel, &light_obj->phys_info.vel, &heavy_obj->phys_info.vel);
	}

	int mc_ret_val = 0;

	if (prop_hit_info->heavy == other_obj && other_obj->type == OBJ_SHIP) {	// ship is heavier, so prop is sphere. Check sphere collision against ship poly model
		mc.model_instance_num = Ships[other_obj->instance].model_instance_num;
		mc.model_num = Ship_info[Ships[other_obj->instance].ship_info_index].model_num;	// Fill in the model to check
		mc.orient = &other_obj->orient;								// The object's orient
		mc.radius = model_get_core_radius(prinfo->model_num);
		mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);

		// copy important data
		int orig_flags = mc.flags;  // make a copy of start end positions of sphere in big ship RF
		vec3d orig_p0 = *mc.p0;
		vec3d orig_p1 = *mc.p1;

		// first test against the sphere - if this fails then don't do any submodel tests
		mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

		if (model_collide(&mc)) {

			// Set earliest hit time
			prop_hit_info->hit_time = FLT_MAX;

			auto pmi = model_get_instance(Ships[heavy_obj->instance].model_instance_num);
			auto pm = model_get(pmi->model_num);

			// Do collision the cool new way
			if (prop_hit_info->collide_rotate) {
				// We collide with the sphere, find the list of moving submodels and test one at a time
				SCP_vector<int> submodel_vector;
				model_get_moving_submodel_list(submodel_vector, heavy_obj);

				// turn off all moving submodels, collide against only 1 at a time.
				// turn off collision detection for all moving submodels
				for (auto submodel : submodel_vector) {
					pmi->submodel[submodel].collision_checked = true;
				}

				// Only check single submodel now, since children of moving submodels are handled as moving as well
				mc.flags = orig_flags | MC_SUBMODEL;

				if (Ship_info[Ships[other_obj->instance].ship_info_index].collision_lod > -1) {
					mc.lod = Ship_info[Ships[other_obj->instance].ship_info_index].collision_lod;
				}

				// check each submodel in turn
				for (auto submodel : submodel_vector) {
					auto smi = &pmi->submodel[submodel];

					// turn on just one submodel for collision test
					smi->collision_checked = false;

					// find the start and end positions of the sphere in submodel RF
					model_instance_global_to_local_point(&p0, &light_obj->last_pos, pm, pmi, submodel, &heavy_obj->last_orient, &heavy_obj->last_pos, true);
					model_instance_global_to_local_point(&p1, &light_obj->pos, pm, pmi, submodel, &heavy_obj->orient, &heavy_obj->pos);

					mc.p0 = &p0;
					mc.p1 = &p1;

					mc.orient = &vmd_identity_matrix;
					mc.submodel_num = submodel;

					if (model_collide(&mc)) {
						if (mc.hit_dist < prop_hit_info->hit_time) {
							mc_ret_val = 1;

							// set up debris_hit_info common
							set_hit_struct_info(prop_hit_info, &mc, true);
							model_instance_local_to_global_point(&prop_hit_info->hit_pos, &mc.hit_point, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);

							// set up debris_hit_info for rotating submodel
							if (!prop_hit_info->edge_hit) {
								model_instance_local_to_global_dir(&prop_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
							}

							// find position in submodel RF of light object at collison
							vec3d diff = *mc.p1 - *mc.p0;
							vec3d int_light_pos = *mc.p0 + diff * mc.hit_dist;
							model_instance_local_to_global_point(&prop_hit_info->light_collision_cm_pos, &int_light_pos, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);
						}
					}

					// Don't look at this submodel again
					smi->collision_checked = true;
				}
			}

			// Now complete base model collision checks that do not take into account rotating submodels.
			mc.flags = orig_flags;
			mc.p0 = &orig_p0;
			mc.p1 = &orig_p1;
			mc.orient = &heavy_obj->orient;

			// usual ship_ship collision test
			if (model_collide(&mc)) {
				// check if this is the earliest hit
				if (mc.hit_dist < prop_hit_info->hit_time) {
					mc_ret_val = 1;

					set_hit_struct_info(prop_hit_info, &mc, false);

					// get collision normal if not edge hit
					if (!prop_hit_info->edge_hit) {
						model_instance_local_to_global_dir(&prop_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
					}

					// find position in submodel RF of light object at collison
					vec3d diff = *mc.p1 - *mc.p0;
					prop_hit_info->light_collision_cm_pos = *mc.p0 + diff * mc.hit_dist;
				}
			}
		}
	} else { // any case OTHER than big ship small prop, simpler code
		
		if (prop_obj == light_obj) {
			mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);
			int instance = heavy_obj->instance;

			// fill the appropriate model data
			if (heavy_obj->type == OBJ_DEBRIS) {
				mc.model_instance_num = -1;
				mc.model_num = Debris[instance].model_num;	
				mc.submodel_num = Debris[instance].submodel_num;
				mc.flags |= MC_SUBMODEL;
			} else if (heavy_obj->type == OBJ_ASTEROID) {
				asteroid* astp = &Asteroids[instance];
				mc.model_instance_num = astp->model_instance_num;
				mc.model_num = Asteroid_info[astp->asteroid_type].subtypes[astp->asteroid_subtype].model_number;	
				mc.submodel_num - 1; // Typo from asteroth?
			}

			mc.orient = &heavy_obj->orient;				// The object's orient
			mc.radius = model_get_core_radius(prinfo->model_num);

			mc_ret_val = model_collide(&mc);

			if (mc_ret_val) {
				set_hit_struct_info(prop_hit_info, &mc, false);

				// set normal if not edge hit
				if (!prop_hit_info->edge_hit) {
					vm_vec_unrotate(&prop_hit_info->collision_normal, &mc.hit_normal, &heavy_obj->orient);
				}

				// find position in submodel RF of light object at collison
				vec3d diff = *mc.p1 - *mc.p0;
				prop_hit_info->light_collision_cm_pos = *mc.p0 + diff * mc.hit_dist;

			}
		} else { // prop is the heavy object
			mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);
			//int instance = heavy_obj->instance;

			// fill the appropriate model data
			mc.model_instance_num = propp->model_instance_num;
			mc.model_num = prinfo->model_num;

			mc.orient = &heavy_obj->orient;

			switch (light_obj->type) {
			case OBJ_SHIP:
				mc.radius = model_get_core_radius(Ship_info[Ships[light_obj->instance].ship_info_index].model_num);
				break;
			case OBJ_ASTEROID: // FALLTHROUGH
			case OBJ_DEBRIS:
				mc.radius = light_obj->radius;
				break;
			default:
				UNREACHABLE("Unknown object type in prop_check_collision");
			};

			mc_ret_val = model_collide(&mc);

			if (mc_ret_val) {
				set_hit_struct_info(prop_hit_info, &mc, false);

				// set normal if not edge hit
				if (!prop_hit_info->edge_hit) {
					vm_vec_unrotate(&prop_hit_info->collision_normal, &mc.hit_normal, &heavy_obj->orient);
				}

				// find position in submodel RF of light object at collison
				vec3d diff = *mc.p1 - *mc.p0;
				prop_hit_info->light_collision_cm_pos = *mc.p0 + diff * mc.hit_dist;
			}
		}
	}

	if (mc_ret_val && other_obj->type == OBJ_SHIP) {
		WarpEffect* warp_effect = nullptr;
		ship* shipp = &Ships[other_obj->instance];

		// this is extremely confusing but mc.hit_point_world isn't actually in world coords
		// everything above was calculated relative to the heavy's position
		vec3d actual_world_hit_pos = mc.hit_point_world + heavy_obj->pos;
		if ((shipp->is_arriving()) && (shipp->warpin_effect != nullptr))
			warp_effect = shipp->warpin_effect;
		else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && (shipp->warpout_effect != nullptr))
			warp_effect = shipp->warpout_effect;

		if (warp_effect != nullptr && point_is_clipped_by_warp(&actual_world_hit_pos, warp_effect))
			mc_ret_val = 0;
	}


	if (mc_ret_val) {
		// SET PHYSICS PARAMETERS
		// already have (hitpos - heavy) and light_cm_pos

		// get r_heavy and r_light
		prop_hit_info->r_heavy = prop_hit_info->hit_pos;
		prop_hit_info->r_light = prop_hit_info->hit_pos - prop_hit_info->light_collision_cm_pos;

		// set normal for edge hit
		if (prop_hit_info->edge_hit) {
			vm_vec_copy_normalize(&prop_hit_info->collision_normal, &prop_hit_info->r_light);
			vm_vec_negate(&prop_hit_info->collision_normal);
		}

		// get world hitpos
		vm_vec_add(hitpos, &prop_hit_info->heavy->pos, &prop_hit_info->r_heavy);

		return 1;
	}
	else {
		// no hit
		return 0;
	}
}