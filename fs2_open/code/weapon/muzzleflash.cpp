/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/MuzzleFlash.cpp $
 * $Revision: 2.4 $
 * $Date: 2004-07-12 16:33:09 $
 * $Author: Kazan $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/03/18 10:07:06  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/11/04 03:02:29  randomtiger
 *
 * I have made some fairly drastic changes to the bumpman system. Now functionality can be engine dependant.
 * This is so D3D8 can call its own loading code that will allow good efficient loading and use of textures that it desparately needs without
 * turning bumpman.cpp into a total hook infested nightmare. Note the new bumpman code is still relying on a few of the of the old functions and all of the old bumpman arrays.
 *
 * I have done this by adding to the gr_screen list of function pointers that are set up by the engines init functions.
 * I have named the define calls the same name as the original 'bm_' functions so that I havent had to change names all through the code.
 *
 * Rolled back to an old version of bumpman and made a few changes.
 * Added new files: grd3dbumpman.cpp and .h
 * Moved the bitmap init function to after the 3D engine is initialised
 * Added includes where needed
 * Disabled (for now) the D3D8 TGA loading - RT
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 6     5/18/99 1:30p Dave
 * Added muzzle flash table stuff.
 * 
 * 5     4/25/99 3:02p Dave
 * Build defines for the E3 build.
 * 
 * 4     4/12/99 11:03p Dave
 * Removed contrails and muzzle flashes from MULTIPLAYER_BETA builds.
 * 
 * 3     3/19/99 9:52a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 2     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 
 * $NoKeywords: $
 */

#include "globalincs/systemvars.h"
#include "parse/parselo.h"
#include "weapon/muzzleflash.h"
#include "particle/particle.h"
#include "graphics/2d.h"
#include "math/vecmat.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// muzzle flash info - read from a table
#define MAX_MFLASH_NAME_LEN		32
#define MAX_MFLASH_BLOBS			5
typedef struct mflash_info {
	char		name[MAX_MFLASH_NAME_LEN+1];
	char		blob_names[MAX_MFLASH_BLOBS][MAX_MFLASH_NAME_LEN+1];			// blob anim name
	int		blob_anims[MAX_MFLASH_BLOBS];											// blob anim
	float		blob_offset[MAX_MFLASH_BLOBS];										// blob offset from muzzle
	float		blob_radius[MAX_MFLASH_BLOBS];										// blob radius
	int		num_blobs;																	// # of blobs
} mflash_info;
mflash_info Mflash_info[MAX_MUZZLE_FLASH_TYPES];
int Num_mflash_types = 0;

#define MAX_MFLASH				50

// Stuff for missile trails doesn't need to be saved or restored... or does it?
/*
typedef struct mflash {	
	struct	mflash * prev;
	struct	mflash * next;

	ubyte		type;																			// muzzle flash type
	int		blobs[MAX_MFLASH_BLOBS];												// blobs
} mflash;

int Num_mflash = 0;
mflash Mflash[MAX_MFLASH];

mflash Mflash_free_list;
mflash Mflash_used_list;
*/

// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH FUNCTIONS
// 

// initialize muzzle flash stuff for the whole game
void mflash_game_init()
{
	mflash_info bogus;
	mflash_info *m;	
	char name[MAX_MFLASH_NAME_LEN];
	float offset, radius;
	int idx;

	read_file_text("mflash.tbl");
	reset_parse();

	// header
	required_string("#Muzzle flash types");

	// read em in
	Num_mflash_types = 0;	
	while(optional_string("$Mflash:")){
		if(Num_mflash_types < MAX_MUZZLE_FLASH_TYPES){
			m = &Mflash_info[Num_mflash_types++];
		} else {
			m = &bogus;
		}
		memset(m, 0, sizeof(mflash_info));
		for(idx=0; idx<MAX_MFLASH_BLOBS; idx++){
			m->blob_anims[idx] = -1;
		}

		required_string("+name:");
		stuff_string(m->name, F_NAME, NULL);

		// read in all blobs
		m->num_blobs = 0;
		while(optional_string("+blob_name:")){
			stuff_string(name, F_NAME, NULL, MAX_MFLASH_NAME_LEN);

			required_string("+blob_offset:");
			stuff_float(&offset);

			required_string("+blob_radius:");
			stuff_float(&radius);

			// if we have room left
			if(m->num_blobs < MAX_MFLASH_BLOBS){
				strcpy(m->blob_names[m->num_blobs], name);
				m->blob_offset[m->num_blobs] = offset;
				m->blob_radius[m->num_blobs] = radius;				

				m->num_blobs++;
			}
		}
	}

	// close
	required_string("#end");
}

// initialize muzzle flash stuff for the level
void mflash_level_init()
{
	int i, idx;
	int num_frames, fps;

	/*
	Num_mflash = 0;
	list_init( &Mflash_free_list );
	list_init( &Mflash_used_list );

	// Link all object slots into the free list
	for (i=0; i<MAX_MFLASH; i++)	{
		memset(&Mflash[i], 0, sizeof(mflash));
		list_append(&Mflash_free_list, &Mflash[i] );
	}
	*/

	// load up all anims
	for(i=0; i<Num_mflash_types; i++){
		// blobs
		for(idx=0; idx<Mflash_info[i].num_blobs; idx++){
			Mflash_info[i].blob_anims[idx] = -1;
			Mflash_info[i].blob_anims[idx] = bm_load_animation(Mflash_info[i].blob_names[idx], &num_frames, &fps, 1);
			Assert(Mflash_info[i].blob_anims[idx] >= 0);
		}
	}
}

// shutdown stuff for the level
void mflash_level_close()
{
}

// create a muzzle flash on the guy
void mflash_create(vector *gun_pos, vector *gun_dir, int mflash_type)
{	
	// mflash *mflashp;
	mflash_info *mi;
	particle_info p;
	int idx;

	// standalone server should never create trails
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// illegal value
	if((mflash_type >= Num_mflash_types) || (mflash_type < 0)){
		return;
	}

	/*
	if (Num_mflash >= MAX_MFLASH ) {
		#ifndef NDEBUG
		mprintf(("Muzzle flash creation failed - too many trails!\n" ));
		#endif
		return;
	}

	// Find next available trail
	mflashp = GET_FIRST(&Mflash_free_list);
	Assert( mflashp != &Mflash_free_list );		// shouldn't have the dummy element

	// remove trailp from the free list
	list_remove( &Mflash_free_list, mflashp );
	
	// insert trailp onto the end of used list
	list_append( &Mflash_used_list, mflashp );

	// store some stuff
	mflashp->type = (ubyte)mflash_type;	
	*/

	// create the actual animations	
	mi = &Mflash_info[mflash_type];
	for(idx=0; idx<mi->num_blobs; idx++){		

		// bogus anim
		if(mi->blob_anims[idx] < 0){
			continue;
		}
		
		// fire it up
		memset(&p, 0, sizeof(particle_info));
		vm_vec_scale_add(&p.pos, gun_pos, gun_dir, mi->blob_offset[idx]);
		p.vel = vmd_zero_vector;		
		p.rad = mi->blob_radius[idx];
		p.type = PARTICLE_BITMAP;
		p.optional_data = mi->blob_anims[idx];
		p.attached_objnum = -1;
		p.attached_sig = 0;
		particle_create(&p);
	}

	// increment counter
	// Num_mflash++;		
}

// process muzzle flash stuff
void mflash_process_all()
{
	/*
	mflash *mflashp;

	// if the timestamp has elapsed recycle it
	mflashp = GET_FIRST(&Mflash_used_list);

	while ( mflashp!=END_OF_LIST(&Mflash_used_list) )	{			
		if((mflashp->stamp == -1) || timestamp_elapsed(mflashp->stamp)){
			// delete it from the list!
			mflash *next_one = GET_NEXT(mflashp);

			// remove objp from the used list
			list_remove( &Mflash_used_list, mflashp );

			// add objp to the end of the free
			list_append( &Mflash_free_list, mflashp );

			// decrement counter
			Num_mflash--;

			Assert(Num_mflash >= 0);
			
			mflashp = next_one;			
		} else {	
			mflashp = GET_NEXT(mflashp);
		}
	}
	*/
}

void mflash_render_all()
{
}

// lookup type by name
int mflash_lookup(char *name)
{	
	int idx;

	// look it up
	for(idx=0; idx<Num_mflash_types; idx++){
		if(!stricmp(name, Mflash_info[idx].name)){
			return idx;
		}
	}

	// couldn't find it
	return -1;	
}
