/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Nebula/NebLightning.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:00 $
 * $Author: penguin $
 *
 * Nebula effect
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 4     7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 3     5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 2     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * $NoKeywords: $
 */

#ifndef __FS2_NEBULA_LIGHTNING_HEADER_FILE
#define __FS2_NEBULA_LIGHTNING_HEADER_FILE

// ------------------------------------------------------------------------------------------------------
// NEBULA LIGHTNING DEFINES/VARS
//

// lightning bolt types
#define MAX_BOLT_TYPES					10
#define DEBUG_BOLT						MAX_BOLT_TYPES

// storm types - needs to be here for Fred. blech
#define MAX_STORM_TYPES							10
typedef struct storm_type {
	char		name[NAME_LENGTH];

	ubyte		num_bolt_types;					// how many different bolt types you'll see in the nebula

	char		bolt_types[MAX_BOLT_TYPES];	// indices into the lightning types	

	vector	flavor;								// flavor of the storm

	int		min, max;							// min and max delay between bolt firing.	
	int		min_count, max_count;			// # of bolts spewed
} storm_type;

extern int Num_storm_types;
extern storm_type Storm_types[MAX_STORM_TYPES];

// nebula lightning intensity (0.0 to 1.0)
extern float Nebl_intensity;

// min and max times for random lightning
extern int Nebl_random_min;			// min random time
extern int Nebl_random_max;			// max random time

// min and max times for cruiser lightning
extern int Nebl_cruiser_min;			// min cruiser time
extern int Nebl_cruiser_max;			// max cruiser time

// min and max times for cap ships
extern int Nebl_cap_min;				// min cap time
extern int Nebl_cap_max;				// max cap time

// min and max time for super caps
extern int Nebl_supercap_min;			// min supercap time
extern int Nebl_supercap_max;			// max supercap time

#define BOLT_TYPE_ANY					-2

// ------------------------------------------------------------------------------------------------------
// NEBULA LIGHTNING FUNCTIONS
//

// initialize nebula lightning at game startup
void nebl_init();

// initialize lightning before entering a level
void nebl_level_init();

// render all lightning bolts
void nebl_render_all();

// create a lightning bolt - pass BOLT_TYPE_ANY to get a random bolt
void nebl_bolt(int bolt_type, vector *start, vector *strike);

// process lightning (randomly generate bolts, etc, etc);
void nebl_process();

// get the current # of active lightning bolts
int nebl_get_active_bolts();

// get the current # of active nodes
int nebl_get_active_nodes();

// set the storm (call from mission parse)
void nebl_set_storm(char *name);

#endif
