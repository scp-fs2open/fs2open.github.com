#include "prop.h"

#include "model/model.h"
#include "parse/parselo.h"
#include "render/3d.h"

#include "tracing/Monitor.h"

MONITOR(NumPropsRend)

SCP_vector<prop_info> Prop_info;

SCP_vector<prop> Props;

static SCP_vector<SCP_string> Removed_props;

/**
 * Return the index of Prop_info[].name that is *token.
 */
static int prop_info_lookup_sub(const char* token)
{
	Assertion(token != nullptr, "NULL token passed to prop_info_lookup_sub");

	for (auto it = Prop_info.cbegin(); it != Prop_info.cend(); ++it)
		if (!stricmp(token, it->name))
			return (int)std::distance(Prop_info.cbegin(), it);

	return -1;
}

int prop_name_lookup(const char *name)
{
	Assertion(name != nullptr, "NULL name passed to prop_name_lookup");

	for (int i=0; i<static_cast<int>(Props.size()); i++){
		if (Props[i].objnum >= 0){
			if (Objects[Props[i].objnum].type == OBJ_PROP){
				if (!stricmp(name, Props[i].prop_name)){
					return i;
				}
			}
		}
	}
	
	// couldn't find it
	return -1;
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
		int prop_id = prop_info_lookup_sub(fname);

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

		if(optional_string("$Detail distance:")) {
			pip->num_detail_levels = (int)stuff_int_list(pip->detail_distance, MAX_PROP_DETAIL_LEVELS, RAW_INTEGER_TYPE);
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
	propp = &Props.back();
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
	// JAS: Nav buoys don't need to do collisions!
	// G5K: Corrected to apply specifically for ships with the no-collide flag.  (In retail, navbuoys already have this
	// flag, so this doesn't break anything.)
	default_ship_object_flags.set(Object::Object_Flags::Collides, !pip->flags[Prop::Info_Flags::No_collide]);

	int objnum = obj_create(OBJ_PROP,
		-1,
		static_cast<int>(Props.size() - 1),
		orient,
		pos,
		model_get_radius(pip->model_num),
		default_ship_object_flags);
	Assert(objnum >= 0);

	propp->objnum = objnum;

	propp->model_instance_num = model_create_instance(objnum, pip->model_num);

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
	Assert(num >= 0 && num <= static_cast<int>(Props.size()));

	int objnum = OBJ_INDEX(obj);
	Assert(Props[num].objnum == objnum);

	prop* propp = &Props[num];

	propp->objnum = -1;

	//animation::ModelAnimationSet::stopAnimations(model_get_instance(propp->model_instance_num));

	// glow point banks
	propp->glow_point_bank_active.clear();

	model_delete_instance(propp->model_instance_num);

	Props.erase(Props.begin() + num);
}

void prop_render(object* obj, model_draw_list* scene)
{
	int num = obj->instance;
	Assert(num >= 0 && num <= static_cast<int>(Props.size()));

	prop* propp = &Props[num];
	prop_info* pip = &Prop_info[Props[num].prop_info_index];

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
	int prop_idx = prop_info_lookup_sub("TestProp"); // whatever’s in your props.tbl
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
	while (!Props.empty()) {
		prop_delete(&Objects[Props.back().objnum]);
	}
}