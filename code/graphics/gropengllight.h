/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLLight.h $
 * $Revision: 1.9.2.2 $
 * $Date: 2007-03-22 20:50:27 $
 * $Author: taylor $
 *
 * header file containing definitions for HT&L lighting in OpenGL
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.9.2.1  2006/12/07 18:10:16  taylor
 * restore default ambient light values to retail-like settings (I based the previous values on default D3D, which was rather dumb of me)
 * restore lighting falloff capability from retail and non-HTL modes (only used for asteroids as far as I know)
 * various cleanups and speedups for dealing with lights
 *
 * Revision 1.9  2006/04/12 01:10:35  taylor
 * some cleanup and slight reorg
 *  - remove special uv offsets for non-standard res, they were stupid anyway and don't actually fix the problem (which should actually be fixed now)
 *  - avoid some costly math where possible in the drawing functions
 *  - add opengl_error_string(), this is part of a later update but there wasn't a reason to not go ahead and commit this peice now
 *  - minor cleanup to Win32 extension defines
 *  - make opengl_lights[] allocate only when using OGL
 *  - cleanup some costly per-frame lighting stuff
 *  - clamp textures for interface and aabitmap (font) graphics since they shouldn't normally repeat anyway (the default)
 *    (doing this for D3D, if it doesn't already, may fix the blue-lines problem since a similar issue was seen with OGL)
 *
 * Revision 1.8  2006/01/30 06:40:49  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 * some basic lighting code cleanup
 *
 * Revision 1.7  2005/09/05 09:36:41  taylor
 * merge of OSX tree
 * fix OGL fullscreen switch for SDL since the old way only worked under Linux and not OSX or Windows
 * fix OGL version check, it would allow a required major version to be higher if the required minor version was lower than current
 *
 * Revision 1.6  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.5  2005/04/12 02:04:56  phreak
 * gr_set_ambient_light() function for the ambient light sliders in FRED
 *
 * Revision 1.4  2005/02/04 10:12:29  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 1.3  2005/01/03 18:45:22  taylor
 * dynamic allocation of num supported OpenGL lights
 * add config option for more realistic light settings
 * don't render spec maps in nebula to address lighting issue
 *
 * Revision 1.2  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
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

#ifndef _GROPENGLLIGHT_H
#define _GROPENGLLIGHT_H

#include "graphics/gropengl.h"


struct ogl_light_color {
	float r,g,b,a;
};

// Structures
struct opengl_light
{
	ogl_light_color Diffuse, Specular, Ambient;

	// light position
	struct {
		float x, y, z, w;
	} Position;

	// spotlight direction (for tube lights)
	struct {
		float x, y, z;
	} SpotDir;

	float SpotExp, SpotCutOff;
	float ConstantAtten, LinearAtten, QuadraticAtten;

	bool occupied;
	int priority;
	int type;

//	opengl_light() : occupied(false), priority(1), type(0) {};
};

struct light_data;

//Variables
extern bool lighting_is_enabled;
extern GLint GL_max_lights;
extern int Num_active_gl_lights;
extern int GL_center_alpha;

//Functions
int	gr_opengl_make_light(light *fs_light, int idx, int priority);		//unused -- stub function
void gr_opengl_modify_light(light *fs_light, int idx, int priority);	//unused -- stub function
void gr_opengl_destroy_light(int idx);									//unused -- stub function
void gr_opengl_set_light(light *fs_light);
void gr_opengl_reset_lighting();
void gr_opengl_set_lighting(bool set, bool state);
void gr_opengl_center_alpha(int type);
void gr_opengl_set_center_alpha(int type);
void gr_opengl_set_ambient_light(int red, int green, int blue);

void opengl_change_active_lights(int pos, int d_offset = 0);
void opengl_light_init();
void opengl_light_shutdown();
void opengl_default_light_settings(int amb = 1, int emi = 1, int spec = 1);

#endif //_GROPENGLLIGHT_H
