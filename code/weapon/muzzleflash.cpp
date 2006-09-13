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
 * $Revision: 2.11 $
 * $Date: 2006-09-13 03:56:29 $
 * $Author: taylor $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.10  2006/09/11 06:51:17  taylor
 * fixes for stuff_string() bounds checking
 *
 * Revision 2.9  2006/09/11 05:44:23  taylor
 * make muzzle flash info dynamic
 * add support for modular mflash tables (*-mfl.tbm)
 *
 * Revision 2.8  2006/06/07 04:48:38  wmcoolmon
 * Limbo flag support; removed unneeded muzzle flash flag
 *
 * Revision 2.7  2005/10/30 06:44:59  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 2.6  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.5  2004/07/26 20:47:56  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:33:09  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
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

#include <vector>

#include "globalincs/systemvars.h"
#include "parse/parselo.h"
#include "weapon/muzzleflash.h"
#include "particle/particle.h"
#include "graphics/2d.h"
#include "math/vecmat.h"


// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// muzzle flash info - read from a table
typedef struct mflash_blob_info {
	char name[MAX_FILENAME_LEN];
	int anim_id;
	float offset;
	float radius;

	mflash_blob_info() { memset(this, 0, sizeof(mflash_blob_info)); anim_id = -1; };
} mflash_blob_info;

typedef struct mflash_info {
	char name[MAX_FILENAME_LEN];
	int	 used_this_level;
	std::vector<mflash_blob_info> blobs;

	mflash_info() { memset(this, 0, sizeof(mflash_info)); };
} mflash_info;

std::vector<mflash_info> Mflash_info;


//#define MAX_MFLASH				50

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

void parse_mflash_tbl(char *filename)
{
	int rval;
	uint i;

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("Unable to parse %s!  Code = %i.\n", rval, filename));
	} else {
		read_file_text(filename);
		reset_parse();		
	}

	// header
	required_string("#Muzzle flash types");

	while ( optional_string("$Mflash:") ) {
		mflash_info mflash;

		required_string("+name:");
		stuff_string(mflash.name, F_NAME, MAX_FILENAME_LEN);

		// read in all blobs
		while ( optional_string("+blob_name:") ) {
			mflash_blob_info mblob;

			stuff_string(mblob.name, F_NAME, MAX_FILENAME_LEN);

			required_string("+blob_offset:");
			stuff_float(&mblob.offset);

			required_string("+blob_radius:");
			stuff_float(&mblob.radius);

			mflash.blobs.push_back(mblob);
		}

		for (i = 0; i < Mflash_info.size(); i++) {
			if ( !stricmp(mflash.name, Mflash_info[i].name) )
				break;
		}

		// no matching name exists so add as new
		if (i == Mflash_info.size()) {
			Mflash_info.push_back(mflash);
		}
		// a mflash of the same name exists, don't add it again
		else {
			Warning(LOCATION, "Muzzle flash \"%s\" already exists!  Using existing entry instead.", mflash.name);
		}
	}

	// close
	required_string("#end");
}

// initialize muzzle flash stuff for the whole game
void mflash_game_init()
{
	// parse main table first
	parse_mflash_tbl("mflash.tbl");

	// look for any modular tables
	parse_modular_table(NOX("*-mfl.tbm"), parse_mflash_tbl);
}

void mflash_mark_as_used(int index)
{
	if (index < 0)
		return;

	Assert( index < (int)Mflash_info.size() );

	Mflash_info[index].used_this_level++;
}

void mflash_page_in(bool load_all)
{
	uint i, idx;
	int num_frames, fps;

	// load up all anims
	for ( i = 0; i < Mflash_info.size(); i++) {
		// skip if it's not used
		if ( !load_all && !Mflash_info[i].used_this_level )
			continue;

		// blobs
		for ( idx = 0; idx < Mflash_info[i].blobs.size(); idx++) {
			Mflash_info[i].blobs[idx].anim_id = bm_load_animation(Mflash_info[i].blobs[idx].name, &num_frames, &fps, 1);
		//	Assert( Mflash_info[i].blobs[idx].anim_id >= 0 );
			if (Mflash_info[i].blobs[idx].anim_id < 0)
				Warning(LOCATION, "Missing muzzle flash blob '%s' in mflash.tbl", Mflash_info[i].blobs[idx].name);
			else
				bm_page_in_xparent_texture( Mflash_info[i].blobs[idx].anim_id );
		}
	}
}

// initialize muzzle flash stuff for the level
void mflash_level_init()
{
	uint i, idx;

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

	// reset all anim usage for this level
	for ( i = 0; i < Mflash_info.size(); i++) {
		for ( idx = 0; idx < Mflash_info[i].blobs.size(); idx++) {
			Mflash_info[i].used_this_level = 0;
		}
	}
}

// shutdown stuff for the level
void mflash_level_close()
{
	uint i, idx;

	// release all anims
	for ( i = 0; i < Mflash_info.size(); i++) {
		// blobs
		for ( idx = 0; idx < Mflash_info[i].blobs.size(); idx++) {
			if ( Mflash_info[i].blobs[idx].anim_id < 0 )
				continue;

			bm_release( Mflash_info[i].blobs[idx].anim_id );
			Mflash_info[i].blobs[idx].anim_id = -1;
		}
	}
}

// create a muzzle flash on the guy
void mflash_create(vec3d *gun_pos, vec3d *gun_dir, physics_info *pip, int mflash_type)
{	
	// mflash *mflashp;
	mflash_info *mi;
	mflash_blob_info *mbi;
	particle_info p;
	uint idx;

	// standalone server should never create trails
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// illegal value
	if ( (mflash_type < 0) || (mflash_type >= (int)Mflash_info.size()) )
		return;

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

	for (idx = 0; idx < mi->blobs.size(); idx++) {
		mbi = &mi->blobs[idx];

		// bogus anim
		if (mbi->anim_id < 0)
			continue;

		// fire it up
		memset(&p, 0, sizeof(particle_info));
		vm_vec_scale_add(&p.pos, gun_pos, gun_dir, mbi->offset);
		vm_vec_scale_add(&p.vel, &pip->rotvel, &pip->vel, 1.0f);
		p.rad = mbi->radius;
		p.type = PARTICLE_BITMAP;
		p.optional_data = mbi->anim_id;
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
	uint idx;

	// look it up
	for (idx = 0; idx < Mflash_info.size(); idx++) {
		if ( !stricmp(name, Mflash_info[idx].name) )
			return idx;
	}

	// couldn't find it
	return -1;	
}
