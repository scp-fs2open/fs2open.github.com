/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FS2_MUZZLEFLASH_HEADER_FILE
#define __FS2_MUZZLEFLASH_HEADER_FILE

#include "physics/physics.h"

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// prototypes
struct object;
struct vec3d;

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH FUNCTIONS
// 

// initialize muzzle flash stuff for the whole game
void mflash_game_init();

// initialize muzzle flash stuff for the level
void mflash_level_init();

// shutdown stuff for the level
void mflash_level_close();

// create a muzzle flash on the guy
void mflash_create(vec3d *gun_pos, vec3d *gun_dir, physics_info *pip, int mflash_type, object *local = NULL);

// lookup type by name
int mflash_lookup(char *name);

// mark as used
void mflash_mark_as_used(int index = -1);

// level page in
void mflash_page_in(bool load_all = false);

#endif
