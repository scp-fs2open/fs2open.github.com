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
#include "gropengl.h"
#include "gropengltexture.h"
#include "gropenglshader.h"
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
		/**
		 * @brief Directly enables a texture on the specified unit
		 *
		 * This is a more efficient version of the standard activeTexture -> Enable since it does not cause an
		 * OpenGL call if the texture is already set correctly
		 *
		 * @warning This may not actually set the active texture unit to the specified value! If you require that then
		 * you need to call SetActiveUnit!
		 *
		 * @param unit The texture unit to use
		 * @param tex_target The texture target of the texture id
		 * @param tex_id The ID of the texture to enable
		 */
		void Enable(GLuint unit, GLenum tex_target, GLuint tex_id);
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

		SCP_map<GLuint, opengl_vertex_attrib_unit> vertex_attrib_units;

		GLuint array_buffer;
		bool array_buffer_valid = false;

		GLuint element_array_buffer;
		bool element_array_buffer_valid = false;

		GLuint texture_array_buffer;
		GLuint uniform_buffer;

		GLuint uniform_buffer_index_bindings[MAX_UNIFORM_BUFFERS];

		struct vertex_buffer_binding {
			GLuint buffer = 0;
			GLintptr offset = 0;
			GLsizei stride = 0;
			bool valid_data = false;
		};
		SCP_vector<vertex_buffer_binding> vertex_buffer_bindings; //!< Use for keeping track of glBindVertexBuffer calls

		/**
		 * @brief Signals that the VAO binding changed which invalidates the state of our array and element buffer state
		 */
		void VertexArrayChanged();
	public:
		opengl_array_state(): active_client_texture_unit(0), client_texture_units(NULL) {
			for ( int i = 0; i < MAX_UNIFORM_BUFFERS; ++i ) {
				uniform_buffer_index_bindings[i] = 0;
			}
		}
		~opengl_array_state();

		void init(GLuint n_units);

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

		void BindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);

		friend class opengl_state;
};

class opengl_constant_state {
	GLint _uniform_buffer_offset_alignment;
	GLint _max_uniform_block_size;
	GLint _max_uniform_block_bindings;

 public:
	opengl_constant_state();

	void init();

	GLint GetUniformBufferOffsetAlignment();

	GLint GetMaxUniformBlockSize();

	GLint GetMaxUniformBlockBindings();
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
		bool clipdistance_Status[6];
		GLboolean depthmask_Status;
        bvec4 colormask_Status;

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

		GLenum stencilFunc;
		GLint stencilFuncRef;
		GLuint stencilFuncMask;

		GLuint stencilMask;

		GLenum stencilOpFrontStencilFail;
		GLenum stencilOpFrontDepthFail;
		GLenum stencilOpFrontPass;

		GLenum stencilOpBackStencilFail;
		GLenum stencilOpBackDepthFail;
		GLenum stencilOpBackPass;

		GLuint current_program;

		// The framebuffer state actually consists of draw and read buffers but we only use both at the same time
		GLuint current_framebuffer;
		SCP_vector<GLuint> framebuffer_stack;

		GLuint current_vao = 0;
	public:
		opengl_state() {}
		~opengl_state() {}

		void init();

		opengl_texture_state Texture;
		opengl_array_state Array;
		opengl_constant_state Constants;

		void SetAlphaBlendMode(gr_alpha_blend ab);
		void SetZbufferType(gr_zbuffer_type zt);
		void SetPolygonOffset(GLfloat factor, GLfloat units);
		void SetPolygonMode(GLenum face, GLenum mode);
		void SetLineWidth(GLfloat width);

		void StencilFunc(GLenum func, GLint ref, GLuint mask);
		void StencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
		void StencilMask(GLuint mask);

		// the GLboolean functions will return the current state if no argument
		// and the previous state if an argument is passed
		GLboolean Blend(GLint state = -1);
		GLboolean AlphaTest(GLint state = -1);
		GLboolean DepthTest(GLint state = -1);
		GLboolean ScissorTest(GLint state = -1);
        GLboolean StencilTest(GLint state = -1);
		GLboolean CullFace(GLint state = -1);
		GLboolean PolygonOffsetFill(GLint state = -1);
		GLboolean ClipDistance(GLint num, bool state = false);
		GLboolean DepthMask(GLint state = -1);
        bvec4 ColorMask(bool red, bool green, bool blue, bool alpha);

		inline GLenum FrontFaceValue(GLenum new_val = GL_INVALID_ENUM);
		inline GLenum CullFaceValue(GLenum new_val = GL_INVALID_ENUM);
		inline void BlendFunc(GLenum s_val, GLenum d_val);
		inline GLenum BlendFuncSource();
		inline GLenum BlendFuncDest();
		inline GLenum DepthFunc(GLenum new_val = GL_INVALID_ENUM);
		inline void InvalidateColor();

		void UseProgram(GLuint program);
		bool IsCurrentProgram(GLuint program);

		void BindFrameBuffer(GLuint name);

		void PushFramebufferState();
		void PopFramebufferState();

		void BindVertexArray(GLuint vao);
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
