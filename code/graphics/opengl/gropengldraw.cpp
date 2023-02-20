/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

#include "gropengldraw.h"

#include "globalincs/pstypes.h"

#include "ShaderProgram.h"
#include "freespace.h"
#include "gropengl.h"
#include "gropengldeferred.h"
#include "gropenglpostprocessing.h"
#include "gropenglshader.h"
#include "gropengltexture.h"
#include "gropengltnl.h"

#include "cmdline/cmdline.h"
#include "graphics/light.h"
#include "graphics/matrix.h"
#include "graphics/paths/PathRenderer.h"
#include "graphics/util/uniform_structs.h"
#include "render/3d.h"
#include "tracing/tracing.h"

GLuint Scene_framebuffer;
GLuint Scene_ldr_texture;
GLuint Scene_color_texture;
GLuint Scene_position_texture;
GLuint Scene_normal_texture;
GLuint Scene_specular_texture;
GLuint Scene_emissive_texture;
GLuint Scene_luminance_texture;
GLuint Scene_effect_texture;
GLuint Scene_depth_texture;
GLuint Cockpit_depth_texture;
GLuint Scene_stencil_buffer;

GLuint Distortion_framebuffer = 0;
GLuint Distortion_texture[2];
int Distortion_switch = 0;

int Scene_texture_initialized;

int Scene_texture_width;
int Scene_texture_height;

GLfloat Scene_texture_u_scale = 1.0f;
GLfloat Scene_texture_v_scale = 1.0f;

inline GLenum opengl_primitive_type(primitive_type prim_type)
{
	switch ( prim_type ) {
	case PRIM_TYPE_POINTS:
		return GL_POINTS;
	case PRIM_TYPE_TRIS:
		return GL_TRIANGLES;
	case PRIM_TYPE_TRISTRIP:
		return GL_TRIANGLE_STRIP;
	case PRIM_TYPE_TRIFAN:
		return GL_TRIANGLE_FAN;
	case PRIM_TYPE_LINES:
		return GL_LINES;
	case PRIM_TYPE_LINESTRIP:
		return GL_LINE_STRIP;
	default:
		return GL_TRIANGLE_FAN;
	}
}

void gr_opengl_sphere(material* material_def, float  /*rad*/)
{
	opengl_tnl_set_material(material_def, true);

	opengl_draw_sphere();
}

extern int opengl_check_framebuffer();
void opengl_setup_scene_textures()
{
	Scene_texture_initialized = 0;

	if ( Cmdline_no_fbo ) {
		Gr_post_processing_enabled = false;
		Gr_enable_soft_particles = false;
		Gr_framebuffer_effects = {};

		Scene_ldr_texture = 0;
		Scene_color_texture = 0;
		Scene_effect_texture = 0;
		Scene_depth_texture = 0;
		return;
	}

	// clamp size, if needed
	Scene_texture_width = gr_screen.max_w;
	Scene_texture_height = gr_screen.max_h;

	if ( Scene_texture_width > GL_max_renderbuffer_size ) {
		Scene_texture_width = GL_max_renderbuffer_size;
	}

	if ( Scene_texture_height > GL_max_renderbuffer_size) {
		Scene_texture_height = GL_max_renderbuffer_size;
	}

	// create framebuffer
	glGenFramebuffers(1, &Scene_framebuffer);
	GL_state.BindFrameBuffer(Scene_framebuffer);
	opengl_set_object_label(GL_FRAMEBUFFER, Scene_framebuffer, "Scene framebuffer");

	// setup main render texture

	// setup high dynamic range color texture
	glGenTextures(1, &Scene_color_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_color_texture, 0);
	opengl_set_object_label(GL_TEXTURE, Scene_color_texture, "Scene color texture");

	// setup low dynamic range color texture
	glGenTextures(1, &Scene_ldr_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_ldr_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_ldr_texture, "Scene LDR texture");

	// setup position render texture
	glGenTextures(1, &Scene_position_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_position_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_position_texture, "Scene Position texture");

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Scene_position_texture, 0);

	// setup normal render texture
	glGenTextures(1, &Scene_normal_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_normal_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_normal_texture, "Scene Normal texture");

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, Scene_normal_texture, 0);

	// setup specular render texture
	glGenTextures(1, &Scene_specular_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_specular_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_specular_texture, "Scene Specular texture");

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, Scene_specular_texture, 0);

	// setup emissive render texture
	glGenTextures(1, &Scene_emissive_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_emissive_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_emissive_texture, "Scene Emissive texture");

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, Scene_emissive_texture, 0);

	//Set up luminance texture (used as input for FXAA)
	// also used as a light accumulation buffer during the deferred pass
	glGenTextures(1, &Scene_luminance_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_luminance_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_luminance_texture, "Scene Luminance texture");

	// setup effect texture
	glGenTextures(1, &Scene_effect_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_effect_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_effect_texture, "Scene Effect texture");

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, Scene_effect_texture, 0);

	// setup cockpit depth texture
	glGenTextures(1, &Cockpit_depth_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Cockpit_depth_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Scene_texture_width, Scene_texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	opengl_set_object_label(GL_TEXTURE, Cockpit_depth_texture, "Cockpit depth texture");

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Cockpit_depth_texture, 0);
	gr_zbuffer_set(GR_ZBUFF_FULL);
	glClear(GL_DEPTH_BUFFER_BIT);

	// setup main depth texture
	glGenTextures(1, &Scene_depth_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_depth_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Scene_texture_width, Scene_texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	opengl_set_object_label(GL_TEXTURE, Scene_depth_texture, "Scene depth texture");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Scene_depth_texture, 0);

	//setup main stencil buffer
	glGenRenderbuffers(1, &Scene_stencil_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, Scene_stencil_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Scene_texture_width, Scene_texture_height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	if ( opengl_check_framebuffer() ) {
		GL_state.BindFrameBuffer(0);
		glDeleteFramebuffers(1, &Scene_framebuffer);
		Scene_framebuffer = 0;

		glDeleteTextures(1, &Scene_color_texture);
		Scene_color_texture = 0;

		glDeleteTextures(1, &Scene_position_texture);
		Scene_position_texture = 0;

		glDeleteTextures(1, &Scene_normal_texture);
		Scene_normal_texture = 0;

		glDeleteTextures(1, &Scene_specular_texture);
		Scene_specular_texture = 0;

		glDeleteTextures(1, &Scene_emissive_texture);
		Scene_emissive_texture = 0;

		glDeleteTextures(1, &Scene_effect_texture);
		Scene_effect_texture = 0;

		glDeleteTextures(1, &Scene_depth_texture);
		Scene_depth_texture = 0;

		glDeleteTextures(1, &Scene_luminance_texture);
		Scene_luminance_texture = 0;

		//glDeleteTextures(1, &Scene_fxaa_output_texture);
		//Scene_fxaa_output_texture = 0;

		Gr_post_processing_enabled = false;
		Gr_enable_soft_particles = false;
		return;
	}

	//Setup thruster distortion framebuffer
    if (Gr_framebuffer_effects.any_set())
    {
        glGenFramebuffers(1, &Distortion_framebuffer);
		GL_state.BindFrameBuffer(Distortion_framebuffer);

        glGenTextures(2, Distortion_texture);

        GL_state.Texture.SetActiveUnit(0);
        GL_state.Texture.SetTarget(GL_TEXTURE_2D);
        GL_state.Texture.Enable(Distortion_texture[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

        GL_state.Texture.Enable(Distortion_texture[1]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Distortion_texture[0], 0);
    }


	if ( opengl_check_framebuffer() ) {
		GL_state.BindFrameBuffer(0);
		glDeleteFramebuffers(1, &Distortion_framebuffer);
		Distortion_framebuffer = 0;

		glDeleteTextures(2, Distortion_texture);
		Distortion_texture[0] = 0;
		Distortion_texture[1] = 0;
		return;
	}

	if ( opengl_check_for_errors("post_init_framebuffer()") ) {
		Scene_color_texture = 0;
		Scene_depth_texture = 0;

		Gr_post_processing_enabled = false;
		Gr_enable_soft_particles = false;
		return;
	}

	GL_state.BindFrameBuffer(0);

	Scene_texture_initialized = 1;
	Scene_framebuffer_in_frame = false;
}

void opengl_scene_texture_shutdown()
{
	if ( !Scene_texture_initialized ) {
		return;
	}

	if ( Scene_color_texture ) {
		glDeleteTextures(1, &Scene_color_texture);
		Scene_color_texture = 0;
	}

	if ( Scene_position_texture ) {
		glDeleteTextures(1, &Scene_position_texture);
		Scene_position_texture = 0;
	}

	if ( Scene_normal_texture ) {
		glDeleteTextures(1, &Scene_normal_texture);
		Scene_normal_texture = 0;
	}

	if ( Scene_specular_texture ) {
		glDeleteTextures(1, &Scene_specular_texture);
		Scene_specular_texture = 0;
	}

	if (Scene_emissive_texture) {
		glDeleteTextures(1, &Scene_emissive_texture);
		Scene_emissive_texture = 0;
	}

	if ( Scene_effect_texture ) {
		glDeleteTextures(1, &Scene_effect_texture);
		Scene_effect_texture = 0;
	}

	if ( Scene_depth_texture ) {
		glDeleteTextures(1, &Scene_depth_texture);
		Scene_depth_texture = 0;
	}

	if ( Scene_framebuffer ) {
		glDeleteFramebuffers(1, &Scene_framebuffer);
		Scene_framebuffer = 0;
	}

	glDeleteTextures(2, Distortion_texture);
	Distortion_texture[0] = 0;
	Distortion_texture[1] = 0;

	if ( Distortion_framebuffer ) {
		glDeleteFramebuffers(1, &Distortion_framebuffer);
		Distortion_framebuffer = 0;
	}

	Scene_texture_initialized = 0;
	Scene_framebuffer_in_frame = false;
}

void gr_opengl_scene_texture_begin()
{
	if ( !Scene_texture_initialized ) {
		return;
	}

	if ( Scene_framebuffer_in_frame ) {
		return;
	}

	GR_DEBUG_SCOPE("Begin scene texture");
	TRACE_SCOPE(tracing::SceneTextureBegin);

	GL_state.PushFramebufferState();
	GL_state.BindFrameBuffer(Scene_framebuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Scene_depth_texture, 0);

	if (GL_rendering_to_texture)
	{
		Scene_texture_u_scale = i2fl(gr_screen.max_w) / i2fl(Scene_texture_width);
		Scene_texture_v_scale = i2fl(gr_screen.max_h) / i2fl(Scene_texture_height);

		CLAMP(Scene_texture_u_scale, 0.0f, 1.0f);
		CLAMP(Scene_texture_v_scale, 0.0f, 1.0f);
	}
	else
	{
		Scene_texture_u_scale = 1.0f;
		Scene_texture_v_scale = 1.0f;
	}

	if ( Cmdline_no_deferred_lighting ) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	} else {
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(5, buffers);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		opengl_clear_deferred_buffers();

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}

	Scene_framebuffer_in_frame = true;

	if ( Gr_post_processing_enabled && !PostProcessing_override ) {
		High_dynamic_range = true;
	}
}

float time_buffer = 0.0f;
void gr_opengl_scene_texture_end()
{

	if ( !Scene_framebuffer_in_frame ) {
		return;
	}

	GR_DEBUG_SCOPE("End scene texture");
	TRACE_SCOPE(tracing::SceneTextureEnd);

	time_buffer+=flFrametime;
	if(time_buffer>0.03f)
	{
		gr_opengl_update_distortion();
		time_buffer = 0.0f;
	}

	if ( Gr_post_processing_enabled && !PostProcessing_override ) {
		gr_opengl_post_process_end();
	} else {
		GR_DEBUG_SCOPE("Draw scene texture");
		TRACE_SCOPE(tracing::DrawSceneTexture);

		GLboolean depth = GL_state.DepthTest(GL_FALSE);
		GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
		GLboolean blend = GL_state.Blend(GL_FALSE);
		GLboolean cull = GL_state.CullFace(GL_FALSE);

		GL_state.PopFramebufferState();

		GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_color_texture);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		opengl_shader_set_passthrough(true, High_dynamic_range);

		GL_state.Array.BindArrayBuffer(0);

		if (GL_rendering_to_texture)
		{
			opengl_draw_textured_quad(0.0f, 0.0f, 0.0f, 0.0f, (float)gr_screen.max_w, (float)gr_screen.max_h, Scene_texture_u_scale, Scene_texture_v_scale);
		}
		else
		{
			opengl_draw_textured_quad(0.0f, 0.0f, 0.0f, Scene_texture_v_scale, (float)gr_screen.max_w, (float)gr_screen.max_h, Scene_texture_u_scale, 0.0f);
		}

		// reset state
		GL_state.DepthTest(depth);
		GL_state.DepthMask(depth_mask);
		GL_state.Blend(blend);
		GL_state.CullFace(cull);
	}

	// Reset the UV scale values

	Scene_texture_u_scale = 1.0f;
	Scene_texture_v_scale = 1.0f;

	Scene_framebuffer_in_frame = false;
	High_dynamic_range = false;
}

void gr_opengl_copy_effect_texture()
{
	GR_DEBUG_SCOPE("Copy effect texture");

	if ( !Scene_framebuffer_in_frame ) {
		return;
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT5);
	glBlitFramebuffer(0, 0, gr_screen.max_w, gr_screen.max_h, 0, 0, gr_screen.max_w, gr_screen.max_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void gr_opengl_render_shield_impact(shield_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	gr_buffer_handle buffer_handle,
	int n_verts)
{
	matrix4 impact_transform;
	matrix4 impact_projection;
	vec3d min;
	vec3d max;

	opengl_tnl_set_material(material_info, false);

	float radius = material_info->get_impact_radius();
	min.xyz.x = min.xyz.y = min.xyz.z = -radius;
	max.xyz.x = max.xyz.y = max.xyz.z = radius;

	vm_matrix4_set_orthographic(&impact_projection, &max, &min);

	matrix impact_orient = material_info->get_impact_orient();
	vec3d impact_pos = material_info->get_impact_pos();

	vm_matrix4_set_inverse_transform(&impact_transform, &impact_orient, &impact_pos);

	uint32_t array_index = 0;
	if (material_info->get_texture_map(TM_BASE_TYPE) >= 0) {
		float u_scale, v_scale;

		if (!gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE), material_info->get_texture_type(),
								  &u_scale, &v_scale, &array_index)) {
			mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->get_texture_map(TM_BASE_TYPE)));
		}
	}

	opengl_set_generic_uniform_data<graphics::generic_data::shield_impact_data>(
		[&](graphics::generic_data::shield_impact_data* data) {
			data->hitNormal             = impact_orient.vec.fvec;
			data->shieldProjMatrix      = impact_projection;
			data->shieldModelViewMatrix = impact_transform;
			data->shieldMapIndex        = array_index;
			data->srgb                  = High_dynamic_range ? 1 : 0;
			data->color                 = material_info->get_color();
		});

	gr_matrix_set_uniforms();

	opengl_render_primitives(prim_type, layout, n_verts, buffer_handle, 0, 0);
}

void gr_opengl_update_distortion()
{
	if (Distortion_framebuffer == 0) {
		// distortion is disabled
		return;
	}

	GR_DEBUG_SCOPE("Update distortion");
	TRACE_SCOPE(tracing::UpdateDistortion);

	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	opengl_shader_set_passthrough(true, High_dynamic_range);

	GL_state.PushFramebufferState();
	GL_state.BindFrameBuffer(Distortion_framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Distortion_texture[!Distortion_switch], 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glViewport(0,0,32,32);
	GL_state.Texture.Enable(0, GL_TEXTURE_2D, Distortion_texture[Distortion_switch]);
	glClearColor(0.5f, 0.5f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	vertex vertices[4];

	vertices[0].texture_position.u = 0.0f;
	vertices[0].texture_position.v = 0.0f;

	vertices[1].texture_position.u = 0.96875f;
	vertices[1].texture_position.v = 0.0f;

	vertices[2].texture_position.u = 0.0f;
	vertices[2].texture_position.v = 1.0f;

	vertices[3].texture_position.u = 0.96875f;
	vertices[3].texture_position.v = 1.0f;
	
	vertices[0].screen.xyw.x = 0.03f*(float)gr_screen.max_w;
	vertices[0].screen.xyw.y = (float)gr_screen.max_h;

	vertices[1].screen.xyw.x = (float)gr_screen.max_w;
	vertices[1].screen.xyw.y = (float)gr_screen.max_h;

	vertices[2].screen.xyw.x = 0.03f*(float)gr_screen.max_w;
	vertices[2].screen.xyw.y = 0.0f;

	vertices[3].screen.xyw.x = (float)gr_screen.max_w;
	vertices[3].screen.xyw.y = 0.0f;

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), offsetof(vertex, screen));
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(vertex), offsetof(vertex, texture_position));

	opengl_render_primitives_immediate(PRIM_TYPE_TRISTRIP, &vert_def, 4, vertices, sizeof(vertex) * 4);

	opengl_shader_set_passthrough(false, High_dynamic_range);

	vertex distortion_verts[33];

	for(int i = 0; i < 33; i++)
	{
		distortion_verts[i].r = (ubyte)Random::next(256);
		distortion_verts[i].g = (ubyte)Random::next(256);
		distortion_verts[i].b = 255;
		distortion_verts[i].a = 255;

		distortion_verts[i].screen.xyw.x = 1.f;
		distortion_verts[i].screen.xyw.y = (float)gr_screen.max_h*0.03125f*i;
	}

	vert_def = vertex_layout();

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), offsetof(vertex, screen));
	vert_def.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), offsetof(vertex, r));

	opengl_render_primitives_immediate(PRIM_TYPE_POINTS, &vert_def, 33, distortion_verts, 33 * sizeof(vertex));

	Distortion_switch = !Distortion_switch;

	// reset state
	GL_state.PopFramebufferState();

	glViewport(0,0,gr_screen.max_w,gr_screen.max_h);

	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);
}

void opengl_render_primitives(primitive_type prim_type,
	vertex_layout* layout,
	int n_verts,
	gr_buffer_handle buffer_handle,
	size_t vert_offset,
	size_t byte_offset)
{
	GR_DEBUG_SCOPE("Render primitives");

	Assertion(buffer_handle.isValid(),
		"A valid buffer handle is required! Use the immediate buffer if data is not in GPU buffer yet.");

	opengl_bind_vertex_layout(*layout, opengl_buffer_get_id(GL_ARRAY_BUFFER, buffer_handle), 0, byte_offset);

	glDrawArrays(opengl_primitive_type(prim_type), (GLint)vert_offset, n_verts);
}

void opengl_render_primitives_immediate(primitive_type prim_type, vertex_layout* layout, int n_verts, void* data, int size)
{
	auto offset = gr_add_to_immediate_buffer(size, data);

	opengl_render_primitives(prim_type, layout, n_verts, gr_immediate_buffer_handle, 0, offset);
}

void gr_opengl_render_primitives(material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle,
	size_t buffer_offset)
{
	GL_CHECK_FOR_ERRORS("start of gr_opengl_render_primitives()");

	opengl_tnl_set_material(material_info, true);

	opengl_render_primitives(prim_type, layout, n_verts, buffer_handle, offset, buffer_offset);

	GL_CHECK_FOR_ERRORS("end of gr_opengl_render_primitives()");
}

void gr_opengl_render_primitives_particle(particle_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	GL_CHECK_FOR_ERRORS("start of gr_opengl_render_primitives_particle()");

	opengl_tnl_set_material_particle(material_info);

	opengl_render_primitives(prim_type, layout, n_verts, buffer_handle, offset, 0);

	GL_CHECK_FOR_ERRORS("end of gr_opengl_render_primitives_particle()");
}

void gr_opengl_render_primitives_distortion(distortion_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	GL_CHECK_FOR_ERRORS("start of gr_opengl_render_primitives_distortion()");

	opengl_tnl_set_material_distortion(material_info);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	opengl_render_primitives(prim_type, layout, n_verts, buffer_handle, offset, 0);

	GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, buffers);

	GL_CHECK_FOR_ERRORS("start of gr_opengl_render_primitives_distortion()");
}

void gr_opengl_render_movie(movie_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int n_verts,
	gr_buffer_handle buffer,
	size_t buffer_offset)
{
	GR_DEBUG_SCOPE("Render movie frame");

	gr_set_2d_matrix();

	opengl_tnl_set_material_movie(material_info);

	opengl_render_primitives(prim_type, layout, n_verts, buffer, 0, buffer_offset);

	gr_end_2d_matrix();
}

void gr_opengl_render_nanovg(nanovg_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	GR_DEBUG_SCOPE("Render NanoVG primitives");

	opengl_tnl_set_material_nanovg(material_info);

	opengl_render_primitives(prim_type, layout, n_verts, buffer_handle, offset, 0);
}

void gr_opengl_render_primitives_batched(batched_bitmap_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int offset,
	int n_verts,
	gr_buffer_handle buffer_handle)
{
	GR_DEBUG_SCOPE("Render batched primitives");

	opengl_tnl_set_material_batched(material_info);

	opengl_render_primitives(prim_type, layout, n_verts, buffer_handle, offset, 0);
}
void gr_opengl_render_rocket_primitives(interface_material* material_info,
	primitive_type prim_type,
	vertex_layout* layout,
	int n_indices,
	gr_buffer_handle vertex_buffer,
	gr_buffer_handle index_buffer)
{
	GR_DEBUG_SCOPE("Render rocket ui primitives");

	gr_set_2d_matrix();

	opengl_tnl_set_rocketui_material(material_info);

	opengl_bind_vertex_layout(*layout,
		opengl_buffer_get_id(GL_ARRAY_BUFFER, vertex_buffer),
		opengl_buffer_get_id(GL_ELEMENT_ARRAY_BUFFER, index_buffer));

	glDrawElements(opengl_primitive_type(prim_type), n_indices, GL_UNSIGNED_INT, nullptr);

	gr_end_2d_matrix();
}

void opengl_draw_textured_quad(GLfloat x1,
							   GLfloat y1,
							   GLfloat u1,
							   GLfloat v1,
							   GLfloat x2,
							   GLfloat y2,
							   GLfloat u2,
							   GLfloat v2) {
	GR_DEBUG_SCOPE("Draw textured quad");

	GLfloat glVertices[4][4] = {
		{ x1, y1, u1, v1 },
		{ x1, y2, u1, v2 },
		{ x2, y1, u2, v1 },
		{ x2, y2, u2, v2 }
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(GLfloat) * 4, 0);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(GLfloat) * 4, sizeof(GLfloat) * 2);

	opengl_render_primitives_immediate(PRIM_TYPE_TRISTRIP, &vert_def, 4, glVertices, sizeof(glVertices));
}

void opengl_draw_full_screen_textured(GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2)
{
	GR_DEBUG_SCOPE("Draw full screen triangle");

	GLfloat glVertices[3][4] = {
	    {-1.f, -1.f, u1, v1},
	    {3.f, -1.f, u2 * 2.f, v1},
	    {-1.f, 3.f, u1, v2 * 2.f},
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(GLfloat) * 4, 0);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(GLfloat) * 4, sizeof(GLfloat) * 2);

	opengl_render_primitives_immediate(PRIM_TYPE_TRIS, &vert_def, 3, glVertices, sizeof(glVertices));
}

void gr_opengl_render_decals(decal_material* material_info,
							 primitive_type prim_type,
							 vertex_layout* layout,
							 int num_elements,
							 const indexed_vertex_source& binding) {
	opengl_tnl_set_material_decal(material_info);

	opengl_bind_vertex_layout(*layout,
							  opengl_buffer_get_id(GL_ARRAY_BUFFER, binding.Vbuffer_handle),
							  opengl_buffer_get_id(GL_ELEMENT_ARRAY_BUFFER, binding.Ibuffer_handle));

	glDrawElements(opengl_primitive_type(prim_type), num_elements, GL_UNSIGNED_INT, nullptr);
}

void gr_opengl_start_decal_pass() {
	// For now we only render into the diffuse channel of the framebuffer
	GLenum buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT4,
	};
	glDrawBuffers(3, buffers);
}
void gr_opengl_stop_decal_pass() {
	GLenum buffers2[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
	};
	glDrawBuffers(5, buffers2);
}

void gr_opengl_calculate_irrmap()
{

	auto previous_target = gr_screen.rendering_to_texture;
	int irr_shader = gr_opengl_maybe_create_shader(SDR_TYPE_IRRADIANCE_MAP_GEN, 0);
	opengl_shader_set_current(irr_shader);
	auto env_tex = bm_get_gr_info<tcache_slot_opengl>(ENVMAP);
	glBindTexture(env_tex->texture_target, env_tex->texture_id);
	GL_state.Texture.Enable(0, GL_TEXTURE_CUBE_MAP, env_tex->texture_id);
	Current_shader->program->Uniforms.setTextureUniform("envmap", 0);

	for (int i = 0; i < 6; i++) {
		bm_set_render_target(gr_screen.irrmap_render_target, i);
		gr_clear();
		opengl_set_generic_uniform_data<graphics::generic_data::irrmap_data>(
			[&](graphics::generic_data::irrmap_data* data) { data->face = i; });
		opengl_draw_full_screen_textured(-1.0f, -1.0f, 1.0f, 1.0f);
	}

	bm_set_render_target(previous_target);
}
