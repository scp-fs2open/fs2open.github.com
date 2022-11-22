/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "graphics/opengl/gropenglshader.h"

#include "ShaderProgram.h"

#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/matrix.h"
#include "graphics/opengl/gropengldraw.h"
#include "graphics/opengl/gropenglpostprocessing.h"
#include "graphics/opengl/gropenglstate.h"
#include "graphics/opengl/gropengltexture.h"
#include "graphics/opengl/gropengltnl.h"
#include "graphics/util/uniform_structs.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "render/3d.h"

#include <jansson.h>
#include <md5.h>

SCP_vector<opengl_shader_t> GL_shader;

typedef std::pair<int, uint32_t> shader_descriptor_t;

struct key_hasher
{
    size_t operator()(const shader_descriptor_t &obj) const
    {
        return obj.first ^ obj.second;
    }
};

SCP_unordered_map<shader_descriptor_t, size_t, key_hasher> GL_shader_map;

GLuint Framebuffer_fallback_texture_id = 0;

SCP_vector<opengl_vert_attrib> GL_vertex_attrib_info =
	{
		{ opengl_vert_attrib::POSITION,		"vertPosition",		{{{ 0.0f, 0.0f, 0.0f, 1.0f }}} },
		{ opengl_vert_attrib::COLOR,		"vertColor",		{{{ 1.0f, 1.0f, 1.0f, 1.0f }}} },
		{ opengl_vert_attrib::TEXCOORD,		"vertTexCoord",		{{{ 1.0f, 1.0f, 1.0f, 1.0f }}} },
		{ opengl_vert_attrib::NORMAL,		"vertNormal",		{{{ 0.0f, 0.0f, 1.0f, 0.0f }}} },
		{ opengl_vert_attrib::TANGENT,		"vertTangent",		{{{ 1.0f, 0.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::MODEL_ID,		"vertModelID",		{{{ 0.0f, 0.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::RADIUS,		"vertRadius",		{{{ 1.0f, 0.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::UVEC,			"vertUvec",			{{{ 0.0f, 1.0f, 0.0f, 0.0f }}} },
		{ opengl_vert_attrib::WORLD_MATRIX,	"vertWorldMatrix",	{{{ 1.0f, 0.0f, 0.0f, 0.0f }}} },
	};

struct opengl_uniform_block_binding {
	uniform_block_type block_type;
	const char* name;
};

opengl_uniform_block_binding GL_uniform_blocks[] = {
    {uniform_block_type::Lights, "lightData"},
    {uniform_block_type::ModelData, "modelData"},
    {uniform_block_type::NanoVGData, "NanoVGUniformData"},
    {uniform_block_type::DecalInfo, "decalInfoData"},
    {uniform_block_type::DecalGlobals, "decalGlobalData"},
    {uniform_block_type::DeferredGlobals, "globalDeferredData"},
    {uniform_block_type::Matrices, "matrixData"},
	{uniform_block_type::MovieData, "movieData"},
    {uniform_block_type::GenericData, "genericData"},
};

/**
 * Static lookup reference for shader uniforms
 * When adding a new shader, list all associated uniforms and attributes here
 */
// clang-format off
static opengl_shader_type_t GL_shader_types[] = {
	{ SDR_TYPE_MODEL, "main-v.sdr", "main-f.sdr", "main-g.sdr",
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::NORMAL, opengl_vert_attrib::TANGENT, opengl_vert_attrib::MODEL_ID }, "Model Rendering", false },

	{ SDR_TYPE_EFFECT_PARTICLE, "effect-v.sdr", "effect-f.sdr", "effect-g.sdr",
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::RADIUS, opengl_vert_attrib::COLOR }, "Particle Effects", false },

	{ SDR_TYPE_EFFECT_DISTORTION, "effect-distort-v.sdr", "effect-distort-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::RADIUS, opengl_vert_attrib::COLOR }, "Distortion Effects", false },

	{ SDR_TYPE_POST_PROCESS_MAIN, "post-v.sdr", "post-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Post Processing", false },

	{ SDR_TYPE_POST_PROCESS_BLUR, "post-v.sdr", "blur-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Gaussian Blur", false },

	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP, "post-v.sdr", "bloom-comp-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Bloom Compositing", false },

	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS, "post-v.sdr", "brightpass-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Bloom Brightpass", false },

	{ SDR_TYPE_POST_PROCESS_FXAA, "fxaa-v.sdr", "fxaa-f.sdr", 0,
		{ opengl_vert_attrib::POSITION }, "FXAA", false },

	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS, "post-v.sdr", "fxaapre-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "FXAA Prepass", false },

	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, "post-v.sdr", "ls-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Lightshafts", false },

	{ SDR_TYPE_POST_PROCESS_TONEMAPPING, "post-v.sdr", "tonemapping-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Tonemapping", false },

	{ SDR_TYPE_DEFERRED_LIGHTING, "deferred-v.sdr", "deferred-f.sdr", 0,
		{ opengl_vert_attrib::POSITION }, "Deferred Lighting", false },

	{ SDR_TYPE_DEFERRED_CLEAR, "deferred-clear-v.sdr", "deferred-clear-f.sdr", 0,
		{ opengl_vert_attrib::POSITION }, "Clear Deferred Lighting Buffer", false },

	{ SDR_TYPE_VIDEO_PROCESS, "video-v.sdr", "video-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Video Playback", false },

	{ SDR_TYPE_PASSTHROUGH_RENDER, "passthrough-v.sdr", "passthrough-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::COLOR }, "Passthrough", false },

	{ SDR_TYPE_SHIELD_DECAL, "shield-impact-v.sdr",	"shield-impact-f.sdr", 0,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::NORMAL }, "Shield Decals", false },

	{ SDR_TYPE_BATCHED_BITMAP, "batched-v.sdr", "batched-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::COLOR }, "Batched bitmaps", false },

	{ SDR_TYPE_DEFAULT_MATERIAL, "default-material.vert.spv.glsl", "default-material.frag.spv.glsl", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD, opengl_vert_attrib::COLOR }, "Default material", true },

	{ SDR_TYPE_NANOVG, "nanovg-v.sdr", "nanovg-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "NanoVG shader", false },

	{ SDR_TYPE_DECAL, "decal-v.sdr", "decal-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::WORLD_MATRIX }, "Decal rendering", false },

	{ SDR_TYPE_SCENE_FOG, "post-v.sdr", "fog-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Scene fogging", false },

	{ SDR_TYPE_ROCKET_UI, "rocketui-v.sdr",	"rocketui-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::COLOR, opengl_vert_attrib::TEXCOORD }, "libRocket UI", false },

	{ SDR_TYPE_POST_PROCESS_SMAA_EDGE, "smaa-edge-v.sdr", "smaa-edge-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "SMAA Edge detection", false },

	{ SDR_TYPE_POST_PROCESS_SMAA_BLENDING_WEIGHT, "smaa-blend-v.sdr", "smaa-blend-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "SMAA Blending weight calculation", false },

	{ SDR_TYPE_POST_PROCESS_SMAA_NEIGHBORHOOD_BLENDING, "smaa-neighbour-v.sdr", "smaa-neighbour-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "SMAA Neighborhood Blending", false },

	{ SDR_TYPE_ENVMAP_SPHERE_WARP, "post-v.sdr", "envmap-sphere-warp-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Environment Map Export", false },

	{ SDR_TYPE_IRRADIANCE_MAP_GEN, "post-v.sdr", "irrmap-f.sdr", nullptr,
		{ opengl_vert_attrib::POSITION, opengl_vert_attrib::TEXCOORD }, "Irradiance Map Generation", false },
};
// clang-format on

/**
 * Static lookup reference for shader variant uniforms
 * When adding a new shader variant for a shader, list all associated uniforms and attributes here
 */
static opengl_shader_variant_t GL_shader_variants[] = {
	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_LIGHT, "FLAG_LIGHT", {}, "Lighting"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_FOG, "FLAG_FOG", {}, "Fog Effect"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_DIFFUSE_MAP, "FLAG_DIFFUSE_MAP", {}, "Diffuse Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_GLOW_MAP, "FLAG_GLOW_MAP", {}, "Glow Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_SPEC_MAP, "FLAG_SPEC_MAP", {}, "Specular Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_NORMAL_MAP, "FLAG_NORMAL_MAP", {}, "Normal Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_HEIGHT_MAP, "FLAG_HEIGHT_MAP", {}, "Parallax Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ENV_MAP, "FLAG_ENV_MAP", {}, "Environment Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ANIMATED, "FLAG_ANIMATED", {}, "Animated Effects"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_MISC_MAP, "FLAG_MISC_MAP", {}, "Utility mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_TEAMCOLOR, "FLAG_TEAMCOLOR", {}, "Team Colors"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_DEFERRED, "FLAG_DEFERRED", {}, "Deferred lighting"},

	{SDR_TYPE_MODEL, true, SDR_FLAG_MODEL_SHADOW_MAP, "FLAG_SHADOW_MAP", {}, "Shadow Mapping"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_SHADOWS, "FLAG_SHADOWS", {}, "Shadows"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_THRUSTER, "FLAG_THRUSTER", {}, "Thruster scaling"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_TRANSFORM, "FLAG_TRANSFORM", {}, "Submodel Transforms"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_CLIP, "FLAG_CLIP", {}, "Clip Plane"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_HDR, "FLAG_HDR", {}, "High Dynamic Range"},

	{ SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_AMBIENT_MAP, "FLAG_AMBIENT_MAP",
		{  }, "Ambient Occlusion Map"},

	{SDR_TYPE_MODEL, true, SDR_FLAG_MODEL_THICK_OUTLINES, "FLAG_THICK_OUTLINE", {}, "Thick outlines"},

	{SDR_TYPE_MODEL, false, SDR_FLAG_MODEL_ALPHA_MULT, "FLAG_ALPHA_MULT", {}, "Alpha multiplier"},

	{SDR_TYPE_EFFECT_PARTICLE,
	 true,
	 SDR_FLAG_PARTICLE_POINT_GEN,
	 "FLAG_EFFECT_GEOMETRY",
	 {opengl_vert_attrib::UVEC},
	 "Geometry shader point-based particles"},

	{SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_HORIZONTAL, "PASS_0", {}, "Horizontal blur pass"},

	{SDR_TYPE_POST_PROCESS_BLUR, false, SDR_FLAG_BLUR_VERTICAL, "PASS_1", {}, "Vertical blur pass"},

	{SDR_TYPE_NANOVG, false, SDR_FLAG_NANOVG_EDGE_AA, "EDGE_AA", {}, "NanoVG edge anti-alias"},

	{SDR_TYPE_DECAL, false, SDR_FLAG_DECAL_USE_NORMAL_MAP, "USE_NORMAL_MAP", {}, "Decal use scene normal map"},
};

static const int GL_num_shader_variants = sizeof(GL_shader_variants) / sizeof(opengl_shader_variant_t);

opengl_shader_t *Current_shader = NULL;

opengl_shader_t::opengl_shader_t() : shader(SDR_TYPE_NONE), flags(0), flags2(0)
{
}
opengl_shader_t::opengl_shader_t(opengl_shader_t&& other) noexcept {
	*this = std::move(other);
}
// NOLINTNEXTLINE(misc-unconventional-assign-operator)
opengl_shader_t& opengl_shader_t::operator=(opengl_shader_t&& other) noexcept {
	// VS2013 doesn't support implicit move constructors so we need to explicitly declare it
	shader = other.shader;
	flags = other.flags;
	flags2 = other.flags2;

	program = std::move(other.program);

	return *this;
}

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

size_t opengl_get_shader_idx(shader_type shader_t, unsigned int flags)
{
	auto found = GL_shader_map.find(shader_descriptor_t(shader_t, flags));
	if (found != GL_shader_map.end()) {
		return found->second;
	}
	return GL_shader.size();
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
	size_t idx = opengl_get_shader_idx(shader_t, flags);

	if (idx < GL_shader.size())
		return (int)idx;

	// If we are here, it means we need to compile a new shader
	return opengl_compile_shader(shader_t, flags);
}

void opengl_delete_shader(int sdr_handle)
{
	Assert(sdr_handle >= 0);
	Assert(sdr_handle < (int)GL_shader.size());
	opengl_shader_t &victim = GL_shader[sdr_handle];
	GL_shader_map.erase(shader_descriptor_t(victim.shader, victim.flags));

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
	GL_shader_map.clear();
}

static SCP_string opengl_shader_get_header(shader_type type_id, int flags, bool has_geo_shader) {
	SCP_stringstream sflags;

	sflags << "#version " << GLSL_version << " core\n";

	if (Detail.lighting < 3) {
		sflags << "#define FLAG_LIGHT_MODEL_BLINN_PHONG\n";
	}

	if (has_geo_shader) {
		// If there is a geometry shader then we define a special preprocessor symbol to make writing shaders easier
		sflags << "#define HAS_GEOMETRY_SHADER\n";
	}

	if (type_id == SDR_TYPE_POST_PROCESS_MAIN || type_id == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS ||
	    type_id == SDR_TYPE_POST_PROCESS_FXAA || type_id == SDR_TYPE_POST_PROCESS_SMAA_EDGE ||
	    type_id == SDR_TYPE_POST_PROCESS_SMAA_BLENDING_WEIGHT ||
	    type_id == SDR_TYPE_POST_PROCESS_SMAA_NEIGHBORHOOD_BLENDING) {
		// ignore looking for variants. main post process, lightshafts, and FXAA shaders need special headers to be hacked in
		opengl_post_shader_header(sflags, type_id, flags);
	} else {
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
static SCP_string opengl_load_shader(const char* filename) {
	SCP_string content;
	if (Enable_external_shaders) {
		CFILE* cf_shader = cfopen(filename, "rt", CFILE_NORMAL, CF_TYPE_EFFECTS);

		if (cf_shader != NULL) {
			int len = cfilelength(cf_shader);
			content.resize(len);

			cfread(&content[0], len + 1, 1, cf_shader);
			cfclose(cf_shader);

			return content;
		}
	}

	//If we're still here, proceed with internals
	nprintf(("shaders","   Loading built-in default shader for: %s\n", filename));
	auto def_shader = defaults_get_file(filename);
	content.assign(reinterpret_cast<const char*>(def_shader.data), def_shader.size);

	return content;
}

static void handle_includes_impl(SCP_vector<SCP_string>& include_stack,
								 SCP_stringstream& output,
								 int& include_counter,
								 const SCP_string& filename,
								 const SCP_string& original) {
	include_stack.emplace_back(filename);
	auto current_source_number = include_counter + 1;

	const char* INCLUDE_STRING = "#include";
	SCP_stringstream input(original);

	int line_num = 1;
	for (SCP_string line; std::getline(input, line);) {
		auto include_start = line.find(INCLUDE_STRING);
		if (include_start != SCP_string::npos) {
			auto first_quote = line.find('"', include_start + strlen(INCLUDE_STRING));
			auto second_quote = line.find('"', first_quote + 1);

			if (first_quote == SCP_string::npos || second_quote == SCP_string::npos) {
				Error(LOCATION,
					  "Shader %s:%d: Malformed include line. Could not find both quote charaters.",
					  filename.c_str(),
					  line_num);
			}

			auto file_name = line.substr(first_quote + 1, second_quote - first_quote - 1);
			auto existing_name = std::find_if(include_stack.begin(), include_stack.end(), [&file_name](const SCP_string& str) {
				return str == file_name;
			});
			if (existing_name != include_stack.end()) {
				SCP_stringstream stack_string;
				for (auto& name : include_stack) {
					stack_string << "\t" << name << "\n";
				}

				Error(LOCATION,
					  "Shader %s:%d: Detected cyclic include! Previous includes (top level file first):\n%s",
					  filename.c_str(),
					  line_num,
					  stack_string.str().c_str());
			}

			++include_counter;
			// The second parameter defines which source string we are currently working with. We keep track of how many
			// excludes have been in the file so far to specify this
			output << "#line 1 " << include_counter + 1 << "\n";

			handle_includes_impl(include_stack,
								 output,
								 include_counter,
								 file_name,
								 opengl_load_shader(file_name.c_str()));

			// We are done with the include file so now we can return to the original file
			output << "#line " << line_num + 1 << " " << current_source_number << "\n";
		} else {
			output << line << "\n";
		}

		++line_num;
	}

	include_stack.pop_back();
}

static SCP_string handle_includes(const char* filename, const SCP_string& original) {
	SCP_stringstream output;
	SCP_vector<SCP_string> include_stack;
	auto include_counter = 0;

	handle_includes_impl(include_stack, output, include_counter, filename, original);

	return output.str();
}

static SCP_vector<SCP_string>
opengl_get_shader_content(shader_type type_id, const char* filename, int flags, bool has_geo_shader, bool spirv_shader)
{
	SCP_vector<SCP_string> parts;
	if (spirv_shader) {
		// No need to add a header here or handle includes since the original compiler did that
		parts.push_back(opengl_load_shader(filename));
	} else {
		parts.push_back(opengl_shader_get_header(type_id, flags, has_geo_shader));

		parts.push_back(handle_includes(filename, opengl_load_shader(filename)));
	}

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
	for (uint32_t i = 0; i < (uint32_t)GL_vertex_attrib_info.size(); ++i) {
		md5.update(GL_vertex_attrib_info[i].name.c_str(), (MD5::size_type) GL_vertex_attrib_info[i].name.size());
		md5.update(reinterpret_cast<const char*>(&i), sizeof(i));
	}

	md5.update(GL_implementation_id.data(),
	           (MD5::size_type)GL_implementation_id.size() * sizeof(SCP_string::value_type));

	md5.finalize();

	return md5.hexdigest();
}

static bool do_shader_caching() {
	if (!GLAD_GL_ARB_get_program_binary) {
		// Not supported until OpenGL 4.1
		return false;
	}
	if (Cmdline_noshadercache) {
		// Shader cache is disabled
		return false;
	}
	return true;
}

static bool load_cached_shader_binary(opengl::ShaderProgram* program, const SCP_string& hash) {
	if (!do_shader_caching()) {
		return false;
	}

	auto base_filename = SCP_string("ogl_shader-") + hash;

	auto metadata = base_filename + ".json";
	auto binary = base_filename + ".bin";

	auto metadata_fp = cfopen(metadata.c_str(), "rb", CFILE_NORMAL, CF_TYPE_CACHE, false,
	                          CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
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

	bool supported = false;
	for (auto supported_fmt : GL_binary_formats) {
		if ((GLenum)supported_fmt == binary_format) {
			supported = true;
			break;
		}
	}

	if (!supported) {
		// This can happen in case an implementation stops supporting a particular binary format
		nprintf(("ShaderCache", "Unsupported binary format %d encountered in shader cache.\n", binary_format));
		return false;
	}

	auto binary_fp = cfopen(binary.c_str(), "rb", CFILE_NORMAL, CF_TYPE_CACHE, false,
	                        CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
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

	auto metadata_fp = cfopen(metadata_name.c_str(), "wb", CFILE_NORMAL, CF_TYPE_CACHE, false,
	                          CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
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

	auto binary_fp = cfopen(binary_name.c_str(), "wb", CFILE_NORMAL, CF_TYPE_CACHE, false,
	                        CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	if (!binary_fp) {
		mprintf(("Could not open shader cache binary file!\n"));
		return;
	}
	cfwrite(binary.data(), 1, (int) binary.size(), binary_fp);
	cfclose(binary_fp);
}

static void opengl_set_default_uniforms(const opengl_shader_t& sdr) {
	switch (sdr.shader) {
	case SDR_TYPE_DEFERRED_LIGHTING:
		Current_shader->program->Uniforms.setTextureUniform("ColorBuffer", 0);
		Current_shader->program->Uniforms.setTextureUniform("NormalBuffer", 1);
		Current_shader->program->Uniforms.setTextureUniform("PositionBuffer", 2);
		Current_shader->program->Uniforms.setTextureUniform("SpecBuffer", 3);
		Current_shader->program->Uniforms.setTextureUniform("shadow_map", 4);
		break;

	case SDR_TYPE_PASSTHROUGH_RENDER:
		Current_shader->program->Uniforms.setTextureUniform("baseMap", 0);
		Current_shader->program->Uniforms.setTextureUniform("clipEnabled", 0);
		break;

	default:
		// No default values for this shader type.
		break;
	}
}

void opengl_compile_shader_actual(shader_type sdr, const uint &flags, opengl_shader_t &new_shader)
{
	opengl_shader_type_t *sdr_info = &GL_shader_types[sdr];

	Assert(sdr_info->type_id == sdr);
	nprintf(("shaders","Compiling new shader:\n"));
	nprintf(("shaders","	%s\n", sdr_info->description));

	// figure out if the variant requested needs a geometry shader
	bool use_geo_sdr = false;

	// do we even have a geometry shader?
	if (sdr_info->geo != NULL) {
		for (int i = 0; i < GL_num_shader_variants; ++i) {
			opengl_shader_variant_t *variant = &GL_shader_variants[i];

			if (variant->type_id == sdr && flags & variant->flag && variant->use_geometry_sdr) {
				use_geo_sdr = true;
				break;
			}
		}
	}

	auto vert_content =
		opengl_get_shader_content(sdr_info->type_id, sdr_info->vert, flags, use_geo_sdr, sdr_info->spirv_shader);
	auto frag_content =
		opengl_get_shader_content(sdr_info->type_id, sdr_info->frag, flags, use_geo_sdr, sdr_info->spirv_shader);
	SCP_vector<SCP_string> geom_content;

	if (use_geo_sdr) {
		// read geometry shader
		geom_content =
			opengl_get_shader_content(sdr_info->type_id, sdr_info->geo, flags, use_geo_sdr, sdr_info->spirv_shader);
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

			for (size_t i = 0; i < GL_vertex_attrib_info.size(); ++i) {
				// Check that the enum values match the position in the vector to make accessing that information more efficient
				Assertion(GL_vertex_attrib_info[i].attribute_id == (int)i, "Mistmatch between enum values and attribute vector detected!");

				// assign vert attribute binding locations before we link the shader
				glBindAttribLocation(program->getShaderHandle(), (GLint)i, GL_vertex_attrib_info[i].name.c_str());
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
		}
		catch (const std::exception&) {
			// Since all shaders are required a compilation failure is a fatal error
			Error(LOCATION, "A shader failed to compile! Check the debug log for more information.");
		}

		cache_program_binary(program->getShaderHandle(), shader_hash);
	}

	new_shader.shader = sdr_info->type_id;
	new_shader.flags = flags;
	new_shader.program = std::move(program);

	opengl_shader_set_current(&new_shader);

	// initialize the attributes
	for (auto& attr : sdr_info->attributes) {
		new_shader.program->initAttribute(GL_vertex_attrib_info[attr].name, GL_vertex_attrib_info[attr].attribute_id, GL_vertex_attrib_info[attr].default_value);
	}

	for (auto& uniform_block : GL_uniform_blocks) {
		auto blockIndex = glGetUniformBlockIndex(new_shader.program->getShaderHandle(), uniform_block.name);

		if (blockIndex != GL_INVALID_INDEX) {
			glUniformBlockBinding(new_shader.program->getShaderHandle(), blockIndex, static_cast<GLuint>(uniform_block.block_type));
		}
	}

	nprintf(("shaders","Shader Variant Features:\n"));

	// initialize all uniforms and attributes that are specific to this variant
	for (int i = 0; i < GL_num_shader_variants; ++i) {
		opengl_shader_variant_t &variant = GL_shader_variants[i];

		if (sdr_info->type_id == variant.type_id && variant.flag & flags) {
			for (auto& attr : variant.attributes) {
				auto& attr_info = GL_vertex_attrib_info[attr];
				new_shader.program->initAttribute(attr_info.name, attr_info.attribute_id, attr_info.default_value);
			}

			nprintf(("shaders","	%s\n", variant.description));
		}
	}

	opengl_set_default_uniforms(new_shader);
}

/**
 * Compiles a new shader, and creates an opengl_shader_t that will be put into the GL_shader vector
 * if compilation is successful.
 *
 * @param sdr		Identifier defined with the program we wish to compile
 * @param flags		Combination of SDR_* flags
 * @param replacement_idx	The index of the shader this replaces. If -1, the newly compiled shader will be appended to the GL_shader vector
 *					or inserted at the first available empty slot
 */
int opengl_compile_shader(shader_type sdr, uint flags)
{
	GR_DEBUG_SCOPE("Creating new shader");

	int sdr_index = -1;
	int empty_idx;
	opengl_shader_t new_shader;

	Assert(sdr < NUM_SHADER_TYPES);

	opengl_compile_shader_actual(sdr, flags, new_shader);

	opengl_shader_set_current();

	// add it to our list of embedded shaders
	// see if we have empty shader slots
	empty_idx = -1;
	for (int i = 0; i < (int)GL_shader.size(); ++i) {
		if (GL_shader[i].shader == NUM_SHADER_TYPES) {
			empty_idx = i;
			break;
		}
	}

	int new_shader_shader = new_shader.shader;
	uint32_t new_shader_flags = new_shader.flags;
	// then insert it at an empty slot or at the end
	if ( empty_idx >= 0 ) {
		GL_shader[empty_idx] = std::move(new_shader);
		sdr_index = empty_idx;
	} else {
		sdr_index = (int)GL_shader.size();
		GL_shader.push_back(std::move(new_shader));
	}
	GL_shader_map[shader_descriptor_t(new_shader_shader, new_shader_flags)] = sdr_index;
	return sdr_index;
}

void gr_opengl_recompile_all_shaders(const std::function<void(size_t, size_t)>& progress_callback)
{
	for (auto sdr = GL_shader.begin(); sdr != GL_shader.end(); ++sdr)
	{
		if (progress_callback)
			progress_callback(std::distance(GL_shader.begin(), sdr), GL_shader.size());
		sdr->program.reset();
		opengl_compile_shader_actual(sdr->shader, sdr->flags, *sdr);
	}
}

static void opengl_purge_shader_cache_type(const char* ext) {

	SCP_string filter("*.");
	filter += ext;

	// Previously the cache files were stored in the mod directory. Since we have a better system now, those files
	// should be cleaned out. This is only needed if we have a mod directory since otherwise we would delete the actual
	// cache files
	if (Cmdline_mod != nullptr && strlen(Cmdline_mod) > 0) {
		SCP_vector<SCP_string> cache_files;
		cf_get_file_list(cache_files, CF_TYPE_CACHE, filter.c_str(), CF_SORT_NONE, nullptr,
		                 CF_LOCATION_TYPE_PRIMARY_MOD | CF_LOCATION_TYPE_SECONDARY_MODS);

		for (auto& file : cache_files) {
			cf_delete((file + "." + ext).c_str(), CF_TYPE_CACHE,
			          CF_LOCATION_TYPE_PRIMARY_MOD | CF_LOCATION_TYPE_SECONDARY_MODS);
		}
	}

	SCP_vector<SCP_string> cache_files;
	SCP_vector<file_list_info> file_info;
	cf_get_file_list(cache_files, CF_TYPE_CACHE, filter.c_str(), CF_SORT_NONE, &file_info,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	Assertion(cache_files.size() == file_info.size(),
			  "cf_get_file_list returned different sizes for file names and file informations!");

	const auto TIMEOUT = 2.0 * 30.0 * 24.0 * 60.0 * 60.0; // purge timeout in seconds which is ~2 months
	const SCP_string PREFIX = "ogl_shader-";

	auto now = std::time(nullptr);
	for (size_t i = 0; i < cache_files.size(); ++i) {
		auto& name = cache_files[i];
		auto write_time = file_info[i].write_time;

		if (name.compare(0, PREFIX.size(), PREFIX) != 0) {
			// Not an OpenGL cache file
			continue;
		}

		auto diff = std::difftime(now, write_time);

		if (diff > TIMEOUT) {
			auto full_name = name + "." + ext;

			cf_delete(full_name.c_str(), CF_TYPE_CACHE);
		}
	}
}

static void opengl_purge_old_shader_cache()
{
	opengl_purge_shader_cache_type("json");
	opengl_purge_shader_cache_type("bin");
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
	GL_shader_map.clear();

	// Reserve 32 shader slots. This should cover most use cases in real life.
	GL_shader.reserve(32);

	opengl_purge_old_shader_cache();

	// compile effect shaders
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, 0);
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, SDR_FLAG_PARTICLE_POINT_GEN);
	gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0);

	gr_opengl_maybe_create_shader(SDR_TYPE_SHIELD_DECAL, 0);

	// compile deferred lighting shaders
	gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_LIGHTING, 0);
	gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_CLEAR, 0);

	// compile passthrough shader
	nprintf(("shaders","Compiling passthrough shader...\n"));
	gr_opengl_maybe_create_shader(SDR_TYPE_PASSTHROUGH_RENDER, 0);

	nprintf(("shaders","\n"));
}

/**
 * Get the internal OpenGL location for a given attribute. Requires that the Current_shader global variable is valid
 *
 * @param attribute_text	Name of the attribute
 * @return					Internal OpenGL location for the attribute
 */
GLint opengl_shader_get_attribute(opengl_vert_attrib::attrib_id attribute)
{
	Assertion(Current_shader != nullptr, "Current shader may not be null!");

	return Current_shader->program->getAttributeLocation(attribute);
}

void opengl_shader_set_passthrough(bool textured, bool hdr)
{
	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_PASSTHROUGH_RENDER, 0));

	gr_matrix_set_uniforms();

	opengl_set_generic_uniform_data<graphics::generic_data::passthrough_data>(
		[=](graphics::generic_data::passthrough_data* data) {
			data->noTexturing = textured ? 0 : 1;
			data->srgb        = hdr ? 1 : 0;
		});
}

void opengl_shader_set_default_material(bool textured, bool alpha, vec4* clr, float color_scale, uint32_t array_index,
										const material::clip_plane& clip_plane)
{
	Current_shader->program->Uniforms.setTextureUniform("baseMap", 0);

	opengl_set_generic_uniform_data<genericData_default_material_vert>(
		[=](genericData_default_material_vert* data) {
			if (textured) {
				data->noTexturing  = 0;
				data->baseMapIndex = array_index;
			} else {
				data->noTexturing = 1;
				// array_index is probably not valid here
				data->baseMapIndex = 0;
			}

			if (alpha) {
				data->alphaTexture = 1;
			} else {
				data->alphaTexture = 0;
			}

			if (High_dynamic_range) {
				data->srgb      = 1;
				data->intensity = color_scale;
			} else {
				data->srgb      = 0;
				data->intensity = 1.0f;
			}

			data->alphaThreshold = GL_alpha_threshold;

			if (clr != nullptr) {
				data->color = *clr;
			} else {
				data->color.xyzw.x = 1.0f;
				data->color.xyzw.y = 1.0f;
				data->color.xyzw.z = 1.0f;
				data->color.xyzw.w = 1.0f;
			}

			if (clip_plane.enabled) {
				data->clipEnabled = 1;

				vec4 clip_equation;
				clip_equation.xyzw.x = clip_plane.normal.xyz.x;
				clip_equation.xyzw.y = clip_plane.normal.xyz.y;
				clip_equation.xyzw.z = clip_plane.normal.xyz.z;
				clip_equation.xyzw.w = -vm_vec_dot(&clip_plane.normal, &clip_plane.position);

				data->clipEquation = clip_equation;
				data->modelMatrix  = gr_model_matrix_stack.get_transform();
			} else {
				data->clipEnabled = 0;
			}
		});

	gr_matrix_set_uniforms();
}
