/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Lighting/Lighting.cpp $
 * $Revision: 2.16 $
 * $Date: 2005-03-01 06:55:41 $
 * $Author: bobboau $
 *
 * Code to calculate dynamic lighting on a vertex.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.15  2004/07/26 20:47:35  Kazan
 * remove MCD complete
 *
 * Revision 2.14  2004/07/12 16:32:52  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.13  2004/03/05 09:02:04  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.12  2003/10/16 00:17:17  randomtiger
 * Added incomplete code to allow selection of non-standard modes in D3D (requires new launcher).
 * As well as initialised in a different mode, bitmaps are stretched and for these modes
 * previously point filtered textures now use linear to keep them smooth.
 * I also had to shuffle some of the GR_1024 a bit.
 * Put my HT&L flags in ready for my work to sort out some of the render order issues.
 * Tided some other stuff up.
 *
 * Revision 2.11  2003/10/14 17:39:14  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.10  2003/10/13 19:39:20  matt
 * prelim reworking of lighting code, dynamic lights work properly now
 * albeit at only 8 lights per object, although it looks just as good as
 * the old software version --Sticks
 *
 * Revision 2.9  2003/10/10 03:59:41  matt
 * Added -nohtl command line param to disable HT&L, nothing is IFDEFd
 * out now. -Sticks
 *
 * Revision 2.8  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.7  2003/09/14 19:00:36  wmcoolmon
 * Changed "nospec" to "Cmdline_nospec" -C
 *
 * Revision 2.6  2003/09/09 21:26:23  fryday
 * Fixed specular for dynamic lights, optimized it a bit
 *
 * Revision 2.5  2003/09/09 17:10:55  matt
 * Added -nospec cmd line param to disable specular -Sticks
 *
 * Revision 2.4  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.3  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.2  2003/08/12 03:18:33  bobboau
 * Specular 'shine' mapping;
 * useing a phong lighting model I have made specular highlights
 * that are mapped to the model,
 * rendering them is still slow, but they look real purdy
 *
 * also 4 new (untested) comand lines, the XX is a floating point value
 * -spec_exp XX
 * the n value, you can set this from 0 to 200 (actualy more than that, but this is the recomended range), it will make the highlights bigger or smaller, defalt is 16.0 so start playing around there
 * -spec_point XX
 * -spec_static XX
 * -spec_tube XX
 * these are factors for the three diferent types of lights that FS uses, defalt is 1.0,
 * static is the local stars,
 * point is weapons/explosions/warp in/outs,
 * tube is beam weapons,
 * for thouse of you who think any of these lights are too bright you can configure them you're self for personal satisfaction
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     6/22/99 2:22p Dave
 * Doh. Fixed a type bug.
 * 
 * 4     6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 3     5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 37    4/12/98 9:56a John
 * Made lighting detail flags work.   Made explosions cast light on
 * highest.
 * 
 * 36    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 35    4/04/98 5:17p John
 * Added sun stuff.  Made Glide optionally use 8-bpp textures.  (looks
 * bad)
 * 
 * 34    3/14/98 3:07p Adam
 * re-added some lighting changes
 * 
 * 33    3/13/98 4:10p John
 * Put back in Adam's old lighting values.
 * 
 * 32    3/12/98 8:42a John
 * Checked in Adam's new lighting values.
 * Checked in changes to timing models code
 * 
 * 31    2/26/98 5:42p John
 * 
 * 30    2/26/98 3:25p John
 * Added code to turn on/off lighting.   Made lighting used dist*dist
 * instead of dist
 * 
 * 29    2/19/98 10:51p John
 * Enabled colored lighting for hardware (Glide)
 * 
 * 28    2/13/98 5:00p John
 * Made lighting push functions return number of releveent lights.
 * 
 * 27    1/29/98 3:36p Johnson
 * JAS:  Fixed some problems with pre_player_entry.
 * 
 * 26    1/29/98 3:16p Allender
 * fix compiler warnings
 * 
 * 25    1/29/98 8:14a John
 * Added support for RGB lighting
 * 
 * 24    1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 23    1/10/98 1:14p John
 * Added explanation to debug console commands
 * 
 * 22    1/02/98 11:53a Adam
 * Changed lighting mode to darken.  Changed ambient and reflect to .75
 * and .50 to compensate.
 * 
 * 21    1/02/98 11:28a Adam
 * Changed default ambient to 0.55f from 0.45f.
 * 
 * 20    12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 19    12/17/97 7:53p John
 * Fixed a bug where gunpoint for flashes were in world coordinates,
 * should have been object.
 * 
 * 18    12/17/97 5:11p John
 * Added brightening back into fade table.  Added code for doing the fast
 * dynamic gun flashes and thruster flashes.
 * 
 * 17    12/12/97 3:02p John
 * First Rev of Ship Shadows
 * 
 * 16    11/07/97 7:24p John
 * changed lighting to take two ranges.
 * In textest, added test code to draw nebulas
 * 
 * 15    11/04/97 9:19p John
 * Optimized dynamic lighting more.
 * 
 * 14    10/29/97 5:05p John
 * Changed dynamic lighting to only rotate and calculate lighting for
 * point lights that are close to an object.  Changed lower framerate cap
 * from 4 to .5.
 * 
 * 13    9/24/97 12:37p Mike
 * Crank ambient lighting from 0.2 to 0.6
 * 
 * 12    9/17/97 9:44a John
 * fixed bug with lighting set to default
 * 
 * 11    9/11/97 5:36p Jasen
 * Changed ambient and reflective lighting values to give a bit more
 * realism to the game.  Yeah, yeah, I know I am not a programmer.
 * 
 * 10    4/24/97 2:58p John
 * 
 * 9     4/24/97 11:49a John
 * added new lighting commands to console.
 * 
 * 8     4/23/97 5:26p John
 * First rev of new debug console stuff.
 * 
 * 7     4/22/97 3:14p John
 * upped the ambient light
 * 
 * 6     4/17/97 6:06p John
 * New code/data for v19 of BSPGEN with smoothing and zbuffer
 * optimizations.
 * 
 * 5     4/08/97 5:18p John
 * First rev of decent (dynamic, correct) lighting in FreeSpace.
 * 
 * 4     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 * 
 * 3     1/30/97 9:35a Hoffoss
 * Added header for files.
 *
 * $NoKeywords: $
 */

#include "math/vecmat.h"
#include "render/3d.h"
#include "lighting/lighting.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"




#define MAX_LIGHTS 256
#define MAX_LIGHT_LEVELS 16

#define LT_DIRECTIONAL	0		// A light like a sun
#define LT_POINT			1		// A point light, like an explosion
#define LT_TUBE			2		// A tube light, like a fluorescent light

int cell_shaded_lightmap = -1;
typedef struct light {
	int		type;							// What type of light this is
	vector	vec;							// location in world space of a point light or the direction of a directional light or the first point on the tube for a tube light
	vector	vec2;							// second point on a tube light
	vector	local_vec;					// rotated light vector
	vector	local_vec2;					// rotated 2nd light vector for a tube light
	float		intensity;					// How bright the light is.
	float		rad1, rad1_squared;		// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float		rad2, rad2_squared;		// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float		r,g,b;						// The color components of the light
	float		spec_r,spec_g,spec_b;		// The specular color components of the light
	int		ignore_objnum;				// Don't light this object.  Used to optimize weapons casting light on parents.
	int		affected_objnum;			// for "unique lights". ie, lights which only affect one object (trust me, its useful)
	int instance;
} light;

light Lights[MAX_LIGHTS];
int Num_lights=0;
extern int Cmdline_nohtl;

light *Relevent_lights[MAX_LIGHTS][MAX_LIGHT_LEVELS];
int Num_relevent_lights[MAX_LIGHT_LEVELS];
int Num_light_levels = 0;

#define MAX_STATIC_LIGHTS			10
light * Static_light[MAX_STATIC_LIGHTS];
int Static_light_count = 0;

static int Light_in_shadow = 0;	// If true, this means we're in a shadow

#define LM_BRIGHTEN  0
#define LM_DARKEN    1

#define MIN_LIGHT 0.03f	// When light drops below this level, ignore it.  Must be non-zero! (1/32)


int Lighting_off = 0;

// For lighting values, 0.75 is full intensity

#if 1		// ADAM'S new stuff
	int Lighting_mode = LM_BRIGHTEN;
	#define AMBIENT_LIGHT_DEFAULT		0.15f		//0.10f
	#define REFLECTIVE_LIGHT_DEFAULT 0.75f		//0.90f
#else
	int Lighting_mode = LM_DARKEN;
	#define AMBIENT_LIGHT_DEFAULT		0.75f		//0.10f
	#define REFLECTIVE_LIGHT_DEFAULT 0.50f		//0.90f
#endif

float Ambient_light = AMBIENT_LIGHT_DEFAULT;
float Reflective_light = REFLECTIVE_LIGHT_DEFAULT;

int Lighting_flag = 1;

DCF(light,"Changes lighting parameters")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "ambient" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				Ambient_light = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "reflect" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				Reflective_light = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "default" ))	{
			Lighting_mode = LM_BRIGHTEN;
			Ambient_light = AMBIENT_LIGHT_DEFAULT;
			Reflective_light = REFLECTIVE_LIGHT_DEFAULT;
			Lighting_flag = 0;
		} else if ( !strcmp( Dc_arg, "mode" ))	{
			dc_get_arg(ARG_STRING);
			if ( !strcmp(Dc_arg, "light") )	{
				Lighting_mode = LM_BRIGHTEN;
			} else if ( !strcmp(Dc_arg, "darken"))	{ 
				Lighting_mode = LM_DARKEN;
			} else {
				Dc_help = 1;
			}
		} else if ( !strcmp( Dc_arg, "dynamic" ))	{
			dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
			if ( Dc_arg_type & ARG_TRUE )	Lighting_flag = 1;	
			else if ( Dc_arg_type & ARG_FALSE ) Lighting_flag = 0;	
			else if ( Dc_arg_type & ARG_NONE ) Lighting_flag ^= 1;	
		} else if ( !strcmp( Dc_arg, "on" ) )	{
			Lighting_off = 0;
		} else if ( !strcmp( Dc_arg, "off" ) )	{
			Lighting_off = 1;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: light keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "light on|off          Turns all lighting on/off\n" );
		dc_printf( "light default         Resets lighting to all default values\n" );
		dc_printf( "light ambient X       Where X is the ambient light between 0 and 1.0\n" );
		dc_printf( "light reflect X       Where X is the material reflectiveness between 0 and 1.0\n" );
		dc_printf( "light dynamic [bool]  Toggles dynamic lighting on/off\n" );
		dc_printf( "light mode [light|darken]   Changes the lighting mode.\n" );
		dc_printf( "   Where 'light' means the global light adds light.\n");
		dc_printf( "   and 'darken' means the global light subtracts light.\n");
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Ambient light is set to %.2f\n", Ambient_light );
		dc_printf( "Reflective light is set to %.2f\n", Reflective_light );
		dc_printf( "Dynamic lighting is: %s\n", (Lighting_flag?"on":"off") );
		switch( Lighting_mode )	{
		case LM_BRIGHTEN:   dc_printf( "Lighting mode is: light\n" ); break;
		case LM_DARKEN:   dc_printf( "Lighting mode is: darken\n" ); break;
		default: dc_printf( "Lighting mode is: UNKNOWN\n" ); break;
		}
	}
}

void light_reset()
{
	int idx;

	// reset static (sun) lights
	for(idx=0; idx<MAX_STATIC_LIGHTS; idx++){
		Static_light[idx] = NULL;
	}
	Static_light_count = 0;

	Num_lights = 0;
	light_filter_reset();

	if(!Cmdline_nohtl) {
		for(int i = 0; i<MAX_LIGHTS; i++)gr_destroy_light(i);
	}
}
extern vector Object_position;
// Rotates the light into the current frame of reference
void light_rotate(light * l)
{
	switch( l->type )	{
	case LT_DIRECTIONAL:
		// Rotate the light direction into local coodinates

		if(!Cmdline_nohtl) {
			light_data L = *(light_data*)(void*)l;
			gr_modify_light(&L,l->instance, 2);
		}
		
		vm_vec_rotate(&l->local_vec, &l->vec, &Light_matrix );
		break;
	
	case LT_POINT:	{
			vector tempv;
			// Rotate the point into local coordinates
	
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );

			if(!Cmdline_nohtl) {
				light_data L = *(light_data*)(void*)l;
				gr_modify_light(&L,l->instance, 2);
			}
		}
		break;
	
	case LT_TUBE:{
			vector tempv;

			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
			
			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec2, &Light_base );
			vm_vec_rotate(&l->local_vec2, &tempv, &Light_matrix );

			if(!Cmdline_nohtl) {

				//move the point to the neares to the object on the line
					vector pos;

	/*				vm_vec_unrotate(&temp2, &temp, &obj->orient);
					vm_vec_add2(&temp, &temp2);
					vm_vec_scale_add(&temp, &temp2, &obj->orient.vec.fvec, Weapon_info[swp->primary_bank_weapons[swp->current_primary_bank]].b_info.range);
	*/
					switch(vm_vec_dist_to_line(&Object_position, &l->local_vec, &l->local_vec2, &pos, NULL)){
						// behind the beam, so use the start pos
					case -1:
						pos = l->vec;
						break;
				
						// use the closest point
					case 0:
						// already calculated in vm_vec_dist_to_line(...)
						break;

						// past the beam, so use the shot pos
					case 1:
						pos = l->vec2;
						break;
					}
				light_data L = *(light_data*)(void*)l;
				L.vec = pos;
				L.local_vec = pos;
				gr_modify_light(&L,l->instance, 2);
			}
		}
		break;

	default:
		Int3();	// Invalid light type
	}
}

// Sets the ambient lighting level.
// Ignored for now.
void light_set_ambient(float ambient_light)
{
}

void light_add_directional( vector *dir, float intensity, float r, float g, float b, float spec_r, float spec_g, float spec_b, bool specular )
{
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if ( Num_lights >= MAX_LIGHTS ) return;

	l = &Lights[Num_lights++];

	l->type = LT_DIRECTIONAL;

	if ( Lighting_mode == LM_BRIGHTEN )	{
		vm_vec_copy_scale( &l->vec, dir, -1.0f );
	} else {
		vm_vec_copy_scale( &l->vec, dir, 1.0f );
	}

	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rad1 = 0.0f;
	l->rad2 = 0.0f;
	l->rad1_squared = l->rad1*l->rad1;
	l->rad2_squared = l->rad2*l->rad2;
	l->ignore_objnum = -1;
	l->affected_objnum = -1;
	l->instance = Num_lights-1;
		
	Assert( Num_light_levels <= 1 );
//	Relevent_lights[Num_relevent_lights[Num_light_levels-1]++][Num_light_levels-1] = l;

	if(Static_light_count < MAX_STATIC_LIGHTS){		
		Static_light[Static_light_count++] = l;
	}
	if(!Cmdline_nohtl) {
		light_data *L = (light_data*)(void*)l;
		gr_make_light(L,Num_lights-1, 1);
	}
}


void light_add_point( vector * pos, float rad1, float rad2, float intensity, float r, float g, float b, int ignore_objnum, float spec_r, float spec_g, float spec_b, bool specular  )
{
	Assert(rad1 >0);
	Assert(rad2 >0);
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_POINT;
	l->vec = *pos;
	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rad1 = rad1;
	l->rad2 = rad2;
	l->rad1_squared = l->rad1*l->rad1;
	l->rad2_squared = l->rad2*l->rad2;
	l->ignore_objnum = ignore_objnum;
	l->affected_objnum = -1;
	l->instance = Num_lights-1;

	Assert( Num_light_levels <= 1 );
	if(!Cmdline_nohtl) {
		light_data *L = (light_data*)(void*)l;
		gr_make_light(L,Num_lights-1, 2);
	}
//	Relevent_lights[Num_relevent_lights[Num_light_levels-1]++][Num_light_levels-1] = l;
//	light_data *L = (light_data*)(void*)l;
//	l->API_index = gr_make_light(L);
}

void light_add_point_unique( vector * pos, float rad1, float rad2, float intensity, float r, float g, float b, int affected_objnum, float spec_r, float spec_g, float spec_b, bool specular )
{
	Assert(rad1 >0);
	Assert(rad2 >0);
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_POINT;
	l->vec = *pos;
	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rad1 = rad1;
	l->rad2 = rad2;
	l->rad1_squared = l->rad1*l->rad1;
	l->rad2_squared = l->rad2*l->rad2;
	l->ignore_objnum = -1;
	l->affected_objnum = affected_objnum;
	l->instance = Num_lights-1;

	Assert( Num_light_levels <= 1 );
	if(!Cmdline_nohtl) {
		light_data *L = (light_data*)(void*)l;
		gr_make_light(L,Num_lights-1, 2);
	}
//	light_data *L = (light_data*)(void*)l;
//	l->API_index = gr_make_light(L);
}

// for now, tube lights only affect one ship (to keep the filter stuff simple)
void light_add_tube(vector *p0, vector *p1, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum, float spec_r, float spec_g, float spec_b, bool specular )
{
	Assert(r1 >0);
	Assert(r2 >0);
	if(!specular){
		spec_r = r;
		spec_g = g;
		spec_b = b;
	}
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_TUBE;
	l->vec = *p0;
	l->vec2 = *p1;
	l->r = r;
	l->g = g;
	l->b = b;
	l->spec_r = spec_r;
	l->spec_g = spec_g;
	l->spec_b = spec_b;
	l->intensity = intensity;
	l->rad1 = r1;
	l->rad2 = r2;
	l->rad1_squared = l->rad1*l->rad1;
	l->rad2_squared = l->rad2*l->rad2;
	l->ignore_objnum = -1;
	l->affected_objnum = affected_objnum;
	l->instance = Num_lights-1;

	Assert( Num_light_levels <= 1 );
	if(!Cmdline_nohtl) {
		light_data *L = (light_data*)(void*)l;
		gr_make_light(L,Num_lights-1, 3);
	}
//	light_data *L = (light_data*)(void*)l;
//	l->API_index = gr_make_light(L);
}

// Reset the list of lights to point to all lights.
void light_filter_reset()
{
	int i;
	light *l;

	if ( Lighting_off ) return;

	Num_light_levels = 1;

	int n = Num_light_levels-1;
	Num_relevent_lights[n] = 0;

	l = Lights;
	for (i=0; i<Num_lights; i++, l++ )	{
		Relevent_lights[Num_relevent_lights[n]++][n] = l;
	}
}


// Makes a list of only the lights that will affect
// the sphere specified by 'pos' and 'rad' and 'objnum'
int light_filter_push( int objnum, vector *pos, float rad )
{
	int i;
	light *l;

	if ( Lighting_off ) return 0;

	light_filter_reset();

	int n1,n2;
	n1 = Num_light_levels-1;
	n2 = Num_light_levels;
	Num_light_levels++;
	Assert( Num_light_levels < MAX_LIGHT_LEVELS );

	Num_relevent_lights[n2] = 0;

	for (i=0; i<Num_relevent_lights[n1]; i++ )	{
		l = Relevent_lights[i][n1];

		switch( l->type )	{
		case LT_DIRECTIONAL:
			//Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			break;

		case LT_POINT:	{
				// if this is a "unique" light source, it only affects one guy
				if(l->affected_objnum >= 0){
					if(objnum == l->affected_objnum){
						vector to_light;
						float dist_squared, max_dist_squared;
						vm_vec_sub( &to_light, &l->vec, pos );
						dist_squared = vm_vec_mag_squared(&to_light);

						max_dist_squared = l->rad2+rad;
						max_dist_squared *= max_dist_squared;
						
						if ( dist_squared < max_dist_squared )	{
							Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
						}
					}
				}
				// otherwise check all relevant objects
				else {
					// if ( (l->ignore_objnum<0) || (l->ignore_objnum != objnum) )	{
						vector to_light;
						float dist_squared, max_dist_squared;
						vm_vec_sub( &to_light, &l->vec, pos );
						dist_squared = vm_vec_mag_squared(&to_light);

						max_dist_squared = l->rad2+rad;
						max_dist_squared *= max_dist_squared;
						
						if ( dist_squared < max_dist_squared )	{
							Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
						}
					// }
				}
			}
			break;

		// hmm. this could probably be more optimal
		case LT_TUBE:
			// all tubes are "unique" light sources for now
			if((l->affected_objnum >= 0) && (objnum == l->affected_objnum)){
				Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			}
			break;

		default:
			Int3();	// Invalid light type
		}
	}

	return Num_relevent_lights[n2];
}

int is_inside( vector *min, vector *max, vector * p0, float rad )
{
	float *origin = (float *)&p0->xyz.x;
	float *minB = (float *)min;
	float *maxB = (float *)max;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] - rad )	{
			return 0;
		} else if (origin[i] > maxB[i] + rad )	{
			return 0;
		}
	}
	return 1;
}


int light_filter_push_box( vector *min, vector *max )
{
	int i;
	light *l;

	if ( Lighting_off ) return 0;

	int n1,n2;
	n1 = Num_light_levels-1;
	n2 = Num_light_levels;
	Num_light_levels++;

//	static int mll = -1;
//	if ( Num_light_levels > mll )	{
//		mll = Num_light_levels;
//		mprintf(( "Max level = %d\n", mll ));
//	}

	Assert( Num_light_levels < MAX_LIGHT_LEVELS );

	Num_relevent_lights[n2] = 0;

	for (i=0; i<Num_relevent_lights[n1]; i++ )	{
		l = Relevent_lights[i][n1];

		switch( l->type )	{
		case LT_DIRECTIONAL:
			Int3();	//Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			break;

		case LT_POINT:	{
				// l->local_vec
				if ( is_inside( min, max, &l->local_vec, l->rad2 ) )	{
					Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
				}
			}
			break;

		case LT_TUBE:
			if ( is_inside(min, max, &l->local_vec, l->rad2) || is_inside(min, max, &l->local_vec2, l->rad2) )	{
				Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			}
			break;

		default:
			Int3();	// Invalid light type
		}
	}

	return Num_relevent_lights[n2];
}

void light_filter_pop()
{
	if ( Lighting_off ) return;

	Num_light_levels--;
	Assert( Num_light_levels > 0 );
}

int l_num_points=0, l_num_lights=0;


void light_rotate_all()
{
	int i;
	light *l;

	if ( Lighting_off ) return;

	int n = Num_light_levels-1;

	l = Lights;
	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];
		light_rotate(l);
	}

	for(i=0; i<Static_light_count; i++){	
		light_rotate(Static_light[i]);
	}

//	l = Lights;
//	for (i=0; i<Num_lights; i++, l++ )	{
//		light_rotate(l);
//	}
}

// return the # of global light sources
int light_get_global_count()
{
	return Static_light_count;
}

int light_get_global_dir(vector *pos, int n)
{
	if((n > MAX_STATIC_LIGHTS) || (n > Static_light_count-1)){
		return 0;
	}

	if ( Static_light[n] == NULL ) {
		return 0;
	}

	if (pos) {
		*pos = Static_light[n]->vec;

		if ( Lighting_mode != LM_DARKEN )	{
			vm_vec_scale( pos, -1.0f );
		}
	}
	return 1;
}


void light_set_shadow( int state )
{
	Light_in_shadow = state;
}


void light_set_all_relevent(){
	int i = 0;
	gr_reset_lighting();
	for(int idx=0; idx<Static_light_count; idx++)gr_set_light((light_data*)(void*)Static_light[idx]);
//	for simplicity sake were going to forget about dynamic lights for the moment
	int n = Num_light_levels-1;
	for( i=0; i<Num_relevent_lights[n]; i++ )gr_set_light((light_data*)(void*)Relevent_lights[i][n]);
}


ubyte light_apply( vector *pos, vector * norm, float static_light_level )
{
	int i, idx;
	float lval;
	light *l;

	if (Detail.lighting==0) {
		// No static light
		ubyte l = ubyte(fl2i(static_light_level*255.0f));
		return l;
	}

	if ( Lighting_off ) return 191;

	// Factor in ambient light
	lval = Ambient_light;
	
	// Factor in light from suns if there are any
	if ( !Light_in_shadow ){
		// apply all sun lights
		for(idx=0; idx<Static_light_count; idx++){		
			float ltmp;

			// sanity 
			if(Static_light[idx] == NULL){
				continue;
			}

			// calculate light from surface normal
			ltmp = -vm_vec_dot(&Static_light[idx]->local_vec, norm )*Static_light[idx]->intensity*Reflective_light;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )
					lval += ltmp;
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )
					lval -= ltmp;

				if ( lval < 0.0f ) 
					lval = 0.0f;
				break;
			}
		}
	}

	// At this point, l must be between 0 and 0.75 (0.75-1.0 is for dynamic light only)
	if ( lval < 0.0f ) {
		lval = 0.0f;
	} else if ( lval > 0.75f ) {
		lval = 0.75f;
	}

	lval *= static_light_level;

	int n = Num_light_levels-1;

	l_num_lights += Num_relevent_lights[n];
	l_num_points++;

	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];

		vector to_light;
		float dot, dist;
		vm_vec_sub( &to_light, &l->local_vec, pos );
		dot = vm_vec_dot(&to_light, norm );
		if ( dot > 0.0f )	{
			dist = vm_vec_mag_squared(&to_light);
			if ( dist < l->rad1_squared )	{
				lval += l->intensity*dot;
			} else if ( dist < l->rad2_squared )	{
				// dist from 0 to 
				float n = dist - l->rad1_squared;
				float d = l->rad2_squared - l->rad1_squared;
				float ltmp = (1.0f - n / d )*dot*l->intensity;
				lval += ltmp;
			}
			if ( lval > 1.0f ) {
				return 255;
			}
		}
	}

	return ubyte(fl2i(lval*255.0f));
}

int spec = 0;
float static_light_factor = 1.0f;
float static_tube_factor = 1.0f;
float static_point_factor = 1.0f;
double specular_exponent_value = 16.0;

void light_apply_specular(ubyte *param_r, ubyte *param_g, ubyte *param_b, vector *pos, vector * norm, vector * cam){

	light *l;
	float rval = 0, gval = 0, bval = 0;

	if ( Cmdline_nospec ) {
		*param_r = 0;
		*param_g = 0;
		*param_b = 0;
		return;
	}

	if (Detail.lighting==0) {
		*param_r = 0;
		*param_g = 0;
		*param_b = 0;
		return;
	}

	if ( Lighting_off ) {
		*param_r = 0;
		*param_g = 0;
		*param_b = 0;
		return;
	}

	vector V, N;
	vm_vec_sub(&V, cam,pos);
	vm_vec_normalize(&V);

	N = *norm;
	vm_vec_normalize(&N);

	// Factor in light from sun if there is one
		// apply all sun lights
		for(int idx=0; idx<Static_light_count; idx++){			
			float ltmp;

			// sanity
			if(Static_light[idx] == NULL){
				continue;
			}

			vector R;
			vm_vec_sub(&R,&V, &Static_light[idx]->local_vec);
			vm_vec_normalize(&R);

			ltmp = (float)pow((double)vm_vec_dot(&R, &N ), specular_exponent_value) * Static_light[idx]->intensity * static_light_factor;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )	{
					rval += Static_light[idx]->spec_r * ltmp;
					gval += Static_light[idx]->spec_g * ltmp;
					bval += Static_light[idx]->spec_b * ltmp;
				}
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )	{
					rval -= ltmp; if ( rval < 0.0f ) rval = 0.0f;
					gval -= ltmp; if ( gval < 0.0f ) gval = 0.0f;
					bval -= ltmp; if ( bval < 0.0f ) bval = 0.0f; 
				}
				break;
			}
		}


	if ( rval < 0.0f ) {
		rval = 0.0f;
	} else if ( rval > 0.75f ) {
		rval = 0.75f;
	}
	if ( gval < 0.0f ) {
		gval = 0.0f;
	} else if ( gval > 0.75f ) {
		gval = 0.75f;
	}
	if ( bval < 0.0f ) {
		bval = 0.0f;
	} else if ( bval > 0.75f ) {
		bval = 0.75f;
	}

	//dynamic lights

	int n = Num_light_levels-1;

	l_num_lights += Num_relevent_lights[n];
	l_num_points++;

	vector to_light;
	float dot, dist;
	vector temp;
	float factor = 1.0f;
	for ( int i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];

		dist = -1.0f;
		switch(l->type){
		// point lights
		case LT_POINT:			
			vm_vec_sub( &to_light, &l->local_vec, pos );
			factor = static_point_factor;
			break;

		// tube lights
		case LT_TUBE:						
			if(vm_vec_dist_to_line(pos, &l->local_vec, &l->local_vec2, &temp, &dist) != 0){
				continue;
			}
			factor = static_tube_factor;
			vm_vec_sub(&to_light, &temp, pos);
			dist *= dist;	// since we use radius squared
			break;

		// others. BAD
		default:
			Int3();
		}

		vector R;
		vm_vec_normalize(&to_light);
		vm_vec_add(&R,&V, &to_light);
		vm_vec_normalize(&R);

		dot = (float)pow((double)vm_vec_dot(&R, &N ), specular_exponent_value) * l->intensity * factor;		// reflective light
	
		if ( dot > 0.0f )	{
			// indicating that we already calculated the distance (vm_vec_dist_to_line(...) does this for us)
			if(dist < 0.0f){
				dist = vm_vec_mag_squared(&to_light);
			}
			if ( dist < l->rad1_squared )	{
				float ratio;
				ratio = l->intensity*dot*factor;
				ratio *= 0.25f;
				rval += l->spec_r*ratio;
				gval += l->spec_g*ratio;
				bval += l->spec_b*ratio;
			} else if ( dist < l->rad2_squared )	{
				float ratio;
				// dist from 0 to 
				float n = dist - l->rad1_squared;
				float d = l->rad2_squared - l->rad1_squared;
				ratio = (1.0f - n / d)*dot*l->intensity*factor;
				ratio *= 0.25f;
				rval += l->spec_r*ratio;
				gval += l->spec_g*ratio;
				bval += l->spec_b*ratio;
			}
		}
	}

	if ( rval < 0.0f ) {
		rval = 0.0f;
	} else if ( rval > 1.0f ) {
		rval = 1.0f;
	}
	if ( gval < 0.0f ) {
		gval = 0.0f;
	} else if ( gval > 1.0f ) {
		gval = 1.0f;
	}
	if ( bval < 0.0f ) {
		bval = 0.0f;
	} else if ( bval > 1.0f ) {
		bval = 1.0f;
	}

	*param_r = ubyte(fl2i(rval*254.0f));
	*param_g = ubyte(fl2i(gval*254.0f));
	*param_b = ubyte(fl2i(bval*254.0f));
}

void light_apply_rgb( ubyte *param_r, ubyte *param_g, ubyte *param_b, vector *pos, vector * norm, float static_light_level )
{
	int i, idx;
	float rval, gval, bval;
	light *l;

	if (Detail.lighting==0) {
		// No static light
		ubyte l = ubyte(fl2i(static_light_level*255.0f));
		*param_r = l;
		*param_g = l;
		*param_b = l;
		return;
	}

	if ( Lighting_off ) {
		*param_r = 255;
		*param_g = 255;
		*param_b = 255;
		return;
	}

	// Factor in ambient light
	rval = Ambient_light;
	gval = Ambient_light;
	bval = Ambient_light;

	// Factor in light from sun if there is one
	if ( !Light_in_shadow ){
		// apply all sun lights
		for(idx=0; idx<Static_light_count; idx++){			
			float ltmp;

			// sanity
			if(Static_light[idx] == NULL){
				continue;
			}

			// calculate light from surface normal
			ltmp = -vm_vec_dot(&Static_light[idx]->local_vec, norm )*Static_light[idx]->intensity*Reflective_light;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )	{
					rval += Static_light[idx]->r * ltmp;
					gval += Static_light[idx]->g * ltmp;
					bval += Static_light[idx]->b * ltmp;
				}
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )	{
					rval -= ltmp; if ( rval < 0.0f ) rval = 0.0f;
					gval -= ltmp; if ( gval < 0.0f ) gval = 0.0f;
					bval -= ltmp; if ( bval < 0.0f ) bval = 0.0f; 
				}
				break;
			}
		}
	}

	// At this point, l must be between 0 and 0.75 (0.75-1.0 is for dynamic light only)
	if ( rval < 0.0f ) {
		rval = 0.0f;
	} else if ( rval > 0.75f ) {
		rval = 0.75f;
	}
	if ( gval < 0.0f ) {
		gval = 0.0f;
	} else if ( gval > 0.75f ) {
		gval = 0.75f;
	}
	if ( bval < 0.0f ) {
		bval = 0.0f;
	} else if ( bval > 0.75f ) {
		bval = 0.75f;
	}

	rval *= static_light_level;
	gval *= static_light_level;
	bval *= static_light_level;

	int n = Num_light_levels-1;

	l_num_lights += Num_relevent_lights[n];
	l_num_points++;

	vector to_light;
	float dot, dist;
	vector temp;
	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];
break;
		dist = -1.0f;
		switch(l->type){
		// point lights
		case LT_POINT:			
			vm_vec_sub( &to_light, &l->local_vec, pos );
			break;

		// tube lights
		case LT_TUBE:						
			if(vm_vec_dist_to_line(pos, &l->local_vec, &l->local_vec2, &temp, &dist) != 0){
				continue;
			}
			vm_vec_sub(&to_light, &temp, pos);
			dist *= dist;	// since we use radius squared
			break;

		// others. BAD
		default:
			Int3();
		}

		dot = vm_vec_dot(&to_light, norm);
	//		dot = 1.0f;
		if ( dot > 0.0f )	{
			// indicating that we already calculated the distance (vm_vec_dist_to_line(...) does this for us)
			if(dist < 0.0f){
				dist = vm_vec_mag_squared(&to_light);
			}
			if ( dist < l->rad1_squared )	{
				float ratio;
				ratio = l->intensity*dot;
				ratio *= 0.25f;
				rval += l->r*ratio;
				gval += l->g*ratio;
				bval += l->b*ratio;
			} else if ( dist < l->rad2_squared )	{
				float ratio;
				// dist from 0 to 
				float n = dist - l->rad1_squared;
				float d = l->rad2_squared - l->rad1_squared;
				ratio = (1.0f - n / d)*dot*l->intensity;
				ratio *= 0.25f;
				rval += l->r*ratio;
				gval += l->g*ratio;
				bval += l->b*ratio;
			}
		}
	}

	float m = rval;
	if ( gval > m ) m = gval;
	if ( bval > m ) m = bval;

	if ( m > 1.0f )	{
		float im = 1.0f / m;

		rval *= im;
		gval *= im;
		bval *= im;
	}
	
	if ( rval < 0.0f ) {
		rval = 0.0f;
	} else if ( rval > 1.0f ) {
		rval = 1.0f;
	}
	if ( gval < 0.0f ) {
		gval = 0.0f;
	} else if ( gval > 1.0f ) {
		gval = 1.0f;
	}
	if ( bval < 0.0f ) {
		bval = 0.0f;
	} else if ( bval > 1.0f ) {
		bval = 1.0f;
	}

	*param_r = ubyte(fl2i(rval*255.0f));
	*param_g = ubyte(fl2i(gval*255.0f));
	*param_b = ubyte(fl2i(bval*255.0f));
}


/*
float light_apply( vector *pos, vector * norm )
{
#if 1
	float r,g,b;
	light_apply_rgb( &r, &g, &b, pos, norm );
	return (r+g+b) / 3.0f;
#else
	return light_apply_ramp( pos, norm );
#endif

}
*/



