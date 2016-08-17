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

#include <algorithm>

#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include <globalincs/systemvars.h>
#include "graphics/2d.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltnl.h"
#include "lighting/lighting.h"
#include "render/3d.h"



// Variables
opengl_light *opengl_lights = NULL;
opengl_light_uniform_data opengl_light_uniforms;

bool lighting_is_enabled = true;
int Num_active_gl_lights = 0;
int GL_center_alpha = 0;
float GL_light_factor = 1.0f;

extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;
extern double specular_exponent_value;
extern float Cmdline_ogl_spec;


GLint GL_max_lights = 0;

// OGL defaults
const float GL_light_color[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
const float GL_light_spec[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float GL_light_zero[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float GL_light_emission[4] = { 0.09f, 0.09f, 0.09f, 1.0f };
const float GL_light_true_zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float GL_light_ambient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };

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

			if ( is_minimum_GLSL_version() ) {
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
		
		case LT_CONE:
			break;

		default:
			Int3();
			break;
	}
}

void opengl_set_light_fixed_pipeline(int light_num, opengl_light *ltp)
{
	Assert(light_num < GL_max_lights);

	GLfloat diffuse[4];
	memcpy(diffuse, ltp->Diffuse, sizeof(GLfloat) * 4);

	if ( (ltp->type == LT_DIRECTIONAL) && (GL_light_factor < 1.0f) ) {
		diffuse[0] *= GL_light_factor;
		diffuse[1] *= GL_light_factor;
		diffuse[2] *= GL_light_factor;
	}

	// transform lights into eyespace

	vec3d light_pos_world;
	vec3d light_pos_eye;
	vec4 light_pos_eye_4d;

	light_pos_world.xyz.x = ltp->Position[0];
	light_pos_world.xyz.y = ltp->Position[1];
	light_pos_world.xyz.z = ltp->Position[2];

	if ( ltp->type == LT_POINT ) {
		vm_vec_sub2(&light_pos_world, &Object_position);
	}

	vm_vec_rotate(&light_pos_eye, &light_pos_world, &Object_matrix);

	light_pos_eye_4d.a1d[0] = light_pos_eye.a1d[0];
	light_pos_eye_4d.a1d[1] = light_pos_eye.a1d[1];
	light_pos_eye_4d.a1d[2] = light_pos_eye.a1d[2];
	light_pos_eye_4d.a1d[3] = 1.0f;

	vec3d light_dir_world;
	vec3d light_dir_eye;

	light_dir_world.xyz.x = ltp->SpotDir[0];
	light_dir_world.xyz.y = ltp->SpotDir[1];
	light_dir_world.xyz.z = ltp->SpotDir[2];
	
	vm_vec_rotate(&light_dir_eye, &light_dir_world, &Object_matrix);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glLightfv(GL_LIGHT0 + light_num, GL_POSITION, light_pos_eye_4d.a1d);
	glLightfv(GL_LIGHT0 + light_num, GL_AMBIENT, ltp->Ambient);
	glLightfv(GL_LIGHT0 + light_num, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0 + light_num, GL_SPECULAR, ltp->Specular);
	glLightfv(GL_LIGHT0 + light_num, GL_SPOT_DIRECTION, light_dir_eye.a1d);
	//glLightf(GL_LIGHT0+light_num, GL_CONSTANT_ATTENUATION, ltp->ConstantAtten); Default is 1.0 and we only use 1.0 - Valathil
	glLightf(GL_LIGHT0 + light_num, GL_LINEAR_ATTENUATION, ltp->LinearAtten);
	//glLightf(GL_LIGHT0+light_num, GL_QUADRATIC_ATTENUATION, ltp->QuadraticAtten); Default is 0.0 and we only use 0.0 - Valathil
	glLightf(GL_LIGHT0 + light_num, GL_SPOT_EXPONENT, ltp->SpotExp);
	glLightf(GL_LIGHT0 + light_num, GL_SPOT_CUTOFF, ltp->SpotCutOff);

	glPopMatrix();
}

void opengl_set_light(int light_num, opengl_light *ltp)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_set_light_fixed_pipeline(light_num, ltp);
		return;
	}

	Assert(light_num < GL_max_lights);
		
	vec4 light_pos_world;
	light_pos_world.xyzw.x = ltp->Position[0];
	light_pos_world.xyzw.y = ltp->Position[1];
	light_pos_world.xyzw.z = ltp->Position[2];
	light_pos_world.xyzw.w = ltp->Position[3];

	vec3d light_dir_world;
	light_dir_world.xyz.x = ltp->SpotDir[0];
	light_dir_world.xyz.y = ltp->SpotDir[1];
	light_dir_world.xyz.z = ltp->SpotDir[2];

	vm_vec_transform(&opengl_light_uniforms.Position[light_num], &light_pos_world, &GL_view_matrix);
	vm_vec_transform(&opengl_light_uniforms.Direction[light_num], &light_dir_world, &GL_view_matrix, false);

	opengl_light_uniforms.Diffuse_color[light_num].xyz.x = ltp->Diffuse[0];
	opengl_light_uniforms.Diffuse_color[light_num].xyz.y = ltp->Diffuse[1];
	opengl_light_uniforms.Diffuse_color[light_num].xyz.z = ltp->Diffuse[2];

	opengl_light_uniforms.Spec_color[light_num].xyz.x = ltp->Specular[0];
	opengl_light_uniforms.Spec_color[light_num].xyz.y = ltp->Specular[1];
	opengl_light_uniforms.Spec_color[light_num].xyz.z = ltp->Specular[2];

	opengl_light_uniforms.Light_type[light_num] = ltp->type;

	opengl_light_uniforms.Attenuation[light_num] = ltp->LinearAtten;
}

bool opengl_sort_active_lights(const opengl_light &la, const opengl_light &lb)
{
	// directional lights always go first
	if ( (la.type != LT_DIRECTIONAL) && (lb.type == LT_DIRECTIONAL) )
		return false;
	else if ( (la.type == LT_DIRECTIONAL) && (lb.type != LT_DIRECTIONAL) )
		return true;

	// tube lights go next, they are generally large and intense
	if ( (la.type != LT_TUBE) && (lb.type == LT_TUBE) )
		return false;
	else if ( (la.type == LT_TUBE) && (lb.type != LT_TUBE) )
		return true;

	// everything else is sorted by linear atten (light size)
	// NOTE: smaller atten is larger light radius!
	if ( la.LinearAtten > lb.LinearAtten )
		return false;
	else if ( la.LinearAtten < lb.LinearAtten )
		return true;

	// as one extra check, if we're still here, go with overall brightness of light

	float la_value = la.Diffuse[0] + la.Diffuse[1] + la.Diffuse[2];
	float lb_value = lb.Diffuse[0] + lb.Diffuse[1] + lb.Diffuse[2];

	if ( la_value < lb_value )
		return false;
	else if ( la_value > lb_value )
		return true;

	// the two are equal
	return false;
}

void opengl_pre_render_init_lights()
{
	// sort the lights to try and get the most visible lights on the first pass
	std::sort(opengl_lights, opengl_lights + Num_active_gl_lights, opengl_sort_active_lights);
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
	extern bool Deferred_lighting;
	if(!Deferred_lighting)
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
	opengl_light zero;
	memset(&zero,0,sizeof(opengl_light));
	zero.Position[0] = 1.0f;

	// make sure that we turn off any lights that we aren't using right now
	for ( ; i < GL_max_lights; i++) {
		GL_state.Light(i, GL_FALSE);
		opengl_set_light(i, &zero);
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
	if (!type)
		return;

	if(Num_active_gl_lights == MAX_LIGHTS)
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
	glight.type = type;

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
	glight.SpotDir[0] = 0.0f;
	glight.SpotDir[1] = 0.0f;
	glight.SpotDir[2] = -1.0f;
	glight.SpotExp = Cmdline_ogl_spec * 0.5f;
	glight.SpotCutOff = 180.0f;
	glight.ConstantAtten = 1.0f;
	glight.LinearAtten = 0.0f;
	glight.QuadraticAtten = 0.0f;
	glight.occupied = false;

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

void gr_opengl_set_light_factor(float factor)
{
	GL_light_factor = factor;
}

void gr_opengl_reset_lighting()
{
	int i;

//	memset( opengl_lights, 0, sizeof(opengl_light) * MAX_LIGHTS );

	for (i = 0; i < GL_max_lights; i++) {
		if ( is_minimum_GLSL_version() ) {
			GL_state.Light(i, GL_FALSE);
		}

		opengl_lights[i].occupied = false;
	}

	Num_active_gl_lights = 0;
	GL_center_alpha = 0;
}

void opengl_calculate_ambient_factor()
{
	float amb_user = i2fl(Cmdline_ambient_factor) / 128.0f;

	// set the ambient light
	GL_light_ambient[0] *= amb_user;
	GL_light_ambient[1] *= amb_user;
	GL_light_ambient[2] *= amb_user;

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

	if ( opengl_light_uniforms.Position != NULL ) {
		vm_free(opengl_light_uniforms.Position);
		opengl_light_uniforms.Position = NULL;
	}

	if ( opengl_light_uniforms.Diffuse_color != NULL ) {
		vm_free(opengl_light_uniforms.Diffuse_color);
		opengl_light_uniforms.Diffuse_color = NULL;
	}

	if ( opengl_light_uniforms.Spec_color != NULL ) {
		vm_free(opengl_light_uniforms.Spec_color);
		opengl_light_uniforms.Spec_color = NULL;
	}

	if ( opengl_light_uniforms.Direction != NULL ) {
		vm_free(opengl_light_uniforms.Direction);
		opengl_light_uniforms.Direction = NULL;
	}

	if ( opengl_light_uniforms.Light_type != NULL ) {
		vm_free(opengl_light_uniforms.Light_type);
		opengl_light_uniforms.Light_type = NULL;
	}

	if ( opengl_light_uniforms.Attenuation != NULL ) {
		vm_free(opengl_light_uniforms.Attenuation);
		opengl_light_uniforms.Attenuation = NULL;
	}
}

void opengl_light_init()
{
	opengl_calculate_ambient_factor();

	if ( !is_minimum_GLSL_version() ) {
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

		glMaterialf(GL_FRONT, GL_SHININESS, Cmdline_ogl_spec /*80.0f*/);

		// more realistic lighting model
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

		glGetIntegerv(GL_MAX_LIGHTS, &GL_max_lights); // Get the max number of lights supported

		Verify(GL_max_lights > 0);
	} else {
		GL_max_lights = 8;
	}
	
	// allocate memory for enabled lights
	if ( opengl_lights == NULL )
		opengl_lights = (opengl_light *) vm_malloc(MAX_LIGHTS * sizeof(opengl_light), memory::quiet_alloc);

	if (opengl_lights == NULL)
		Error( LOCATION, "Unable to allocate memory for lights!\n");

	memset( opengl_lights, 0, MAX_LIGHTS * sizeof(opengl_light) );

	opengl_light_uniforms.Position = (vec4 *)vm_malloc(GL_max_lights * sizeof(vec4));
	opengl_light_uniforms.Diffuse_color = (vec3d *)vm_malloc(GL_max_lights * sizeof(vec3d));
	opengl_light_uniforms.Spec_color = (vec3d *)vm_malloc(GL_max_lights * sizeof(vec3d));
	opengl_light_uniforms.Direction = (vec3d *)vm_malloc(GL_max_lights * sizeof(vec3d));
	opengl_light_uniforms.Light_type = (int *)vm_malloc(GL_max_lights * sizeof(int));
	opengl_light_uniforms.Attenuation = (float *)vm_malloc(GL_max_lights * sizeof(float));
}

bool ambient_state = false;
bool emission_state = false;
bool specular_state = false;
void opengl_default_light_settings(int ambient, int emission, int specular)
{
	if (!lighting_is_enabled)
		return;

	if (ambient) {
		if (!ambient_state) {
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, GL_light_color);
			ambient_state = true;
		}
	} else {
		if (ambient_state) {
			if (GL_center_alpha) {
				glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, GL_light_true_zero );
			} else {
				glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, GL_light_zero );
			}
			ambient_state = false;
		}
	}

	if (emission && !Cmdline_no_emissive) {
		// emissive light is just a general glow but without it things are *terribly* dark if there is no light on them
		if (!emission_state) {
			glMaterialfv( GL_FRONT, GL_EMISSION, GL_light_emission );
			emission_state = true;
		}
	} else {
		if (emission_state) {
			glMaterialfv( GL_FRONT, GL_EMISSION, GL_light_zero );
			emission_state = false;
		}
	}

	if (specular) {
		if (!specular_state) {
			glMaterialfv( GL_FRONT, GL_SPECULAR, GL_light_spec );
			specular_state = true;
		}
	} else {
		if (specular_state) {
			glMaterialfv( GL_FRONT, GL_SPECULAR, GL_light_zero );
			specular_state = false;
		}
	}
}

void opengl_set_lighting_fixed_pipeline(bool set, bool state)
{
	lighting_is_enabled = set;

	opengl_default_light_settings();

	if ( (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) && !set ) {
		float amb[4] = { gr_screen.current_alpha, gr_screen.current_alpha, gr_screen.current_alpha, gr_screen.current_alpha };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
	} else {
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GL_light_ambient);
	}

	GL_state.Lighting((state) ? GL_TRUE : GL_FALSE);

	if ( !state ) {
		for ( int i = 0; i < GL_max_lights; i++ ) {
			GL_state.Light(i, GL_FALSE);
		}

		return;
	}

	//Valathil: Sort lights by priority
	extern bool Deferred_lighting;
	if ( !Deferred_lighting )
		opengl_pre_render_init_lights();

	int i = 0;

	for ( i = 0; i < GL_max_lights; i++ ) {
		if ( i >= Num_active_gl_lights ) {
			break;
		}

		if ( opengl_lights[i].occupied ) {
			opengl_set_light(i, &opengl_lights[i]);

			GL_state.Light(i, GL_TRUE);
		}
	}

	opengl_light zero;
	memset(&zero, 0, sizeof(opengl_light));
	zero.Position[0] = 1.0f;

	// make sure that we turn off any lights that we aren't using right now
	for ( ; i < GL_max_lights; i++ ) {
		GL_state.Light(i, GL_FALSE);
		opengl_set_light(i, &zero);
	}
}

void gr_opengl_set_lighting(bool set, bool state)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_set_lighting_fixed_pipeline(set, state);
		return;
	}

	lighting_is_enabled = set;

	if ( !state ) {
		return;
	}

	//Valathil: Sort lights by priority
	extern bool Deferred_lighting;
	if ( !Deferred_lighting )
		opengl_pre_render_init_lights();
	
	int i = 0;

	for ( i = 0; i < GL_max_lights; i++ ) {
		if ( i >= Num_active_gl_lights ) {
			break;
		}

		if ( opengl_lights[i].occupied ) {
			opengl_set_light(i, &opengl_lights[i]);
		}
	}

	opengl_light zero;
	memset(&zero, 0, sizeof(opengl_light));
	zero.Position[0] = 1.0f;

	// make sure that we turn off any lights that we aren't using right now
	for ( ; i < GL_max_lights; i++ ) {
		opengl_set_light(i, &zero);
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
