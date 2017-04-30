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
#include "graphics/opengl/gropengldraw.h"
#include "graphics/opengl/gropengllight.h"
#include "graphics/opengl/gropenglpostprocessing.h"
#include "graphics/opengl/gropenglshader.h"
#include "graphics/opengl/gropenglstate.h"
#include "graphics/opengl/gropengltexture.h"
#include "graphics/opengl/gropengltnl.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "render/3d.h"

#include <md5.h>
#include <jansson.h>

SCP_vector<opengl_shader_t> GL_shader;

GLuint Framebuffer_fallback_texture_id = 0;

opengl_vert_attrib GL_vertex_attrib_info[] =
	{
		{ opengl_vert_attrib::POSITION,		"vertPosition",		{{{ 0.0f, 0.0f, 0.0f, 1.0f }}} },
		{ opengl_vert_attrib::COLOR,		"vertColor",		{{{ 1.0f, 1.0f, 1.0f, 1.0f }}} },
		{ opengl_vert_attrib::TEXCOORD,		"vertTexCoord",		{{{ 1.0f, 1.0f, 1.0f, 1.0f }}} },
		{ opengl_vert_attrib::NORMAL,		"vertNormal",		{{{ 0.0f, 0.0f, 1.0f, 0.0f }}} },
		{ opengl_vert_attrib::TANGENT,		"vertTangent",		{{{ 1.0f, 0.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::MODEL_ID,		"vertModelID",		{{{ 0.0f, 0.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::RADIUS,		"vertRadius",		{{{ 1.0f, 0.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::UVEC,			"vertUvec",			{{{ 0.0f, 1.0f, 0.0f, 0.0f }}} }
	};

/**
 * Static lookup reference for shader uniforms
 * When adding a new shader, list all associated uniforms and attributes here
 */
static opengl_shader_type_t GL_shader_types[] = {
	{ SDR_TYPE_MODEL, "main-v.sdr", "main-f.sdr", "main-g.sdr", 
		{ "modelViewMatrix", "modelMatrix", "viewMatrix", "projMatrix", "textureMatrix", "color" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::NORMAL, opengl_vert_attrib::TANGENT, opengl_vert_attrib::MODEL_ID }, "Model Rendering" },

	{ SDR_TYPE_EFFECT_PARTICLE, "effect-v.sdr", "effect-particle-f.sdr", "effect-screen-g.sdr", 
		{ "modelViewMatrix", "projMatrix", "baseMap", "depthMap", "window_width", "window_height", "nearZ", "farZ", "linear_depth", "srgb", "blend_alpha" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::RADIUS, opengl_vert_attrib::COLOR }, "Particle Effects" },

	{ SDR_TYPE_EFFECT_DISTORTION, "effect-distort-v.sdr", "effect-distort-f.sdr", 0, 
		{ "modelViewMatrix", "projMatrix", "baseMap", "window_width", "window_height", "distMap", "frameBuffer", "use_offset" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::RADIUS, opengl_vert_attrib::COLOR }, "Distortion Effects" },

	{ SDR_TYPE_POST_PROCESS_MAIN, "post-v.sdr", "post-f.sdr", 0, 
		{ "tex", "depth_tex", "timer" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Post Processing" },

	{ SDR_TYPE_POST_PROCESS_BLUR, "post-v.sdr", "blur-f.sdr", 0, 
		{ "tex", "texSize", "level", "tapSize", "debug" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Gaussian Blur" },

	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP, "post-v.sdr", "bloom-comp-f.sdr", 0, 
		{ "bloomed", "bloom_intensity", "levels" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Bloom Compositing" },

	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS, "post-v.sdr", "brightpass-f.sdr", 0, 
		{ "tex" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Bloom Brightpass" },

	{ SDR_TYPE_POST_PROCESS_FXAA, "fxaa-v.sdr", "fxaa-f.sdr", 0, 
		{ "tex0", "rt_w", "rt_h" },
		{ opengl_vert_attrib::POSITION }, "FXAA" },

	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS, "post-v.sdr", "fxaapre-f.sdr", 0, 
		{ "tex" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "FXAA Prepass" },

	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, "post-v.sdr", "ls-f.sdr", 0, 
		{ "scene", "cockpit", "sun_pos", "weight", "intensity", "falloff", "density", "cp_intensity" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Lightshafts" },

	{ SDR_TYPE_POST_PROCESS_TONEMAPPING, "post-v.sdr", "tonemapping-f.sdr", 0, 
		{ "tex", "exposure" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Tonemapping" },

	{ SDR_TYPE_DEFERRED_LIGHTING, "deferred-v.sdr", "deferred-f.sdr", 0, 
		{ "modelViewMatrix", "projMatrix", "scale", "ColorBuffer", "NormalBuffer", "PositionBuffer", "SpecBuffer", "invScreenWidth", "invScreenHeight", "lightType", "lightRadius",
		"diffuseLightColor", "specLightColor", "dualCone", "coneDir", "coneAngle", "coneInnerAngle", "specFactor" }, 
		{ opengl_vert_attrib::POSITION }, "Deferred Lighting" },
	
	{ SDR_TYPE_DEFERRED_CLEAR, "deferred-clear-v.sdr", "deferred-clear-f.sdr", 0, 
		{  },
		{ opengl_vert_attrib::POSITION }, "Clear Deferred Lighting Buffer" },

	{ SDR_TYPE_VIDEO_PROCESS, "video-v.sdr", "video-f.sdr", 0, 
		{ "modelViewMatrix", "projMatrix", "ytex", "utex", "vtex" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Video Playback" },

	{ SDR_TYPE_PASSTHROUGH_RENDER, "passthrough-v.sdr", "passthrough-f.sdr", 0,
		{ "modelViewMatrix", "projMatrix", "baseMap", "noTexturing", "alphaTexture", "srgb", "intensity", "color", "alphaThreshold", "clipEnabled", "clipEquation", "modelMatrix" },
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::COLOR }, "Passthrough" },

	{ SDR_TYPE_SHIELD_DECAL, "shield-impact-v.sdr",	"shield-impact-f.sdr", 0,
		{ "modelViewMatrix", "projMatrix", "shieldMap", "shieldModelViewMatrix", "shieldProjMatrix", "hitNormal", "srgb", "color" }, 
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::NORMAL }, "Shield Decals" }
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
		{ "sBasemap", "desaturate", "blend_alpha", "overrideDiffuse", "diffuseClr" }, {  },
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
		{ "use_clip_plane", "clip_equation" }, {  },
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

/**
 * Set the currently active shader
 * @param shader_obj	Pointer to an opengl_shader_t object. This function calls glUseProgramARB with parameter 0 if shader_obj is NULL or if function is called without parameters, causing OpenGL to revert to fixed-function processing
 */
void opengl_shader_set_current(opengl_shader_t *shader_obj)
{
	if (Current_shader != shader_obj) {
		GR_DEBUG_SCOPE("Set shader");

		GL_state.Array.ResetVertexAttribs();

		if(shader_obj) {
			shader_obj->program->use();
		} else {
			GL_state.UseProgram(0);
		}

		Current_shader = shader_obj;
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

	GL_shader[sdr_handle].program.reset();

	GL_shader[sdr_handle].flags = 0;
	GL_shader[sdr_handle].flags2 = 0;
	GL_shader[sdr_handle].shader = NUM_SHADER_TYPES;
}

/**
 * Go through GL_shader and call glDeleteObject() for all created shaders, then clear GL_shader
 */
void opengl_shader_shutdown()
{
	GL_shader.clear();
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

static void add_shader_parts(MD5& md5, const SCP_vector<SCP_string>& parts) {
	for (auto& part : parts) {
		md5.update(part.c_str(), (MD5::size_type) part.size());
	}
}

static SCP_string get_shader_hash(const SCP_vector<SCP_string>& vert,
								  const SCP_vector<SCP_string>& geom_content,
								  const SCP_vector<SCP_string>& frag) {
	MD5 md5;
	add_shader_parts(md5, vert);
	add_shader_parts(md5, geom_content);
	add_shader_parts(md5, frag);

	// Add the attribute locations so that changes get detected
	for (int i = 0; i < opengl_vert_attrib::NUM_ATTRIBS; ++i) {
		md5.update(GL_vertex_attrib_info[i].name.c_str(), (MD5::size_type) GL_vertex_attrib_info[i].name.size());
		md5.update(reinterpret_cast<const char*>(&i), sizeof(i));
	}

	md5.finalize();

	return md5.hexdigest();
}

static bool do_shader_caching() {
	if (GL_version < 41) {
		// Not supported until OpenGL 4.1
		return false;
	}
	if (Cmdline_noshadercache) {
		// Shader cache is disabled
		return false;
	}
	return true;
}

static bool load_cached_shader_binary(opengl::ShaderProgram* program, SCP_string hash) {
	if (!do_shader_caching()) {
		return false;
	}

	auto base_filename = SCP_string("ogl_shader-") + hash;

	auto metadata = base_filename + ".json";
	auto binary = base_filename + ".bin";

	auto metadata_fp = cfopen(metadata.c_str(), "rb", CFILE_NORMAL, CF_TYPE_CACHE);
	if (!metadata_fp) {
		nprintf(("ShaderCache", "Metadata file does not exist.\n"));
		return false;
	}

	auto size = cfilelength(metadata_fp);
	SCP_string metadata_content;
	metadata_content.resize((size_t) size);
	cfread(&metadata_content[0], 1, size, metadata_fp);

	cfclose(metadata_fp);

	auto metadata_root = json_loads(metadata_content.c_str(), 0, nullptr);
	if (!metadata_root) {
		mprintf(("Loading of cache metadata failed! Falling back to GLSL shader...\n"));
		return false;
	}

	json_int_t format;
	if (json_unpack(metadata_root, "{sI}", "format", &format) != 0) {
		mprintf(("Failed to unpack values from metadata JSON! Falling back to GLSL shader...\n"));
		return false;
	}
	auto binary_format = (GLenum) format;
	json_decref(metadata_root);

	auto binary_fp = cfopen(binary.c_str(), "rb", CFILE_NORMAL, CF_TYPE_CACHE);
	if (!binary_fp) {
		nprintf(("ShaderCache", "Binary file does not exist.\n"));
		return false;
	}
	
	GR_DEBUG_SCOPE("Loading cached shader");
	
	SCP_vector<uint8_t> buffer;
	int length = cfilelength(binary_fp);
	buffer.resize((size_t) length);
	cfread(&buffer[0], 1, length, binary_fp);

	cfclose(binary_fp);

	// Load the data!
	glProgramBinary(program->getShaderHandle(), binary_format, buffer.data(), (GLsizei) buffer.size());

	// Check the status...
	GLint status;
	glGetProgramiv(program->getShaderHandle(), GL_LINK_STATUS, &status);

	return status == GL_TRUE;
}
static int json_write_callback(const char *buffer, size_t size, void *data) {
	CFILE* cfp = (CFILE*)data;

	if ((size_t)cfwrite(buffer, 1, (int)size, cfp) != size) {
		return -1; // Error
	} else {
		return 0; // Success
	}
}

static void cache_program_binary(GLuint program, const SCP_string& hash) {
	if (!do_shader_caching()) {
		return;
	}
	
	GR_DEBUG_SCOPE("Saving shader binary");

	GLint size;
	glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &size);

	if (size <= 0) {
		// No binary available (I'm looking at you Mesa...)
		return;
	}

	SCP_vector<uint8_t> binary;
	binary.resize((size_t) size);
	GLenum binary_fmt;
	GLsizei length;
	glGetProgramBinary(program, (GLsizei) binary.size(), &length, &binary_fmt, binary.data());
	if (length == 0) {
		return;
	}

	auto base_filename = SCP_string("ogl_shader-") + hash;

	auto metadata_name = base_filename + ".json";
	auto binary_name = base_filename + ".bin";

	auto metadata_fp = cfopen(metadata_name.c_str(), "wb", CFILE_NORMAL, CF_TYPE_CACHE);
	if (!metadata_fp) {
		mprintf(("Could not open shader cache metadata file!\n"));
		return;
	}

	auto metadata = json_pack("{sI}", "format", (json_int_t)binary_fmt);
	if (json_dump_callback(metadata, json_write_callback, metadata_fp, 0) != 0) {
		mprintf(("Failed to write shader cache metadata file!\n"));
		cfclose(metadata_fp);
		return;
	}
	cfclose(metadata_fp);
	json_decref(metadata);

	auto binary_fp = cfopen(binary_name.c_str(), "wb", CFILE_NORMAL, CF_TYPE_CACHE);
	if (!binary_fp) {
		mprintf(("Could not open shader cache binary file!\n"));
		return;
	}
	cfwrite(binary.data(), 1, (int) binary.size(), binary_fp);
	cfclose(binary_fp);
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
	GR_DEBUG_SCOPE("Creating new shader");

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

	auto vert_content = opengl_get_shader_content(sdr_info->type_id, sdr_info->vert, flags, SDR_STAGE_VERTEX);
	auto frag_content = opengl_get_shader_content(sdr_info->type_id, sdr_info->frag, flags, SDR_STAGE_FRAGMENT);
	SCP_vector<SCP_string> geom_content;

	if (use_geo_sdr) {
		// read geometry shader
		geom_content = opengl_get_shader_content(sdr_info->type_id, sdr_info->geo, flags, SDR_STAGE_GEOMETRY);
	}

	auto shader_hash = get_shader_hash(vert_content, geom_content, frag_content);
	std::unique_ptr<opengl::ShaderProgram> program(new opengl::ShaderProgram(sdr_info->description));

	if (!load_cached_shader_binary(program.get(), shader_hash)) {
		GR_DEBUG_SCOPE("Compiling shader code");
		try {
			program->addShaderCode(opengl::STAGE_VERTEX, sdr_info->vert, vert_content);
			program->addShaderCode(opengl::STAGE_FRAGMENT, sdr_info->frag, frag_content);
			if (use_geo_sdr) {
				program->addShaderCode(opengl::STAGE_GEOMETRY, sdr_info->geo, geom_content);
			}

			for (int i = 0; i < opengl_vert_attrib::NUM_ATTRIBS; ++i) {
				// assign vert attribute binding locations before we link the shader
				glBindAttribLocation(program->getShaderHandle(), i, GL_vertex_attrib_info[i].name.c_str());
			}

			// bind fragment data locations before we link the shader
			glBindFragDataLocation(program->getShaderHandle(), 0, "fragOut0");
			glBindFragDataLocation(program->getShaderHandle(), 1, "fragOut1");
			glBindFragDataLocation(program->getShaderHandle(), 2, "fragOut2");
			glBindFragDataLocation(program->getShaderHandle(), 3, "fragOut3");
			glBindFragDataLocation(program->getShaderHandle(), 4, "fragOut4");

			if (do_shader_caching()) {
				// Enable shader caching
				glProgramParameteri(program->getShaderHandle(), GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
			}

			program->linkProgram();
		} catch (const std::exception&) {
			// Since all shaders are required a compilation failure is a fatal error
			Error(LOCATION, "A shader failed to compile! Check the debug log for more information.");
		}

		cache_program_binary(program->getShaderHandle(), shader_hash);
	}

	new_shader.shader = sdr_info->type_id;
	new_shader.flags = flags;
	new_shader.program = std::move(program);

	opengl_shader_set_current(&new_shader);

	// initialize uniforms and attributes
	for (auto& unif : sdr_info->uniforms) {
		new_shader.program->Uniforms.initUniform(unif);
	}

	for (auto& attr : sdr_info->attributes) {
		new_shader.program->initAttribute(GL_vertex_attrib_info[attr].name, GL_vertex_attrib_info[attr].default_value);
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
				new_shader.program->Uniforms.initUniform(unif);
			}

			for (auto& attr : variant.attributes) {
				auto& attr_info = GL_vertex_attrib_info[attr];
				new_shader.program->initAttribute(attr_info.name, attr_info.default_value);
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
		GL_shader[empty_idx] = std::move(new_shader);
		sdr_index = empty_idx;
	} else {
		sdr_index = (int)GL_shader.size();
		GL_shader.push_back(std::move(new_shader));
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

	gr_opengl_maybe_create_shader(SDR_TYPE_SHIELD_DECAL, 0);

	// compile deferred lighting shaders
	opengl_shader_compile_deferred_light_shader();

	// compile passthrough shader
	opengl_shader_compile_passthrough_shader();

	mprintf(("\n"));

	opengl_shader_compile_passthrough_shader();
}

/**
 * Get the internal OpenGL location for a given attribute. Requires that the Current_shader global variable is valid
 *
 * @param attribute_text	Name of the attribute
 * @return					Internal OpenGL location for the attribute
 */
GLint opengl_shader_get_attribute(const char *attribute_text)
{
	Assertion(Current_shader != nullptr, "Current shader may not be null!");
	Assertion(attribute_text != nullptr, "Attribute name must be valid!");

	return Current_shader->program->getAttributeLocation(attribute_text);
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

		Current_shader->program->Uniforms.setUniformi("ColorBuffer", 0);
		Current_shader->program->Uniforms.setUniformi("NormalBuffer", 1);
		Current_shader->program->Uniforms.setUniformi("PositionBuffer", 2);
		Current_shader->program->Uniforms.setUniformi("SpecBuffer", 3);
		Current_shader->program->Uniforms.setUniformf("invScreenWidth", 1.0f / gr_screen.max_w);
		Current_shader->program->Uniforms.setUniformf("invScreenHeight", 1.0f / gr_screen.max_h);
		Current_shader->program->Uniforms.setUniformf("specFactor", Cmdline_ogl_spec);
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
		Current_shader->program->Uniforms.setUniformi("baseMap", 0);
		Current_shader->program->Uniforms.setUniformi("noTexturing", 0);
		Current_shader->program->Uniforms.setUniformi("alphaTexture", 0);
		Current_shader->program->Uniforms.setUniformi("srgb", 0);
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

void opengl_shader_set_passthrough(bool textured, bool alpha, vec4 *clr, float color_scale, const material::clip_plane& clip_plane)
{
	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_PASSTHROUGH_RENDER, 0));

	if ( textured ) {
		Current_shader->program->Uniforms.setUniformi("noTexturing", 0);
	} else {
		Current_shader->program->Uniforms.setUniformi("noTexturing", 1);
	}

	if ( alpha ) {
		Current_shader->program->Uniforms.setUniformi("alphaTexture", 1);
	} else {
		Current_shader->program->Uniforms.setUniformi("alphaTexture", 0);
	}

	if ( High_dynamic_range ) {
		Current_shader->program->Uniforms.setUniformi("srgb", 1);
		Current_shader->program->Uniforms.setUniformf("intensity", color_scale);
	} else {
		Current_shader->program->Uniforms.setUniformi("srgb", 0);
		Current_shader->program->Uniforms.setUniformf("intensity", 1.0f);
	}

	Current_shader->program->Uniforms.setUniformf("alphaThreshold", GL_alpha_threshold);

	if ( clr != NULL ) {
		Current_shader->program->Uniforms.setUniform4f("color", *clr);
	} else {
		Current_shader->program->Uniforms.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (clip_plane.enabled) {
		Current_shader->program->Uniforms.setUniformi("clipEnabled", 1);

		vec4 clip_equation;
		clip_equation.xyzw.x = clip_plane.normal.xyz.x;
		clip_equation.xyzw.y = clip_plane.normal.xyz.y;
		clip_equation.xyzw.z = clip_plane.normal.xyz.z;
		clip_equation.xyzw.w = -vm_vec_dot(&clip_plane.normal, &clip_plane.position);

		Current_shader->program->Uniforms.setUniform4f("clipEquation", clip_equation);
		Current_shader->program->Uniforms.setUniformMatrix4f("modelMatrix", GL_model_matrix_stack.get_transform());
	} else {
		Current_shader->program->Uniforms.setUniformi("clipEnabled", 0);
	}

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);
}

void opengl_shader_set_passthrough(bool textured, bool alpha, color *clr)
{
	vec4 normalized_clr = {{{ i2fl(clr->red) / 255.0f,
		i2fl(clr->green) / 255.0f,
		i2fl(clr->blue) / 255.0f,
		clr->is_alphacolor ? i2fl(clr->alpha) / 255.0f : 1.0f }}};

	opengl_shader_set_passthrough(textured, alpha, &normalized_clr, 1.0f);
}
