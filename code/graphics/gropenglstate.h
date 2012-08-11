/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef _GROPENGLSTATE_H
#define _GROPENGLSTATE_H

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"

struct opengl_texture_unit {
	GLboolean active;	// unit is active
	GLboolean enabled;	// has texture target enabled

	GLenum texture_target;
	GLuint texture_id;

	GLboolean texgen_S;
	GLboolean texgen_T;
	GLboolean texgen_R;
	GLboolean texgen_Q;

	GLenum texgen_mode_S;
	GLenum texgen_mode_T;
	GLenum texgen_mode_R;
	GLenum texgen_mode_Q;

	GLenum env_mode;
	GLenum env_combine_rgb;
	GLenum env_combine_alpha;

	GLfloat rgb_scale;
	GLfloat alpha_scale;

	GLboolean used;
};

class opengl_texture_state
{
	private:
		void default_values(GLint unit, GLenum target = GL_INVALID_ENUM);

		GLuint active_texture_unit;
		GLboolean shader_mode;

		opengl_texture_unit *units;
		GLuint num_texture_units;


	public:
		opengl_texture_state(): active_texture_unit(0), shader_mode(GL_FALSE), units(NULL), Current_texture_source(TEXTURE_SOURCE_NONE) {}
		~opengl_texture_state();

		gr_texture_source Current_texture_source;

		void init(GLuint n_units);

		GLboolean TexgenS(GLint state = -1);
		GLboolean TexgenT(GLint state = -1);
		GLboolean TexgenR(GLint state = -1);
		GLboolean TexgenQ(GLint state = -1);
		void SetTarget(GLenum tex_target);
		void SetActiveUnit(GLuint id = 0);
		void Enable(GLuint tex_id = 0);
		void Disable(bool force = false);
		void DisableUnused();
		void DisableAll();
		void ResetUsed();
		void Delete(GLuint tex_id);
		GLfloat AnisoFilter(GLfloat aniso = 0.0f);
		
		inline void SetRGBScale(GLfloat scale);
		inline void SetAlphaScale(GLfloat scale);
		inline void SetEnvMode(GLenum mode);
		inline void SetEnvCombineMode(GLenum cmode, GLenum cfunc);
		inline void SetWrapS(GLenum mode);
		inline void SetWrapT(GLenum mode);
		inline void SetWrapR(GLenum mode);
		inline void SetTexgenModeS(GLenum mode);
		inline void SetTexgenModeT(GLenum mode);
		inline void SetTexgenModeR(GLenum mode);
		inline void SetTexgenModeQ(GLenum mode);
		inline GLenum GetTarget();
		inline void SetShaderMode(GLboolean mode);
};

inline void opengl_texture_state::SetRGBScale(GLfloat scale)
{
	Assert( ((scale == 1.0f) || (scale == 2.0f) || (scale == 4.0f)) );
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, scale);
}

inline void opengl_texture_state::SetAlphaScale(GLfloat scale)
{
	Assert( ((scale == 1.0f) || (scale == 2.0f) || (scale == 4.0f)) );
	glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, scale);
}

inline void opengl_texture_state::SetEnvMode(GLenum mode)
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
}

inline void opengl_texture_state::SetEnvCombineMode(GLenum cmode, GLenum cfunc)
{
	SetEnvMode(GL_COMBINE);

	if (cmode == GL_COMBINE_RGB) {
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, cfunc);
	} else if (cmode == GL_COMBINE_ALPHA) {
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, cfunc);
	}
}

inline void opengl_texture_state::SetWrapS(GLenum mode)
{
//	glTexParameteri(units[active_texture_unit].texture_target, GL_TEXTURE_WRAP_S, mode);
}

inline void opengl_texture_state::SetWrapT(GLenum mode)
{
//	glTexParameteri(units[active_texture_unit].texture_target, GL_TEXTURE_WRAP_T, mode);
}

inline void opengl_texture_state::SetWrapR(GLenum mode)
{
//	glTexParameteri(units[active_texture_unit].texture_target, GL_TEXTURE_WRAP_R, mode);
}

inline void opengl_texture_state::SetTexgenModeS(GLenum mode)
{
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, mode);
}

inline void opengl_texture_state::SetTexgenModeT(GLenum mode)
{
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, mode);
}

inline void opengl_texture_state::SetTexgenModeR(GLenum mode)
{
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, mode);
}

inline void opengl_texture_state::SetTexgenModeQ(GLenum mode)
{
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, mode);
}

inline GLenum opengl_texture_state::GetTarget()
{
	return units[active_texture_unit].texture_target;
}

inline void opengl_texture_state::SetShaderMode(GLboolean mode)
{
	shader_mode = mode;
}

struct opengl_client_texture_unit
{
	GLboolean status;

	GLuint buffer;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLvoid *pointer;
	bool reset;
};

struct opengl_vertex_attrib_unit
{
	GLboolean status;

	GLuint buffer;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	GLvoid *pointer;

	bool used;
	bool initialized;
	bool reset;
};

class opengl_array_state
{
	private:
		GLuint active_client_texture_unit;
		GLuint num_client_texture_units;

		opengl_client_texture_unit *client_texture_units;

		GLboolean color_array_Status;
		GLuint color_array_Buffer;
		GLint color_array_size;
		GLenum color_array_type;
		GLsizei color_array_stride;
		GLvoid *color_array_pointer;
		bool color_array_reset;

		GLboolean normal_array_Status;
		GLuint normal_array_Buffer;
		GLenum normal_array_Type;
		GLsizei normal_array_Stride;
		GLvoid *normal_array_Pointer;
		bool normal_array_reset;

		GLboolean vertex_array_Status;
		GLuint vertex_array_Buffer;
		GLint vertex_array_Size;
		GLenum vertex_array_Type;
		GLsizei vertex_array_Stride;
		GLvoid *vertex_array_Pointer;
		bool vertex_array_reset;

		SCP_map<GLuint, opengl_vertex_attrib_unit> vertex_attrib_units;

		GLuint array_buffer;
		GLuint element_array_buffer;
	public:
		opengl_array_state(): active_client_texture_unit(0), client_texture_units(NULL) {}
		~opengl_array_state();

		void init(GLuint n_units);

		void SetActiveClientUnit(GLuint id);
		void EnableClientTexture();
		void DisableClientTexture();
		void TexPointer(GLint size, GLenum type, GLsizei stride, GLvoid *pointer);

		void EnableClientColor();
		void DisableClientColor();
		void ColorPointer(GLint size, GLenum type, GLsizei stride, GLvoid *pointer);

		void EnableClientNormal();
		void DisableClientNormal();
		void NormalPointer(GLenum type, GLsizei stride, GLvoid *pointer);

		void EnableClientVertex();
		void DisableClientVertex();
		void VertexPointer(GLint size, GLenum type, GLsizei stride, GLvoid *pointer);
		void ResetVertexPointer();

		void ResetVertexAttribUsed();
		void DisabledVertexAttribUnused();
		void EnableVertexAttrib(GLuint index);
		void DisableVertexAttrib(GLuint index);
		void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer);

		void BindArrayBuffer(GLuint id);
		void BindElementBuffer(GLuint id);
};

class opengl_state
{
	friend class opengl_texture_state;

	private:
		GLboolean fog_Status;
		GLboolean blend_Status;
		GLboolean alphatest_Status;
		GLboolean depthtest_Status;
		GLboolean scissortest_Status;
		GLboolean cullface_Status;
		GLboolean polygonoffsetfill_Status;
		GLboolean normalize_Status;
		GLboolean clipplane_Status[6];
		GLboolean *light_Status;
		GLboolean depthmask_Status;
		GLboolean lighting_Status;

		GLenum frontface_Value;
		GLenum cullface_Value;
		GLenum blendfunc_Value[2];
		GLenum depthfunc_Value;

		gr_alpha_blend Current_alpha_blend_mode;
		gr_zbuffer_type Current_zbuffer_type;


	public:
		opengl_state() : light_Status(NULL) {}
		~opengl_state();

		void init();

		opengl_texture_state Texture;
		opengl_array_state Array;

		void SetTextureSource(gr_texture_source ts);
		void SetAlphaBlendMode(gr_alpha_blend ab);
		void SetZbufferType(gr_zbuffer_type zt);

		// the GLboolean functions will return the current state if no argument
		// and the previous state if an argument is passed
		GLboolean Lighting(GLint state = -1);
		GLboolean Fog(GLint state = -1);
		GLboolean Blend(GLint state = -1);
		GLboolean AlphaTest(GLint state = -1);
		GLboolean DepthTest(GLint state = -1);
		GLboolean ScissorTest(GLint state = -1);
		GLboolean CullFace(GLint state = -1);
		GLboolean PolygonOffsetFill(GLint state = -1);
		GLboolean Normalize(GLint state = -1);
		GLboolean Light(GLint num, GLint state = -1);
		GLboolean ClipPlane(GLint num, GLint state = -1);
		GLboolean DepthMask(GLint state = -1);

		inline GLenum FrontFaceValue(GLenum new_val = GL_INVALID_ENUM);
		inline GLenum CullFaceValue(GLenum new_val = GL_INVALID_ENUM);
		inline void BlendFunc(GLenum s_val, GLenum d_val);
		inline GLenum BlendFuncSource();
		inline GLenum BlendFuncDest();
		inline GLenum DepthFunc(GLenum new_val = GL_INVALID_ENUM);
};

inline GLenum opengl_state::FrontFaceValue(GLenum new_val)
{
	if (new_val != frontface_Value) {
		if (new_val != GL_INVALID_ENUM) {
			glFrontFace(new_val);
			frontface_Value = new_val;
		}
	}

	return frontface_Value;
}

inline GLenum opengl_state::CullFaceValue(GLenum new_val)
{
	if (new_val != cullface_Value) {
		if (new_val != GL_INVALID_ENUM) {
			glCullFace(new_val);
			cullface_Value = new_val;
		}
	}

	return cullface_Value;
}

inline void opengl_state::BlendFunc(GLenum s_val, GLenum d_val)
{
	if ( !((s_val == blendfunc_Value[0]) && (d_val == blendfunc_Value[1])) ) {
		glBlendFunc(s_val, d_val);
		blendfunc_Value[0] = s_val;
		blendfunc_Value[1] = d_val;

		Current_alpha_blend_mode = (gr_alpha_blend)(-1);
	}
}

inline GLenum opengl_state::BlendFuncSource()
{
	return blendfunc_Value[0];
}

inline GLenum opengl_state::BlendFuncDest()
{
	return blendfunc_Value[1];
}

inline GLenum opengl_state::DepthFunc(GLenum new_val)
{
	if (new_val != depthfunc_Value) {
		if (new_val != GL_INVALID_ENUM) {
			glDepthFunc(new_val);
			depthfunc_Value = new_val;

			Current_zbuffer_type = (gr_zbuffer_type)(-1);
		}
	}

	return depthfunc_Value;
}


extern opengl_state GL_state;

void gr_opengl_flush_data_states();
void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler = 0);


#endif	// _GROPENGLSTATE_H
