#pragma once

#include "globalincs/pstypes.h"

#include "mission/mission_flags.h"
#include "object/object.h"
#include "prop/prop_flags.h"
#include "ship/ship.h"

#define MAX_PROP_DETAIL_LEVELS    MAX_SHIP_DETAIL_LEVELS

typedef struct prop_info {
	char name[NAME_LENGTH];               // Prop name
	char pof_file[MAX_FILENAME_LEN];      // Pof filename
	int model_num;                        // The model number of the loaded POF
	//int model_instance;                   // The model instance
	int num_detail_levels;                // Detail levels of the model
	int detail_distance[MAX_PROP_DETAIL_LEVELS]; // distance to change detail levels at
	SCP_unordered_map<int, void*> glowpoint_bank_override_map;
	flagset<Prop::Info_Flags> flags; // Info flags
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
	std::deque<bool> glow_point_bank_active;
	flagset<Prop::Prop_Flags> flags;
} prop;

typedef struct parsed_prop {
	char name[NAME_LENGTH];
	int prop_info_index;
	matrix orientation;
	vec3d position;
	flagset<Mission::Parse_Object_Flags> flags;
} parsed_prop;

// Global prop info array
extern SCP_vector<prop_info> Prop_info;

extern SCP_vector<prop> Props;

// Load all props from table
void prop_init();

// Object management
int prop_create(matrix* orient, vec3d* pos, int prop_type, const char* ship_name = nullptr);
void prop_delete(object* obj);
void prop_render(object* obj, model_draw_list* scene);

void props_level_close();

void spawn_test_prop();