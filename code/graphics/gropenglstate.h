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
#include "graphics/gropengltexture.h"
#include "graphics/gropenglshader.h"
#include "graphics/material.h"

#include <glad/glad.h>

#define MAX_UNIFORM_BUFFERS 6
#define MAX_UNIFORM_LOCATIONS 256

struct opengl_texture_unit {
	GLboolean enabled;	// has texture target enabled

	GLenum texture_target;
	GLuint texture_id;
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
		opengl_texture_state(): active_texture_unit(0), shader_mode(GL_FALSE), units(NULL) {}
		~opengl_texture_state();
		
		void init(GLuint n_units);

		void SetTarget(GLenum tex_target);
		void SetActiveUnit(GLuint id = 0);
		void Enable(GLuint tex_id = 0);
		void Delete(GLuint tex_id);
		
		inline GLenum GetTarget();
		inline void SetShaderMode(GLboolean mode);
};

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

	bool reset_ptr;
	bool used_for_draw;
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

	bool status_init;
	bool ptr_init;

	bool reset_ptr;
	bool used_for_draw;
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
		bool color_array_reset_ptr;
		bool color_array_used_for_draw;

		GLboolean normal_array_Status;
		GLuint normal_array_Buffer;
		GLenum normal_array_Type;
		GLsizei normal_array_Stride;
		GLvoid *normal_array_Pointer;
		bool normal_array_reset_ptr;
		bool normal_array_used_for_draw;

		GLboolean vertex_array_Status;
		GLuint vertex_array_Buffer;
		GLint vertex_array_Size;
		GLenum vertex_array_Type;
		GLsizei vertex_array_Stride;
		GLvoid *vertex_array_Pointer;
		bool vertex_array_reset_ptr;
		bool vertex_array_used_for_draw;

		SCP_map<GLuint, opengl_vertex_attrib_unit> vertex_attrib_units;

		GLuint array_buffer;
		GLuint element_array_buffer;
		GLuint texture_array_buffer;
		GLuint uniform_buffer;

		GLuint uniform_buffer_index_bindings[MAX_UNIFORM_BUFFERS];

		GLuint vertex_array_object;
	public:
		opengl_array_state(): active_client_texture_unit(0), client_texture_units(NULL) {
			for ( int i = 0; i < MAX_UNIFORM_BUFFERS; ++i ) {
				uniform_buffer_index_bindings[i] = 0;
			}
		}
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

		void EnableVertexAttrib(GLuint index);
		void DisableVertexAttrib(GLuint index);
		void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer);
		void ResetVertexAttribs();

		void BindPointersBegin();
		void BindPointersEnd();

		void BindArrayBuffer(GLuint id);
		void BindElementBuffer(GLuint id);
		void BindTextureBuffer(GLuint id);
		void BindUniformBuffer(GLuint id);
		void BindUniformBufferBindingIndex(GLuint id, GLuint index);
};

struct uniform_bind
{
	SCP_string name;

	enum data_type {
		INT,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		MATRIX4
	};

	uniform_bind::data_type type;
	size_t index;

	int count;
	int tranpose;
};

class opengl_uniform_state
{
	SCP_vector<uniform_bind> uniforms;

	SCP_vector<int> uniform_data_ints;
	SCP_vector<float> uniform_data_floats;
	SCP_vector<vec2d> uniform_data_vec2d;
	SCP_vector<vec3d> uniform_data_vec3d;
	SCP_vector<vec4> uniform_data_vec4;
	SCP_vector<matrix4> uniform_data_matrix4;

	SCP_map<SCP_string, size_t> uniform_lookup;

	size_t findUniform(const SCP_string &name);
public:
	opengl_uniform_state();

	void setUniformi(const SCP_string &name, const int value);
	void setUniform1iv(const SCP_string &name, const int count, const int *val);
	void setUniformf(const SCP_string &name, const float value);
	void setUniform2f(const SCP_string &name, const float x, const float y);
	void setUniform2f(const SCP_string &name, const vec2d &val);
	void setUniform3f(const SCP_string &name, const float x, const float y, const float z);
	void setUniform3f(const SCP_string &name, const vec3d &value);
	void setUniform4f(const SCP_string &name, const float x, const float y, const float z, const float w);
	void setUniform4f(const SCP_string &name, const vec4 &val);
	void setUniform1fv(const SCP_string &name, const int count, const float *val);
	void setUniform3fv(const SCP_string &name, const int count, const vec3d *val);
	void setUniform4fv(const SCP_string &name, const int count, const vec4 *val);
	void setUniformMatrix4fv(const SCP_string &name, const int count, const matrix4 *value);
	void setUniformMatrix4f(const SCP_string &name, const matrix4 &val);

	void reset();
};

class opengl_light_state
{
	int Light_num;
	bool Enabled;
	
	GLfloat Position[4];
	bool InvalidPosition;

	GLfloat Ambient[4];
	GLfloat Diffuse[4];
	GLfloat Specular[4];

	GLfloat ConstantAttenuation;
	GLfloat LinearAttenuation;
	GLfloat QuadraticAttenuation;

	GLfloat SpotExponent;
	GLfloat SpotCutoff;

public:
	opengl_light_state(int light_num);

	void Enable();
	void Disable();
	void Invalidate();
	void SetPosition(GLfloat *val);
	void SetAmbient(GLfloat *val);
	void SetDiffuse(GLfloat *val);
	void SetSpecular(GLfloat *val);
	void SetConstantAttenuation(GLfloat val);
	void SetLinearAttenuation(GLfloat val);
	void SetQuadraticAttenuation(GLfloat val);
	void SetSpotExponent(GLfloat val);
	void SetSpotCutoff(GLfloat val);
};

class opengl_state
{
	friend class opengl_texture_state;

	private:
		GLboolean blend_Status;
		GLboolean depthtest_Status;
		GLboolean scissortest_Status;
        GLboolean stenciltest_Status;
		GLboolean cullface_Status;
		GLboolean polygonoffsetfill_Status;
		GLboolean normalize_Status;
		GLboolean clipplane_Status[6];
		GLboolean clipdistance_Status[6];
		GLboolean depthmask_Status;
        GLboolean colormask_Status;

		GLenum frontface_Value;
		GLenum cullface_Value;
		GLenum blendfunc_Value[2];
		GLenum depthfunc_Value;

		GLenum polygon_mode_Face;
		GLenum polygon_mode_Mode;

		GLfloat polygon_offset_Factor;
		GLfloat polygon_offset_Unit;

		GLfloat line_width_Value;

		gr_alpha_blend Current_alpha_blend_mode;
        gr_stencil_type Current_stencil_type;
	public:
		opengl_state() {}
		~opengl_state() {}

		void init();

		opengl_texture_state Texture;
		opengl_array_state Array;
		opengl_uniform_state Uniform;

		void SetAlphaBlendMode(gr_alpha_blend ab);
		void SetZbufferType(gr_zbuffer_type zt);
        void SetStencilType(gr_stencil_type st);
		void SetPolygonOffset(GLfloat factor, GLfloat units);
		void SetPolygonMode(GLenum face, GLenum mode);
		void SetLineWidth(GLfloat width);

		// the GLboolean functions will return the current state if no argument
		// and the previous state if an argument is passed
		GLboolean Blend(GLint state = -1);
		GLboolean AlphaTest(GLint state = -1);
		GLboolean DepthTest(GLint state = -1);
		GLboolean ScissorTest(GLint state = -1);
        GLboolean StencilTest(GLint state = -1);
		GLboolean CullFace(GLint state = -1);
		GLboolean PolygonOffsetFill(GLint state = -1);
		GLboolean ClipPlane(GLint num, GLint state = -1);
		GLboolean ClipDistance(GLint num, GLint state = -1);
		GLboolean DepthMask(GLint state = -1);
        GLboolean ColorMask(GLint state = -1);

		inline GLenum FrontFaceValue(GLenum new_val = GL_INVALID_ENUM);
		inline GLenum CullFaceValue(GLenum new_val = GL_INVALID_ENUM);
		inline void BlendFunc(GLenum s_val, GLenum d_val);
		inline GLenum BlendFuncSource();
		inline GLenum BlendFuncDest();
		inline GLenum DepthFunc(GLenum new_val = GL_INVALID_ENUM);
		inline void InvalidateColor();
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
		}
	}

	return depthfunc_Value;
}

extern opengl_state GL_state;

void gr_opengl_clear_states();
void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler = 0);


#endif	// _GROPENGLSTATE_H
