/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengltnl.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "render/3d.h"

SCP_vector<opengl_shader_t> GL_shader;

static char *GLshader_info_log = NULL;
static const int GLshader_info_log_size = 8192;
GLuint Framebuffer_fallback_texture_id = 0;

geometry_sdr_params *Current_geo_sdr_params = NULL;

opengl_vert_attrib GL_vertex_attrib_info[] =
{
	{ opengl_vert_attrib::POSITION,		"vertPosition",		{ 0.0f, 0.0f, 0.0f, 1.0f } },
	{ opengl_vert_attrib::COLOR,		"vertColor",		{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ opengl_vert_attrib::TEXCOORD,		"vertTexCoord",		{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ opengl_vert_attrib::NORMAL,		"vertNormal",		{ 0.0f, 0.0f, 1.0f, 0.0f } },
	{ opengl_vert_attrib::TANGENT,		"vertTangent",		{ 1.0f, 0.0f, 0.0f, 0.0f } },
	{ opengl_vert_attrib::MODEL_ID,		"vertModelID",		{ 0.0f, 0.0f, 0.0f, 0.0f } },
	{ opengl_vert_attrib::RADIUS,		"vertRadius",		{ 1.0f, 0.0f, 0.0f, 0.0f } },
	{ opengl_vert_attrib::UVEC,			"vertUvec",			{ 0.0f, 1.0f, 0.0f, 0.0f } }
};

/**
 * Static lookup reference for shader uniforms
 * When adding a new shader, list all associated uniforms and attributes here
 */
static opengl_shader_type_t GL_shader_types[] = {
	{ SDR_TYPE_MODEL, "main-v.sdr", "main-f.sdr", "main-g.sdr", {GL_TRIANGLES, GL_TRIANGLE_STRIP, 3}, 
		{ "modelViewMatrix", "modelMatrix", "viewMatrix", "projMatrix", "textureMatrix", "color" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::NORMAL, opengl_vert_attrib::TANGENT, opengl_vert_attrib::MODEL_ID }, "Model Rendering" },

	{ SDR_TYPE_EFFECT_PARTICLE, "effect-v.sdr", "effect-particle-f.sdr", "effect-screen-g.sdr", {GL_POINTS, GL_TRIANGLE_STRIP, 4}, 
		{ "modelViewMatrix", "projMatrix", "baseMap", "depthMap", "window_width", "window_height", "nearZ", "farZ", "linear_depth", "srgb", "blend_alpha" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::RADIUS, opengl_vert_attrib::COLOR }, "Particle Effects" },

	{ SDR_TYPE_EFFECT_DISTORTION, "effect-distort-v.sdr", "effect-distort-f.sdr", 0, { 0, 0, 0 }, 
		{ "modelViewMatrix", "projMatrix", "baseMap", "window_width", "window_height", "distMap", "frameBuffer", "use_offset" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::RADIUS, opengl_vert_attrib::COLOR }, "Distortion Effects" },

	{ SDR_TYPE_POST_PROCESS_MAIN, "post-v.sdr", "post-f.sdr", 0, {0, 0, 0}, 
		{ "tex", "depth_tex", "timer" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Post Processing" },

	{ SDR_TYPE_POST_PROCESS_BLUR, "post-v.sdr", "blur-f.sdr", 0, {0, 0, 0}, 
		{ "tex", "texSize", "level", "tapSize", "debug" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Gaussian Blur" },

	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP, "post-v.sdr", "bloom-comp-f.sdr", 0, {0, 0, 0}, 
		{ "bloomed", "bloom_intensity", "levels" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Bloom Compositing" },

	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS, "post-v.sdr", "brightpass-f.sdr", 0, { 0, 0, 0 },
		{ "tex" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Bloom Brightpass" },

	{ SDR_TYPE_POST_PROCESS_FXAA, "fxaa-v.sdr", "fxaa-f.sdr", 0, {0, 0, 0}, 
		{ "tex0", "rt_w", "rt_h" },
		{ opengl_vert_attrib::POSITION }, "FXAA" },

	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS, "post-v.sdr", "fxaapre-f.sdr", 0, {0, 0, 0}, 
		{ "tex" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "FXAA Prepass" },

	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, "post-v.sdr", "ls-f.sdr", 0, {0, 0, 0}, 
		{ "scene", "cockpit", "sun_pos", "weight", "intensity", "falloff", "density", "cp_intensity" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Lightshafts" },

	{ SDR_TYPE_POST_PROCESS_TONEMAPPING, "post-v.sdr", "tonemapping-f.sdr", 0, {0, 0, 0},
		{ "tex", "exposure" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Tonemapping" },

	{ SDR_TYPE_DEFERRED_LIGHTING, "deferred-v.sdr", "deferred-f.sdr", 0, { 0, 0, 0 }, 
		{ "modelViewMatrix", "projMatrix", "scale", "ColorBuffer", "NormalBuffer", "PositionBuffer", "SpecBuffer", "invScreenWidth", "invScreenHeight", "lightType", "lightRadius",
		"diffuseLightColor", "specLightColor", "dualCone", "coneDir", "coneAngle", "coneInnerAngle", "specFactor" }, 
		{ opengl_vert_attrib::POSITION }, "Deferred Lighting" },
	
	{ SDR_TYPE_DEFERRED_CLEAR, "deferred-clear-v.sdr", "deferred-clear-f.sdr", 0, {0, 0, 0}, 
		{  },
		{ opengl_vert_attrib::POSITION }, "Clear Deferred Lighting Buffer" },

	{ SDR_TYPE_VIDEO_PROCESS, "video-v.sdr", "video-f.sdr", 0, {0, 0, 0}, 
		{ "modelViewMatrix", "projMatrix", "ytex", "utex", "vtex" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Video Playback" },

	{ SDR_TYPE_PASSTHROUGH_RENDER, "passthrough-v.sdr", "passthrough-f.sdr", 0,{ 0, 0, 0 },
		{ "modelViewMatrix", "projMatrix", "baseMap", "noTexturing", "alphaTexture", "srgb", "intensity", "color", "alphaThreshold" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::COLOR }, "Passthrough" }
};

/**
 * Static lookup reference for shader variant uniforms
 * When adding a new shader variant for a shader, list all associated uniforms and attributes here
 */
static opengl_shader_variant_t GL_shader_variants[] = {
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_LIGHT, "FLAG_LIGHT", 
		{ "n_lights", "lightPosition", "lightDirection", "lightDiffuseColor", "lightSpecColor", "lightType", "lightAttenuation", "ambientFactor", "diffuseFactor", "emissionFactor", "defaultGloss" }, {  },
		"Lighting" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_FOG, "FLAG_FOG", 
		{ "fogStart", "fogScale", "fogColor" }, {  },
		"Fog Effect" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_DIFFUSE_MAP, "FLAG_DIFFUSE_MAP", 
		{ "sBasemap", "desaturate", "desaturate_clr", "blend_alpha", "overrideDiffuse", "diffuseClr" }, {  },
		"Diffuse Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_GLOW_MAP, "FLAG_GLOW_MAP", 
		{ "sGlowmap", "overrideGlow", "glowClr" }, {  },
		"Glow Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_SPEC_MAP, "FLAG_SPEC_MAP", 
		{ "sSpecmap", "overrideSpec", "specClr", "gammaSpec", "alphaGloss" }, {  },
		"Specular Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_NORMAL_MAP, "FLAG_NORMAL_MAP", 
		{ "sNormalmap" }, {  },
		"Normal Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_HEIGHT_MAP, "FLAG_HEIGHT_MAP", 
		{ "sHeightmap" }, {  },
		"Parallax Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ENV_MAP, "FLAG_ENV_MAP", 
		{ "sEnvmap", "envGloss", "envMatrix" }, {  },
		"Environment Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ANIMATED, "FLAG_ANIMATED", 
		{ "sFramebuffer", "effect_num", "anim_timer", "vpwidth", "vpheight" }, {  },
		"Animated Effects" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_MISC_MAP, "FLAG_MISC_MAP", 
		{ "sMiscmap" }, {  },
		"Utility mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_TEAMCOLOR, "FLAG_TEAMCOLOR", 
		{ "stripe_color", "base_color", "team_glow_enabled" }, {  },
		"Team Colors" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_DEFERRED, "FLAG_DEFERRED", 
		{ }, {  },
		"Deferred lighting" },
	
	{ SDR_TYPE_MODEL, true, SDR_FLAG_MODEL_SHADOW_MAP, "FLAG_SHADOW_MAP", 
		{ "shadow_proj_matrix" }, {  },
		"Shadow Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_SHADOWS, "FLAG_SHADOWS", 
		{ "shadow_map", "shadow_mv_matrix", "shadow_proj_matrix", "veryneardist", "neardist", "middist", "fardist" }, { },
		"Shadows" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_THRUSTER, "FLAG_THRUSTER", 
		{ "thruster_scale" }, {  },
		"Thruster scaling" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_TRANSFORM, "FLAG_TRANSFORM", 
		{ "transform_tex", "buffer_matrix_offset" }, {  },
		"Submodel Transforms" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_CLIP, "FLAG_CLIP", 
		{ "use_clip_plane", "clip_normal", "clip_position" }, {  },
		"Clip Plane" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_HDR, "FLAG_HDR",
		{ }, {  },
		"High Dynamic Range" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_AMBIENT_MAP, "FLAG_AMBIENT_MAP",
		{ "sAmbientmap" }, {  },
		"Ambient Occlusion Map" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_NORMAL_ALPHA, "FLAG_NORMAL_ALPHA",
		{ "normalAlphaMinMax" }, { },
		"Normal Alpha" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_NORMAL_EXTRUDE, "FLAG_NORMAL_EXTRUDE",
		{ "extrudeWidth" }, { },
		"Normal Extrusion" },

	{ SDR_TYPE_EFFECT_PARTICLE, true, SDR_FLAG_PARTICLE_POINT_GEN, "FLAG_EFFECT_GEOMETRY", 
		{ }, { opengl_vert_attrib::UVEC },
		"Geometry shader point-based particles" },
	
	{ SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_HORIZONTAL, "PASS_0", 
		{ }, {  },
		"Horizontal blur pass" },
	
	{ SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_VERTICAL, "PASS_1", 
		{ }, {  },
		"Vertical blur pass" }
};

static const int GL_num_shader_variants = sizeof(GL_shader_variants) / sizeof(opengl_shader_variant_t);

opengl_shader_t *Current_shader = NULL;

// Forward declarations
GLuint opengl_shader_create(const SCP_vector<SCP_string>& vs, const SCP_vector<SCP_string>& fs, const SCP_vector<SCP_string>& gs);
void opengl_shader_check_info_log(GLuint shader_object);

/**
 * Set the currently active shader 
 * @param shader_obj	Pointer to an opengl_shader_t object. This function calls glUseProgramARB with parameter 0 if shader_obj is NULL or if function is called without parameters, causing OpenGL to revert to fixed-function processing 
 */
void opengl_shader_set_current(opengl_shader_t *shader_obj)
{
	if (shader_obj != NULL) {
		if(!Current_shader || (Current_shader->program_id != shader_obj->program_id)) {
			GL_state.Array.ResetVertexAttribs();
			GL_state.Uniform.reset();
			Current_shader = shader_obj;
			glUseProgram(Current_shader->program_id);
			GL_state.Uniform.reset();
#ifndef NDEBUG
			if ( opengl_check_for_errors("shader_set_current()") ) {
				glValidateProgram(Current_shader->program_id);

				GLint obj_status = 0;
				glGetProgramiv(Current_shader->program_id, GL_VALIDATE_STATUS, &obj_status);

				if ( !obj_status ) {
					opengl_program_check_info_log(Current_shader->program_id);
	
					mprintf(("VALIDATE INFO-LOG:\n"));

					if (strlen(GLshader_info_log) > 5) {
						mprintf(("%s\n", GLshader_info_log));
					} else {
						mprintf(("<EMPTY>\n"));
					}
				}
			}
#endif
		}
	} else {
		GL_state.Array.ResetVertexAttribs();
		GL_state.Uniform.reset();
		Current_shader = NULL;
		glUseProgram(0);
	}
}

void opengl_shader_set_current(int handle)
{
	Assert(handle >= 0);
	Assert(handle < (int)GL_shader.size());

	opengl_shader_set_current(&GL_shader[handle]);
}

/**
 * Given a set of flags, determine whether a shader with these flags exists within the GL_shader vector. If no shader with the requested flags exists, attempt to compile one.
 *
 * @param shader_t  shader_type variable, a reference to the shader program needed
 * @param flags	Integer variable, holding a combination of SDR_* flags
 * @return 		Index into GL_shader, referencing a valid shader, or -1 if shader compilation failed
 */
int gr_opengl_maybe_create_shader(shader_type shader_t, unsigned int flags)
{
	size_t idx;
	size_t max = GL_shader.size();

	for (idx = 0; idx < max; idx++) {
		if (GL_shader[idx].shader == shader_t && GL_shader[idx].flags == flags) {
			return (int)idx;
		}
	}

	// If we are here, it means we need to compile a new shader
	return opengl_compile_shader(shader_t, flags);
}

void opengl_delete_shader(int sdr_handle)
{
	Assert(sdr_handle >= 0);
	Assert(sdr_handle < (int)GL_shader.size());

	if (GL_shader[sdr_handle].program_id) {
		glDeleteProgram(GL_shader[sdr_handle].program_id);
		GL_shader[sdr_handle].program_id = 0;
	}

	GL_shader[sdr_handle].flags = 0;
	GL_shader[sdr_handle].flags2 = 0;
	GL_shader[sdr_handle].shader = NUM_SHADER_TYPES;

	GL_shader[sdr_handle].uniforms.clear();
	GL_shader[sdr_handle].attributes.clear();
	GL_shader[sdr_handle].uniform_blocks.clear();
}

/**
 * Go through GL_shader and call glDeleteObject() for all created shaders, then clear GL_shader
 */
void opengl_shader_shutdown()
{
	size_t i;

	for (i = 0; i < GL_shader.size(); i++) {
		if (GL_shader[i].program_id) {
			glDeleteProgram(GL_shader[i].program_id);
			GL_shader[i].program_id = 0;
		}

		GL_shader[i].uniforms.clear();
		GL_shader[i].attributes.clear();
		GL_shader[i].uniform_blocks.clear();
	}

	GL_shader.clear();

	if (GLshader_info_log != NULL) {
		vm_free(GLshader_info_log);
		GLshader_info_log = NULL;
	}
}

static SCP_string opengl_shader_get_header(shader_type type_id, int flags, shader_stage stage) {
	SCP_stringstream sflags;

	sflags << "#version " << GLSL_version << " core\n";
	
	if (type_id == SDR_TYPE_POST_PROCESS_MAIN || type_id == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS || type_id == SDR_TYPE_POST_PROCESS_FXAA) {
		// ignore looking for variants. main post process, lightshafts, and FXAA shaders need special headers to be hacked in
		opengl_post_shader_header(sflags, type_id, flags);
	}
	else {
		for (int i = 0; i < GL_num_shader_variants; ++i) {
			opengl_shader_variant_t &variant = GL_shader_variants[i];

			if (type_id == variant.type_id && flags & variant.flag) {
				sflags << "#define " << variant.flag_text << "\n";
			}
		}
	}

	return sflags.str();
}

/**
 * Load a shader file from disc or from the builtin defaults in def_files.cpp if none can be found.
 * This function will also create a list of preprocessor defines for the GLSL compiler based on the shader flags
 * and the supported GLSL version as reported by the GPU driver.
 *
 * @param shader	shader_type enum defined with which shader we're loading
 * @param filename	C-string holding the filename (with extension) of the shader file
 * @param flags		integer variable holding a combination of SDR_* flags
 * @return			C-string holding the complete shader source code
 */
static SCP_string opengl_load_shader(const char *filename)
{
	SCP_string content;
	if (Enable_external_shaders) {
		CFILE *cf_shader = cfopen(filename, "rt", CFILE_NORMAL, CF_TYPE_EFFECTS);

		if (cf_shader != NULL) {
			int len = cfilelength(cf_shader);
			content.resize(len);

			cfread(&content[0], len + 1, 1, cf_shader);
			cfclose(cf_shader);

			return content;
		}
	}

	//If we're still here, proceed with internals
	mprintf(("   Loading built-in default shader for: %s\n", filename));
	auto def_shader = defaults_get_file(filename);
	content.assign(reinterpret_cast<const char*>(def_shader.data), def_shader.size);

	return content;
}

static SCP_vector<SCP_string> opengl_get_shader_content(shader_type type_id, const char* filename, int flags, shader_stage stage) {
	SCP_vector<SCP_string> parts;
	parts.push_back(opengl_shader_get_header(type_id, flags, stage));

	parts.push_back(opengl_load_shader(filename));

	return parts;
}

/**
 * Compiles a new shader, and creates an opengl_shader_t that will be put into the GL_shader vector
 * if compilation is successful.
 *
 * @param sdr		Identifier defined with the program we wish to compile
 * @param flags		Combination of SDR_* flags
 */
int opengl_compile_shader(shader_type sdr, uint flags)
{
	int sdr_index = -1;
	int empty_idx;
	opengl_shader_t new_shader;

	Assert(sdr < NUM_SHADER_TYPES);

	opengl_shader_type_t *sdr_info = &GL_shader_types[sdr];

	Assert(sdr_info->type_id == sdr);
	mprintf(("Compiling new shader:\n"));
	mprintf(("	%s\n", sdr_info->description));

	// figure out if the variant requested needs a geometry shader
	bool use_geo_sdr = false;

	// do we even have a geometry shader?
	if ( sdr_info->geo != NULL ) {
		for (int i = 0; i < GL_num_shader_variants; ++i) {
			opengl_shader_variant_t *variant = &GL_shader_variants[i];

			if (variant->type_id == sdr && flags & variant->flag && variant->use_geometry_sdr) {
				use_geo_sdr = true;
				break;
			}
		}
	}

	auto vertex_content = opengl_get_shader_content(sdr_info->type_id, sdr_info->vert, flags, SDR_STAGE_VERTEX);
	auto fragment_content = opengl_get_shader_content(sdr_info->type_id, sdr_info->frag, flags, SDR_STAGE_FRAGMENT);
	SCP_vector<SCP_string> geom_content;

	if ( use_geo_sdr ) {
		// read geometry shader
		geom_content = opengl_get_shader_content(sdr_info->type_id, sdr_info->geo, flags, SDR_STAGE_GEOMETRY);

		Current_geo_sdr_params = &sdr_info->geo_sdr_info;
	}

	new_shader.program_id = opengl_shader_create(vertex_content, fragment_content, geom_content);

	if (!new_shader.program_id) {
		return -1;
	}

	new_shader.shader = sdr_info->type_id;
	new_shader.flags = flags;

	opengl_shader_set_current(&new_shader);

	// bind fragment data locations
	if ( GL_version >= 32 && GLSL_version >= 150 ) {
		glBindFragDataLocation(new_shader.program_id, 0, "fragOut0");
		glBindFragDataLocation(new_shader.program_id, 1, "fragOut1");
		glBindFragDataLocation(new_shader.program_id, 2, "fragOut2");
		glBindFragDataLocation(new_shader.program_id, 3, "fragOut3");
		glBindFragDataLocation(new_shader.program_id, 4, "fragOut4");
	}

	// initialize uniforms and attributes
	for ( auto& unif : sdr_info->uniforms ) {
		opengl_shader_init_uniform( unif );
	}

	for ( auto& attr : sdr_info->attributes ) {
		opengl_shader_init_attribute(&GL_vertex_attrib_info[attr]);
	}

	// if this shader is POST_PROCESS_MAIN, hack in the user-defined flags
	if ( sdr_info->type_id == SDR_TYPE_POST_PROCESS_MAIN ) {
		opengl_post_init_uniforms(flags);
	}

	mprintf(("Shader Variant Features:\n"));

	// initialize all uniforms and attributes that are specific to this variant
	for ( int i = 0; i < GL_num_shader_variants; ++i ) {
		opengl_shader_variant_t &variant = GL_shader_variants[i];

		if ( sdr_info->type_id == variant.type_id && variant.flag & flags ) {
			for (auto& unif : variant.uniforms) {
				opengl_shader_init_uniform(unif);
			}

			for (auto& attrs : variant.attributes) {
				opengl_shader_init_attribute(&GL_vertex_attrib_info[attrs]);
			}

			mprintf(("	%s\n", variant.description));
		}
	}

	opengl_shader_set_current();

	// add it to our list of embedded shaders
	// see if we have empty shader slots
	empty_idx = -1;
	for ( int i = 0; i < (int)GL_shader.size(); ++i ) {
		if ( GL_shader[i].shader == NUM_SHADER_TYPES ) {
			empty_idx = i;
			break;
		}
	}

	// then insert it at an empty slot or at the end
	if ( empty_idx >= 0 ) {
		GL_shader[empty_idx] = new_shader;
		sdr_index = empty_idx;
	} else {
		sdr_index = (int)GL_shader.size();
		GL_shader.push_back(new_shader);
	}

	return sdr_index;
}

/**
 * Initializes the shader system. Creates a 1x1 texture that can be used as a fallback texture when framebuffer support is missing.
 * Also compiles the shaders used for particle rendering.
 */
void opengl_shader_init()
{
	glGenTextures(1,&Framebuffer_fallback_texture_id);
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Framebuffer_fallback_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	GLuint pixels[4] = {0,0,0,0};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixels);

	GL_shader.clear();
	
	// Reserve 32 shader slots. This should cover most use cases in real life.
	GL_shader.reserve(32);

	// compile effect shaders
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, 0);
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, SDR_FLAG_PARTICLE_POINT_GEN);
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0);

	// compile deferred lighting shaders
	opengl_shader_compile_deferred_light_shader();

	// compile passthrough shader
	opengl_shader_compile_passthrough_shader();

	mprintf(("\n"));

	opengl_shader_compile_passthrough_shader();
}

/**
 * Retrieve the compilation log for a given shader object, and store it in the GLshader_info_log global variable
 *
 * @param shader_object		OpenGL handle of a shader object
 */
void opengl_shader_check_info_log(GLuint shader_object)
{
	if (GLshader_info_log == NULL) {
		GLshader_info_log = (char *) vm_malloc(GLshader_info_log_size);
	}

	memset(GLshader_info_log, 0, GLshader_info_log_size);

	glGetShaderInfoLog(shader_object, GLshader_info_log_size-1, 0, GLshader_info_log);
}

/**
* Retrieve the compilation log for a given shader object, and store it in the GLshader_info_log global variable
*
* @param program_object		OpenGL handle of a shader object
*/
void opengl_program_check_info_log(GLuint program_object)
{
	if (GLshader_info_log == NULL) {
		GLshader_info_log = (char *)vm_malloc(GLshader_info_log_size);
	}

	memset(GLshader_info_log, 0, GLshader_info_log_size);

	glGetProgramInfoLog(program_object, GLshader_info_log_size - 1, 0, GLshader_info_log);
}

/**
 * Pass a GLSL shader source to OpenGL and compile it into a usable shader object.
 * Prints compilation errors (if any) to the log.
 * Note that this will only compile shaders into objects, linking them into executables happens later
 *
 * @param shader_source		GLSL sourcecode for the shader
 * @param shader_type		OpenGL ID for the type of shader being used, like GL_FRAGMENT_SHADER_ARB, GL_VERTEX_SHADER_ARB
 * @return 					OpenGL handle for the compiled shader object
 */
GLuint opengl_shader_compile_object(const SCP_vector<SCP_string>& shader_source, GLenum shader_type)
{
	GLuint shader_object = 0;
	GLint status = 0;

	SCP_vector<const GLcharARB*> sources;
	sources.reserve(shader_source.size());
	for (auto it = shader_source.begin(); it != shader_source.end(); ++it) {
		sources.push_back(it->c_str());
	}

	shader_object = glCreateShader(shader_type);

	glShaderSource(shader_object, static_cast<GLsizei>(sources.size()), &sources[0], NULL);
	glCompileShader(shader_object);

	// check if the compile was successful
	glGetShaderiv(shader_object, GL_COMPILE_STATUS, &status);

	opengl_shader_check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		// basic error check
		mprintf(("%s shader failed to compile:\n%s\n", (shader_type == GL_VERTEX_SHADER) ? "Vertex" : ((shader_type == GL_GEOMETRY_SHADER) ? "Geometry" : "Fragment"), GLshader_info_log));

		// this really shouldn't exist, but just in case
		if (shader_object) {
			glDeleteProgram(shader_object);
		}

		return 0;
	}

	// we succeeded, maybe output warnings too
	if (strlen(GLshader_info_log) > 5) {
		nprintf(("SHADER-DEBUG", "%s shader compiled with warnings:\n%s\n", (shader_type == GL_VERTEX_SHADER) ? "Vertex" : ((shader_type == GL_GEOMETRY_SHADER) ? "Geometry" : "Fragment"), GLshader_info_log));
	}

	return shader_object;
}

/**
 * Link vertex shader, fragment shader and geometry shader objects into a
 * usable shader executable.
 *
 * Prints linker errors (if any) to the log.
 * 
 * @param vertex_object		Compiled vertex shader object
 * @param fragment_object	Compiled fragment shader object
 * @param geometry_object	Compiled geometry shader object
 * @return			Shader executable
 */
GLuint opengl_shader_link_object(GLuint vertex_object, GLuint fragment_object, GLuint geometry_object)
{
	GLuint shader_object = 0;
	GLint status = 0;

	shader_object = glCreateProgram();

	if (vertex_object) {
		glAttachShader(shader_object, vertex_object);
	}

	if (fragment_object) {
		glAttachShader(shader_object, fragment_object);
	}

	if (geometry_object) {
		glAttachShader(shader_object, geometry_object);
	}

	for ( int i = 0; i < opengl_vert_attrib::NUM_ATTRIBS; ++i ) {
		// assign vert attribute binding locations before we link the shader
		glBindAttribLocation(shader_object, i, GL_vertex_attrib_info[i].name.c_str());
	}

	glLinkProgram(shader_object);

	// check if the link was successful
	glGetProgramiv(shader_object, GL_LINK_STATUS, &status);

	opengl_program_check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		mprintf(("Shader failed to link:\n%s\n", GLshader_info_log));

		if (shader_object) {
			glDeleteProgram(shader_object);
		}

		return 0;
	}

	// we succeeded, maybe output warnings too
	if (strlen(GLshader_info_log) > 5) {
		nprintf(("SHADER-DEBUG", "Shader linked with warnings:\n%s\n", GLshader_info_log));
	}

	return shader_object;
}

/**
 * Creates an executable shader.
 *
 * @param vs	Vertex shader source code
 * @param fs	Fragment shader source code
 * @param gs	Geometry shader source code
 * @return 	Internal ID of compiled and linked shader generated by OpenGL
 */
GLuint opengl_shader_create(const SCP_vector<SCP_string>& vs, const SCP_vector<SCP_string>& fs, const SCP_vector<SCP_string>& gs)
{
	GLuint vs_o = 0;
	GLuint fs_o = 0;
	GLuint gs_o = 0;
	GLuint program = 0;

	if (!vs.empty()) {
		vs_o = opengl_shader_compile_object( vs, GL_VERTEX_SHADER );

		if ( !vs_o ) {
			mprintf(("ERROR! Unable to create vertex shader!\n"));
			goto Done;
		}
	}

	if (!fs.empty()) {
		fs_o = opengl_shader_compile_object( fs, GL_FRAGMENT_SHADER );

		if ( !fs_o ) {
			mprintf(("ERROR! Unable to create fragment shader!\n"));
			goto Done;
		}
	}

	if (!gs.empty()) {
		gs_o = opengl_shader_compile_object( gs, GL_GEOMETRY_SHADER );

		if ( !gs_o ) {
			mprintf(("ERROR! Unable to create fragment shader!\n"));
			goto Done;
		}
	}

	program = opengl_shader_link_object(vs_o, fs_o, gs_o);

	if ( !program ) {
		mprintf(("ERROR! Unable to create shader program!\n"));
	}

Done:
	if (vs_o) {
		glDeleteShader(vs_o);
	}

	if (fs_o) {
		glDeleteShader(fs_o);
	}

	if (gs_o) {
		glDeleteShader(gs_o);
	}

	return program;
}

/**
 * Initialize a shader attribute. Requires that the Current_shader global variable is valid.
 *
 * @param attribute_text	Name of the attribute to be initialized
 */
void opengl_shader_init_attribute(const opengl_vert_attrib *attrib_info)
{
	opengl_shader_uniform_t new_attribute;

	if ( (Current_shader == NULL) || attrib_info->name.empty() ) {
		Int3();
		return;
	}

	new_attribute.text_id = attrib_info->name;
	new_attribute.location = glGetAttribLocation(Current_shader->program_id, attrib_info->name.c_str());

	if ( new_attribute.location < 0 ) {
		nprintf(("SHADER-DEBUG", "WARNING: Unable to get shader attribute location for \"%s\"!\n", attrib_info->name.c_str()));
		return;
	}

	// assign default value to vertex attribute
	glVertexAttrib4f(
		new_attribute.location,
		attrib_info->default_value.xyzw.x,
		attrib_info->default_value.xyzw.y,
		attrib_info->default_value.xyzw.z,
		attrib_info->default_value.xyzw.w
	);

	Current_shader->attributes.push_back(new_attribute);
}

/**
 * Get the internal OpenGL location for a given attribute. Requires that the Current_shader global variable is valid
 *
 * @param attribute_text	Name of the attribute
 * @return					Internal OpenGL location for the attribute
 */
GLint opengl_shader_get_attribute(const char *attribute_text)
{
	if ( (Current_shader == NULL) || (attribute_text == NULL) ) {
		Int3();
		return -1;
	}

	SCP_vector<opengl_shader_uniform_t>::iterator attribute;

	for (attribute = Current_shader->attributes.begin(); attribute != Current_shader->attributes.end(); ++attribute) {
		if ( !attribute->text_id.compare(attribute_text) ) {
			return attribute->location;
		}
	}

	return -1;
}

/**
 * Initialize a shader uniform. Requires that the Current_shader global variable is valid.
 *
 * @param uniform_text		Name of the uniform to be initialized
 */
void opengl_shader_init_uniform(const char *uniform_text)
{
	opengl_shader_uniform_t new_uniform;

	if ( (Current_shader == NULL) || (uniform_text == NULL) ) {
		Int3();
		return;
	}

	new_uniform.text_id = uniform_text;
	new_uniform.location = glGetUniformLocation(Current_shader->program_id, uniform_text);

	if (new_uniform.location < 0) {
		nprintf(("SHADER-DEBUG", "WARNING: Unable to get shader uniform location for \"%s\"!\n", uniform_text));
		return;
	}

	Current_shader->uniforms.push_back( new_uniform );
}

/**
 * Get the internal OpenGL location for a given uniform. Requires that the Current_shader global variable is valid
 *
 * @param uniform_text	Name of the uniform
 * @return				Internal OpenGL location for the uniform
 */
GLint opengl_shader_get_uniform(const char *uniform_text)
{
	if ( (Current_shader == NULL) || (uniform_text == NULL) ) {
		Int3();
		return -1;
	}

	SCP_vector<opengl_shader_uniform_t>::iterator uniform;
	SCP_vector<opengl_shader_uniform_t>::iterator uniforms_end = Current_shader->uniforms.end();
	
	for (uniform = Current_shader->uniforms.begin(); uniform != uniforms_end; ++uniform) {
		if ( !uniform->text_id.compare(uniform_text) ) {
			return uniform->location;
		}
	}

	return -1;
}

/**
 * Initialize a shader uniform. Requires that the Current_shader global variable is valid.
 *
 * @param uniform_text		Name of the uniform to be initialized
 */
void opengl_shader_init_uniform_block(const char *uniform_text)
{
	opengl_shader_uniform_t new_uniform_block;

	if ( (Current_shader == NULL) || (uniform_text == NULL) ) {
		Int3();
		return;
	}

	new_uniform_block.text_id = uniform_text;
#ifdef __APPLE__
	new_uniform_block.location = glGetUniformBlockIndex((long)Current_shader->program_id, uniform_text);
#else
	new_uniform_block.location = glGetUniformBlockIndex(Current_shader->program_id, uniform_text);
#endif
	if (new_uniform_block.location < 0) {
		nprintf(("SHADER-DEBUG", "WARNING: Unable to get shader uniform block location for \"%s\"!\n", uniform_text));
		return;
	}

	Current_shader->uniform_blocks.push_back( new_uniform_block );
}

/**
 * Get the internal OpenGL location for a given uniform. Requires that the Current_shader global variable is valid
 *
 * @param uniform_text	Name of the uniform
 * @return				Internal OpenGL location for the uniform
 */
GLint opengl_shader_get_uniform_block(const char *uniform_text)
{
	if ( (Current_shader == NULL) || (uniform_text == NULL) ) {
		Int3();
		return -1;
	}

	SCP_vector<opengl_shader_uniform_t>::iterator uniform_block;
	
	for (uniform_block = Current_shader->uniform_blocks.begin(); uniform_block != Current_shader->uniform_blocks.end(); ++uniform_block) {
		if ( !uniform_block->text_id.compare(uniform_text) ) {
			return uniform_block->location;
		}
	}

	return -1;
}

/**
 * Compile the deferred light shader and the clear shader.
 */
void opengl_shader_compile_deferred_light_shader()
{
	bool in_error = false;

	int sdr_handle = gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_LIGHTING, 0);
	
	if ( sdr_handle >= 0 ) {
		opengl_shader_set_current(sdr_handle);

		GL_state.Uniform.setUniformi("ColorBuffer", 0);
		GL_state.Uniform.setUniformi("NormalBuffer", 1);
		GL_state.Uniform.setUniformi("PositionBuffer", 2);
		GL_state.Uniform.setUniformi("SpecBuffer", 3);
		GL_state.Uniform.setUniformf("invScreenWidth", 1.0f / gr_screen.max_w);
		GL_state.Uniform.setUniformf("invScreenHeight", 1.0f / gr_screen.max_h);
		GL_state.Uniform.setUniformf("specFactor", Cmdline_ogl_spec);
	} else {
		opengl_shader_set_current();
		mprintf(("Failed to compile deferred lighting shader!\n"));
		in_error = true;
	}

	if ( gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_CLEAR, 0) < 0 ) {
		mprintf(("Failed to compile deferred lighting buffer clear shader!\n"));
		in_error = true;
	}

	if ( in_error ) {
		mprintf(("  Shader in_error! Disabling deferred lighting!\n"));
		Cmdline_no_deferred_lighting = 1;
	}
}

void opengl_shader_compile_passthrough_shader()
{
	bool in_error = false;

	mprintf(("Compiling passthrough shader...\n"));

	int sdr_handle = gr_opengl_maybe_create_shader(SDR_TYPE_PASSTHROUGH_RENDER, 0);

	if ( sdr_handle >= 0 ) {
		opengl_shader_set_current(sdr_handle);

		//Hardcoded Uniforms
		GL_state.Uniform.setUniformi("baseMap", 0);
		GL_state.Uniform.setUniformi("noTexturing", 0);
		GL_state.Uniform.setUniformi("alphaTexture", 0);
		GL_state.Uniform.setUniformi("srgb", 0);
	} else {
		opengl_shader_set_current();
		mprintf(("Failed to compile passthrough shader!\n"));
		in_error = true;
	}

	if ( in_error ) {
		mprintf(("  Shader in_error! Passthrough shader unavailable!\n"));
	}

	opengl_shader_set_current();
}

void opengl_shader_set_passthrough(bool textured, bool alpha, color *clr, float color_scale)
{
	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_PASSTHROUGH_RENDER, 0));

	if ( textured ) {
		GL_state.Uniform.setUniformi("noTexturing", 0);
	} else {
		GL_state.Uniform.setUniformi("noTexturing", 1);
	}

	if ( alpha ) {
		GL_state.Uniform.setUniformi("alphaTexture", 1);
	} else {
		GL_state.Uniform.setUniformi("alphaTexture", 0);
	}

 	if ( High_dynamic_range ) {
 		GL_state.Uniform.setUniformi("srgb", 1);
		GL_state.Uniform.setUniformf("intensity", color_scale);
 	} else {
 		GL_state.Uniform.setUniformi("srgb", 0);
		GL_state.Uniform.setUniformf("intensity", 1.0f);
 	}

	GL_state.Uniform.setUniformf("alphaThreshold", GL_alpha_threshold);

	if ( clr != NULL ) {
		GL_state.Uniform.setUniform4f("color", i2fl(clr->red) / 255.0f, i2fl(clr->green) / 255.0f, i2fl(clr->blue) / 255.0f, i2fl(clr->alpha) / 255.0f);
	} else {
		GL_state.Uniform.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	}

	GL_state.Uniform.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	GL_state.Uniform.setUniformMatrix4f("projMatrix", GL_projection_matrix);
}
