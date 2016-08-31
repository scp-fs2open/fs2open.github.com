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

struct compiled_shader {
	std::unique_ptr<opengl::ShaderProgram> program;

	shader_type type;
	int flags;

	compiled_shader() : type(SDR_TYPE_NONE), flags(0) {}

	compiled_shader(compiled_shader&& other) {
		*this = std::move(other);
	}
	compiled_shader& operator=(compiled_shader&& other) {
		// VS2013 doesn't support implicit move constructors so we need to explicitly declare it
		type = other.type;
		flags = other.flags;

		program = std::move(other.program);

		return *this;
	}

	compiled_shader(const opengl_shader_t&) = delete;
	compiled_shader& operator=(const opengl_shader_t&) = delete;
};

SCP_vector<compiled_shader> Compiled_shaders;

SCP_vector<opengl_shader_t> GL_shader;

GLuint Framebuffer_fallback_texture_id = 0;

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
		{ "modelViewMatrix", "projMatrix", "baseMap", "noTexturing", "alphaTexture", "srgb", "intensity", "color", "alphaThreshold" },
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
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_LIGHT, "FLAG_LIGHT",
		{ "n_lights", "lightPosition", "lightDirection", "lightDiffuseColor", "lightSpecColor", "lightType", "lightAttenuation", "ambientFactor", "diffuseFactor", "emissionFactor", "defaultGloss" }, {  },
		"Lighting" },

	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_FOG, "FLAG_FOG",
		{ "fogStart", "fogScale", "fogColor" }, {  },
		"Fog Effect" },

	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_DIFFUSE_MAP, "FLAG_DIFFUSE_MAP",
		{ "sBasemap", "desaturate", "desaturate_clr", "blend_alpha", "overrideDiffuse", "diffuseClr" }, {  },
		"Diffuse Mapping" },
	
	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_GLOW_MAP, "FLAG_GLOW_MAP",
		{ "sGlowmap", "overrideGlow", "glowClr" }, {  },
		"Glow Mapping" },
	
	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_SPEC_MAP, "FLAG_SPEC_MAP",
		{ "sSpecmap", "overrideSpec", "specClr", "gammaSpec", "alphaGloss" }, {  },
		"Specular Mapping" },
	
	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_NORMAL_MAP, "FLAG_NORMAL_MAP",
		{ "sNormalmap" }, {  },
		"Normal Mapping" },
	
	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_HEIGHT_MAP, "FLAG_HEIGHT_MAP",
		{ "sHeightmap" }, {  },
		"Parallax Mapping" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_ENV_MAP, "FLAG_ENV_MAP",
		{ "sEnvmap", "envGloss", "envMatrix" }, {  },
		"Environment Mapping" },
	
	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_ANIMATED, "FLAG_ANIMATED",
		{ "sFramebuffer", "effect_num", "anim_timer", "vpwidth", "vpheight" }, {  },
		"Animated Effects" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_MISC_MAP, "FLAG_MISC_MAP",
		{ "sMiscmap" }, {  },
		"Utility mapping" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_TEAMCOLOR, "FLAG_TEAMCOLOR",
		{ "stripe_color", "base_color", "team_glow_enabled" }, {  },
		"Team Colors" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_DEFERRED, "FLAG_DEFERRED",
		{ }, {  },
		"Deferred lighting" },
	
	{ SDR_TYPE_MODEL, true, true, SDR_FLAG_MODEL_SHADOW_MAP, "FLAG_SHADOW_MAP",
		{ "shadow_proj_matrix" }, {  },
		"Shadow Mapping" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_SHADOWS, "FLAG_SHADOWS",
		{ "shadow_map", "shadow_mv_matrix", "shadow_proj_matrix", "veryneardist", "neardist", "middist", "fardist" }, { },
		"Shadows" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_THRUSTER, "FLAG_THRUSTER",
		{ "thruster_scale" }, {  },
		"Thruster scaling" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_TRANSFORM, "FLAG_TRANSFORM",
		{ "transform_tex", "buffer_matrix_offset" }, {  },
		"Submodel Transforms" },
	
	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_CLIP, "FLAG_CLIP",
		{ "use_clip_plane", "clip_normal", "clip_position" }, {  },
		"Clip Plane" },

	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_HDR, "FLAG_HDR",
		{ }, {  },
		"High Dynamic Range" },

	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_AMBIENT_MAP, "FLAG_AMBIENT_MAP",
		{ "sAmbientmap" }, {  },
		"Ambient Occlusion Map" },

	{ SDR_TYPE_MODEL, false, true, SDR_FLAG_MODEL_NORMAL_ALPHA, "FLAG_NORMAL_ALPHA",
		{ "normalAlphaMinMax" }, { },
		"Normal Alpha" },

	{ SDR_TYPE_MODEL, false, false, SDR_FLAG_MODEL_NORMAL_EXTRUDE, "FLAG_NORMAL_EXTRUDE",
		{ "extrudeWidth" }, { },
		"Normal Extrusion" },

	{ SDR_TYPE_EFFECT_PARTICLE, true, true, SDR_FLAG_PARTICLE_POINT_GEN, "FLAG_EFFECT_GEOMETRY",
		{ }, { opengl_vert_attrib::UVEC },
		"Geometry shader point-based particles" },
	
	{ SDR_TYPE_POST_PROCESS_BLUR, false, true, SDR_FLAG_BLUR_HORIZONTAL, "PASS_0",
		{ }, {  },
		"Horizontal blur pass" },
	
	{ SDR_TYPE_POST_PROCESS_BLUR, false, true, SDR_FLAG_BLUR_VERTICAL, "PASS_1",
		{ }, {  },
		"Vertical blur pass" }
};

opengl_shader_t *Current_shader = NULL;

static int get_compile_flags(shader_type type, int flags) {
	int compile_flags = 0;
	for (auto& variant : GL_shader_variants) {
		if (variant.type_id == type && flags & variant.flag && variant.use_define) {
			compile_flags |= variant.flag;
		}
	}
	return compile_flags;
}

static int add_shader_slot() {
	int empty_idx = -1;
	for ( int i = 0; i < (int)GL_shader.size(); ++i ) {
		if ( GL_shader[i].shader == NUM_SHADER_TYPES ) {
			empty_idx = i;
			break;
		}
	}

	// then insert it at an empty slot or at the end
	if ( empty_idx >= 0 ) {
		return empty_idx;
	} else {
		// Add a new element
		GL_shader.emplace_back();
		return (int) (GL_shader.size() - 1);
	}
}

static void init_uniform_variants(opengl_shader_t* sdr) {
	sdr->variant_uniforms.clear();
	for (auto& variant : GL_shader_variants) {
		if (sdr->shader == variant.type_id && !variant.use_define) {
			auto value = (sdr->flags & variant.flag) != 0;
			sdr->variant_uniforms.push_back(std::make_pair(variant.flag_text, value));
		}
	}
}

/**
 * Set the currently active shader
 * @param shader_obj	Pointer to an opengl_shader_t object. This function calls glUseProgramARB with parameter 0 if shader_obj is NULL or if function is called without parameters, causing OpenGL to revert to fixed-function processing
 */
void opengl_shader_set_current(opengl_shader_t *shader_obj)
{
	if (Current_shader != shader_obj) {
		GL_state.Array.ResetVertexAttribs();

		if(shader_obj) {
			shader_obj->program->use();

			for (auto& unif : shader_obj->variant_uniforms) {
				shader_obj->program->Uniforms.setUniformi(unif.first, unif.second ? 1 : 0);
			}
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

	auto compile_flags = get_compile_flags(shader_t, flags);
	for (auto& compiled : Compiled_shaders) {
		if (compiled.type == shader_t && compiled.flags == compile_flags) {
			// Found an existing compiled shader
			auto new_sdr_idx = add_shader_slot();
			auto& new_sdr = GL_shader[new_sdr_idx];

			new_sdr.shader = shader_t;
			new_sdr.flags = flags;
			new_sdr.program = compiled.program.get();

			init_uniform_variants(&new_sdr);
			return new_sdr_idx;
		}
	}

	// If we are here, it means we need to compile a new shader
	return opengl_compile_shader(shader_t, flags);
}

void opengl_delete_shader(int sdr_handle)
{
	Assert(sdr_handle >= 0);
	Assert(sdr_handle < (int)GL_shader.size());

	GL_shader[sdr_handle].program = nullptr;

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

	// This will free the actual shader resources
	Compiled_shaders.clear();
}

static SCP_string opengl_shader_get_header(shader_type type_id, int flags, bool with_geometry) {
	SCP_stringstream sflags;

	sflags << "#version " << GLSL_version << " core\n";
	if (with_geometry) {
		sflags << "#define HAS_GEOMETRY_SHADER\n";
	}

	if (type_id == SDR_TYPE_POST_PROCESS_MAIN || type_id == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS || type_id == SDR_TYPE_POST_PROCESS_FXAA) {
		// ignore looking for variants. main post process, lightshafts, and FXAA shaders need special headers to be hacked in
		opengl_post_shader_header(sflags, type_id, flags);
	}
	else {
		for (auto& variant : GL_shader_variants) {
			if (type_id == variant.type_id && flags & variant.flag && variant.use_define) {
				sflags << "#define " << variant.flag_text << "\n";
			}
		}
	}

	// Reset line so that the error messages use the correct file lines
	sflags << "#line 1\n";

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

static SCP_vector<SCP_string> opengl_get_shader_content(shader_type type_id, const char* filename, int flags, bool with_geometry) {
	SCP_vector<SCP_string> parts;
	parts.push_back(opengl_shader_get_header(type_id, flags, with_geometry));

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
		for (auto& variant : GL_shader_variants) {
			if (variant.type_id == sdr && flags & variant.flag && variant.use_geometry_sdr) {
				use_geo_sdr = true;
				break;
			}
		}
	}

	std::unique_ptr<opengl::ShaderProgram> program(new opengl::ShaderProgram());

	try {
		program->addShaderCode(opengl::STAGE_VERTEX,
							   opengl_get_shader_content(sdr_info->type_id, sdr_info->vert, flags, use_geo_sdr));
		program->addShaderCode(opengl::STAGE_FRAGMENT,
							   opengl_get_shader_content(sdr_info->type_id, sdr_info->frag, flags, use_geo_sdr));

		if (use_geo_sdr) {
			// read geometry shader
			program->addShaderCode(opengl::STAGE_GEOMETRY,
								   opengl_get_shader_content(sdr_info->type_id,
															 sdr_info->geo,
															 flags,
															 use_geo_sdr));
		}

		for (int i = 0; i < opengl_vert_attrib::NUM_ATTRIBS; ++i) {
			// assign vert attribute binding locations before we link the shader
			glBindAttribLocation(program->getShaderHandle(), i, GL_vertex_attrib_info[i].name.c_str());
		}

		program->linkProgram();
	} catch (const std::exception&) {
		// Since all shaders are required a compilation failure is a fatal error
		Error(LOCATION, "A shader failed to compile! Check the debug log for more information.");
	}

	// Add a new compiled shader
	Compiled_shaders.emplace_back();
	auto& compiled_shader = Compiled_shaders.back();
	compiled_shader.program = std::move(program);
	compiled_shader.type = sdr;
	compiled_shader.flags = get_compile_flags(sdr, flags);

	new_shader.shader = sdr_info->type_id;
	new_shader.flags = flags;
	new_shader.program = compiled_shader.program.get();

	init_uniform_variants(&new_shader);

	new_shader.program->use();

	// bind fragment data locations
	if ( GL_version >= 32 && GLSL_version >= 150 ) {
		glBindFragDataLocation(new_shader.program->getShaderHandle(), 0, "fragOut0");
		glBindFragDataLocation(new_shader.program->getShaderHandle(), 1, "fragOut1");
		glBindFragDataLocation(new_shader.program->getShaderHandle(), 2, "fragOut2");
		glBindFragDataLocation(new_shader.program->getShaderHandle(), 3, "fragOut3");
		glBindFragDataLocation(new_shader.program->getShaderHandle(), 4, "fragOut4");
	}

	// initialize attributes
	for (auto& attr : sdr_info->attributes) {
		new_shader.program->initAttribute(GL_vertex_attrib_info[attr].name, GL_vertex_attrib_info[attr].default_value);
	}

	mprintf(("Shader Variant Features:\n"));

	// initialize all uniforms and attributes that are specific to this variant
	for (auto& variant : GL_shader_variants) {
		if ( sdr_info->type_id == variant.type_id ) {
			if (variant.flag & flags) {
				for (auto& attr : variant.attributes) {
					auto& attr_info = GL_vertex_attrib_info[attr];
					new_shader.program->initAttribute(attr_info.name, attr_info.default_value);
				}
			}

			mprintf(("	%s\n", variant.description));
		}
	}

	opengl_shader_set_current();

	// add it to our list of embedded shaders
	// see if we have empty shader slots
	auto sdr_index = add_shader_slot();
	GL_shader[sdr_index] = new_shader;

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

void opengl_shader_set_passthrough(bool textured, bool alpha, vec4 *clr, float color_scale)
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

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);
}

void opengl_shader_set_passthrough(bool textured, bool alpha, color *clr)
{
	vec4 normalized_clr = { i2fl(clr->red) / 255.0f, i2fl(clr->green) / 255.0f, i2fl(clr->blue) / 255.0f, clr->is_alphacolor ? i2fl(clr->alpha) / 255.0f : 1.0f };

	opengl_shader_set_passthrough(textured, alpha, &normalized_clr, 1.0f);
}
