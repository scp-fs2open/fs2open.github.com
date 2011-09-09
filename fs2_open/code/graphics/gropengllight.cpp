/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglstate.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"
#include "lighting/lighting.h"



// Variables
opengl_light *opengl_lights = NULL;
bool lighting_is_enabled = true;
int Num_active_gl_lights = 0;
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
static const float GL_light_emission[4] = { 0.09f, 0.09f, 0.09f, 1.0f };
static const float GL_light_true_zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
static float GL_light_ambient[4] = { 0.47f, 0.47f, 0.47f, 1.0f };

void FSLight2GLLight(light *FSLight, opengl_light *GLLight)
{
	GLLight->Ambient[0] = 0.0f;
	GLLight->Ambient[1] = 0.0f;
	GLLight->Ambient[2] = 0.0f;
	GLLight->Ambient[3] = 1.0f;

	GLLight->Diffuse[0] = FSLight->r * FSLight->intensity;
	GLLight->Diffuse[1] = FSLight->g * FSLight->intensity;
	GLLight->Diffuse[2] = FSLight->b * FSLight->intensity;
	GLLight->Diffuse[3] = 1.0f;

	GLLight->Specular[0] = FSLight->spec_r * FSLight->intensity;
	GLLight->Specular[1] = FSLight->spec_g * FSLight->intensity;
	GLLight->Specular[2] = FSLight->spec_b * FSLight->intensity;
	GLLight->Specular[3] = 1.0f;

	GLLight->type = FSLight->type;

	// GL default values...
	// spot direction
	GLLight->SpotDir[0] = 0.0f;
	GLLight->SpotDir[1] = 0.0f;
	GLLight->SpotDir[2] = -1.0f;
	// spot exponent
	GLLight->SpotExp = Cmdline_ogl_spec * 0.5f;
	// spot cutoff
	GLLight->SpotCutOff = 180.0f; // special value, light in all directions
	// defaults to disable attenuation
	GLLight->ConstantAtten = 1.0f;
	GLLight->LinearAtten = 0.0f;
	GLLight->QuadraticAtten = 0.0f;
	// position
	GLLight->Position[0] = FSLight->vec.xyz.x;
	GLLight->Position[1] = FSLight->vec.xyz.y;
	GLLight->Position[2] = FSLight->vec.xyz.z; // flipped axis for FS2
	GLLight->Position[3] = 1.0f;


	switch (FSLight->type) {
		case LT_POINT: {
			// this crap still needs work...
			GLLight->ConstantAtten = 1.0f;
			GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb)) * 1.25f;

			GLLight->Specular[0] *= static_point_factor;
			GLLight->Specular[1] *= static_point_factor;
			GLLight->Specular[2] *= static_point_factor;

			break;
		}

		case LT_TUBE: {
			GLLight->ConstantAtten = 1.0f;
			GLLight->LinearAtten = (1.0f / MAX(FSLight->rada, FSLight->radb)) * 1.25f;
			GLLight->QuadraticAtten = (1.0f / MAX(FSLight->rada_squared, FSLight->radb_squared)) * 1.25f;

			GLLight->Specular[0] *= static_tube_factor;
			GLLight->Specular[1] *= static_tube_factor;
			GLLight->Specular[2] *= static_tube_factor;

			GLLight->Position[0] = FSLight->vec2.xyz.x; // Valathil: Use endpoint of tube as light position
			GLLight->Position[1] = FSLight->vec2.xyz.y;
			GLLight->Position[2] = FSLight->vec2.xyz.z;
			GLLight->Position[3] = 1.0f;

			if ( Use_GLSL > 1 ) {
				// Valathil: When using shaders pass the beam direction (not normalized IMPORTANT for calculation of tube)
				vec3d a;
				vm_vec_sub(&a, &FSLight->vec2, &FSLight->vec);
				GLLight->SpotDir[0] = a.xyz.x; 
				GLLight->SpotDir[1] = a.xyz.y;
				GLLight->SpotDir[2] = a.xyz.z;
				GLLight->SpotCutOff = 90.0f; // Valathil: So shader dectects tube light
			} else {
				GLLight->SpotDir[0] = 1.0f; // Valathil: When not using shaders pass a fake spotdir
				GLLight->SpotDir[1] = 0.0f;
				GLLight->SpotDir[2] = 0.0f;
				GLLight->SpotCutOff = 180.0f; // Valathil: Should be a point light not a spot; using tube only for the light sorting
			}
			break;
		}

		case LT_DIRECTIONAL: {
			GLLight->Position[0] = -FSLight->vec.xyz.x;
			GLLight->Position[1] = -FSLight->vec.xyz.y;
			GLLight->Position[2] = -FSLight->vec.xyz.z;
			GLLight->Position[3] = 0.0f; // Directional lights in OpenGL have w set to 0

			GLLight->Specular[0] *= static_light_factor;
			GLLight->Specular[1] *= static_light_factor;
			GLLight->Specular[2] *= static_light_factor;

			break;
		}

		default:
			Int3();
			break;
	}
}

extern float Interp_light;
void opengl_set_light(int light_num, opengl_light *ltp)
{
	Assert(light_num < GL_max_lights);

	GLfloat diffuse[4];
	memcpy(diffuse, ltp->Diffuse, sizeof(GLfloat) * 4);

	if ( (ltp->type == LT_DIRECTIONAL) && (Interp_light < 1.0f) ) {
		diffuse[0] *= Interp_light;
		diffuse[1] *= Interp_light;
		diffuse[2] *= Interp_light;
	}

	glLightfv(GL_LIGHT0+light_num, GL_POSITION, ltp->Position);
	glLightfv(GL_LIGHT0+light_num, GL_AMBIENT, ltp->Ambient);
	glLightfv(GL_LIGHT0+light_num, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0+light_num, GL_SPECULAR, ltp->Specular);
	glLightfv(GL_LIGHT0+light_num, GL_SPOT_DIRECTION, ltp->SpotDir);
	glLightf(GL_LIGHT0+light_num, GL_CONSTANT_ATTENUATION, ltp->ConstantAtten);
	glLightf(GL_LIGHT0+light_num, GL_LINEAR_ATTENUATION, ltp->LinearAtten);
	glLightf(GL_LIGHT0+light_num, GL_QUADRATIC_ATTENUATION, ltp->QuadraticAtten);
	glLightf(GL_LIGHT0+light_num, GL_SPOT_EXPONENT, ltp->SpotExp);
	glLightf(GL_LIGHT0+light_num, GL_SPOT_CUTOFF, ltp->SpotCutOff);
}

#include <globalincs/systemvars.h>
int opengl_sort_active_lights(const void *a, const void *b)
{
	opengl_light *la, *lb;

	la = (opengl_light *) a;
	lb = (opengl_light *) b;

	// directional lights always go first
	if ( (la->type != LT_DIRECTIONAL) && (lb->type == LT_DIRECTIONAL) )
		return 1;
	else if ( (la->type == LT_DIRECTIONAL) && (lb->type != LT_DIRECTIONAL) )
		return -1;

	// tube lights go next, they are generally large and intense
	if ( (la->type != LT_TUBE) && (lb->type == LT_TUBE) )
		return 1;
	else if ( (la->type == LT_TUBE) && (lb->type != LT_TUBE) )
		return -1;

	// everything else is sorted by linear atten (light size)
	// NOTE: smaller atten is larger light radius!
	if ( la->LinearAtten > lb->LinearAtten )
		return 1;
	else if ( la->LinearAtten < lb->LinearAtten )
		return -1;

	// as one extra check, if we're still here, go with overall brightness of light

	float la_value = la->Diffuse[0] + la->Diffuse[1] + la->Diffuse[2];
	float lb_value = lb->Diffuse[0] + lb->Diffuse[1] + lb->Diffuse[2];

	if ( la_value < lb_value )
		return 1;
	else if ( la_value > lb_value )
		return -1;

	// the two are equal
	return 0;
}

void opengl_pre_render_init_lights()
{
	// sort the lights to try and get the most visible lights on the first pass
	qsort(opengl_lights, Num_active_gl_lights, sizeof(opengl_light), opengl_sort_active_lights);
}

static GLdouble eyex, eyey, eyez;
static GLdouble vmatrix[16];

static vec3d last_view_pos;
static matrix last_view_orient;

static bool use_last_view = false;

void opengl_change_active_lights(int pos, int d_offset)
{
	int i, offset;

	if ( !lighting_is_enabled ) {
		return;
	}

	Assert( d_offset < GL_max_lights );

	offset = (pos * GL_max_lights) + d_offset;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if ( !memcmp(&Eye_position, &last_view_pos, sizeof(vec3d)) && !memcmp(&Eye_matrix, &last_view_orient, sizeof(matrix)) ) {
		use_last_view = true;
	} else {
		memcpy(&last_view_pos, &Eye_position, sizeof(vec3d));
		memcpy(&last_view_orient, &Eye_matrix, sizeof(matrix));

		use_last_view = false;
	}

	if ( !use_last_view ) {
		// should already be normalized
		eyex =  (GLdouble)Eye_position.xyz.x;
		eyey =  (GLdouble)Eye_position.xyz.y;
		eyez = -(GLdouble)Eye_position.xyz.z;

		// should already be normalized
		GLdouble fwdx =  (GLdouble)Eye_matrix.vec.fvec.xyz.x;
		GLdouble fwdy =  (GLdouble)Eye_matrix.vec.fvec.xyz.y;
		GLdouble fwdz = -(GLdouble)Eye_matrix.vec.fvec.xyz.z;

		// should already be normalized
		GLdouble upx =  (GLdouble)Eye_matrix.vec.uvec.xyz.x;
		GLdouble upy =  (GLdouble)Eye_matrix.vec.uvec.xyz.y;
		GLdouble upz = -(GLdouble)Eye_matrix.vec.uvec.xyz.z;

		GLdouble mag;

		// setup Side vector (crossprod of forward and up vectors)
		GLdouble Sx = (fwdy * upz) - (fwdz * upy);
		GLdouble Sy = (fwdz * upx) - (fwdx * upz);
		GLdouble Sz = (fwdx * upy) - (fwdy * upx);

		// normalize Side
		mag = 1.0 / sqrt( (Sx*Sx) + (Sy*Sy) + (Sz*Sz) );

		Sx *= mag;
		Sy *= mag;
		Sz *= mag;

		// setup Up vector (crossprod of s and forward vectors)
		GLdouble Ux = (Sy * fwdz) - (Sz * fwdy);
		GLdouble Uy = (Sz * fwdx) - (Sx * fwdz);
		GLdouble Uz = (Sx * fwdy) - (Sy * fwdx);

		// normalize Up
		mag = 1.0 / sqrt( (Ux*Ux) + (Uy*Uy) + (Uz*Uz) );

		Ux *= mag;
		Uy *= mag;
		Uz *= mag;

		// store the result in our matrix
		memset( vmatrix, 0, sizeof(GLdouble) * 16 );
		vmatrix[0]  = Sx;   vmatrix[1]  = Ux;   vmatrix[2]  = -fwdx;
		vmatrix[4]  = Sy;   vmatrix[5]  = Uy;   vmatrix[6]  = -fwdy;
		vmatrix[8]  = Sz;   vmatrix[9]  = Uz;   vmatrix[10] = -fwdz;
		vmatrix[15] = 1.0;
	}

	glLoadMatrixd(vmatrix);

	glTranslated(-eyex, -eyey, -eyez);
	glScalef(1.0f, 1.0f, -1.0f);
	
	//Valathil: Sort lights by priority
	opengl_pre_render_init_lights();

	for (i = 0; i < GL_max_lights; i++) {
		if ( (offset + i) >= Num_active_gl_lights ) {
			break;
		}

		if (opengl_lights[offset+i].occupied) {
			opengl_set_light(i, &opengl_lights[offset+i]);

			GL_state.Light(i, GL_TRUE);
		}
	}

	// make sure that we turn off any lights that we aren't using right now
	for ( ; i < GL_max_lights; i++) {
		GL_state.Light(i, GL_FALSE);
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

	if (Num_active_gl_lights >= MAX_LIGHTS)
		return;

//if (fs_light->type == LT_POINT)
//	return;

	// init the light
	FSLight2GLLight(fs_light, &opengl_lights[Num_active_gl_lights]);
	opengl_lights[Num_active_gl_lights++].occupied = true;
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

	opengl_light glight;

	vec3d dir;
	vm_vec_sub(&dir, &Eye_position, &Object_position);
	vm_vec_normalize(&dir);

	if (type == 1) {
		glight.Diffuse[0] = 0.0f;
		glight.Diffuse[1] = 0.0f;
		glight.Diffuse[2] = 0.0f;
		glight.Ambient[0] = gr_screen.current_alpha;
		glight.Ambient[1] = gr_screen.current_alpha;
		glight.Ambient[2] = gr_screen.current_alpha;
	} else {
		glight.Diffuse[0] = gr_screen.current_alpha;
		glight.Diffuse[1] = gr_screen.current_alpha;
		glight.Diffuse[2] = gr_screen.current_alpha;
		glight.Ambient[0] = 0.0f;
		glight.Ambient[1] = 0.0f;
		glight.Ambient[2] = 0.0f;
	}

	glight.Specular[0] = 0.0f;
	glight.Specular[1] = 0.0f;
	glight.Specular[2] = 0.0f;
	glight.Specular[3] = 0.0f;

	glight.Ambient[3] = 1.0f;
	glight.Diffuse[3] = 1.0f;

	glight.Position[0] = -dir.xyz.x;
	glight.Position[1] = -dir.xyz.y;
	glight.Position[2] = -dir.xyz.z;
	glight.Position[3] = 0.0f;

	// defaults
	glight.ConstantAtten = 1.0f;
	glight.LinearAtten = 0.0f;
	glight.QuadraticAtten = 0.0f;

	// first light
	memcpy( &opengl_lights[Num_active_gl_lights], &glight, sizeof(opengl_light) );
	opengl_lights[Num_active_gl_lights++].occupied = true;

	// second light
	glight.Position[0] = dir.xyz.x;
	glight.Position[1] = dir.xyz.y;
	glight.Position[2] = dir.xyz.z;

	memcpy( &opengl_lights[Num_active_gl_lights], &glight, sizeof(opengl_light) );
	opengl_lights[Num_active_gl_lights++].occupied = true;

	// reset center alpha
	GL_center_alpha = 0;
}

void gr_opengl_reset_lighting()
{
	int i;

	if (Cmdline_nohtl)
		return;

//	memset( opengl_lights, 0, sizeof(opengl_light) * MAX_LIGHTS );

	for (i = 0; i < GL_max_lights; i++) {
		GL_state.Light(i, GL_FALSE);
	}

	Num_active_gl_lights = 0;
	GL_center_alpha = 0;
}

void opengl_calculate_ambient_factor()
{
	float amb_user = 0.0f;

	// assuming that the default is "128", just skip this if not a user setting
	if (Cmdline_ambient_factor == 128)
		return;

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

void opengl_light_shutdown()
{
	if (opengl_lights != NULL) {
		vm_free(opengl_lights);
		opengl_lights = NULL;
	}
}

void opengl_light_init()
{
	opengl_calculate_ambient_factor();

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	glMaterialf(GL_FRONT, GL_SHININESS, Cmdline_ogl_spec /*80.0f*/ );

	// more realistic lighting model
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	glGetIntegerv(GL_MAX_LIGHTS, &GL_max_lights); // Get the max number of lights supported

	// allocate memory for enabled lights
	Verify(GL_max_lights > 0);

	if ( opengl_lights == NULL )
		opengl_lights = (opengl_light *) vm_malloc_q(MAX_LIGHTS * sizeof(opengl_light));

	if (opengl_lights == NULL)
		Error( LOCATION, "Unable to allocate memory for lights!\n");

	memset( opengl_lights, 0, MAX_LIGHTS * sizeof(opengl_light) );
}

extern int Cmdline_no_emissive;
void opengl_default_light_settings(int ambient, int emission, int specular)
{
	if (!lighting_is_enabled)
		return;

	if (ambient) {
		glMaterialfv( GL_FRONT, GL_DIFFUSE, GL_light_color );
		glMaterialfv( GL_FRONT, GL_AMBIENT, GL_light_ambient );
	} else {
		if (GL_center_alpha) {
			glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, GL_light_true_zero );
		} else {
			glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, GL_light_zero );
		}
	}

	if (emission && !Cmdline_no_emissive) {
		// emissive light is just a general glow but without it things are *terribly* dark if there is no light on them
		glMaterialfv( GL_FRONT, GL_EMISSION, GL_light_emission );
	} else {
		glMaterialfv( GL_FRONT, GL_EMISSION, GL_light_zero );
	}

	if (specular) {
		glMaterialfv( GL_FRONT, GL_SPECULAR, GL_light_spec );
	} else {
		glMaterialfv( GL_FRONT, GL_SPECULAR, GL_light_zero );
	}
}

void gr_opengl_set_lighting(bool set, bool state)
{
	if (Cmdline_nohtl) {
		return;
	}

	lighting_is_enabled = set;

	opengl_default_light_settings();

	if ( (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) && !set ) {
		float amb[4] = { gr_screen.current_alpha, gr_screen.current_alpha, gr_screen.current_alpha, gr_screen.current_alpha };
		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb );
	} else {
		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, GL_light_ambient );
	}

	for (int i = 0; i < GL_max_lights; i++) {
		GL_state.Light(i, GL_FALSE);
	}

	GL_state.Lighting( (state) ? GL_TRUE : GL_FALSE );
}

void gr_opengl_set_ambient_light(int red, int green, int blue)
{
	GL_light_ambient[0] = i2fl(red)/255.0f;
	GL_light_ambient[1] = i2fl(green)/255.0f;
	GL_light_ambient[2] = i2fl(blue)/255.0f;
	GL_light_ambient[3] = 1.0f;

	opengl_calculate_ambient_factor();
}
