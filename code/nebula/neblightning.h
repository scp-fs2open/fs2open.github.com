/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FS2_NEBULA_LIGHTNING_HEADER_FILE
#define __FS2_NEBULA_LIGHTNING_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

// ------------------------------------------------------------------------------------------------------
// NEBULA LIGHTNING DEFINES/VARS
//

#define MAX_LIGHTNING_NODES					500
#define MAX_LIGHTNING_BOLTS					10
#define MAX_BOLT_TYPES_PER_STORM			10
#define DEBUG_BOLT							0

// lightning nodes
typedef struct l_node {
	vec3d	pos;				// world position
	l_node	*links[3];			// 3 links for lightning children
	
	l_node	*next, *prev;		// for used and free-lists only
} l_node;

// lightning bolts
typedef struct l_bolt {
	l_node	*head;				// head of the lightning bolt
	int		bolt_life;			// remaining life timestamp
	ubyte	used;				// used or not
	ubyte	first_frame;		// if he hasn't been rendered at least once	
	size_t	type;				// used as index into Bolt_types
	
	// bolt info
	vec3d	start, strike, midpoint;
	int		delay;				// delay stamp
	int		strikes_left;		// #of strikes left
	float	width;
} l_bolt;

// one cross-section of a lightning bolt
typedef struct l_section {		
	vertex	vex[3];
	vertex	glow_vex[2];
} l_section;

// bolt_type - see lightning.tbl for explanations of these values
typedef struct bolt_type {
	char		name[NAME_LENGTH];
	
	float		b_scale;
	float		b_shrink;
	float		b_poly_pct;	
	float		b_add;
	float		b_rand;
	
	float		noise;
	int			lifetime;
	int			num_strikes;
	
	float		emp_intensity;
	float		emp_time;
	
	int			texture;
	int			glow;	
	
	float		b_bright;
} bolt_type;

// storm_type - see lightning.tbl for explanations of these values
typedef struct storm_type {
	char		name[NAME_LENGTH];
	ubyte		num_bolt_types;							// how many different bolt types you'll see in the nebula
	size_t		bolt_types[MAX_BOLT_TYPES_PER_STORM];	// indices into Bolt types	
	vec3d		flavor;									// flavor of the storm
	
	int			min, max;								// min and max delay between bolt firing.	
	int			min_count, max_count;					// # of bolts spewed
	
	storm_type() : num_bolt_types(0) {}
} storm_type;

extern SCP_vector<storm_type>	Storm_types;
extern SCP_vector<bolt_type>	Bolt_types;

// nebula lightning intensity (0.0 to 1.0)
extern float Nebl_intensity;

// ------------------------------------------------------------------------------------------------------
// NEBULA LIGHTNING FUNCTIONS
//

// initialize nebula lightning at game startup
void nebl_init();

// initialize lightning before entering a level
void nebl_level_init();

// set the storm (call from mission parse)
void nebl_set_storm(char *name);

// render all lightning bolts
void nebl_render_all();

// process lightning (randomly generate bolts, etc, etc);
void nebl_process();

// create a lightning bolt
void nebl_bolt(size_t type, vec3d *start, vec3d *strike);

// get the current # of active lightning bolts
int nebl_get_active_bolts();

// get the current # of active nodes
int nebl_get_active_nodes();

// "new" a lightning node
l_node *nebl_new();

// "delete" a lightning node
void nebl_delete(l_node *lp);

// free up a the nodes of the passed in bolt
void nebl_release(l_node *bolt_head);

// generate a lightning bolt, returns l_left (the "head") and l_right (the "tail")
int nebl_gen(vec3d *left, vec3d *right, float depth, float max_depth, int child, l_node **l_left, l_node **l_right);

// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void nebl_calc_facing_pts_smart(vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add );

// render a section of the bolt
void nebl_render_section(bolt_type *bi, l_section *a, l_section *b);

// generate a section
void nebl_generate_section(bolt_type *bi, float width, l_node *a, l_node *b, l_section *c, l_section *cap, int pinch_a, int pinch_b);

// render the bolt
void nebl_render(bolt_type *bi, l_node *whee, float width, l_section *prev = NULL);

// given a valid, complete bolt, jitter him based upon his noise
void nebl_jitter(l_bolt *b);

// return the index of a given bolt type by name
size_t nebl_get_bolt_index(char *name);

// return the index of a given storm type by name
size_t nebl_get_storm_index(char *name);

#endif
