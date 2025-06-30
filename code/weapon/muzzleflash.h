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
#include "particle/ParticleManager.h"

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// prototypes
class object;
struct vec3d;

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH FUNCTIONS
// 

// initialize muzzle flash stuff for the whole game
void mflash_game_init();

// create a muzzle flash on the guy
void mflash_create(const vec3d *gun_pos, const vec3d *gun_dir, const physics_info *pip, int mflash_type, const object *local = nullptr);

// lookup type by name
particle::ParticleEffectHandle mflash_lookup(const char *name);

#endif
