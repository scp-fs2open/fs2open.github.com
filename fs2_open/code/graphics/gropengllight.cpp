/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLLight.cpp $
 * $Revision: 1.25 $
 * $Date: 2006-01-30 06:40:49 $
 * $Author: taylor $
 *
 * code to implement lighting in HT&L opengl
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.24  2006/01/20 17:15:16  taylor
 * gr_*_bitmap_ex() stuff, D3D side is 100% untested to even compile
 * several other very minor changes as well
 *
 * Revision 1.23  2006/01/19 05:40:19  wmcoolmon
 * Possible overflow check
 *
 * Revision 1.22  2006/01/14 19:25:55  taylor
 * minor OGL lighting fix
 *
 * Revision 1.21  2005/12/06 02:50:41  taylor
 * clean up some init stuff and fix a minor SDL annoyance
 * make debug messages a bit more readable
 * clean up the debug console commands for minimize and anisotropic filter setting
 * make anisotropic filter actually work correctly and have it settable with a reg option
 * give opengl_set_arb() the ability to disable all features on all arbs at once so I don't have to everywhere
 *
 * Revision 1.20  2005/10/23 11:45:06  taylor
 * add -ogl_shine to adjust the OGL shininess value so that people can play around and find the best value to use
 *
 * Revision 1.19  2005/09/30 09:47:06  taylor
 * remove -rlm, it's always on now since there was never a complaint and pretty much everyone uses it
 * add -cache_bitmaps and have bitmap caching between levels off by default
 * when -cache_bitmaps is used then use C-BMP for top-right memory listing, and just BMP otherwise
 *
 * Revision 1.18  2005/09/05 09:36:41  taylor
 * merge of OSX tree
 * fix OGL fullscreen switch for SDL since the old way only worked under Linux and not OSX or Windows
 * fix OGL version check, it would allow a required major version to be higher if the required minor version was lower than current
 *
 * Revision 1.17  2005/05/23 05:56:26  taylor
 * Jens, again:
 *  - compiler warning fixes
 *  - sanity and readability fixes for bmpman
 *
 * Revision 1.16  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 1.15  2005/04/24 12:56:42  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 1.14  2005/04/15 11:42:27  taylor
 * fix ambient light
 *
 * Revision 1.13  2005/04/12 02:04:56  phreak
 * gr_set_ambient_light() function for the ambient light sliders in FRED
 *
 * Revision 1.12  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 1.11  2005/02/05 00:30:49  taylor
 * fix a few things post merge
 *
 * Revision 1.10  2005/02/04 23:29:31  taylor
 * merge with Linux/OSX tree - p0204-3
 *
 * Revision 1.9  2005/01/21 21:12:52  taylor
 * crap
 *
 * Revision 1.8  2005/01/21 08:29:04  taylor
 * add -rlm cmdline option to switch to local viewpoint lighting calculations (OGL only for now)
 *
 * Revision 1.7  2005/01/07 14:00:04  argv
 * Added missing #include.
 *
 * -- _argv[-1]
 *
 * Revision 1.6  2005/01/03 18:45:22  taylor
 * dynamic allocation of num supported OpenGL lights
 * add config option for more realistic light settings
 * don't render spec maps in nebula to address lighting issue
 *
 * Revision 1.5  2005/01/01 11:24:23  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 1.4  2004/10/31 21:45:13  taylor
 * Linux tree merge, single array for VBOs/HTL
 *
 * Revision 1.3  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 1.2  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
 *
 * Revision 2.7  2004/05/13 00:17:20  taylor
 * disable COLOR_MATERIAL, for nvidia drivers
 *
 * Revision 2.6  2004/05/12 23:24:35  phreak
 * got rid of the color material call, nebulas should blind the player
 *
 * Revision 2.5  2004/05/11 19:39:29  phreak
 * fixed an array bounds error that was causing debug builds to crash when
 * rendering a ship
 *
 * Revision 2.4  2004/04/26 13:02:27  taylor
 * light setup changes, support cmdline ambient value
 *
 * Revision 2.3  2004/04/13 01:55:41  phreak
 * put in the correct fields for the CVS comments to register
 * fixed a glowmap problem that occured when rendering glowmapped and non-glowmapped ships
 *
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengllight.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"
#include "lighting/lighting.h"



// Variables
opengl_light opengl_lights[MAX_LIGHTS];
bool active_light_list[MAX_LIGHTS];
int *currently_enabled_lights = NULL;
bool lighting_is_enabled = true;
int active_gl_lights = 0;
int n_active_gl_lights = 0;
int GL_center_alpha = 0;

extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;
extern double specular_exponent_value;
extern float Cmdline_ogl_spec;


GLint GL_max_lights = 0;

// OGL defaults
static const float GL_light_color[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
static const float GL_light_spec[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const float GL_light_zero[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
static const float GL_light_emission[4] = { 0.1f, 0.1f, 0.1f, 0.7f };
static const float GL_light_true_zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
static float GL_light_ambient[4] = { 0.8f, 0.8f, 0.8f, 1.0f };


void FSLight2GLLight(opengl_light *GLLight, light *FSLight)
{
	GLLight->Diffuse.r = FSLight->r * FSLight->intensity;
	GLLight->Diffuse.g = FSLight->g * FSLight->intensity;
	GLLight->Diffuse.b = FSLight->b * FSLight->intensity;
	GLLight->Specular.r = FSLight->spec_r * FSLight->intensity;
	GLLight->Specular.g = FSLight->spec_g * FSLight->intensity;
	GLLight->Specular.b = FSLight->spec_b * FSLight->intensity;
	GLLight->Ambient.r = 0.0f;
	GLLight->Ambient.g = 0.0f;
	GLLight->Ambient.b = 0.0f;
	GLLight->Ambient.a = 1.0f;
	GLLight->Specular.a = 1.0f;
	GLLight->Diffuse.a = 1.0f;

	GLLight->type = FSLight->type;

	// GL default values...
	// spot direction
	GLLight->SpotDir.x = 0.0f;
	GLLight->SpotDir.y = 0.0f;
	GLLight->SpotDir.z = -1.0f;
	// spot exponent
	GLLight->SpotExp = Cmdline_ogl_spec;
	// spot cutoff
	GLLight->SpotCutOff = 180.0f; // special value, light in all directions
	// defaults to disable attenuation
	GLLight->ConstantAtten = 1.0f;
	GLLight->LinearAtten = 0.0f;
	GLLight->QuadraticAtten = 0.0f;


	//If the light is a directional light
	if (FSLight->type == LT_DIRECTIONAL) {
		GLLight->Position.x = -FSLight->vec.xyz.x;
		GLLight->Position.y = -FSLight->vec.xyz.y;
		GLLight->Position.z = -FSLight->vec.xyz.z;
		GLLight->Position.w = 0.0f; // Directional lights in OpenGL have w set to 0 and the direction vector in the position field

		GLLight->Specular.r *= static_light_factor;
		GLLight->Specular.g *= static_light_factor;
		GLLight->Specular.b *= static_light_factor;
	}

	//If the light is a point or tube type
	if ((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {
		// this crap still needs work...
		GLLight->ConstantAtten = 0.0f;
		GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb));

		if(FSLight->type == LT_POINT){
			GLLight->Specular.r *= static_point_factor;
			GLLight->Specular.g *= static_point_factor;
			GLLight->Specular.b *= static_point_factor;
			GLLight->LinearAtten *= 2.0f;
		}else{
			GLLight->Specular.r *= static_tube_factor;
			GLLight->Specular.g *= static_tube_factor;
			GLLight->Specular.b *= static_tube_factor;
		}

		GLLight->Position.x = FSLight->vec.xyz.x;
		GLLight->Position.y = FSLight->vec.xyz.y;
		GLLight->Position.z = FSLight->vec.xyz.z; // flipped axis for FS2
		GLLight->Position.w = 1.0f;		

		if (FSLight->type == LT_TUBE) {
			GLLight->SpotDir.x = FSLight->vec2.xyz.x;
			GLLight->SpotDir.y = FSLight->vec2.xyz.y;
			GLLight->SpotDir.z = FSLight->vec2.xyz.z;
			GLLight->SpotCutOff = 90.0f;
		}
	}
}

void opengl_set_light(int light_num, opengl_light *light)
{
	Assert(light_num < GL_max_lights);

	glLightfv(GL_LIGHT0+light_num, GL_POSITION, &light->Position.x);
	glLightfv(GL_LIGHT0+light_num, GL_AMBIENT, &light->Ambient.r);
	glLightfv(GL_LIGHT0+light_num, GL_DIFFUSE, &light->Diffuse.r);
	glLightfv(GL_LIGHT0+light_num, GL_SPECULAR, &light->Specular.r);
	glLightfv(GL_LIGHT0+light_num, GL_SPOT_DIRECTION, &light->SpotDir.x);
	glLightf(GL_LIGHT0+light_num, GL_CONSTANT_ATTENUATION, light->ConstantAtten);
	glLightf(GL_LIGHT0+light_num, GL_LINEAR_ATTENUATION, light->LinearAtten);
	glLightf(GL_LIGHT0+light_num, GL_QUADRATIC_ATTENUATION, light->QuadraticAtten);
	glLightf(GL_LIGHT0+light_num, GL_SPOT_EXPONENT, light->SpotExp);
	glLightf(GL_LIGHT0+light_num, GL_SPOT_CUTOFF, light->SpotCutOff);
}

//finds the first unocupyed light
void opengl_pre_render_init_lights()
{
	for (int i = 0; i < GL_max_lights; i++) {
		if (currently_enabled_lights[i] > -1)
			glDisable(GL_LIGHT0+i);

		currently_enabled_lights[i] = -1;
	}
}


void opengl_change_active_lights(int pos)
{
	if ( !lighting_is_enabled )
		return;

	int i, k = 0, l = 0;
	bool move = false;
	vec3d fwd;
	vec3d *uvec = &Eye_matrix.vec.uvec;

	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix();				
	glLoadIdentity();

	vm_vec_add(&fwd, &Eye_position, &Eye_matrix.vec.fvec);

	gluLookAt(Eye_position.xyz.x, Eye_position.xyz.y, -Eye_position.xyz.z,
				fwd.xyz.x, fwd.xyz.y, -fwd.xyz.z, uvec->xyz.x, uvec->xyz.y, -uvec->xyz.z);

	glScalef(1.0f, 1.0f, -1.0f);

	for (i = 0; (i < GL_max_lights) && (((pos * GL_max_lights) + i) < active_gl_lights); i++) {
		glDisable(GL_LIGHT0+i);
		move = false;

		for (k = 0; (k < MAX_LIGHTS) && !move; k++) {
			int slot = (pos * GL_max_lights) + l;

			if (active_light_list[slot]) {
				if (opengl_lights[slot].occupied) {
					opengl_set_light(i,&opengl_lights[slot]);
					glEnable(GL_LIGHT0+i);
					currently_enabled_lights[i] = slot;
					move = true;
					l++;
				}
			}
		}
	}

	glPopMatrix();
}

int gr_opengl_make_light(light *fs_light, int idx, int priority)
{
	return idx;
}

void gr_opengl_modify_light(light *fs_light, int idx, int priority)
{
}

void gr_opengl_destroy_light(int idx)
{
}

void gr_opengl_set_light(light *fs_light)
{
	if (Cmdline_nohtl)
		return;

	//Init the light
	//WMC - This can cause a memleak in currently_enabled_lights.
	Assert(active_gl_lights < MAX_LIGHTS);

	FSLight2GLLight(&opengl_lights[active_gl_lights], fs_light);
	opengl_lights[active_gl_lights].occupied = true;
	active_light_list[active_gl_lights++] = true;
}

// this sets up a light to be pointinf from the eye to the object, 
// the point being to make the object ether center or edge alphaed with THL
// this effect is used mostly on shockwave models
// -1 == edge
// 1 == center
// 0 == none
// should be called after lighting has been set up, 
// currently not designed for use with lit models
void gr_opengl_center_alpha(int type)
{
	GL_center_alpha = type;
}

void gr_opengl_set_center_alpha(int type)
{
	if (!type || Cmdline_nohtl)
		return;

	opengl_light glight; // = &opengl_lights[active_gl_lights];

	vec3d dir;
	vm_vec_sub(&dir, &Eye_position, &Object_position);
	vm_vec_normalize(&dir);

	if (type == 1) {
		glight.Diffuse.r = 0.0f;
		glight.Diffuse.g = 0.0f;
		glight.Diffuse.b = 0.0f;
		glight.Ambient.r = gr_screen.current_alpha;
		glight.Ambient.g = gr_screen.current_alpha;
		glight.Ambient.b = gr_screen.current_alpha;
	} else {
		glight.Diffuse.r = gr_screen.current_alpha;
		glight.Diffuse.g = gr_screen.current_alpha;
		glight.Diffuse.b = gr_screen.current_alpha;
		glight.Ambient.r = 0.0f;
		glight.Ambient.g = 0.0f;
		glight.Ambient.b = 0.0f;
	}

	glight.Specular.r = 0.0f;
	glight.Specular.g = 0.0f;
	glight.Specular.b = 0.0f;
	glight.Specular.a = 0.0f;

	glight.Ambient.a = 1.0f;
	glight.Diffuse.a = 1.0f;

	glight.Position.x = -dir.xyz.x;
	glight.Position.y = -dir.xyz.y;
	glight.Position.z = -dir.xyz.z;
	glight.Position.w = 0.0f;

	// defaults
	glight.ConstantAtten = 1.0f;
	glight.LinearAtten = 0.0f;
	glight.QuadraticAtten = 0.0f;

	// first light
	memcpy( &opengl_lights[active_gl_lights], &glight, sizeof(opengl_light) );

	opengl_lights[active_gl_lights].occupied = true;
	active_light_list[active_gl_lights++] = true;

	// second light
	glight.Position.x = dir.xyz.x;
	glight.Position.y = dir.xyz.y;
	glight.Position.z = dir.xyz.z;

	memcpy( &opengl_lights[active_gl_lights], &glight, sizeof(opengl_light) );

	opengl_lights[active_gl_lights].occupied = true;
	active_light_list[active_gl_lights++] = true;

	// reset center alpha
	GL_center_alpha = 0;
}

void gr_opengl_reset_lighting()
{
	int i;

	if (Cmdline_nohtl)
		return;

	for (i = 0; i<MAX_LIGHTS; i++) {
		opengl_lights[i].occupied = false;
	}

	for (i = 0; i < GL_max_lights; i++) {
		glDisable(GL_LIGHT0+i);
		active_light_list[i] = false;
	}

	active_gl_lights =0;
	GL_center_alpha = 0;
}

void opengl_calculate_ambient_factor()
{
	float amb_user = 0.0f;

	amb_user = (float)((Cmdline_ambient_factor * 2) - 255) / 255.0f;

	// set the ambient light
	GL_light_ambient[0] += amb_user;
	GL_light_ambient[1] += amb_user;
	GL_light_ambient[2] += amb_user;
	
	CLAMP( GL_light_ambient[0], 0.02f, 1.0f );
	CLAMP( GL_light_ambient[1], 0.02f, 1.0f );
	CLAMP( GL_light_ambient[2], 0.02f, 1.0f );

	GL_light_ambient[3] = 1.0f;
}

void opengl_init_light()
{
	opengl_calculate_ambient_factor();

	// only on front, tends to be more believable
	glMaterialf(GL_FRONT, GL_SHININESS, Cmdline_ogl_spec /*80.0f*/ );

	// more realistic lighting model
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glGetIntegerv(GL_MAX_LIGHTS, &GL_max_lights); // Get the max number of lights supported

	// allocate memory for enabled lights
	Verify(GL_max_lights > 0);

	if ( currently_enabled_lights == NULL )
		currently_enabled_lights = (int *) vm_malloc_q(GL_max_lights * sizeof(int));

	if (currently_enabled_lights == NULL)
		Error( LOCATION, "Unable to allocate memory for lights!\n");

	memset( currently_enabled_lights, -1, GL_max_lights * sizeof(int) );
}

void opengl_default_light_settings(int ambient = 1, int emission = 1, int specular = 1)
{
	if (!lighting_is_enabled)
		return;

	if (ambient) {
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, GL_light_color );
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, GL_light_ambient );
	} else {
		if (GL_center_alpha) {
			glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, GL_light_true_zero );
		} else {
			glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, GL_light_zero );
		}
	}

	if (emission) {
		// emissive light is just a general glow but without it things are *terribly* dark if there is no light on them
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, GL_light_emission );
	} else {
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, GL_light_zero );
	}

	if (specular) {
		// only on front, tends to be more believable
		glMaterialfv( GL_FRONT, GL_SPECULAR, GL_light_spec );
	} else {
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, GL_light_zero );
	}
}

void gr_opengl_set_lighting(bool set, bool state)
{
	if (Cmdline_nohtl)
		return;

	lighting_is_enabled = set;

	opengl_default_light_settings();

	if ( (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) && !set ) {
		float amb[4] = { gr_screen.current_alpha, gr_screen.current_alpha, gr_screen.current_alpha, gr_screen.current_alpha };
		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb );
	} else {
		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, GL_light_ambient );
	}

	for(int i = 0; i<GL_max_lights; i++){
		if(currently_enabled_lights[i] > -1)glDisable(GL_LIGHT0+i);
		currently_enabled_lights[i] = -1;
	}

	if(state) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}

}

void gr_opengl_set_ambient_light(int red, int green, int blue)
{
	GL_light_ambient[0] = i2fl(red)/255.0f;
	GL_light_ambient[1] = i2fl(green)/255.0f;
	GL_light_ambient[2] = i2fl(blue)/255.0f;
	GL_light_ambient[3] = 1.0f;

	opengl_calculate_ambient_factor();
}
