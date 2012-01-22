/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "parse/parselo.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "freespace2/freespace.h"
#include "gamesnd/gamesnd.h"
#include "render/3d.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "weapon/emp.h"
#include "network/multi.h"
#include "network/multimsgs.h"


extern int Cmdline_nohtl;

// ------------------------------------------------------------------------------------------------------
// NEBULA LIGHTNING DEFINES/VARS
//

// Lightning nodes
int Num_lnodes = 0;
l_node Nebl_nodes[MAX_LIGHTNING_NODES];

l_node Nebl_free_list;
l_node Nebl_used_list;

// nodes in a lightning bolt
#define LINK_LEFT	0
#define LINK_RIGHT	1
#define LINK_CHILD	2

// Lightning bolts
int Num_lbolts = 0;
l_bolt Nebl_bolts[MAX_LIGHTNING_BOLTS];

// Lightning bolt types
SCP_vector<bolt_type> Bolt_types;

// Lightning storm types
SCP_vector<storm_type> Storm_types;

// points on the basic cross section
vec3d Nebl_ring[3] = {	
	{ { { -1.0f, 0.0f, 0.0f } } },
	{ { { 1.0f, 0.70f, 0.0f } } },
	{ { { 1.0f, -0.70f, 0.0f } } }	
};

// pinched off cross-section
vec3d Nebl_ring_pinched[3] = {	
	{ { { -0.05f, 0.0f, 0.0f } } },
	{ { { 0.05f, 0.035f, 0.0f } } },
	{ { { 0.05f, -0.035f, 0.0f } } }	
};

// globals used for rendering and generating bolts
int Nebl_flash_count = 0;		// # of points rendered onscreen for this bolt
float Nebl_flash_x = 0.0f;		// avg x of the points rendered
float Nebl_flash_y = 0.0f;		// avg y of the points rendered
float Nebl_bang = 0.0;			// distance to the viewer object
float Nebl_alpha = 0.0f;		// alpha to use when rendering the bolt itself
float Nebl_glow_alpha = 0.0f;	// alpha to use when rendering the bolt glow
int Nebl_stamp = -1;			// random timestamp for making bolts
float Nebl_bolt_len;			// length of the current bolt being generated
bolt_type *Nebl_type;			// bolt type
matrix Nebl_bolt_dir;			// orientation matrix of the bolt being generated
vec3d Nebl_bolt_start;			// start point of the bolt being generated
vec3d Nebl_bolt_strike;			// strike point of the bolt being generated

// the type of active storm
storm_type *Storm = NULL;

// vars
DCF(b_scale, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].b_scale = Dc_arg_float;
}
DCF(b_rand, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].b_rand = Dc_arg_float;
}
DCF(b_shrink, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].b_shrink = Dc_arg_float;
}
DCF(b_poly_pct, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].b_poly_pct = Dc_arg_float;
}
DCF(b_add, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].b_add = Dc_arg_float;
}
DCF(b_strikes, "")
{
	dc_get_arg(ARG_INT);
	Bolt_types[DEBUG_BOLT].num_strikes = Dc_arg_int;
}
DCF(b_noise, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].noise = Dc_arg_float;
}
DCF(b_bright, "")
{
	dc_get_arg(ARG_FLOAT);
	Bolt_types[DEBUG_BOLT].b_bright = Dc_arg_float;
}
DCF(b_lifetime, "")
{
	dc_get_arg(ARG_INT);
	Bolt_types[DEBUG_BOLT].lifetime = Dc_arg_int;
}
DCF(b_list, "")
{
	dc_printf("Debug lightning bolt settings :\n");

	dc_printf("b_scale : %f\n", Bolt_types[DEBUG_BOLT].b_scale);
	dc_printf("b_rand : %f\n", Bolt_types[DEBUG_BOLT].b_rand);
	dc_printf("b_shrink : %f\n", Bolt_types[DEBUG_BOLT].b_shrink);
	dc_printf("b_poly_pct : %f\n", Bolt_types[DEBUG_BOLT].b_poly_pct);
	dc_printf("b_add : %f\n", Bolt_types[DEBUG_BOLT].b_add);
	dc_printf("b_strikes : %d\n", Bolt_types[DEBUG_BOLT].num_strikes);
	dc_printf("b_noise : %f\n", Bolt_types[DEBUG_BOLT].noise);
	dc_printf("b_bright : %f\n", Bolt_types[DEBUG_BOLT].b_bright);
	dc_printf("b_lifetime : %d\n", Bolt_types[DEBUG_BOLT].lifetime);
}

// nebula lightning intensity (0.0 to 1.0)
float Nebl_intensity = 0.6667f;

DCF(lightning_intensity, "")
{
	dc_get_arg(ARG_FLOAT);
	float val = Dc_arg_float;
	if(val < 0.0f){
		val = 0.0f;
	} else if(val > 1.0f){
		val = 1.0f;
	}

	Nebl_intensity = 1.0f - val;
}


// ------------------------------------------------------------------------------------------------------
// NEBULA LIGHTNING FUNCTIONS
//

// initialize nebula lightning at game startup
void nebl_init()
{
	char name[MAX_FILENAME_LEN];
	int rval;

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "lightning.tbl", rval));
		return;
	}

	// parse the lightning table
	read_file_text("lightning.tbl", CF_TYPE_TABLES);
	reset_parse();

	// parse the individual lightning bolt types
	required_string("#Bolts begin");
	while(!optional_string("#Bolts end")){
		bolt_type new_bolt_type;

		// bolt title
		required_string("$Bolt:");
		stuff_string(new_bolt_type.name, F_NAME, NAME_LENGTH);

		// b_scale
		required_string("+b_scale:");
		stuff_float(&new_bolt_type.b_scale);

		// b_shrink
		required_string("+b_shrink:");
		stuff_float(&new_bolt_type.b_shrink);

		// b_poly_pct
		required_string("+b_poly_pct:");
		stuff_float(&new_bolt_type.b_poly_pct);		

		// child rand
		required_string("+b_rand:");
		stuff_float(&new_bolt_type.b_rand);

		// z add
		required_string("+b_add:");
		stuff_float(&new_bolt_type.b_add);

		// # strikes
		required_string("+b_strikes:");
		stuff_int(&new_bolt_type.num_strikes);

		// lifetime
		required_string("+b_lifetime:");
		stuff_int(&new_bolt_type.lifetime);

		// noise
		required_string("+b_noise:");
		stuff_float(&new_bolt_type.noise);

		// emp effect
		required_string("+b_emp:");
		stuff_float(&new_bolt_type.emp_intensity);
		stuff_float(&new_bolt_type.emp_time);

		// texture
		required_string("+b_texture:");
		stuff_string(name, F_NAME, sizeof(name));
		if(!Fred_running){
			new_bolt_type.texture = bm_load(name);
		}

		// glow
		required_string("+b_glow:");
		stuff_string(name, F_NAME, sizeof(name));
		if(!Fred_running){
			new_bolt_type.glow = bm_load(name);
		}

		// brightness
		required_string("+b_bright:");
		stuff_float(&new_bolt_type.b_bright);

		Bolt_types.push_back(new_bolt_type);
	}

	// parse lightning storm types
	required_string("#Storms begin");
	while(!optional_string("#Storms end")){
		storm_type new_storm_type;

		// bolt title
		required_string("$Storm:");
		stuff_string(new_storm_type.name, F_NAME, NAME_LENGTH);

		// bolt types
		while(optional_string("+bolt:")){			
			if(new_storm_type.num_bolt_types < MAX_BOLT_TYPES_PER_STORM){
				stuff_string(name, F_NAME, sizeof(name));	
				
				new_storm_type.bolt_types[new_storm_type.num_bolt_types] = nebl_get_bolt_index(name);
				Assert(new_storm_type.bolt_types[new_storm_type.num_bolt_types] != (size_t)-1);								

				new_storm_type.num_bolt_types++;
			} 			
		}

		// flavor
		required_string("+flavor:");
		stuff_float(&new_storm_type.flavor.xyz.x);
		stuff_float(&new_storm_type.flavor.xyz.y);
		stuff_float(&new_storm_type.flavor.xyz.z);

		// frequencies
		required_string("+random_freq:");
		stuff_int(&new_storm_type.min);
		stuff_int(&new_storm_type.max);

		// counts
		required_string("+random_count:");
		stuff_int(&new_storm_type.min_count);
		stuff_int(&new_storm_type.max_count);

		Storm_types.push_back(new_storm_type);
	}
}

// initialize lightning before entering a level
void nebl_level_init()
{
	size_t idx;	

	// zero all lightning bolts
	for(idx=0; idx<MAX_LIGHTNING_BOLTS; idx++){
		Nebl_bolts[idx].head = NULL;
		Nebl_bolts[idx].bolt_life = -1;
		Nebl_bolts[idx].used = 0;
	}	
	
	// initialize node list
	Num_lnodes = 0;
	list_init( &Nebl_free_list );
	list_init( &Nebl_used_list );

	// Link all object slots into the free list
	for (idx=0; idx<MAX_LIGHTNING_NODES; idx++)	{
		list_append(&Nebl_free_list, &Nebl_nodes[idx] );
	}

	// zero the random timestamp
	Nebl_stamp = -1;		

	// null the storm. let mission parsing set it up
	Storm = NULL;
}

// set the storm (call from mission parse)
void nebl_set_storm(char *name)
{
	// sanity
	Storm = NULL;
	
	size_t index = nebl_get_storm_index(name);
	
	if(index == (size_t)-1)
		return;
	
	Storm = &Storm_types[index];
}

// render all lightning bolts
void nebl_render_all()
{
	l_bolt *b;
	bolt_type *bi;

	// no lightning in non-nebula missions
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}

	// if we have no storm
	if(Storm == NULL){
		return;
	}

	// traverse the list
	for(size_t idx=0; idx<MAX_LIGHTNING_BOLTS; idx++){
		b = &Nebl_bolts[idx];		

		// if this is being used
		if(b->used){
			Assert(b->head != NULL);

			// bogus bolt
			if(b->head == NULL){
				b->used = 0;
				continue;
			}
			if( b->type >= Bolt_types.size() ){
				b->used = 0;
				continue;
			}
			bi = &Bolt_types[b->type];

			// if this guy is still on a delay
			if(b->delay != -1){
				if(timestamp_elapsed(b->delay)){
					b->delay = -1;
				} else {
					continue;
				}
			}

			// if the timestamp on this guy has expired
			if((b->bolt_life < 0) || timestamp_elapsed(b->bolt_life)){
				// if this is a multiple strike bolt, jitter it and reset
				if(b->strikes_left-1 > 0){
					b->bolt_life = timestamp(bi->lifetime / bi->num_strikes);
					b->first_frame = 1;
					b->strikes_left--;
					nebl_jitter(b);

					// by continuing here we skip rendering for one frame, which makes it look more like real lightning
					continue;
				}
				// otherwise he's completely done, so release him
				else {
					// maybe free up node data
					if(b->head != NULL){
						nebl_release(b->head);
						b->head = NULL;

						Num_lbolts--;

						nprintf(("lightning", "Released bolt. %d used nodes!\n", Num_lnodes));
					}

					b->used = 0;
				}
			}

			// pick some cool alpha values
			Nebl_alpha = frand();
			Nebl_glow_alpha = frand();

			// otherwise render him
			Nebl_flash_count = 0;
			Nebl_flash_x = 0.0f;
			Nebl_flash_y = 0.0f;
			Nebl_bang = 10000000.0f;
			nebl_render(bi, b->head, b->width);

			// if this is the first frame he has been rendered, determine if we need to make a flash and sound effect
			if(b->first_frame){
				float flash = 0.0f;				

				b->first_frame = 0;

				// if we rendered any points
				if(Nebl_flash_count){
					Nebl_flash_x /= (float)Nebl_flash_count;
					Nebl_flash_y /= (float)Nebl_flash_count;

					// quick distance from the center of the screen			
					float x = Nebl_flash_x - (gr_screen.max_w / 2.0f);
					float y = Nebl_flash_y - (gr_screen.max_h / 2.0f);
					float dist = fl_sqrt((x * x) + (y * y));		
					if(dist / (gr_screen.max_w / 2.0f) < 1.0f){
						flash = 1.0f - (dist / (gr_screen.max_w / 2.0f));										

						// scale the flash by bolt type
						flash *= bi->b_bright;

						game_flash(flash, flash, flash);										
					}					

					// do some special stuff on the very first strike of the bolt
					if(b->strikes_left == bi->num_strikes){					
						// play a sound						
						float bang;
						if(Nebl_bang < 40.0f){
							bang = 1.0f;
						} else if(Nebl_bang > 400.0f){
							bang = 0.0f;
						} else {
							bang = 1.0f - (Nebl_bang / 400.0f);
						}
						if(frand_range(0.0f, 1.0f) < 0.5f){
							snd_play(&Snds[SND_LIGHTNING_2], 0.0f, bang, SND_PRIORITY_DOUBLE_INSTANCE);
						} else {
							snd_play(&Snds[SND_LIGHTNING_1], 0.0f, bang, SND_PRIORITY_DOUBLE_INSTANCE);
						}						

						// apply em pulse
						if(bi->emp_intensity > 0.0f){
							emp_apply(&b->midpoint, 0.0f, vm_vec_dist(&b->start, &b->strike), bi->emp_intensity, bi->emp_time);
						}
					}
				}				
			}
		}
	}	
}

// process lightning (randomly generate bolts, etc, etc);
void nebl_process()
{		
	uint num_bolts, idx;

	// non-nebula mission
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}		
	
	// non servers in multiplayer don't do this
	if((Game_mode & GM_MULTIPLAYER) && !MULTIPLAYER_MASTER){
		return;
	}

	// standalones shouldn't be doing this either
	if (Is_standalone) {
		return;
	}

	// if there's no chosen storm
	if(Storm == NULL){
		return;
	}

	// don't process lightning bolts unless we're a few seconds in
	if(f2fl(Missiontime) < 3.0f){
		return;
	}
		
	// random stamp
	if(Nebl_stamp == -1){
		Nebl_stamp = timestamp((int)frand_range((float)Storm->min, (float)Storm->max));
		return;
	}	

	// maybe make a bolt
	if(timestamp_elapsed(Nebl_stamp)){
		// determine how many bolts to spew
		num_bolts = (uint)frand_range((float)Storm->min_count, (float)Storm->max_count);
		for(idx=0; idx<num_bolts; idx++){
			// hmm. for now just pick a random bolt type and run with it
			int s1, s2, s3;
			int e1, e2, e3;
			do {
				s1 = (int)frand_range(0.0f, (float)Neb2_slices);
				s2 = (int)frand_range(0.0f, (float)Neb2_slices);
				s3 = (int)frand_range(0.0f, (float)Neb2_slices);

				e1 = (int)frand_range(0.0f, (float)Neb2_slices);
				e2 = (int)frand_range(0.0f, (float)Neb2_slices);
				e3 = (int)frand_range(0.0f, (float)Neb2_slices);
			
				// never choose the middle cube
				if((s1 == 2) && (s2 == 2) && (s3 == 2)){
					s1 = 4;
					s2 = 0;
				}
				if((e1 == 2) && (e2 == 2) && (e3 == 2)){
					e1 = 0;
					e2 = 4;
				}

			// sanity
			} while((s1 == e1) && (s2 == e2) && (s3 == e3));

			vec3d start = Neb2_cubes[s1][s2][s3].pt;
			vec3d strike = Neb2_cubes[e1][e2][e3].pt;

			// add some flavor to the bolt. mmmmmmmm, lightning
			if(!IS_VEC_NULL_SQ_SAFE(&Storm->flavor)){			
				// start with your basic hot sauce. measure how much you have			
				vec3d your_basic_hot_sauce;
				vm_vec_sub(&your_basic_hot_sauce, &strike, &start);
				float how_much_hot_sauce = vm_vec_normalize(&your_basic_hot_sauce);

				// now figure out how much of that good wing sauce to add
				vec3d wing_sauce = Storm->flavor;
				if(frand_range(0.0, 1.0f) < 0.5f){
					vm_vec_scale(&wing_sauce, -1.0f);
				}
				float how_much_of_that_good_wing_sauce_to_add = vm_vec_normalize(&wing_sauce);

				// mix the two together, taking care not to add too much
				vec3d the_mixture;
				if(how_much_of_that_good_wing_sauce_to_add > 1000.0f){
					how_much_of_that_good_wing_sauce_to_add = 1000.0f;
				}
				vm_vec_interp_constant(&the_mixture, &your_basic_hot_sauce, &wing_sauce, how_much_of_that_good_wing_sauce_to_add / 1000.0f);

				// take the final sauce and store it in the proper container
				vm_vec_scale(&the_mixture, how_much_hot_sauce);

				// make sure to put it on everything! whee!			
				vm_vec_add(&strike, &start, &the_mixture);
			}

			size_t type = (size_t)frand_range(0.0f, (float)(Storm->num_bolt_types-1));
			nebl_bolt(Storm->bolt_types[type], &start, &strike);
		}

		// reset the timestamp
		Nebl_stamp = timestamp((int)frand_range((float)Storm->min, (float)Storm->max));
	}	
}

// create a lightning bolt
void nebl_bolt(size_t type, vec3d *start, vec3d *strike)
{
	vec3d dir;
	l_bolt *bolt;
	l_node *tail;
	size_t idx;
	bool found;		
	bolt_type *bi;
	float bolt_len;

	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}

	// find a free bolt
	found = 0;
	for(idx=0; idx<MAX_LIGHTNING_BOLTS; idx++){
		if(!Nebl_bolts[idx].used){
			found = 1;
			break;
		}
	}
	if(!found){
		return;
	}

	if( type >= Bolt_types.size() ){
		return;
	}
	bi = &Bolt_types[type];	

	// get a pointer to the bolt
	bolt = &Nebl_bolts[idx];	

	// setup bolt into
	bolt->start = *start;
	bolt->strike = *strike;
	bolt->strikes_left = bi->num_strikes;
	bolt->delay = -1;
	bolt->type = (char)type;
	bolt->first_frame = 1;
	bolt->bolt_life = timestamp(bi->lifetime / bi->num_strikes);		

	Nebl_bolt_start = *start;
	Nebl_bolt_strike = *strike;

	// setup fire delay
	if(bolt->delay != -1){
		bolt->delay = timestamp(bolt->delay);
	}

	// setup the rest of the important bolt data
	if(vm_vec_same(&Nebl_bolt_start, &Nebl_bolt_strike)){
		Nebl_bolt_strike.xyz.z += 150.0f;
	}
	Nebl_bolt_len = vm_vec_dist(&Nebl_bolt_start, &Nebl_bolt_strike);	
	vm_vec_sub(&dir, &Nebl_bolt_strike, &Nebl_bolt_start);

	// setup midpoint
	vm_vec_scale_add(&bolt->midpoint, &Nebl_bolt_start, &dir, 0.5f);

	bolt_len = vm_vec_normalize(&dir);
	vm_vector_2_matrix(&Nebl_bolt_dir, &dir, NULL, NULL);

	// global type for generating the bolt
	Nebl_type = bi;

	// try and make the bolt
	if(!nebl_gen(&Nebl_bolt_start, &Nebl_bolt_strike, 0, 4, 0, &bolt->head, &tail)){
		if(bolt->head != NULL){
			nebl_release(bolt->head);
		}

		return;
	}

	Num_lbolts++;	
	
	// setup the rest of the data	
	bolt->used = 1;	
	bolt->width = bi->b_poly_pct * bolt_len;

	// if i'm a multiplayer master, send a bolt packet
	if(MULTIPLAYER_MASTER){
		send_lightning_packet(type, start, strike);
	}
}

// get the current # of active lightning bolts
int nebl_get_active_bolts()
{
	return Num_lbolts;
}

// get the current # of active nodes
int nebl_get_active_nodes()
{
	return Num_lnodes;
}

// "new" a lightning node
l_node *nebl_new()
{
	l_node *lp;

	// if we're out of nodes
	if(Num_lnodes >= MAX_LIGHTNING_NODES){
		// Int3();
		nprintf(("lightning", "Out of lightning nodes!\n"));
		return NULL;
	}

	// get a new node off the freelist
	lp = GET_FIRST(&Nebl_free_list);
	Assert( lp != &Nebl_free_list );		// shouldn't have the dummy element

	// remove trailp from the free list
	list_remove( &Nebl_free_list, lp );
	
	// insert trailp onto the end of used list
	list_append( &Nebl_used_list, lp );

	// increment counter
	Num_lnodes++;

	lp->links[0] = NULL;
	lp->links[1] = NULL;
	lp->links[2] = NULL;	

	// return the pointer
	return lp;
}

// "delete" a lightning node
void nebl_delete(l_node *lp)
{
	// remove objp from the used list
	list_remove( &Nebl_used_list, lp );

	// add objp to the end of the free
	list_append( &Nebl_free_list, lp );

	// decrement counter
	Num_lnodes--;
}

// free a lightning bolt
void nebl_release(l_node *whee)
{
	// if we're invalid
	if(whee == NULL){
		return;
	}

	// release all of our children
	if(whee->links[LINK_RIGHT] != NULL){
		nebl_release(whee->links[LINK_RIGHT]);
	}	
	if(whee->links[LINK_CHILD] != NULL){
		nebl_release(whee->links[LINK_CHILD]);
	}	

	// delete this node
	nebl_delete(whee);
}

int nebl_gen(vec3d *left, vec3d *right, float depth, float max_depth, int child, l_node **l_left, l_node **l_right)
{
	l_node *child_node = NULL;
	float d = vm_vec_dist_quick( left, right );		

	// if we've reached the critical point
	if ( d < 0.30f || (depth > max_depth) ){
		// generate ne items
		l_node *new_left = nebl_new();
		if(new_left == NULL){
			return 0;
		}		
		new_left->links[0] = NULL; new_left->links[1] = NULL; new_left->links[2] = NULL;
		new_left->pos = vmd_zero_vector;
		l_node *new_right = nebl_new();
		if(new_right == NULL){
			nebl_delete(new_left);			
			return 0;
		}		
		new_right->links[0] = NULL; new_right->links[1] = NULL; new_right->links[2] = NULL;
		new_right->pos = vmd_zero_vector;

		// left side
		new_left->pos = *left;		
		new_left->links[LINK_RIGHT] = new_right;		
		*l_left = new_left;
		
		// right side
		new_right->pos = *right;
		new_right->links[LINK_LEFT] = new_left;
		*l_right = new_right;

		// done
		return 1;
	}  

	// divide in half
	vec3d tmp;
	vm_vec_avg( &tmp, left, right );

	// sometimes generate children
	if(!child && (frand() <= Nebl_type->b_rand)){
		// get a point on the plane of the strike
		vec3d tmp2;
		vm_vec_random_in_circle(&tmp2, &Nebl_bolt_strike, &Nebl_bolt_dir, Nebl_bolt_len * Nebl_type->b_scale, 0);

		// maybe move away from the plane
		vec3d dir;
		vm_vec_sub(&dir, &tmp2, &tmp);		
		vm_vec_scale_add(&tmp2, &tmp, &dir, Nebl_type->b_shrink);

		// child
		l_node *argh;		
		if(!nebl_gen(&tmp, &tmp2, 0, 2, 1, &child_node, &argh)){
			if(child_node != NULL){
				nebl_release(child_node);
			}
			return 0;
		}
	}
	
	float scaler = 0.30f;
	tmp.xyz.x += (frand()-0.5f)*d*scaler;
	tmp.xyz.y += (frand()-0.5f)*d*scaler;
	tmp.xyz.z += (frand()-0.5f)*d*scaler;

	// generate left half
	l_node *ll = NULL;
	l_node *lr = NULL;
	if(!nebl_gen( left, &tmp, depth+1, max_depth, child, &ll, &lr )){
		if(child_node != NULL){
			nebl_release(child_node);
		}
		if(ll != NULL){
			nebl_release(ll);
		}
		return 0;
	}

	// generate right half
	l_node *rl = NULL;
	l_node *rr = NULL;
	if(!nebl_gen( &tmp, right, depth+1, max_depth, child, &rl, &rr )){
		if(child_node != NULL){
			nebl_release(child_node);
		}
		if(ll != NULL){
			nebl_release(ll);
		}
		if(rl != NULL){
			nebl_release(rl);
		}
		return 0;
	}
	
	// splice the two together
	lr->links[LINK_RIGHT] = rl->links[LINK_RIGHT];
	lr->links[LINK_RIGHT]->links[LINK_LEFT] = lr;
	nebl_delete(rl);

	// if we generated a child, stick him on
	if(child_node != NULL){
		lr->links[LINK_CHILD] = child_node;
	}

	// return these
	*l_left = ll;
	*l_right = rr;

	return 1;
}


// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void nebl_calc_facing_pts_smart( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add )
{
	vec3d uvec, rvec;
	vec3d temp;	

	temp = *pos;

	vm_vec_sub( &rvec, &Eye_position, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	
	
	vm_vec_scale_add2( top, &rvec, z_add );
	vm_vec_scale_add2( bot, &rvec, z_add );
}

// render a section of the bolt
void nebl_render_section(bolt_type *bi, l_section *a, l_section *b)
{		
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	// Sets mode.  Returns previous mode.
	gr_zbuffer_set(GR_ZBUFF_FULL);	

	// draw some stuff
	for(size_t idx=0; idx<2; idx++){		
		v[0] = a->vex[idx];		
		v[0].texture_position.u = 0.0f; v[0].texture_position.v = 0.0f;

		v[1] = a->vex[idx+1];		
		v[1].texture_position.u = 1.0f; v[1].texture_position.v = 0.0f;

		v[2] = b->vex[idx+1];		
		v[2].texture_position.u = 1.0f; v[2].texture_position.v = 1.0f;

		v[3] = b->vex[idx];		
		v[3].texture_position.u = 0.0f; v[3].texture_position.v = 1.0f;

		// draw
		gr_set_bitmap(bi->texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Nebl_alpha);
		g3_draw_poly(4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);		
	}

	// draw
	v[0] = a->vex[2];		
	v[0].texture_position.u = 0.0f; v[0].texture_position.v = 0.0f;

	v[1] = a->vex[0];		
	v[1].texture_position.u = 1.0f; v[1].texture_position.v = 0.0f;

	v[2] = b->vex[0];		
	v[2].texture_position.u = 1.0f; v[2].texture_position.v = 1.0f;

	v[3] = b->vex[2];		
	v[3].texture_position.u = 0.0f; v[3].texture_position.v = 1.0f;

	gr_set_bitmap(bi->texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Nebl_alpha);
	g3_draw_poly(4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);	

	// draw the glow beam	
	verts[0] = &a->glow_vex[0];
	verts[0]->texture_position.v = 0.0f; verts[0]->texture_position.u = 0.0f;

	verts[1] = &a->glow_vex[1];
	verts[1]->texture_position.v = 1.0f; verts[1]->texture_position.u = 0.0f;

	verts[2] = &b->glow_vex[1];
	verts[2]->texture_position.v = 1.0f; verts[2]->texture_position.u = 1.0f;

	verts[3] = &b->glow_vex[0];
	verts[3]->texture_position.v = 0.0f; verts[3]->texture_position.u = 1.0f;

	gr_set_bitmap(bi->glow, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Nebl_glow_alpha);
	g3_draw_poly(4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT);	
}

// generate a section
void nebl_generate_section(bolt_type *bi, float width, l_node *a, l_node *b, l_section *c, l_section *cap, int pinch_a, int pinch_b)
{
	vec3d dir;
	vec3d dir_normal;
	matrix m;
	size_t idx;	
	vec3d temp, pt;
	vec3d glow_a, glow_b;

	// direction matrix
	vm_vec_sub(&dir, &a->pos, &b->pos);
	vm_vec_copy_normalize(&dir_normal, &dir);
	vm_vector_2_matrix(&m, &dir_normal, NULL, NULL);

	// distance to player
	float bang_dist = vm_vec_dist_quick(&Eye_position, &a->pos);
	if(bang_dist < Nebl_bang){
		Nebl_bang = bang_dist;
	}

	// rotate the basic section into world	
	for(idx=0; idx<3; idx++){
		// rotate to world		
		if(pinch_a){			
			vm_vec_rotate(&pt, &Nebl_ring_pinched[idx], &m);
		} else {
			vm_vec_copy_scale(&temp, &Nebl_ring[idx], width);
			vm_vec_rotate(&pt, &temp, &m);
		}
		vm_vec_add2(&pt, &a->pos);
			
		// transform
		if (Cmdline_nohtl) {
			g3_rotate_vertex(&c->vex[idx], &pt);
		} else {
			g3_transfer_vertex(&c->vex[idx], &pt);
		}
		g3_project_vertex(&c->vex[idx]);		

		// if first frame, keep track of the average screen pos
		if((c->vex[idx].screen.xyw.x >= 0)
			&& (c->vex[idx].screen.xyw.x < gr_screen.max_w)
			&& (c->vex[idx].screen.xyw.y >= 0)
			&& (c->vex[idx].screen.xyw.y < gr_screen.max_h))
		{
			Nebl_flash_x += c->vex[idx].screen.xyw.x;
			Nebl_flash_y += c->vex[idx].screen.xyw.y;
			Nebl_flash_count++;
		}
	}
	// calculate the glow points		
	nebl_calc_facing_pts_smart(&glow_a, &glow_b, &dir_normal, &a->pos, pinch_a ? 0.5f : width * 6.0f, Nebl_type->b_add);
	if (Cmdline_nohtl) {
		g3_rotate_vertex(&c->glow_vex[0], &glow_a);
	} else {
		g3_transfer_vertex(&c->glow_vex[0], &glow_a);
	}
	g3_project_vertex(&c->glow_vex[0]);
	if (Cmdline_nohtl) {
		g3_rotate_vertex(&c->glow_vex[1], &glow_b);
	} else {
		g3_transfer_vertex(&c->glow_vex[1], &glow_b);
	}
	g3_project_vertex(&c->glow_vex[1]);	

	// maybe do a cap
	if(cap != NULL){		
		// rotate the basic section into world
		for(idx=0; idx<3; idx++){
			// rotate to world		
			if(pinch_b){
				vm_vec_rotate(&pt, &Nebl_ring_pinched[idx], &m);
			} else {
				vm_vec_copy_scale(&temp, &Nebl_ring[idx], width);
				vm_vec_rotate(&pt, &temp, &m);		
			}
			vm_vec_add2(&pt, &b->pos);
			
			// transform
			if (Cmdline_nohtl) {
				g3_rotate_vertex(&cap->vex[idx], &pt);
			} else {
				g3_transfer_vertex(&cap->vex[idx], &pt);
			}
			g3_project_vertex(&cap->vex[idx]);			

			// if first frame, keep track of the average screen pos			
			if( (c->vex[idx].screen.xyw.x >= 0)
				&& (c->vex[idx].screen.xyw.x < gr_screen.max_w)
				&& (c->vex[idx].screen.xyw.y >= 0)
				&& (c->vex[idx].screen.xyw.y < gr_screen.max_h))
			{
				Nebl_flash_x += c->vex[idx].screen.xyw.x;
				Nebl_flash_y += c->vex[idx].screen.xyw.y;
				Nebl_flash_count++;
			}
		}
		
		// calculate the glow points		
		nebl_calc_facing_pts_smart(&glow_a, &glow_b, &dir_normal, &b->pos, pinch_b ? 0.5f : width * 6.0f, bi->b_add);
		if (Cmdline_nohtl) {
			g3_rotate_vertex(&cap->glow_vex[0], &glow_a);
		} else {
			g3_transfer_vertex(&cap->glow_vex[0], &glow_a);
		}
		g3_project_vertex(&cap->glow_vex[0]);
		if (Cmdline_nohtl) {
			g3_rotate_vertex(&cap->glow_vex[1], &glow_b);
		} else {
			g3_transfer_vertex(&cap->glow_vex[1], &glow_b);
		}
		g3_project_vertex(&cap->glow_vex[1]);
	}
}

// render the bolt
void nebl_render(bolt_type *bi, l_node *whee, float width, l_section *prev)
{		
	l_section start;
	l_section end;
	l_section child_start;

	// bad
	if(whee == NULL){
		return;
	}

	// if prev is NULL, we're just starting so we need our start point
	if(prev == NULL){
		Assert(whee->links[LINK_RIGHT] != NULL);
		nebl_generate_section(bi, width, whee, whee->links[LINK_RIGHT], &start, NULL, 1, 0);
	} else {
		start = *prev;
	}
	
	// if we have a child section	
	if(whee->links[LINK_CHILD]){		
		// generate section
		nebl_generate_section(bi, width * 0.5f, whee, whee->links[LINK_CHILD], &child_start, &end, 0, whee->links[LINK_CHILD]->links[LINK_RIGHT] == NULL ? 1 : 0);

		// render
		nebl_render_section(bi, &child_start, &end);			

		// maybe continue
		if(whee->links[LINK_CHILD]->links[LINK_RIGHT] != NULL){
			nebl_render(bi, whee->links[LINK_CHILD], width * 0.5f, &end);
		}
	}	
		
	// if the next section is an end section
	if(whee->links[LINK_RIGHT]->links[LINK_RIGHT] == NULL){
		l_section temp;

		// generate section
		nebl_generate_section(bi, width, whee, whee->links[LINK_RIGHT], &temp, &end, 0, 1);

		// render the section
		nebl_render_section(bi, &start, &end);		
	}
	// a middle section
	else if(whee->links[LINK_RIGHT]->links[LINK_RIGHT] != NULL){
		// generate section
		nebl_generate_section(bi, width, whee->links[LINK_RIGHT], whee->links[LINK_RIGHT]->links[LINK_RIGHT], &end, NULL, 0, 0);

		// render the section
		nebl_render_section(bi, &start, &end);

		// recurse through him
		nebl_render(bi, whee->links[LINK_RIGHT], width, &end);
	}
}

// given a valid, complete bolt, jitter him based upon his noise
void nebl_jitter(l_bolt *b)
{
	matrix m;
	vec3d temp;
	float length;
	l_node *moveup;
	bolt_type *bi = NULL;

	// sanity
	if(b == NULL){
		return;
	}
	if( b->type >= Bolt_types.size() ){
		return;		
	}
	bi = &Bolt_types[b->type];

	// get the bolt direction
	vm_vec_sub(&temp, &b->strike, &b->start);
	length = vm_vec_normalize_quick(&temp);
	vm_vector_2_matrix(&m, &temp, NULL, NULL);

	// jitter all nodes on the main trunk
	moveup = b->head;
	while(moveup != NULL){
		temp = moveup->pos;
		vm_vec_random_in_circle(&moveup->pos, &temp, &m, frand_range(0.0f, length * bi->noise), 0);

		// just on the main trunk
		moveup = moveup->links[LINK_RIGHT];
	}	
}

// return the index of a given bolt type by name
size_t nebl_get_bolt_index(char *name)
{
	for(size_t idx=0; idx<Bolt_types.size(); idx++){
		if(!strcmp(name, Bolt_types[idx].name)){
			return idx;
		}
	}

	return (size_t)-1;
}

// return the index of a given storm type by name
size_t nebl_get_storm_index(char *name)
{
	if (name == NULL)
		return (size_t)-1;

	for(size_t idx=0; idx<Storm_types.size(); idx++){
		if(!strcmp(name, Storm_types[idx].name)){
			return idx;
		}
	}

	return (size_t)-1;
}
