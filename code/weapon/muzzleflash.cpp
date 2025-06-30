/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "math/vecmat.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "particle/particle.h"
#include "weapon/muzzleflash.h"
#include "model/modelrender.h"


// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// muzzle flash info - read from a table
typedef struct mflash_blob_info {
	char name[MAX_FILENAME_LEN];
	int anim_id;
	float offset;
	float radius;

	mflash_blob_info( const mflash_blob_info& mbi )
	{
		strcpy_s( name, mbi.name );
		anim_id = mbi.anim_id;
		offset = mbi.offset;
		radius = mbi.radius;
	}

	mflash_blob_info() :
		anim_id( -1 ),
		offset( 0.0 ),
		radius( 0.0 )
	{ 
		name[ 0 ] = '\0';
	}

	mflash_blob_info& operator=( const mflash_blob_info& r )
	{
		strcpy_s( name, r.name );
		anim_id = r.anim_id;
		offset = r.offset;
		radius = r.radius;

		return *this;
	}
} mflash_blob_info;

typedef struct mflash_info {
	char name[MAX_FILENAME_LEN];
	SCP_vector<mflash_blob_info> blobs;

	mflash_info()
	{ 
		name[ 0 ] = '\0';
	}

	mflash_info( const mflash_info& mi )
	{
		strcpy_s( name, mi.name );
		blobs = mi.blobs;
	}

	mflash_info& operator=( const mflash_info& r )
	{
		strcpy_s( name, r.name );
		blobs = r.blobs;

		return *this;
	}
} mflash_info;

SCP_vector<mflash_info> Mflash_info;


// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH FUNCTIONS
//

static const SCP_string mflash_particle_prefix = ";MflashParticle;";

void parse_mflash_tbl(const char *filename)
{
	uint i;

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// header
		required_string("#Muzzle flash types");

		while (optional_string("$Mflash:")) {
			mflash_info mflash;
			bool override_mflash = false;

			required_string("+name:");
			stuff_string(mflash.name, F_NAME, MAX_FILENAME_LEN);

			if (optional_string("+override"))
				override_mflash = true;

			// read in all blobs
			while (optional_string("+blob_name:")) {
				mflash_blob_info mblob;

				stuff_string(mblob.name, F_NAME, MAX_FILENAME_LEN);

				required_string("+blob_offset:");
				stuff_float(&mblob.offset);

				required_string("+blob_radius:");
				stuff_float(&mblob.radius);

				mflash.blobs.push_back(mblob);
			}

			for (i = 0; i < Mflash_info.size(); i++) {
				if (!stricmp(mflash.name, Mflash_info[i].name)) {
					if (override_mflash) {
						Mflash_info[i] = mflash;
					}
					break;
				}
			}

			// no matching name exists so add as new
			if (i == Mflash_info.size()) {
				Mflash_info.push_back(mflash);
			}
			// a mflash of the same name exists, don't add it again
			else {
				if (!override_mflash) {
					Warning(LOCATION, "Muzzle flash \"%s\" already exists!  Using existing entry instead.", mflash.name);
				}
			}
		}

		// close
		required_string("#end");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

static void convert_mflash_to_particle() {


	//Clean up no longer required data
	Mflash_info.clear();
	Mflash_info.shrink_to_fit();
}

// initialize muzzle flash stuff for the whole game
void mflash_game_init()
{
	// parse main table first
	parse_mflash_tbl("mflash.tbl");

	// look for any modular tables
	parse_modular_table(NOX("*-mfl.tbm"), parse_mflash_tbl);

	//This should really happen at parse time, but that requires modular particle effects which aren't yet a thing
	convert_mflash_to_particle();
}

particle::ParticleEffectHandle mflash_lookup(const char *name) {
	return particle::ParticleManager::get()->getEffectByName(mflash_particle_prefix + name);
}

// create a muzzle flash on the guy
void mflash_create(const vec3d *gun_pos, const vec3d *gun_dir, const physics_info *pip, int mflash_type, const object *local)
{	
	// mflash *mflashp;
	mflash_info *mi;
	mflash_blob_info *mbi;
	uint idx;

	// standalone server should never create trails
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// illegal value
	if ( (mflash_type < 0) || (mflash_type >= (int)Mflash_info.size()) )
		return;

	// create the actual animations	
	mi = &Mflash_info[mflash_type];

	if (local != NULL) {
		int attached_objnum = OBJ_INDEX(local);

		// This muzzle flash is in local space, so its world position must be derived to apply scaling.
		vec3d gun_world_pos;
		vm_vec_unrotate(&gun_world_pos, gun_pos, &Objects[attached_objnum].orient);
		vm_vec_add2(&gun_world_pos, &Objects[attached_objnum].pos);

		for (idx = 0; idx < mi->blobs.size(); idx++) {
			mbi = &mi->blobs[idx];

			// bogus anim
			if (mbi->anim_id < 0)
				continue;

			// fire it up
			particle::particle_info p;
			vm_vec_scale_add(&p.pos, gun_pos, gun_dir, mbi->offset);
			vm_vec_zero(&p.vel);
			//vm_vec_scale_add(&p.vel, &pip->rotvel, &pip->vel, 1.0f);
			p.bitmap = mbi->anim_id;
			p.attached_objnum = attached_objnum;
			p.attached_sig = local->signature;

			// Scale the radius of the muzzle flash effect so that it always appears some minimum width in pixels.
			p.rad = model_render_get_diameter_clamped_to_min_pixel_size(&gun_world_pos, mbi->radius * 2.0f, Min_pizel_size_muzzleflash) / 2.0f;

			particle::create(&p);
		}
	} else {
		for (idx = 0; idx < mi->blobs.size(); idx++) {
			mbi = &mi->blobs[idx];

			// bogus anim
			if (mbi->anim_id < 0)
				continue;

			// fire it up
			particle::particle_info p;
			vm_vec_scale_add(&p.pos, gun_pos, gun_dir, mbi->offset);
			vm_vec_scale_add(&p.vel, &pip->rotvel, &pip->vel, 1.0f);
			p.bitmap = mbi->anim_id;
			p.attached_objnum = -1;
			p.attached_sig = 0;

			// Scale the radius of the muzzle flash effect so that it always appears some minimum width in pixels.
			p.rad = model_render_get_diameter_clamped_to_min_pixel_size(&p.pos, mbi->radius * 2.0f, Min_pizel_size_muzzleflash) / 2.0f;

			particle::create(&p);
		}
	}		
}
