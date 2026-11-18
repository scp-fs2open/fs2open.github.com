#pragma once

// The two halves of a large ship that breaks apart when it is destroyed

#include "globalincs/pstypes.h"
#include "model/model.h"
#include "physics/physics.h"
#include "ship/ship.h"
#include "sound/sound.h"

typedef struct clip_ship {
	object*			parent_obj;
	float 			length_left;	// uncomsumed length
	matrix			orient;
	physics_info	phys_info;
	vec3d			local_pivot;								// world center of mass position of half ship
	vec3d			model_center_disp_to_orig_center;	// displacement from half ship center to original model center
	vec3d			clip_plane_norm;							// clip plane normal (local [0,0,1] or [0,0,-1])
	float				cur_clip_plane_pt;						// displacement from half ship clip plane to original model center
	float				explosion_vel;
	ubyte				draw_debris[MAX_DEBRIS_OBJECTS];
	int				next_fireball;
} clip_ship;

typedef struct split_ship {
	int used = 0; // 0 if not used, 1 if used
	clip_ship		front_ship;
	clip_ship		back_ship;
	int explosion_flash_timestamp = 0;
	int explosion_flash_started   = 0;
	std::array<sound_handle, NUM_SUB_EXPL_HANDLES> sound_handles;

	split_ship()
	{
		memset(&front_ship, 0, sizeof(front_ship));
		memset(&back_ship, 0, sizeof(back_ship));
		sound_handles.fill(sound_handle::invalid());
	}
} split_ship;
