#pragma once

#include "globalincs/pstypes.h"

#include "mission/mission_flags.h"
#include "object/object.h"
#include "object/objcollide.h"
#include "prop/prop_flags.h"
#include "ship/ship.h"

#define MAX_PROP_DETAIL_LEVELS    MAX_SHIP_DETAIL_LEVELS

typedef struct prop_info {
	SCP_string name;                                            // Prop name
	int category_index;                                         // Index of the category this prop belongs to, -1 if it doesn't belong to any category
	SCP_string pof_file;                                        // Pof filename
	vec3d closeup_pos;                                          // position for camera when using prop in closeup view (eg briefing and techroom)
	float closeup_zoom;                                         // zoom when using prop in closeup view (eg briefing and techroom)
	int model_num;                                              // The model number of the loaded POF
	int num_detail_levels;                                      // Detail levels of the model
	int detail_distance[MAX_PROP_DETAIL_LEVELS];                // distance to change detail levels at
	SCP_unordered_map<int, void*> glowpoint_bank_override_map;  // Glow point bank overrides currently unused
	flagset<Prop::Info_Flags> flags;                            // Info flags
	SCP_map<SCP_string, SCP_string> custom_data;                // Custom data for this prop
	SCP_vector<custom_string> custom_strings;                   // Custom strings for this prop
} prop_info;

typedef struct prop {
	char prop_name[NAME_LENGTH];
	int objnum;
	int prop_info_index;
	int model_instance_num;
	uint create_time;                     // time prop was created, set by gettime()
	fix time_created;
	float alpha_mult;
	// glow points
	SCP_deque<bool> glow_point_bank_active;
	flagset<Prop::Prop_Flags> flags;
} prop;

typedef struct prop_category {
	SCP_string name;
	color list_color;
} prop_category;

typedef struct parsed_prop {
	char name[NAME_LENGTH];
	int prop_info_index;
	matrix orientation;
	vec3d position;
	flagset<Mission::Parse_Object_Flags> flags;
} parsed_prop;

extern bool Props_inited;

// Global prop info
extern SCP_vector<prop_info> Prop_info;

// Global prop categories
extern SCP_vector<prop_category> Prop_categories;

// Global prop objects. Vector of optionals so that we can have stable indices
// and still be able to remove props. Deleted props are set to std::nullopt
// so any access should check if the optional has a value first.
// The vector is cleared at the end of each mission, never during.
extern SCP_vector<std::optional<prop>> Props;

inline int prop_info_size()
{
	return static_cast<int>(Prop_info.size());
}

// Load all props from table
void prop_init();

// Object management
int prop_create(const matrix* orient, const vec3d* pos, int prop_type, const char* name = nullptr);
void prop_delete(object* obj);
void prop_render(object* obj, model_draw_list* scene);

void props_level_init();
void props_level_close();

int prop_info_lookup(const char* token);
int prop_name_lookup(const char* name);
prop* prop_id_lookup(int id);

void change_prop_type(int n, int prop_type);

prop_category* prop_get_category(int index);

int prop_check_collision(object* prop_obj, object* other_obj, vec3d* hitpos, collision_info_struct* prop_hit_info);