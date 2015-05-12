/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglshader.h"
#include "math/vecmat.h"

extern GLfloat GL_max_anisotropy;


opengl_state GL_state;


opengl_texture_state::~opengl_texture_state()
{
	if (units != NULL) {
		vm_free(units);
	}
}

void opengl_texture_state::init(GLuint n_units)
{
	Assert( n_units > 0 );
	units = (opengl_texture_unit*) vm_malloc(n_units * sizeof(opengl_texture_unit));
	num_texture_units = n_units;

	for (unsigned int unit = 0; unit < num_texture_units; unit++) {
		units[unit].active = GL_FALSE;
		units[unit].enabled = GL_FALSE;
		units[unit].used = GL_FALSE;

		default_values(unit);

		vglActiveTextureARB(GL_TEXTURE0 + unit);
		if (unit < (GLuint)GL_supported_texture_units) {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glDisable(GL_TEXTURE_GEN_Q);
		}

		units[unit].texgen_S = GL_FALSE;
		units[unit].texgen_T = GL_FALSE;
		units[unit].texgen_R = GL_FALSE;
		units[unit].texgen_Q = GL_FALSE;

		if (unit < (GLuint)GL_supported_texture_units) {
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		}

		units[unit].texgen_mode_S = GL_EYE_LINEAR;
		units[unit].texgen_mode_T = GL_EYE_LINEAR;
		units[unit].texgen_mode_R = GL_EYE_LINEAR;
		units[unit].texgen_mode_Q = GL_EYE_LINEAR;

		if (unit < (GLuint)GL_supported_texture_units) {
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		}

		units[unit].env_mode = GL_MODULATE;
		units[unit].env_combine_rgb = GL_MODULATE;
		units[unit].env_combine_alpha = GL_MODULATE;

		if (unit < (GLuint)GL_supported_texture_units) {
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.0f);
			glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1.0f);
		}

		units[unit].rgb_scale = 1.0f;
		units[unit].alpha_scale = 1.0f;
	}

	DisableAll();
}

void opengl_texture_state::default_values(GLint unit, GLenum target)
{
	vglActiveTextureARB(GL_TEXTURE0 + unit);

	if (target == GL_INVALID_ENUM) {
		if (unit < GL_supported_texture_units) {
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		units[unit].texture_target = GL_TEXTURE_2D;
		units[unit].texture_id = 0;
	}
}

GLboolean opengl_texture_state::TexgenS(GLint state)
{
	GLboolean save_state = units[active_texture_unit].texgen_S;

	if ( !((state == -1) || (state == units[active_texture_unit].texgen_S)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_TEXTURE_GEN_S);
			units[active_texture_unit].texgen_S = GL_TRUE;
		} else {
			glDisable(GL_TEXTURE_GEN_S);
			units[active_texture_unit].texgen_S = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_texture_state::TexgenT(GLint state)
{
	GLboolean save_state = units[active_texture_unit].texgen_T;

	if ( !((state == -1) || (state == units[active_texture_unit].texgen_T)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_TEXTURE_GEN_T);
			units[active_texture_unit].texgen_T = GL_TRUE;
		} else {
			glDisable(GL_TEXTURE_GEN_T);
			units[active_texture_unit].texgen_T = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_texture_state::TexgenR(GLint state)
{
	GLboolean save_state = units[active_texture_unit].texgen_R;

	if ( !((state == -1) || (state == units[active_texture_unit].texgen_R)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_TEXTURE_GEN_R);
			units[active_texture_unit].texgen_R = GL_TRUE;
		} else {
			glDisable(GL_TEXTURE_GEN_R);
			units[active_texture_unit].texgen_R = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_texture_state::TexgenQ(GLint state)
{
	GLboolean save_state = units[active_texture_unit].texgen_Q;

	if ( !((state == -1) || (state == units[active_texture_unit].texgen_Q)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_TEXTURE_GEN_Q);
			units[active_texture_unit].texgen_Q = GL_TRUE;
		} else {
			glDisable(GL_TEXTURE_GEN_Q);
			units[active_texture_unit].texgen_Q = GL_FALSE;
		}
	}

	return save_state;
}

void opengl_texture_state::SetTarget(GLenum tex_target)
{
	if (units[active_texture_unit].texture_target != tex_target) {
		Disable();

		if (units[active_texture_unit].texture_id) {
			glBindTexture(units[active_texture_unit].texture_target, 0);
			units[active_texture_unit].texture_id = 0;
		}

		// reset modes, since those were only valid for the previous texture target
		default_values(active_texture_unit, tex_target);
		units[active_texture_unit].texture_target = tex_target;
	}
}

void opengl_texture_state::SetActiveUnit(GLuint id)
{
	if (id >= num_texture_units) {
		Int3();
		id = 0;
	}

	vglActiveTextureARB(GL_TEXTURE0 + id);

	active_texture_unit = id;
}

void opengl_texture_state::Enable(GLuint tex_id)
{
	units[active_texture_unit].used = GL_TRUE;

	if ( units[active_texture_unit].active && (units[active_texture_unit].texture_id == tex_id) ) {
		return;
	}

	if ( !shader_mode && (active_texture_unit < (uint)GL_supported_texture_units) ) {
		if( units[active_texture_unit].texture_target != GL_TEXTURE_2D_ARRAY_EXT)
			glEnable( units[active_texture_unit].texture_target );
		units[active_texture_unit].enabled = GL_TRUE;
	}

	if (units[active_texture_unit].texture_id != tex_id) {
		glBindTexture(units[active_texture_unit].texture_target, tex_id);
		units[active_texture_unit].texture_id = tex_id;
	}

	units[active_texture_unit].active = GL_TRUE;
}

void opengl_texture_state::Disable()
{
	if ( !units[active_texture_unit].active ) {
		return;
	}

	if ( units[active_texture_unit].enabled ) {
		if( units[active_texture_unit].texture_target != GL_TEXTURE_2D_ARRAY_EXT)
			glDisable( units[active_texture_unit].texture_target );
		units[active_texture_unit].enabled = GL_FALSE;
	}

	units[active_texture_unit].active = GL_FALSE;
	units[active_texture_unit].used = GL_FALSE;
}

void opengl_texture_state::ResetUsed()
{
	for (unsigned int i = 0; i < num_texture_units; i++) {
		units[i].used = GL_FALSE;
	}
}

void opengl_texture_state::DisableUnused()
{
	for (unsigned int i = 0; i < num_texture_units; i++) {
		if (!units[i].used) {
			SetActiveUnit(i);
			Disable();
		}
	}
}

void opengl_texture_state::DisableAll()
{
	for (unsigned int i = 0; i < num_texture_units; i++) {
		if (units[i].active) {
			SetActiveUnit(i);
			Disable();
		}
	}

	SetActiveUnit();

	Current_texture_source = TEXTURE_SOURCE_NONE;
}

void opengl_texture_state::Delete(GLuint tex_id)
{
	if (tex_id == 0) {
		Int3();
		return;
	}

	GLuint atu_save = active_texture_unit;

	for (unsigned int i = 0; i < num_texture_units; i++) {
		if (units[i].texture_id == tex_id) {
			SetActiveUnit(i);
			Disable();

			glBindTexture(units[i].texture_target, 0);
			units[i].texture_id = 0;

			default_values(i, units[i].texture_target);

			if (i == atu_save) {
				atu_save = 0;
			}
		}
	}

	SetActiveUnit(atu_save);
}

GLfloat opengl_texture_state::AnisoFilter(GLfloat aniso)
{
#if 0
	GLfloat rval = units[active_texture_unit].aniso_filter;

	if ( (aniso > 0.0f) /*&& (aniso != rval)*/ ) {
		if ( Is_Extension_Enabled(OGL_EXT_TEXTURE_FILTER_ANISOTROPIC) ) {
			units[active_texture_unit].aniso_filter = aniso;
			CLAMP(units[active_texture_unit].aniso_filter, 1.0f, GL_max_anisotropy);
	
			glTexParameterf(units[active_texture_unit].texture_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, units[active_texture_unit].aniso_filter);
		}

		if ( Is_Extension_Enabled(OGL_EXT_TEXTURE_LOD_BIAS) ) {
			if (units[active_texture_unit].aniso_filter > 1.0f) {
				glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0.0f);
			} else {
				glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -0.75f);
			}
		}
	}

	return rval;
#endif

	return GL_anisotropy;
}

opengl_state::~opengl_state()
{
	if (light_Status != NULL) {
		vm_free(light_Status);
	}
}

void opengl_state::init()
{
	int i;

	glDisable(GL_FOG);
	fog_Status = GL_FALSE;

	glDisable(GL_BLEND);
	blend_Status = GL_FALSE;

	glDisable(GL_ALPHA_TEST);
	alphatest_Status = GL_FALSE;

	glDisable(GL_DEPTH_TEST);
	depthtest_Status = GL_FALSE;

	glDisable(GL_SCISSOR_TEST);
	scissortest_Status = GL_FALSE;

	glDisable(GL_CULL_FACE);
	cullface_Status = GL_FALSE;

	glDisable(GL_POLYGON_OFFSET_FILL);
	polygonoffsetfill_Status = GL_FALSE;

	polygon_offset_Factor = 0.0f;
	polygon_offset_Unit = 0.0f;

	glDisable(GL_NORMALIZE);
	normalize_Status = GL_FALSE;

	for (i = 0; i < (int)(sizeof(clipplane_Status) / sizeof(GLboolean)); i++) {
		//glDisable(GL_CLIP_PLANE0+i);
		clipplane_Status[i] = GL_FALSE;
	}

	if (GL_version >= 30) {
		for (i = 0; i < (int)(sizeof(clipdistance_Status) / sizeof(GLboolean)); i++) {
			//glDisable(GL_CLIP_DISTANCE0+i);
			clipdistance_Status[i] = GL_FALSE;
		}
	}

	Assert( GL_max_lights > 0 );
	light_Status = (GLboolean*) vm_malloc(GL_max_lights * sizeof(GLboolean));

	for (i = 0; i < GL_max_lights; i++) {
		glDisable(GL_LIGHT0+i);
		light_Status[i] = GL_FALSE;
	}

	glDepthMask(GL_FALSE);
	depthmask_Status = GL_FALSE;

	glDisable(GL_LIGHTING);
	lighting_Status = GL_FALSE;

	glFrontFace(GL_CCW);
	frontface_Value = GL_CCW;

	glCullFace(GL_BACK);
	cullface_Value = GL_BACK;

	glBlendFunc(GL_ONE, GL_ZERO);
	blendfunc_Value[0] = GL_ONE;
	blendfunc_Value[1] = GL_ZERO;

	glDepthFunc(GL_LESS);
	depthfunc_Value = GL_LESS;

	Current_alpha_blend_mode = ALPHA_BLEND_NONE;
	Current_zbuffer_type = ZBUFFER_TYPE_READ;

	red_Status = 255;
	blue_Status = 255;
	green_Status = 255;
	alpha_Status = 255;
	color_invalid = true;
}

GLboolean opengl_state::Lighting(GLint state)
{
	GLboolean save_state = lighting_Status;

	if ( !((state == -1) || (state == lighting_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_LIGHTING);
			lighting_Status = GL_TRUE;
		} else {
			glDisable(GL_LIGHTING);
			lighting_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::Fog(GLint state)
{
	GLboolean save_state = fog_Status;

	if ( !((state == -1) || (state == fog_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_FOG);
			fog_Status = GL_TRUE;
		} else {
			glDisable(GL_FOG);
			fog_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::Blend(GLint state)
{
	GLboolean save_state = blend_Status;

	if ( !((state == -1) || (state == blend_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_BLEND);
			blend_Status = GL_TRUE;
		} else {
			glDisable(GL_BLEND);
			blend_Status = GL_FALSE;
		}

		Current_alpha_blend_mode = (gr_alpha_blend)(-1);
	}

	return save_state;
}

GLboolean opengl_state::AlphaTest(GLint state)
{
	GLboolean save_state = alphatest_Status;

	if ( !((state == -1) || (state == alphatest_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_ALPHA_TEST);
			alphatest_Status = GL_TRUE;
		} else {
			glDisable(GL_ALPHA_TEST);
			alphatest_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::DepthTest(GLint state)
{
	GLboolean save_state = depthtest_Status;

	if ( !((state == -1) || (state == depthtest_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_DEPTH_TEST);
			depthtest_Status = GL_TRUE;
		} else {
			glDisable(GL_DEPTH_TEST);
			depthtest_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::ScissorTest(GLint state)
{
	GLboolean save_state = scissortest_Status;

	if ( !((state == -1) || (state == scissortest_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_SCISSOR_TEST);
			scissortest_Status = GL_TRUE;
		} else {
			glDisable(GL_SCISSOR_TEST);
			scissortest_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::StencilTest(GLint state)
{
    GLboolean save_state = stenciltest_Status;

    if ( !((state == -1) || (state == stenciltest_Status)) ) {
        if (state) {
            Assert( state == GL_TRUE );
            glEnable(GL_STENCIL_TEST);
            stenciltest_Status = GL_TRUE;
        } else {
            glDisable(GL_STENCIL_TEST);
            stenciltest_Status = GL_FALSE;
        }
    }

    return save_state;
}

GLboolean opengl_state::CullFace(GLint state)
{
	GLboolean save_state = cullface_Status;

	if ( !((state == -1) || (state == cullface_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_CULL_FACE);
			cullface_Status = GL_TRUE;
		} else {
			glDisable(GL_CULL_FACE);
			cullface_Status = GL_FALSE;
		}
	}

	return save_state;
}

void opengl_state::SetPolygonMode(GLenum face, GLenum mode)
{
	if ( polygon_mode_Face != face || polygon_mode_Mode != mode ) {
		glPolygonMode(face, mode);

		polygon_mode_Face = face;
		polygon_mode_Mode = mode;
	}
}

void opengl_state::SetPolygonOffset(GLfloat factor, GLfloat units)
{
	if ( polygon_offset_Factor != factor || polygon_offset_Unit != units) {
		glPolygonOffset(factor, units);

		polygon_offset_Factor = factor;
		polygon_offset_Unit = units;
	}
}

GLboolean opengl_state::PolygonOffsetFill(GLint state)
{
	GLboolean save_state = polygonoffsetfill_Status;

	if ( !((state == -1) || (state == polygonoffsetfill_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_POLYGON_OFFSET_FILL);
			polygonoffsetfill_Status = GL_TRUE;
		} else {
			glDisable(GL_POLYGON_OFFSET_FILL);
			polygonoffsetfill_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::Normalize(GLint state)
{
	GLboolean save_state = normalize_Status;

	if ( !((state == -1) || (state == normalize_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_NORMALIZE);
			normalize_Status = GL_TRUE;
		} else {
			glDisable(GL_NORMALIZE);
			normalize_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::Light(GLint num, GLint state)
{
	Assert( (light_Status != NULL) && (num >= 0) && (num < GL_max_lights) );

	GLboolean save_state = light_Status[num];

	if ( !((state == -1) || (state == light_Status[num])) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_LIGHT0+num);
			light_Status[num] = GL_TRUE;
		} else {
			glDisable(GL_LIGHT0+num);
			light_Status[num] = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::ClipPlane(GLint num, GLint state)
{
	Assert( (num >= 0) || (num < (int)(sizeof(clipplane_Status) / sizeof(GLboolean))) );

	GLboolean save_state = clipplane_Status[num];

	if ( !((state == -1) || (state == clipplane_Status[num])) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_CLIP_PLANE0+num);
			clipplane_Status[num] = GL_TRUE;
		} else {
			glDisable(GL_CLIP_PLANE0+num);
			clipplane_Status[num] = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::ClipDistance(GLint num, GLint state)
{
	Assert( (num >= 0) || (num < (int)(sizeof(clipdistance_Status) / sizeof(GLboolean))) || GL_version >= 30 );

	GLboolean save_state = clipdistance_Status[num];

	if ( !((state == -1) || (state == clipdistance_Status[num])) ) {
		if (state) {
			Assert( state == GL_TRUE );
			//glEnable(GL_CLIP_DISTANCE0+num);
			clipdistance_Status[num] = GL_TRUE;
		} else {
			//glDisable(GL_CLIP_DISTANCE0+num);
			clipdistance_Status[num] = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::DepthMask(GLint state)
{
	GLboolean save_state = depthmask_Status;

	if ( !((state == -1) || (state == depthmask_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glDepthMask(GL_TRUE);
			depthmask_Status = GL_TRUE;
		} else {
			glDepthMask(GL_FALSE);
			depthmask_Status = GL_FALSE;
		}

		Current_zbuffer_type = (gr_zbuffer_type)(-1);
	}

	return save_state;
}

GLboolean opengl_state::ColorMask(GLint state)
{
    GLboolean save_state = colormask_Status;

    if ( !((state == -1) || (state == colormask_Status)) ) {
        if (state) {
            Assert( state == GL_TRUE );
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            colormask_Status = GL_TRUE;
        } else {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            colormask_Status = GL_FALSE;
        }
    }

    return save_state;
}

void opengl_state::SetTextureSource(gr_texture_source ts)
{
	if (ts == Texture.Current_texture_source) {
		return;
	}

	switch (ts) {
		case TEXTURE_SOURCE_DECAL:
			GL_state.Texture.SetActiveUnit();
			GL_state.Texture.SetEnvMode(GL_MODULATE);
			break;

		case TEXTURE_SOURCE_NO_FILTERING:
			GL_state.Texture.SetActiveUnit();
			GL_state.Texture.SetEnvMode(GL_MODULATE);
			break;

		case TEXTURE_SOURCE_NONE:
			GL_state.Texture.DisableAll();
			break;

		default:
			Int3();
			break;
	}

	Texture.Current_texture_source = ts;
}

void opengl_state::SetAlphaBlendMode(gr_alpha_blend ab)
{
	if (ab == Current_alpha_blend_mode) {
		return;
	}

	switch (ab) {
		case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
			GL_state.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case ALPHA_BLEND_NONE:
			GL_state.BlendFunc(GL_ONE, GL_ZERO);
			break;

		case ALPHA_BLEND_ADDITIVE:
			GL_state.BlendFunc(GL_ONE, GL_ONE);
			break;

		case ALPHA_BLEND_ALPHA_ADDITIVE:
			GL_state.BlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;

		case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
			GL_state.BlendFunc(/*GL_SRC_COLOR*/GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
			break;

		case ALPHA_BLEND_PREMULTIPLIED:
			GL_state.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;

		default:
			break;
	}

	GL_state.Blend( (ab == ALPHA_BLEND_NONE) ? GL_FALSE : GL_TRUE );

	Current_alpha_blend_mode = ab;
}

void opengl_state::SetZbufferType(gr_zbuffer_type zt)
{
	if (zt == Current_zbuffer_type) {
		return;
	}

	switch (zt) {
		case ZBUFFER_TYPE_NONE:
			GL_state.DepthFunc(GL_ALWAYS);
			GL_state.DepthMask(GL_FALSE);
			break;

		case ZBUFFER_TYPE_READ:
			GL_state.DepthFunc(GL_LESS);
			GL_state.DepthMask(GL_FALSE);
			break;

		case ZBUFFER_TYPE_WRITE:
			GL_state.DepthFunc(GL_ALWAYS);
			GL_state.DepthMask(GL_TRUE);
			break;

		case ZBUFFER_TYPE_FULL:
			GL_state.DepthFunc(GL_LESS);
			GL_state.DepthMask(GL_TRUE);
			break;

		default:
			break;
	}

	GL_state.DepthTest( (zt == ZBUFFER_TYPE_NONE) ? GL_FALSE : GL_TRUE );

	Current_zbuffer_type = zt;
}

void opengl_state::SetStencilType(gr_stencil_type st)
{
    if (st == Current_stencil_type) {
        return;
    }
    
    switch (st) {
        case STENCIL_TYPE_NONE:
            glStencilFunc( GL_NEVER, 1, 0xFFFF );
            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
            break;
            
        case STENCIL_TYPE_READ:
            glStencilFunc( GL_NOTEQUAL, 1, 0XFFFF );
            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
            break;
            
        case STENCIL_TYPE_WRITE:
            glStencilFunc( GL_ALWAYS, 1, 0xFFFF );
            glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
            break;
                     
        default:
            break;
    }
           
    GL_state.StencilTest( (st == STENCIL_TYPE_NONE) ? GL_FALSE : GL_TRUE );
         
    Current_stencil_type = st;
}

opengl_array_state::~opengl_array_state()
{
	if ( client_texture_units != NULL ) {
		vm_free(client_texture_units);
	}
}

void opengl_array_state::init(GLuint n_units)
{
	Assert( n_units > 0 );
	client_texture_units = (opengl_client_texture_unit*) vm_malloc(n_units * sizeof(opengl_client_texture_unit));
	num_client_texture_units = n_units;
	active_client_texture_unit = 0;

	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		client_texture_units[i].pointer = 0;
		client_texture_units[i].size = 4;
		client_texture_units[i].status = GL_FALSE;
		client_texture_units[i].stride = 0;
		client_texture_units[i].type = GL_FLOAT;
		client_texture_units[i].buffer = 0;
		client_texture_units[i].reset_ptr = false;
		client_texture_units[i].used_for_draw = false;
	}

	color_array_Buffer = 0;
	color_array_Status = GL_FALSE;
	color_array_size = 4;
	color_array_type = GL_FLOAT;
	color_array_stride = 0;
	color_array_pointer = 0;
	color_array_reset_ptr = false;
	color_array_used_for_draw = false;

	normal_array_Buffer = 0;
	normal_array_Status = GL_FALSE;
	normal_array_Type = GL_FLOAT;
	normal_array_Stride = 0;
	normal_array_Pointer = 0;
	normal_array_reset_ptr = false;
	normal_array_used_for_draw = false;

	vertex_array_Buffer = 0;
	vertex_array_Status = GL_FALSE;
	vertex_array_Size = 4;
	vertex_array_Type = GL_FLOAT;
	vertex_array_Stride = 0;
	vertex_array_Pointer = 0;
	vertex_array_reset_ptr = false;
	vertex_array_used_for_draw = false;

	array_buffer = 0;
	element_array_buffer = 0;
	texture_array_buffer = 0;
	uniform_buffer = 0;
}

void opengl_array_state::SetActiveClientUnit(GLuint id)
{
	if ( id >= num_client_texture_units ) {
		Int3();
		id = 0;
	}

	if ( active_client_texture_unit == id ) {
		return;
	}

	vglClientActiveTextureARB(GL_TEXTURE0_ARB + id);

	active_client_texture_unit = id;
}

void opengl_array_state::EnableClientTexture()
{
	client_texture_units[active_client_texture_unit].used_for_draw = true;

	if ( client_texture_units[active_client_texture_unit].status == GL_TRUE ) {
		return;
	}

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	client_texture_units[active_client_texture_unit].status = GL_TRUE;
}

void opengl_array_state::DisableClientTexture()
{
	if ( client_texture_units[active_client_texture_unit].status == GL_FALSE ) {
		return;
	}

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	client_texture_units[active_client_texture_unit].status = GL_FALSE;
}

void opengl_array_state::TexPointer(GLint size, GLenum type, GLsizei stride, GLvoid *pointer)
{
	opengl_client_texture_unit *ct_unit = &client_texture_units[active_client_texture_unit];

	if ( 
		!ct_unit->reset_ptr 
		&& ct_unit->pointer == pointer 
		&& ct_unit->size == size 
		&& ct_unit->type == type 
		&& ct_unit->stride == stride 
		&& ct_unit->buffer == array_buffer 
	) {
		return;
	}

	glTexCoordPointer(size, type, stride, pointer);

	ct_unit->size = size;
	ct_unit->type = type;
	ct_unit->stride = stride;
	ct_unit->pointer = pointer;
	ct_unit->buffer = array_buffer;
	ct_unit->reset_ptr = false;
}

void opengl_array_state::EnableClientColor()
{
	color_array_used_for_draw = true;

	if ( color_array_Status == GL_TRUE ) {
		return;
	}

	glEnableClientState(GL_COLOR_ARRAY);

	color_array_Status = GL_TRUE;
}

void opengl_array_state::DisableClientColor()
{
	if ( color_array_Status == GL_FALSE ) {
		return;
	}

	glDisableClientState(GL_COLOR_ARRAY);

	color_array_Status = GL_FALSE;
}

void opengl_array_state::ColorPointer(GLint size, GLenum type, GLsizei stride, GLvoid *pointer)
{
	if ( 
		!color_array_reset_ptr 
		&& color_array_size == size 
		&& color_array_type == type 
		&& color_array_stride == stride 
		&& color_array_pointer == pointer 
		&& color_array_Buffer == array_buffer 
	) {
		return;
	}

	glColorPointer(size, type, stride, pointer);

	color_array_size = size;
	color_array_type = type;
	color_array_stride = stride;
	color_array_pointer = pointer;
	color_array_Buffer = array_buffer;
	color_array_reset_ptr = false;
}

void opengl_array_state::EnableClientNormal()
{
	normal_array_used_for_draw = true;

	if ( normal_array_Status == GL_TRUE ) {
		return;
	}

	glEnableClientState(GL_NORMAL_ARRAY);

	normal_array_Status = GL_TRUE;
}

void opengl_array_state::DisableClientNormal()
{
	if ( normal_array_Status == GL_FALSE ) {
		return;
	}

	glDisableClientState(GL_NORMAL_ARRAY);

	normal_array_Status = GL_FALSE;
}

void opengl_array_state::NormalPointer(GLenum type, GLsizei stride, GLvoid *pointer)
{
	if ( 
		!normal_array_reset_ptr 
		&& normal_array_Type == type 
		&& normal_array_Stride == stride 
		&& normal_array_Pointer == pointer 
		&& normal_array_Buffer == array_buffer 
	) {
		return;
	}

	glNormalPointer(type, stride, pointer);

	normal_array_Type = type;
	normal_array_Stride = stride;
	normal_array_Pointer = pointer;
	normal_array_Buffer = array_buffer;
	normal_array_reset_ptr = false;
}

void opengl_array_state::EnableClientVertex()
{
	vertex_array_used_for_draw = true;

	if ( vertex_array_Status == GL_TRUE ) {
		return;
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	vertex_array_Status = GL_TRUE;
}

void opengl_array_state::DisableClientVertex()
{
	if ( vertex_array_Status == GL_FALSE ) {
		return;
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	vertex_array_Status = GL_FALSE;
}

void opengl_array_state::VertexPointer(GLint size, GLenum type, GLsizei stride, GLvoid *pointer)
{
	if (
		!vertex_array_reset_ptr 
		&& vertex_array_Size == size 
		&& vertex_array_Type == type 
		&& vertex_array_Stride == stride 
		&& vertex_array_Pointer == pointer 
		&& vertex_array_Buffer == array_buffer 
	) {
		return;
	}

	glVertexPointer(size, type, stride, pointer);

	vertex_array_Size = size;
	vertex_array_Type = type;
	vertex_array_Stride = stride;
	vertex_array_Pointer = pointer;
	vertex_array_Buffer = array_buffer;
	vertex_array_reset_ptr = false;
}

void opengl_array_state::EnableVertexAttrib(GLuint index)
{
	opengl_vertex_attrib_unit *va_unit = &vertex_attrib_units[index];

	va_unit->used_for_draw = true;

	if ( va_unit->status_init && va_unit->status == GL_TRUE ) {
		return;
	}

	vglEnableVertexAttribArrayARB(index);
	va_unit->status = GL_TRUE;
	va_unit->status_init = true;
}

void opengl_array_state::DisableVertexAttrib(GLuint index)
{
	opengl_vertex_attrib_unit *va_unit = &vertex_attrib_units[index];

	if ( va_unit->status_init && va_unit->status == GL_FALSE ) {
		return;
	}

	vglDisableVertexAttribArrayARB(index);
	va_unit->status = GL_FALSE;
	va_unit->status_init = true;
}

void opengl_array_state::VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer)
{
	opengl_vertex_attrib_unit *va_unit = &vertex_attrib_units[index];

	if ( 
		!va_unit->reset_ptr 
		&& va_unit->ptr_init 
		&& va_unit->normalized == normalized 
		&& va_unit->pointer == pointer 
		&& va_unit->size == size 
		&& va_unit->stride == stride 
		&& va_unit->type == type 
		&& va_unit->buffer == array_buffer
	) {
		return;
	}

	vglVertexAttribPointerARB(index, size, type, normalized, stride, pointer);

	va_unit->normalized = normalized;
	va_unit->pointer = pointer;
	va_unit->size = size;
	va_unit->stride = stride;
	va_unit->type = type;
	va_unit->buffer = array_buffer;
	va_unit->reset_ptr = false;

	va_unit->ptr_init = true;
}

void opengl_array_state::BindPointersBegin()
{
	// set all available client states to not used
	vertex_array_used_for_draw = false;
	color_array_used_for_draw = false;
	normal_array_used_for_draw = false;

	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		client_texture_units[i].used_for_draw = false;
	}

	SCP_map<GLuint, opengl_vertex_attrib_unit>::iterator it;

	for (it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it) {
		it->second.used_for_draw = false;
	}
}

void opengl_array_state::BindPointersEnd()
{
	// any client states not used, disable them
	if ( !vertex_array_used_for_draw ) DisableClientVertex();
	if ( !color_array_used_for_draw ) DisableClientColor();
	if ( !normal_array_used_for_draw ) DisableClientNormal();

	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		if ( !client_texture_units[i].used_for_draw ) {
			SetActiveClientUnit(i);
			DisableClientTexture();
		}
	}

	SCP_map<GLuint, opengl_vertex_attrib_unit>::iterator it;

	for (it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it) {
		if ( !it->second.used_for_draw ) DisableVertexAttrib(it->first);
	}
}

void opengl_array_state::BindArrayBuffer(GLuint id)
{
	if ( array_buffer == id ) {
		return;
	}

	vglBindBufferARB(GL_ARRAY_BUFFER_ARB, id);

	array_buffer = id;

	vertex_array_reset_ptr = true;
	color_array_reset_ptr = true;
	normal_array_reset_ptr = true;

	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		client_texture_units[i].reset_ptr = true;
	}

	SCP_map<GLuint,opengl_vertex_attrib_unit>::iterator it;

	for ( it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it ) {
		it->second.reset_ptr = true;
	}
}

void opengl_array_state::BindElementBuffer(GLuint id)
{
	if ( element_array_buffer == id ) {
		return;
	}

	vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, id);

	element_array_buffer = id;
}

void opengl_array_state::BindTextureBuffer(GLuint id)
{
	if ( !Is_Extension_Enabled(OGL_ARB_TEXTURE_BUFFER) ) {
		return;
	}

	if ( texture_array_buffer == id ) {
		return;
	}

	vglBindBufferARB(GL_TEXTURE_BUFFER_ARB, id);

	texture_array_buffer = id;
}

void opengl_array_state::BindUniformBufferBindingIndex(GLuint id, GLuint index)
{
	if ( !Is_Extension_Enabled(OGL_ARB_UNIFORM_BUFFER_OBJECT) ) {
		return;
	}

	if ( uniform_buffer_index_bindings[index] == id ) {
		return;
	}

	vglBindBufferBaseEXT(GL_UNIFORM_BUFFER, index, id);

	uniform_buffer_index_bindings[index] = id;
}

void opengl_array_state::BindUniformBuffer(GLuint id)
{
	if ( !Is_Extension_Enabled(OGL_ARB_UNIFORM_BUFFER_OBJECT) ) {
		return;
	}

	if ( uniform_buffer == id ) {
		return;
	}

	vglBindBufferARB(GL_UNIFORM_BUFFER, id);

	uniform_buffer = id;
}

opengl_uniform_state::opengl_uniform_state()
{
}

int opengl_uniform_state::findUniform(const SCP_string &name)
{
	SCP_map<SCP_string, int>::iterator iter;

	iter = uniform_lookup.find(name);

	if ( iter == uniform_lookup.end() ) {
		return -1;
	} else {
		return iter->second;
	}
}

void opengl_uniform_state::setUniformi(const SCP_string &name, const int val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::INT ) {
			if ( uniform_data_ints[bind_info->index] == val ) {
				return;
			} 

			uniform_data_ints[bind_info->index] = val;
			resident = true;
		}
	} 

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		uniform_data_ints.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = uniform_data_ints.size() - 1;
		new_bind.type = uniform_bind::INT;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniform1iARB(opengl_shader_get_uniform(name.c_str()), val);
}

void opengl_uniform_state::setUniformf(const SCP_string &name, const float val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::FLOAT ) {
			if ( fl_equal(uniform_data_floats[bind_info->index], val) ) {
				return;
			}

			uniform_data_floats[bind_info->index] = val;
			resident = true;
		}
	} 

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		uniform_data_floats.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = uniform_data_floats.size() - 1;
		new_bind.type = uniform_bind::FLOAT;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniform1fARB(opengl_shader_get_uniform(name.c_str()), val);
}

void opengl_uniform_state::setUniform2f(const SCP_string &name, const float x, const float y)
{
	vec2d temp;

	temp.x = x;
	temp.y = y;

	setUniform2f(name, temp);
}

void opengl_uniform_state::setUniform2f(const SCP_string &name, const vec2d &val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0 ) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::VEC2 ) {
			if ( vm_vec_equal(uniform_data_vec2d[bind_info->index], val) ) {
				return;
			}

			uniform_data_vec2d[bind_info->index] = val;
			resident = true;
		}
	} 

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		uniform_data_vec2d.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = uniform_data_vec2d.size() - 1;
		new_bind.type = uniform_bind::VEC2;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniform2fARB(opengl_shader_get_uniform(name.c_str()), val.x, val.y);
}

void opengl_uniform_state::setUniform3f(const SCP_string &name, const float x, const float y, const float z)
{
	vec3d temp;

	temp.xyz.x = x;
	temp.xyz.y = y;
	temp.xyz.z = z;

	setUniform3f(name, temp);
}

void opengl_uniform_state::setUniform3f(const SCP_string &name, const vec3d &val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0 ) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::VEC3 ) {
			if ( vm_vec_equal(uniform_data_vec3d[bind_info->index], val) ) {
				return;
			}

			uniform_data_vec3d[bind_info->index] = val;
			resident = true;
		}
	} 

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		uniform_data_vec3d.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = uniform_data_vec3d.size() - 1;
		new_bind.type = uniform_bind::VEC3;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniform3fARB(opengl_shader_get_uniform(name.c_str()), val.a1d[0], val.a1d[1], val.a1d[2]);
}

void opengl_uniform_state::setUniform4f(const SCP_string &name, const vec4 &val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0 ) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::VEC4 ) {
			if ( vm_vec_equal(uniform_data_vec4[bind_info->index], val) ) {
				// if the values are close enough, pass.
				return;
			}

			uniform_data_vec4[bind_info->index] = val;
			resident = true;
		}
	} 

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		uniform_data_vec4.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = uniform_data_vec4.size() - 1;
		new_bind.type = uniform_bind::VEC4;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniform4fARB(opengl_shader_get_uniform(name.c_str()), val.a1d[0], val.a1d[1], val.a1d[2], val.a1d[3]);
}

void opengl_uniform_state::setUniformMatrix4f(const SCP_string &name, const matrix4 &val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::MATRIX4 && bind_info->count == 1 ) {
			if ( vm_matrix_equal(uniform_data_matrix4[bind_info->index], val) ) {
				return;
			}

			uniform_data_matrix4[bind_info->index] = val;
			resident = true;
		}
	}

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		//matrix_uniform_data[num_matrix_uniforms] = val;
		//memcpy(&(matrix_uniform_data[num_matrix_uniforms]), &val, sizeof(matrix4));
		uniform_data_matrix4.push_back(val);
		//	num_matrix_uniforms += 1;

		uniform_bind new_bind;
		new_bind.count = 1;
		new_bind.index = uniform_data_matrix4.size() - 1;
		//	new_bind.index = num_matrix_uniforms - 1;
		new_bind.type = uniform_bind::MATRIX4;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniformMatrix4fvARB(opengl_shader_get_uniform(name.c_str()), 1, GL_FALSE, (const GLfloat*)&val);
}

void opengl_uniform_state::setUniformMatrix4fv(const SCP_string &name, const int count, const matrix4 *val)
{
	int uniform_index = findUniform(name);
	bool resident = false;

	if ( uniform_index >= 0) {
		Assert( (size_t)uniform_index < uniforms.size() );

		uniform_bind *bind_info = &uniforms[uniform_index];

		if ( bind_info->type == uniform_bind::MATRIX4 && bind_info->count == count ) {
			bool equal = true;

			// if the values are close enough, pass.
			for ( int i = 0; i < count; ++i ) {
				if ( !vm_matrix_equal(val[i], uniform_data_matrix4[bind_info->index+i]) ) {
					equal = false;
					break;
				}
			}

			if ( equal ) {
				return;
			}

			resident = true;
			for ( int i = 0; i < count; ++i ) {
				uniform_data_matrix4[bind_info->index+i] = val[i];
			}
		}
	}

	if ( !resident ) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		for ( int i = 0; i < count; ++i ) {
			uniform_data_matrix4.push_back(val[i]);
		}

		uniform_bind new_bind;
		new_bind.count = count;
		new_bind.index = uniform_data_matrix4.size() - count;
		//	new_bind.index = num_matrix_uniforms - count;
		new_bind.type = uniform_bind::MATRIX4;
		new_bind.name = name;

		uniforms.push_back(new_bind);

		uniform_lookup[name] = uniforms.size()-1;
	}

	vglUniformMatrix4fvARB(opengl_shader_get_uniform(name.c_str()), count, GL_FALSE, (const GLfloat*)val);
}

void opengl_uniform_state::reset()
{
	uniforms.clear();

	uniform_data_ints.clear();
	uniform_data_floats.clear();
	uniform_data_vec2d.clear();
	uniform_data_vec3d.clear();
	uniform_data_vec4.clear();
	uniform_data_matrix4.clear();

	uniform_lookup.clear();
}

void gr_opengl_clear_states()
{
	GL_state.Texture.DisableAll();

	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);
	gr_reset_lighting();
	gr_set_lighting(false, false);

	opengl_shader_set_current();
}

void opengl_setup_render_states(int &r, int &g, int &b, int &alpha, int &tmap_type, int flags, int is_scaler)
{
	gr_texture_source texture_source = (gr_texture_source)(-1);
	gr_alpha_blend alpha_blend = (gr_alpha_blend)(-1);
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)(-1);
	
	if (gr_zbuffering) {
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) ) {
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	tmap_type = TCACHE_TYPE_NORMAL;

	if (flags & TMAP_FLAG_TEXTURED) {
		r = g = b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	if (flags & TMAP_FLAG_BW_TEXTURE) {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER ) {
		if ( (gr_screen.current_bitmap >= 0) && bm_has_alpha_channel(gr_screen.current_bitmap) ) {
			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if (factor >= 1.0f) {
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		} else {
			tmap_type = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ADDITIVE;	// ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if (factor < 1.0f) {
				r = fl2i(r * gr_screen.current_alpha);
				g = fl2i(g * gr_screen.current_alpha);
				b = fl2i(b * gr_screen.current_alpha);
			}
		}
	} else {
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = fl2i(gr_screen.current_alpha * 255.0f);
	}

	
	if (flags & TMAP_FLAG_TEXTURED) {
		// use nonfiltered textures for interface graphics
		if (flags & TMAP_FLAG_INTERFACE) {
			tmap_type = TCACHE_TYPE_INTERFACE;
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	} else {
		texture_source = TEXTURE_SOURCE_NONE;
	}

	GL_state.SetTextureSource(texture_source);
	GL_state.SetAlphaBlendMode(alpha_blend);
	GL_state.SetZbufferType(zbuffer_type);
}
