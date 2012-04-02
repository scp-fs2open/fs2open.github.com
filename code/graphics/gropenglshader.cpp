/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"

#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltnl.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglstate.h"

#include "math/vecmat.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"


SCP_vector<opengl_shader_t> GL_shader;

static char *GLshader_info_log = NULL;
static const int GLshader_info_log_size = 8192;
GLuint Framebuffer_fallback_texture_id = 0;

static int Effect_num = 0;
static float Anim_timer = 0.0f;


/*
struct opengl_shader_uniform_reference_t {
	int flag;

	int num_uniforms;
	char* uniforms[MAX_SHADER_UNIFORMS];

	int num_attributes;
	char* attributes[MAX_SDR_ATTRIBUTES];

	SCP_string name;
};
*/

/**
 * Static lookup reference for main shader uniforms
 * When adding a new SDR_ flag, list all associated uniforms and attributes here
 */
static opengl_shader_uniform_reference_t GL_Uniform_Reference_Main[] = {
	{ SDR_FLAG_LIGHT,		1, {"n_lights"}, 0, {}, "Lighting" },
	{ SDR_FLAG_FOG,			0, {}, 0, {}, "Fog Effect" },
	{ SDR_FLAG_DIFFUSE_MAP, 1, {"sBasemap"}, 0, {}, "Diffuse Mapping"},
	{ SDR_FLAG_GLOW_MAP,	1, {"sGlowmap"}, 0, {}, "Glow Mapping" },
	{ SDR_FLAG_SPEC_MAP,	1, {"sSpecmap"}, 0, {}, "Specular Mapping" },
	{ SDR_FLAG_NORMAL_MAP,	1, {"sNormalmap"}, 0, {}, "Normal Mapping" },
	{ SDR_FLAG_HEIGHT_MAP,	1, {"sHeightmap"}, 0, {}, "Parallax Mapping" },
	{ SDR_FLAG_ENV_MAP,		3, {"sEnvmap", "alpha_spec", "envMatrix"}, 0, {}, "Environment Mapping" },
	{ SDR_FLAG_ANIMATED,	5, {"sFramebuffer", "effect_num", "anim_timer", "vpwidth", "vpheight"}, 0, {}, "Animated Effects" }
};

static const int Main_shader_flag_references = sizeof(GL_Uniform_Reference_Main) / sizeof(opengl_shader_uniform_reference_t);

/**
 * Static lookup referene for particle shader uniforms
 */
static opengl_shader_uniform_reference_t GL_Uniform_Reference_Particle[] = {
	{ (SDR_FLAG_SOFT_QUAD | SDR_FLAG_DISTORTION), 5, {"baseMap", "window_width", "window_height", "distMap", "frameBuffer"}, 1, { "offset_in" }, "Distorted Particles" },
	{ (SDR_FLAG_SOFT_QUAD),	6, {"baseMap", "depthMap", "window_width", "window_height", "nearZ", "farZ"}, 1, { "radius_in" }, "Depth-blended Particles" }
};

static const int Particle_shader_flag_references = sizeof(GL_Uniform_Reference_Particle) / sizeof(opengl_shader_uniform_reference_t);

opengl_shader_t *Current_shader = NULL;


void opengl_shader_check_info_log(GLhandleARB shader_object);

/**
 * Set the currently active shader 
 * @param shader_obj	Pointer to an opengl_shader_t object. This function calls glUseProgramARB with parameter 0 if shader_obj is NULL or if function is called without parameters, causing OpenGL to revert to fixed-function processing 
 */
void opengl_shader_set_current(opengl_shader_t *shader_obj)
{
	Current_shader = shader_obj;

	if (Current_shader != NULL) {
		vglUseProgramObjectARB(Current_shader->program_id);

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
	} else {
		vglUseProgramObjectARB(0);
	}
}

/**
 * Given a set of flags, determine whether a shader with these flags exists within the GL_shader vector. If no shader with the requested flags exists, attempt to compile one.
 *
 * @param flags	Integer variable, holding a combination of SDR_* flags
 * @return 		Index into GL_shader, referencing a valid shader, or -1 if shader compilation failed
 */
int opengl_shader_get_index(int flags)
{
	size_t idx;
	size_t max = GL_shader.size();

	for (idx = 0; idx < max; idx++) {
		if (GL_shader[idx].flags == flags) {
			return idx;
		}
	}

	// If we are here, it means we need to compile a new shader
	opengl_compile_main_shader(flags);
	if (GL_shader.back().flags == flags)
		return (int)GL_shader.size() - 1;

	// If even that has failed, bail
	return -1;
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
 * @param filename	C-string holding the filename (with extension) of the shader file
 * @param flags		integer variable holding a combination of SDR_* flags
 * @return			C-string holding the complete shader source code
 */
static char *opengl_load_shader(char *filename, int flags)
{
	SCP_string sflags;

	if (Use_GLSL >= 4) {
		sflags += "#define SHADER_MODEL 4\n";
	} else if (Use_GLSL == 3) {
		sflags += "#define SHADER_MODEL 3\n";
	} else {
		sflags += "#define SHADER_MODEL 2\n";
	}

	if (flags & SDR_FLAG_DIFFUSE_MAP) {
		sflags += "#define FLAG_DIFFUSE_MAP\n";
	}

	if (flags & SDR_FLAG_ENV_MAP) {
		sflags += "#define FLAG_ENV_MAP\n";
	}

	if (flags & SDR_FLAG_FOG) {
		sflags += "#define FLAG_FOG\n";
	}

	if (flags & SDR_FLAG_GLOW_MAP) {
		sflags += "#define FLAG_GLOW_MAP\n";
	}

	if (flags & SDR_FLAG_HEIGHT_MAP) {
		sflags += "#define FLAG_HEIGHT_MAP\n";
	}

	if (flags & SDR_FLAG_LIGHT) {
		sflags += "#define FLAG_LIGHT\n";
	}

	if (flags & SDR_FLAG_NORMAL_MAP) {
		sflags += "#define FLAG_NORMAL_MAP\n";
	}

	if (flags & SDR_FLAG_SPEC_MAP) {
		sflags += "#define FLAG_SPEC_MAP\n";
	}

	if (flags & SDR_FLAG_ANIMATED) {
		sflags += "#define FLAG_ANIMATED\n";
	}

	if (flags & SDR_FLAG_DISTORTION) {
		sflags += "#define FLAG_DISTORTION\n";
	}

	const char *shader_flags = sflags.c_str();
	int flags_len = strlen(shader_flags);

	CFILE *cf_shader = cfopen(filename, "rt", CFILE_NORMAL, CF_TYPE_EFFECTS);
	
	if (cf_shader != NULL) {
		int len = cfilelength(cf_shader);
		char *shader = (char*) vm_malloc(len + flags_len + 1);

		strcpy(shader, shader_flags);
		memset(shader + flags_len, 0, len + 1);
		cfread(shader + flags_len, len + 1, 1, cf_shader);
		cfclose(cf_shader);

		return shader;	
	} else {
		mprintf(("   Loading built-in default shader for: %s\n", filename));
		char* def_shader = defaults_get_file(filename);
		size_t len = strlen(def_shader);
		char *shader = (char*) vm_malloc(len + flags_len + 1);

		strcpy(shader, shader_flags);
		strcat(shader, def_shader);

		return shader;
	}
}

/**
 * Compiles a new shader, and creates an opengl_shader_t that will be put into the GL_shader vector
 * if compilation is successful.
 * This function is used for main (i.e. model rendering) and particle shaders, post processing shaders use their own infrastructure
 *
 * @param flags		Combination of SDR_* flags
 */
void opengl_compile_main_shader(int flags) {
	char *vert = NULL, *frag = NULL;

	mprintf(("Compiling new shader:\n"));

	bool in_error = false;
	opengl_shader_t new_shader;

	// choose appropriate files
	char vert_name[NAME_LENGTH];
	char frag_name[NAME_LENGTH];

	if (flags & SDR_FLAG_SOFT_QUAD) {
		strcpy_s( vert_name, "soft-v.sdr");
		strcpy_s( frag_name, "soft-f.sdr");
	} else {
		strcpy_s( vert_name, "main-v.sdr");
		strcpy_s( frag_name, "main-f.sdr");
	}

	// read vertex shader
	if ( (vert = opengl_load_shader(vert_name, flags)) == NULL ) {
		in_error = true;
		goto Done;
	}

	// read fragment shader
	if ( (frag = opengl_load_shader(frag_name, flags)) == NULL ) {
		in_error = true;
		goto Done;
	}

	Verify( vert != NULL );
	Verify( frag != NULL );

	new_shader.program_id = opengl_shader_create(vert, frag);

	if ( !new_shader.program_id ) {
		in_error = true;
		goto Done;
	}

	new_shader.flags = flags;

	opengl_shader_set_current( &new_shader );
	
	mprintf(("Shader features:\n"));

	//Init all the uniforms
	if (new_shader.flags & SDR_FLAG_SOFT_QUAD) {
		for (int j = 0; j < Particle_shader_flag_references; j++) {
			if (new_shader.flags == GL_Uniform_Reference_Particle[j].flag) {
			// Equality check needed because the combination of SDR_FLAG_SOFT_QUAD and SDR_FLAG_DISTORTION define something very different
			// than just SDR_FLAG_SOFT_QUAD alone
				for (int k = 0; k < GL_Uniform_Reference_Particle[j].num_uniforms; k++) {
					opengl_shader_init_uniform( GL_Uniform_Reference_Particle[j].uniforms[k] );
				}

				for (int k = 0; k < GL_Uniform_Reference_Particle[j].num_attributes; k++) {
					opengl_shader_init_attribute( GL_Uniform_Reference_Particle[j].attributes[k] );
				}

				mprintf(("   %s\n", GL_Uniform_Reference_Particle[j].name.c_str()));
			}
		}
	} else {
		for (int j = 0; j < Main_shader_flag_references; j++) {
			if (new_shader.flags & GL_Uniform_Reference_Main[j].flag) {
				if (GL_Uniform_Reference_Main[j].num_uniforms > 0) {
					for (int k = 0; k < GL_Uniform_Reference_Main[j].num_uniforms; k++) {
						opengl_shader_init_uniform( GL_Uniform_Reference_Main[j].uniforms[k] );
					}
				}

				if (GL_Uniform_Reference_Main[j].num_attributes > 0) {
					for (int k = 0; k < GL_Uniform_Reference_Main[j].num_attributes; k++) {
						opengl_shader_init_attribute( GL_Uniform_Reference_Main[j].attributes[k] );
					}
				}

				mprintf(("   %s\n", GL_Uniform_Reference_Main[j].name.c_str()));
			}
		}
	}

	opengl_shader_set_current();

	// add it to our list of embedded shaders
	GL_shader.push_back( new_shader );

Done:
	if (vert != NULL) {
		vm_free(vert);
		vert = NULL;
	}

	if (frag != NULL) {
		vm_free(frag);
		frag = NULL;
	}

	if (in_error) {
		// shut off relevant usage things ...
		bool dealt_with = false;

		if (flags & SDR_FLAG_HEIGHT_MAP) {
			mprintf(("  Shader in_error!  Disabling height maps!\n"));
			Cmdline_height = 0;
			dealt_with = true;
		}

		if (flags & SDR_FLAG_NORMAL_MAP) {
			mprintf(("  Shader in_error!  Disabling normal maps and height maps!\n"));
			Cmdline_height = 0;
			Cmdline_normal = 0;
			dealt_with = true;
		}

		if (!dealt_with) {
			if (flags == 0) {
				mprintf(("  Shader in_error!  Disabling GLSL!\n"));

				Use_GLSL = 0;
				Cmdline_height = 0;
				Cmdline_normal = 0;

				GL_shader.clear();
			} else {
				// We died on a lighting shader, probably due to instruction count.
				// Drop down to a special var that will use fixed-function rendering
				// but still allow for post-processing to work
				mprintf(("  Shader in_error!  Disabling GLSL model rendering!\n"));
				Use_GLSL = 1;
				Cmdline_height = 0;
				Cmdline_normal = 0;
			}
		}
	}
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

	// Compile the particle shaders, since these are most definitely going to be used
	opengl_compile_main_shader(SDR_FLAG_SOFT_QUAD);
	opengl_compile_main_shader(SDR_FLAG_SOFT_QUAD | SDR_FLAG_DISTORTION);

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
		mprintf(("%s shader failed to compile:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : "Fragment", GLshader_info_log));

		// this really shouldn't exist, but just in case
		if (shader_object) {
			vglDeleteObjectARB(shader_object);
		}

		return 0;
	}

	// we succeeded, maybe output warnings too
	if (strlen(GLshader_info_log) > 5) {
		nprintf(("SHADER-DEBUG", "%s shader compiled with warnings:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : "Fragment", GLshader_info_log));
	}

	return shader_object;
}

/**
 * Link a vertex shader object and a fragment shader object into a usable shader executable.
 * Prints linker errors (if any) to the log.
 * 
 * @param vertex_object		Compiled vertex shader object
 * @param fragment_object	Compiled fragment shader object
 * @return					Shader executable
 */
GLhandleARB opengl_shader_link_object(GLhandleARB vertex_object, GLhandleARB fragment_object)
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
 * @return 		Internal ID of the compiled and linked shader as generated by OpenGL
 */
GLhandleARB opengl_shader_create(const char *vs, const char *fs)
{
	GLhandleARB vs_o = 0;
	GLhandleARB fs_o = 0;
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

	program = opengl_shader_link_object(vs_o, fs_o);

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
	
	for (uniform = Current_shader->uniforms.begin(); uniform != Current_shader->uniforms.end(); ++uniform) {
		if ( !uniform->text_id.compare(uniform_text) ) {
			return uniform->location;
		}
	}

	return -1;
}

/**
 * Sets the currently active animated effect.
 *
 * @param effect	Effect ID, needs to be implemented and checked for in the shader
 */
void opengl_shader_set_animated_effect(int effect)
{
	Assert(effect > -1);
	Effect_num = effect;
}

/**
 * Returns the currently active animated effect ID.
 *
 * @return		Currently active effect ID
 */
int opengl_shader_get_animated_effect()
{
	return Effect_num;
}

/**
 * Set the timer for animated effects.
 *
 * @param timer		Timer value to be passed to the shader
 */
void opengl_shader_set_animated_timer(float timer)
{
	Anim_timer = timer;
}

/**
 * Get the timer for animated effects.
 */
float opengl_shader_get_animated_timer()
{
	return Anim_timer;
}
