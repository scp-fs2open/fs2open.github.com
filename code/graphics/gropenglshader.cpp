/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "cmdline/cmdline.h"
#include "globalincs/def_files.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglextension.h"
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

static int GL_anim_effect_num = 0;
static float GL_anim_timer = 0.0f;

geometry_sdr_params *Current_geo_sdr_params = NULL;

/**
 * Static lookup reference for shader uniforms
 * When adding a new shader, list all associated uniforms and attributes here
 */
static opengl_shader_type_t GL_shader_types[] = {
	{ SDR_TYPE_MODEL, "main-v.sdr", "main-f.sdr", "main-g.sdr", {GL_TRIANGLES, GL_TRIANGLE_STRIP, 3}, 
		0, {}, 0, {}, "Model Rendering" },

	{ SDR_TYPE_EFFECT_PARTICLE, "effect-v.sdr", "effect-particle-f.sdr", "effect-screen-g.sdr", {GL_POINTS, GL_TRIANGLE_STRIP, 4}, 
		7, { "baseMap", "depthMap", "window_width", "window_height", "nearZ", "farZ", "linear_depth" }, 1, {"radius"}, "Particle Effects" },

	{ SDR_TYPE_EFFECT_DISTORTION, "effect-distort-v.sdr", "effect-distort-f.sdr", 0, { 0, 0, 0 }, 
		6, { "baseMap", "window_width", "window_height", "distMap", "frameBuffer", "use_offset" }, 1, { "radius" }, "Distortion Effects" },

	{ SDR_TYPE_POST_PROCESS_MAIN, "post-v.sdr", "post-f.sdr", 0, {0, 0, 0}, 
		5, { "tex", "depth_tex", "timer", "bloomed", "bloom_intensity" }, 0, { NULL }, "Post Processing" },

	{ SDR_TYPE_POST_PROCESS_BLUR, "post-v.sdr", "blur-f.sdr", 0, {0, 0, 0}, 
		2, { "tex", "bsize", "debug" }, 0, { NULL }, "Gaussian Blur" },

	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS, "post-v.sdr", "brightpass-f.sdr", 0, { 0, 0, 0 },
		1, { "tex" }, 0, { NULL }, "Bloom Brightpass" },

	{ SDR_TYPE_POST_PROCESS_FXAA, "fxaa-v.sdr", "fxaa-f.sdr", 0, {0, 0, 0}, 
		3, { "tex0", "rt_w", "rt_h" }, 0, { NULL }, "FXAA" },

	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS, "post-v.sdr", "fxaapre-f.sdr", 0, {0, 0, 0}, 
		1, { "tex" }, 0, { NULL }, "FXAA Prepass" },

	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, "post-v.sdr", "ls-f.sdr", 0, {0, 0, 0}, 
		8, { "scene", "cockpit", "sun_pos", "weight", "intensity", "falloff", "density", "cp_intensity" }, 0, { NULL }, "Lightshafts" },

	{ SDR_TYPE_DEFERRED_LIGHTING, "deferred-v.sdr", "deferred-f.sdr", 0, { 0, 0, 0 }, 
		16, { "scale", "ColorBuffer", "NormalBuffer", "PositionBuffer", "SpecBuffer", "invScreenWidth", "invScreenHeight", "lightType", "lightRadius", "diffuseLightColor", 
		"specLightColor", "dualCone", "coneDir", "coneAngle", "coneInnerAngle", "specFactor" }, 0, { NULL }, "Deferred Lighting" },
	
	{ SDR_TYPE_DEFERRED_CLEAR, "deferred-clear-v.sdr", "deferred-clear-f.sdr", 0, {0, 0, 0}, 
		0, { NULL }, 0, { NULL }, "Clear Deferred Lighting Buffer" },

	{ SDR_TYPE_VIDEO_PROCESS, "video-v.sdr", "video-f.sdr", 0, {0, 0, 0}, 
	3, { "ytex", "utex", "vtex" }, 0, { NULL }, "Video Playback" }
};

/**
 * Static lookup reference for shader variant uniforms
 * When adding a new shader variant for a shader, list all associated uniforms and attributes here
 */
static opengl_shader_variant_t GL_shader_variants[] = {
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_LIGHT, "FLAG_LIGHT", 
		2, { "n_lights", "light_factor" }, 0, { NULL }, 
		"Lighting" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_FOG, "FLAG_FOG", 
		0, { NULL }, 0, { NULL }, 
		"Fog Effect" },

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_DIFFUSE_MAP, "FLAG_DIFFUSE_MAP", 
		4, { "sBasemap", "desaturate", "desaturate_clr", "blend_alpha" }, 0, { NULL }, 
		"Diffuse Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_GLOW_MAP, "FLAG_GLOW_MAP", 
		1, { "sGlowmap" }, 0, { NULL }, 
		"Glow Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_SPEC_MAP, "FLAG_SPEC_MAP", 
		1, { "sSpecmap" }, 0, { NULL }, 
		"Specular Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_NORMAL_MAP, "FLAG_NORMAL_MAP", 
		1, { "sNormalmap" }, 0, { NULL }, 
		"Normal Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_HEIGHT_MAP, "FLAG_HEIGHT_MAP", 
		1, { "sHeightmap" }, 0, { NULL }, 
		"Parallax Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ENV_MAP, "FLAG_ENV_MAP", 
		3, { "sEnvmap", "alpha_spec", "envMatrix" }, 0, { NULL }, 
		"Environment Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ANIMATED, "FLAG_ANIMATED", 
		5, { "sFramebuffer", "effect_num", "anim_timer", "vpwidth", "vpheight" }, 0, { NULL }, 
		"Animated Effects" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_MISC_MAP, "FLAG_MISC_MAP", 
		1, { "sMiscmap" }, 0, { NULL }, 
		"Utility mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_TEAMCOLOR, "FLAG_TEAMCOLOR", 
		2, { "stripe_color", "base_color" }, 0, { NULL }, 
		"Team Colors" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_DEFERRED, "FLAG_DEFERRED", 
		0, { NULL }, 0, { NULL }, 
		"Deferred lighting" },
	
	{ SDR_TYPE_MODEL, true, SDR_FLAG_MODEL_SHADOW_MAP, "FLAG_SHADOW_MAP", 
		1, { "shadow_proj_matrix" }, 0, { NULL }, 
		"Shadow Mapping" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_SHADOWS, "FLAG_SHADOWS", 
		8, { "shadow_map", "shadow_mv_matrix", "shadow_proj_matrix", "model_matrix", "veryneardist", "neardist", "middist", "fardist" }, 0, { NULL }, 
		"Shadows" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_THRUSTER, "FLAG_THRUSTER", 
		1, { "thruster_scale" }, 0, { NULL }, 
		"Thruster scaling" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_TRANSFORM, "FLAG_TRANSFORM", 
		2, { "transform_tex", "buffer_matrix_offset" }, 1, { "model_id" }, 
		"Submodel Transforms" },
	
	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_CLIP, "FLAG_CLIP", 
		4, { "use_clip_plane", "world_matrix", "clip_normal", "clip_position" }, 0, { NULL }, 
		"Clip Plane" },
	
	{ SDR_TYPE_EFFECT_PARTICLE, true, SDR_FLAG_PARTICLE_POINT_GEN, "FLAG_EFFECT_GEOMETRY", 
		0, { NULL }, 1, { "uvec" },
		"Geometry shader point-based particles" },
	
	{ SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_HORIZONTAL, "PASS_0", 
		0, { NULL }, 0, { NULL },
		"Horizontal blur pass" },
	
	{ SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_VERTICAL, "PASS_1", 
		0, { NULL }, 0, { NULL },
		"Vertical blur pass" }
};

static const int GL_num_shader_variants = sizeof(GL_shader_variants) / sizeof(opengl_shader_variant_t);

opengl_shader_t *Current_shader = NULL;

void opengl_shader_check_info_log(GLhandleARB shader_object);

/**
 * Set the currently active shader 
 * @param shader_obj	Pointer to an opengl_shader_t object. This function calls glUseProgramARB with parameter 0 if shader_obj is NULL or if function is called without parameters, causing OpenGL to revert to fixed-function processing 
 */
void opengl_shader_set_current(opengl_shader_t *shader_obj)
{
	if (shader_obj != NULL) {
		if(!Current_shader || (Current_shader->program_id != shader_obj->program_id)) {
			Current_shader = shader_obj;
			vglUseProgramObjectARB(Current_shader->program_id);
			GL_state.Uniform.reset();
#ifndef NDEBUG
			if ( opengl_check_for_errors("shader_set_current()") ) {
				vglValidateProgramARB(Current_shader->program_id);

				GLint obj_status = 0;
				vglGetObjectParameterivARB(Current_shader->program_id, GL_OBJECT_VALIDATE_STATUS_ARB, &obj_status);

				if ( !obj_status ) {
					opengl_shader_check_info_log(Current_shader->program_id);
	
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
		Current_shader = NULL;
		vglUseProgramObjectARB(0);
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
	if (Use_GLSL < 2)
		return -1;

	size_t idx;
	size_t max = GL_shader.size();

	for (idx = 0; idx < max; idx++) {
		if (GL_shader[idx].shader == shader_t && GL_shader[idx].flags == flags) {
			return idx;
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
		vglDeleteObjectARB(GL_shader[sdr_handle].program_id);
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

	if ( !Use_GLSL ) {
		return;
	}

	for (i = 0; i < GL_shader.size(); i++) {
		if (GL_shader[i].program_id) {
			vglDeleteObjectARB(GL_shader[i].program_id);
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
static char *opengl_load_shader(shader_type type_id, char *filename, int flags)
{
	SCP_string sflags;

#ifdef __APPLE__
    sflags += "#version 120\n";
#endif
    
	if (Use_GLSL >= 4) {
		sflags += "#define SHADER_MODEL 4\n";
	}
	else if (Use_GLSL == 3) {
		sflags += "#define SHADER_MODEL 3\n";
	}
	else {
		sflags += "#define SHADER_MODEL 2\n";
	}

#ifdef __APPLE__
	sflags += "#define APPLE\n";
#endif

	if (type_id == SDR_TYPE_POST_PROCESS_MAIN || type_id == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS || type_id == SDR_TYPE_POST_PROCESS_FXAA) {
		// ignore looking for variants. main post process, lightshafts, and FXAA shaders need special headers to be hacked in
		opengl_post_load_shader(sflags, type_id, flags);
	} else {
		for (int i = 0; i < GL_num_shader_variants; ++i) {
			opengl_shader_variant_t &variant = GL_shader_variants[i];

			if (type_id == variant.type_id && flags & variant.flag) {
				sflags += "#define " + variant.flag_text + "\n";
			}
		}
	}
	
	const char *shader_flags = sflags.c_str();
	int flags_len = strlen(shader_flags);

	if (Enable_external_shaders) {
		CFILE *cf_shader = cfopen(filename, "rt", CFILE_NORMAL, CF_TYPE_EFFECTS);

		if (cf_shader != NULL) {
			int len = cfilelength(cf_shader);
			char *shader_c = (char*)vm_malloc(len + flags_len + 1);

			strcpy(shader_c, shader_flags);
			memset(shader_c + flags_len, 0, len + 1);
			cfread(shader_c + flags_len, len + 1, 1, cf_shader);
			cfclose(cf_shader);

			return shader_c;
		}
	}

	//If we're still here, proceed with internals
	mprintf(("   Loading built-in default shader for: %s\n", filename));
	char* def_shader = defaults_get_file(filename);
	size_t len = strlen(def_shader);
	char *shader_c = (char*)vm_malloc(len + flags_len + 1);

	strcpy(shader_c, shader_flags);
	strcat(shader_c, def_shader);

	return shader_c;
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
	char *vert = NULL, *frag = NULL, *geom = NULL;
	opengl_shader_t new_shader;

	Assert(sdr < NUM_SHADER_TYPES);

	opengl_shader_type_t *sdr_info = &GL_shader_types[sdr];

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

	// read vertex shader
	if ((vert = opengl_load_shader(sdr_info->type_id, sdr_info->vert, flags)) == NULL) {
		goto Done;
	}
	
	// read fragment shader
	if ((frag = opengl_load_shader(sdr_info->type_id, sdr_info->frag, flags)) == NULL) {
		goto Done;
	}

	if ( use_geo_sdr ) {
		if (!Is_Extension_Enabled(OGL_EXT_GEOMETRY_SHADER4)) {
			goto Done;
		}

		// read geometry shader
		if ((geom = opengl_load_shader(sdr_info->type_id, sdr_info->geo, flags)) == NULL) {
			goto Done;
		}

		Current_geo_sdr_params = &sdr_info->geo_sdr_info;
	}

	Verify(vert != NULL);
	Verify(frag != NULL);

	new_shader.program_id = opengl_shader_create(vert, frag, geom);

	if (!new_shader.program_id) {
		goto Done;
	}

	new_shader.shader = sdr_info->type_id;
	new_shader.flags = flags;

	opengl_shader_set_current(&new_shader);

	// initialize uniforms and attributes
	for ( int i = 0; i < sdr_info->num_uniforms; ++i ) {
		opengl_shader_init_uniform( sdr_info->uniforms[i] );
	}

	for (int i = 0; i < sdr_info->num_attributes; ++i) {
		opengl_shader_init_attribute(sdr_info->attributes[i]);
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
			for (int j = 0; j < variant.num_uniforms; ++j) {
				opengl_shader_init_uniform(variant.uniforms[j]);
			}

			for (int j = 0; j < variant.num_attributes; ++j) {
				opengl_shader_init_attribute(variant.attributes[j]);
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
		sdr_index = GL_shader.size();
		GL_shader.push_back(new_shader);
	}

Done:
	if (vert != NULL) {
		vm_free(vert);
		vert = NULL;
	}

	if (frag != NULL) {
		vm_free(frag);
		frag = NULL;
	}

	if (geom != NULL ) {
		vm_free(geom);
		geom = NULL;
	}

	return sdr_index;
}

/**
 * Initializes the shader system. Creates a 1x1 texture that can be used as a fallback texture when framebuffer support is missing.
 * Also compiles the shaders used for particle rendering.
 */
void opengl_shader_init()
{
	if ( !Use_GLSL ) {
		return;
	}

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

	if (Cmdline_no_glsl_model_rendering) {
		Use_GLSL = 1;
	}

	GL_shader.clear();
	
	// Reserve 32 shader slots. This should cover most use cases in real life.
	GL_shader.reserve(32);

	// compile effect shaders
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, 0);
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, SDR_FLAG_PARTICLE_POINT_GEN);
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0);

	// compile deferred lighting shaders
	opengl_shader_compile_deferred_light_shader();
	mprintf(("\n"));
}

/**
 * Retrieve the compilation log for a given shader object, and store it in the GLshader_info_log global variable
 *
 * @param shader_object		OpenGL handle of a shader object
 */
void opengl_shader_check_info_log(GLhandleARB shader_object)
{
	if (GLshader_info_log == NULL) {
		GLshader_info_log = (char *) vm_malloc(GLshader_info_log_size);
	}

	memset(GLshader_info_log, 0, GLshader_info_log_size);

	vglGetInfoLogARB(shader_object, GLshader_info_log_size-1, 0, GLshader_info_log);
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
GLhandleARB opengl_shader_compile_object(const GLcharARB *shader_source, GLenum shader_type)
{
	GLhandleARB shader_object = 0;
	GLint status = 0;

	shader_object = vglCreateShaderObjectARB(shader_type);

	vglShaderSourceARB(shader_object, 1, &shader_source, NULL);
	vglCompileShaderARB(shader_object);

	// check if the compile was successful
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &status);

	opengl_shader_check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		// basic error check
		mprintf(("%s shader failed to compile:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : ((shader_type == GL_GEOMETRY_SHADER_EXT) ? "Geometry" : "Fragment"), GLshader_info_log));

		// this really shouldn't exist, but just in case
		if (shader_object) {
			vglDeleteObjectARB(shader_object);
		}

		return 0;
	}

	// we succeeded, maybe output warnings too
	if (strlen(GLshader_info_log) > 5) {
		nprintf(("SHADER-DEBUG", "%s shader compiled with warnings:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : ((shader_type == GL_GEOMETRY_SHADER_EXT) ? "Geometry" : "Fragment"), GLshader_info_log));
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
GLhandleARB opengl_shader_link_object(GLhandleARB vertex_object, GLhandleARB fragment_object, GLhandleARB geometry_object)
{
	GLhandleARB shader_object = 0;
	GLint status = 0;

	shader_object = vglCreateProgramObjectARB();

	if (vertex_object) {
		vglAttachObjectARB(shader_object, vertex_object);
	}

	if (fragment_object) {
		vglAttachObjectARB(shader_object, fragment_object);
	}

	if (geometry_object) {
		vglAttachObjectARB(shader_object, geometry_object);
		
		if ( Current_geo_sdr_params != NULL) {
#ifdef __APPLE__
			vglProgramParameteriEXT((long)shader_object, GL_GEOMETRY_INPUT_TYPE_EXT, Current_geo_sdr_params->input_type);
			vglProgramParameteriEXT((long)shader_object, GL_GEOMETRY_OUTPUT_TYPE_EXT, Current_geo_sdr_params->output_type);
			vglProgramParameteriEXT((long)shader_object, GL_GEOMETRY_VERTICES_OUT_EXT, Current_geo_sdr_params->vertices_out);
#else
			vglProgramParameteriEXT((GLuint)shader_object, GL_GEOMETRY_INPUT_TYPE_EXT, Current_geo_sdr_params->input_type);
			vglProgramParameteriEXT((GLuint)shader_object, GL_GEOMETRY_OUTPUT_TYPE_EXT, Current_geo_sdr_params->output_type);
			vglProgramParameteriEXT((GLuint)shader_object, GL_GEOMETRY_VERTICES_OUT_EXT, Current_geo_sdr_params->vertices_out);
#endif
		}
	}
	vglLinkProgramARB(shader_object);

	// check if the link was successful
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_LINK_STATUS_ARB, &status);

	opengl_shader_check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		mprintf(("Shader failed to link:\n%s\n", GLshader_info_log));

		if (shader_object) {
			vglDeleteObjectARB(shader_object);
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
GLhandleARB opengl_shader_create(const char *vs, const char *fs, const char *gs)
{
	GLhandleARB vs_o = 0;
	GLhandleARB fs_o = 0;
	GLhandleARB gs_o = 0;
	GLhandleARB program = 0;

	if (vs) {
		vs_o = opengl_shader_compile_object( (const GLcharARB*)vs, GL_VERTEX_SHADER_ARB );

		if ( !vs_o ) {
			mprintf(("ERROR! Unable to create vertex shader!\n"));
			goto Done;
		}
	}

	if (fs) {
		fs_o = opengl_shader_compile_object( (const GLcharARB*)fs, GL_FRAGMENT_SHADER_ARB );

		if ( !fs_o ) {
			mprintf(("ERROR! Unable to create fragment shader!\n"));
			goto Done;
		}
	}

	if (gs) {
		gs_o = opengl_shader_compile_object( (const GLcharARB*)gs, GL_GEOMETRY_SHADER_EXT );

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
		vglDeleteObjectARB(vs_o);
	}

	if (fs_o) {
		vglDeleteObjectARB(fs_o);
	}

	if (gs_o) {
		vglDeleteObjectARB(gs_o);
	}

	return program;
}

/**
 * Initialize a shader attribute. Requires that the Current_shader global variable is valid.
 *
 * @param attribute_text	Name of the attribute to be initialized
 */
void opengl_shader_init_attribute(const char *attribute_text)
{
	opengl_shader_uniform_t new_attribute;

	if ( ( Current_shader == NULL ) || ( attribute_text == NULL ) ) {
		Int3();
		return;
	}

	new_attribute.text_id = attribute_text;
	new_attribute.location = vglGetAttribLocationARB(Current_shader->program_id, attribute_text);

	if ( new_attribute.location < 0 ) {
		nprintf(("SHADER-DEBUG", "WARNING: Unable to get shader attribute location for \"%s\"!\n", attribute_text));
		return;
	}

	Current_shader->attributes.push_back( new_attribute );
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
	new_uniform.location = vglGetUniformLocationARB(Current_shader->program_id, uniform_text);

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
	new_uniform_block.location = vglGetUniformBlockIndexARB((long)Current_shader->program_id, uniform_text);
#else
	new_uniform_block.location = vglGetUniformBlockIndexARB(Current_shader->program_id, uniform_text);
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
 * Sets the currently active animated effect.
 *
 * @param effect	Effect ID, needs to be implemented and checked for in the shader
 * @param timer		Timer value to be passed to the shader
 */
void gr_opengl_shader_set_animated_effect(int effect, float timer)
{
	GL_anim_effect_num = effect;
	GL_anim_timer = timer;
}

/**
 * Returns the currently active animated effect ID.
 *
 * @return		Currently active effect ID
 */
int opengl_shader_get_animated_effect()
{
	return GL_anim_effect_num;
}

/**
 * Get the timer for animated effects.
 */
float opengl_shader_get_animated_timer()
{
	return GL_anim_timer;
}

/**
 * Compile the deferred light shader.
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
