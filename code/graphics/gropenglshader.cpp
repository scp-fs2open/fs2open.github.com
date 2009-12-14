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

#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglshader.h"

#include <sstream>

// Legacy stuff
struct opengl_shader_file_t {
	char *vert;
	char *frag;

	int flags;

	int num_uniforms;
	char *uniforms[MAX_SHADER_UNIFORMS];
};

static opengl_shader_file_t GL_shader_file[] = {
	{ "null-v.sdr", "null-f.sdr", (0), 0, { NULL }, },

	{ "b-v.sdr", "b-f.sdr", (SDR_FLAG_DIFFUSE_MAP),
		1, { "sBasemap" } },

	{ "b-v.sdr", "bg-f.sdr", (SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP),
		2, { "sBasemap", "sGlowmap" } },

	{ "l-v.sdr", "lb-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP),
		2, { "sBasemap", "n_lights" } },

	{ "l-v.sdr", "lbg-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP),
		3, { "sBasemap", "sGlowmap", "n_lights" } },

	{ "l-v.sdr", "lbgs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP),
		4, { "sBasemap", "sGlowmap", "sSpecmap", "n_lights" } },

	{ "l-v.sdr", "lbs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP),
		3, { "sBasemap", "sSpecmap", "n_lights" } },

	{ "le-v.sdr", "lbgse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "le-v.sdr", "lbse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		6, { "sBasemap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "ln-v.sdr", "lbgn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sGlowmap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lbgsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		5, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lbn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sBasemap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lbsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sSpecmap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lbgnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sGlowmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "ln-v.sdr", "lbgsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		6, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "ln-v.sdr", "lbnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sBasemap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "ln-v.sdr", "lbsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "lne-v.sdr", "lbgsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lne-v.sdr", "lbsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lne-v.sdr", "lbgsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		9, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lne-v.sdr", "lbsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lf-v.sdr", "lfb-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP),
		2, { "sBasemap", "n_lights" } },

	{ "lf-v.sdr", "lfbg-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP),
		3, { "sBasemap", "sGlowmap", "n_lights" } },

	{ "lf-v.sdr", "lfbgs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP),
		4, { "sBasemap", "sGlowmap", "sSpecmap", "n_lights" } },

	{ "lf-v.sdr", "lfbs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP),
		3, { "sBasemap", "sSpecmap", "n_lights" } },

	{ "lfe-v.sdr", "lfbgse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lfe-v.sdr", "lfbse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		6, { "sBasemap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lfn-v.sdr", "lfbgn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sGlowmap", "sNormalmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbgsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		5, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sBasemap", "sNormalmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sSpecmap", "sNormalmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbgnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sGlowmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbgsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		6, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sBasemap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "lfn-v.sdr", "lfbsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "lfne-v.sdr", "lfbgsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lfne-v.sdr", "lfbsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lfne-v.sdr", "lfbgsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		9, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lfne-v.sdr", "lfbsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "l-v.sdr", "null-f.sdr", (SDR_FLAG_LIGHT), 0, { NULL } },

	{ "l-v.sdr", "lg-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP),
		2, { "sGlowmap", "n_lights" } },

	{ "l-v.sdr", "lgs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP),
		3, { "sGlowmap", "sSpecmap", "n_lights" } },

	{ "l-v.sdr", "ls-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP),
		2, { "sSpecmap", "n_lights" } },

	{ "le-v.sdr", "lgse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		6, { "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "le-v.sdr", "lse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		5, { "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "ln-v.sdr", "lgn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sGlowmap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lgsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		4, { "sGlowmap", "sSpecmap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "ln-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_NORMAL_MAP),
		2, { "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sSpecmap", "sNormalmap", "n_lights" } },

	{ "ln-v.sdr", "lgnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sGlowmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "ln-v.sdr", "lgsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "ln-v.sdr", "lnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		3, { "sNormalmap", "sHeightmap", "n_lights" } },

	{ "ln-v.sdr", "lsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" } },

	{ "lne-v.sdr", "lgsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		7, { "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lne-v.sdr", "lsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		6, { "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lne-v.sdr", "lgsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		8, { "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } },

	{ "lne-v.sdr", "lsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		7, { "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" } }
};

static const int Num_shader_files = sizeof(GL_shader_file) / sizeof(opengl_shader_file_t);

// Resources system - move it to more appropiate file as soon as we get working resource manager
using namespace resources;

text_file::text_file(const char *filename, txt_file_type ftype) {
	// open file
	CFILE *fp = cfopen(const_cast<char*>(filename), "rt", CFILE_NORMAL, get_cfile_type(ftype));

	if (!fp) {
		mprintf(("Could not open text file \'%s\'.\n", filename));
		return;
	}

	// load file
	// we don't support asynchronous io yet
	int len = cfilelength(fp);
	char *bytes = reinterpret_cast<char*>(vm_malloc(len + 1));
	memset(bytes, 0, len + 1);
	cfread(bytes, len + 1, 1, fp);
	cfclose(fp);

	data = SCP_string(bytes);

	// clean
	vm_free(bytes);
}

const SCP_string &text_file::read() const {
	return data;
}

bool text_file::file_exist(const char *filename, txt_file_type ftype) {
	return cf_exists_full(const_cast<char*>(filename), get_cfile_type(ftype)) != 0;
}

int text_file::get_cfile_type(txt_file_type ftype) {
	switch (ftype) {
		case shader_source:
			return CF_TYPE_EFFECTS;
		case table_file:
			return CF_TYPE_TABLES;
		default:
			return CF_TYPE_ANY;
	}
}

// End of resources code

using namespace opengl;

opengl::shader::shader(resources::text_file *vert, resources::text_file *frag) : shader_program(0), last_texture(0) {
	vertex_shader = vert->read();
	fragment_shader = frag->read();

	state = loaded;
}

opengl::shader::~shader() {
	if (shader_program)
		vglDeleteObjectARB(shader_program);
}

bool opengl::shader::compile_link() {
	const char *vert_src = vertex_shader.c_str();
	const char *frag_src = fragment_shader.c_str();

	GLhandleARB vert = 0;
	GLhandleARB frag = 0;

	// compile vertex and fragment shaders
	do {
		vert = compile_object((const GLcharARB*)vert_src, GL_VERTEX_SHADER_ARB);
		if (!vert) {
			mprintf(("ERROR! Unable to create vertex shader!\n"));
			break;
		}

		frag = compile_object((const GLcharARB*)frag_src, GL_FRAGMENT_SHADER_ARB);
		if (!frag) {
			mprintf(("ERROR! Unable to create fragment shader!\n"));
			break;
		}
	} while (false);

	// link programs
	if (vert && frag) {
		shader_program = link_objects(vert, frag);
		if (!shader_program)
			mprintf(("ERROR! Unable to create shader program!\n"));
	}

	// clean
	if (vert)
		vglDeleteObjectARB(vert);

	if (frag)
		vglDeleteObjectARB(frag);

	// check if shader program was created
	bool result = shader_program != 0;
	if (result)
		state = compiled;
	else
		return false;

	// get uniforms locations
	SCP_vector<SCP_string> uniform_names = get_uniform_names();
	for (unsigned int i = 0; i < uniform_names.size(); i++)
		uniforms.push_back(vglGetUniformLocationARB(shader_program, uniform_names[i].c_str()));

	return true;
}

GLint opengl::shader::compile_object(const GLcharARB *shader_source, GLenum shader_type) {
	GLhandleARB shader_object = 0;
	GLint status = 0;

	shader_object = vglCreateShaderObjectARB(shader_type);

	vglShaderSourceARB(shader_object, 1, &shader_source, NULL);
	vglCompileShaderARB(shader_object);

	// check if the compile was successful
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &status);

	// get compilation log
	SCP_string info_log = check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		// basic error check
		mprintf(("%s shader failed to compile:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : "Fragment", info_log.c_str()));
		if (shader_object) {
			vglDeleteObjectARB(shader_object);
			shader_object = 0;
		}
	}
	// we succeeded, maybe output warnings too
	else if (info_log.length() > 5) {
		nprintf(("SHADER-DEBUG", "%s shader compiled with warnings:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : "Fragment", info_log.c_str()));
	}

	return shader_object;
}

GLhandleARB opengl::shader::link_objects(GLhandleARB vertex_object, GLhandleARB fragment_object) {
	GLhandleARB shader_object = 0;
	GLint status = 0;

	shader_object = vglCreateProgramObjectARB();

	if (vertex_object)
		vglAttachObjectARB(shader_object, vertex_object);

	if (fragment_object)
		vglAttachObjectARB(shader_object, fragment_object);
	
	vglLinkProgramARB(shader_object);

	// check if the link was successful
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_LINK_STATUS_ARB, &status);

	// get consolidation log
	SCP_string info_log = check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		mprintf(("Shader failed to link:\n%s\n", info_log.c_str()));
		if (shader_object) {
			vglDeleteObjectARB(shader_object);
			shader_object = 0;
		}
	}
	// we succeeded, maybe output warnings too
	else if (info_log.length() > 5) {
		nprintf(("SHADER-DEBUG", "Shader linked with warnings:\n%s\n", info_log.c_str()));
	}

	return shader_object;
}

SCP_string opengl::shader::check_info_log(GLint shader_object) {
	GLint length = 0;
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);

	char *log = (char*)vm_malloc(length + 1);
	memset(log, 0, length + 1);

	vglGetInfoLogARB(shader_object, length, 0, log);

	SCP_string str = log;
	vm_free(log);

	return str;
}

void opengl::shader::apply() {
	last_texture = 0;

	vglUseProgramObjectARB(shader_program);

#ifndef NDEBUG
		if ( opengl_check_for_errors("shader_set_current()") ) {
			vglValidateProgramARB(shader_program);

			GLint obj_status = 0;
			vglGetObjectParameterivARB(shader_program, GL_OBJECT_VALIDATE_STATUS_ARB, &obj_status);

			if ( !obj_status ) {
				SCP_string info_log = check_info_log(shader_program);
	
				mprintf(("VALIDATE INFO-LOG:\n"));

				if (info_log.length() > 5) {
					mprintf(("%s\n", info_log.c_str()));
				} else {
					mprintf(("<EMPTY>\n"));
				}
			}
		}
#endif

	state = applied;
}

GLint opengl::shader::get_uniform_location(int name) {
	// get uniform location from vector
	if (uniforms.size() > (unsigned int)name || uniforms[name] >= 0)
		return uniforms[name];
	else {
		mprintf(("WARNING: Unable to get shader uniform location for \"%s\"!\n", get_uniform_names()[name]));
		return -1;
	}
}

void opengl::shader::set_uniform(int name, GLint value) {
	Assert(state == applied);

	GLint uniform_location;
	if ((uniform_location = get_uniform_location(name)) >= 0)
		vglUniform1iARB(uniform_location, value);
}

void opengl::shader::set_uniform(int name, GLfloat value) {
	Assert(state == applied);

	GLint uniform_location;
	if ((uniform_location = get_uniform_location(name)) >= 0)
		vglUniform1fARB(uniform_location, value);
}

void opengl::shader::set_uniformMatrix4f(int name, GLfloat *matrix) {
	Assert(state == applied);

	GLint uniform_location;
	if ((uniform_location = get_uniform_location(name)) >= 0)
		vglUniformMatrix4fvARB(uniform_location, 1, GL_FALSE, matrix);
}

// list of all uniforms used by main shaders
const char *opengl::main_shader::uniform_names[] = {
	"n_lights",
	"sBasemap",
	"sGlowmap",
	"sSpecmap",
	"alpha_spec",
	"envMatrix",
	"sEnvmap",
	"sNormalmap",
	"sHeightmap",
	""
};
SCP_vector<SCP_string> main_shader::uninames;

SCP_vector<SCP_string> &main_shader::get_uniform_names() {
	for (int i = 0; i < sizeof(uniform_names) / sizeof(const char*); i++)
		uninames.push_back(uniform_names[i]);

	return uninames;
}

void main_shader::configure(int flags) {
	Assert(state == loaded);

	id_flags = flags;

	SCP_string sflags;

	// create #defines
	if (flags & flag_diffuse_map)
		sflags += "#define FLAG_DIFFUSE_MAP\n";
	if (flags & flag_env_map)
		sflags += "#define FLAG_ENV_MAP\n";
	if (flags & flag_fog)
		sflags += "#define FLAG_FOG\n";
	if (flags & flag_glow_map)
		sflags += "#define FLAG_GLOW_MAP\n";
	if (flags & flag_height_map)
		sflags += "#define FLAG_HEIGHT_MAP\n";
	if (flags & flag_light)
		sflags += "#define FLAG_LIGHT\n";
	if (flags & flag_normal_map)
		sflags += "#define FLAG_NORMAL_MAP\n";
	if (flags & flag_specular_map)
		sflags += "#define FLAG_SPEC_MAP\n";

	// append #defines to the shaders source code
	SCP_string vert = sflags;
	vert += vertex_shader;
	vertex_shader = vert;

	SCP_string frag = sflags;
	frag += fragment_shader;
	fragment_shader = frag;
}

int opengl::post_shader::last_one = 0;
SCP_vector<int> opengl::post_shader::unif_classes;

int post_shader::choose_post_shader() {
	// if nothing has changed then don'n waste time
	if (!post_effect::changed)
		return last_one;

	// check which effects should be enabled
	int flags = 0;
	SCP_vector<post_effect> &effects = get_effects();
	for (unsigned int i = 0; i < effects.size(); i++)
		if (effects[i].always_on || effects[i].intensity != effects[i].default_intensity)
			flags |= 1 << i;

	post_effect::changed = false;
	last_one = flags;

	return flags;
}

void post_shader::configure(int flags) {
	Assert(state == loaded);

	id_flags = flags;

	SCP_string sflags;

	// create #defines
	SCP_vector<post_effect> &effects = get_effects();
	for (unsigned int i = 0; i < effects.size(); i++) {
		if (flags & (1 << i)) {
			sflags += "\n#define ";
			sflags += effects[i].define_name;
		}
	}

	sflags += "\n";

	// append #defines to the shaders source code
	SCP_string vert = sflags;
	vert += vertex_shader;
	vertex_shader = vert;

	SCP_string frag = sflags;
	frag += fragment_shader;
	fragment_shader = frag;
}

SCP_vector<SCP_string> post_shader::uniform_names;
SCP_vector<SCP_string> &post_shader::get_uniform_names() {
	if (uniform_names.empty()) {
		// Uniforms
		uniform_names.push_back("tex");
		uniform_names.push_back("timer");

		unif_classes.push_back(uniform_names.size());
		bloom::get_uniforms(uniform_names);
#if DEPTH_OF_FIELD
		unif_classes.push_back(uniform_names.size());
		depth_of_field::get_uniforms(uniform_names);
#endif
		unif_classes.push_back(uniform_names.size());
		simple_effects::get_uniforms(uniform_names);
	}

	return uniform_names;
}

// List of special shaders
const special_shader::shader_data special_shader::shaders[] = {
	{ special_shader::blur, 120, "post-v.sdr", "blur-f.sdr", { "tex", "bsize", "" } },
	{ special_shader::bright_pass, 0, "post-v.sdr", "brightpass-f.sdr", { "tex", "" } },
};

SCP_string special_shader::get_vert_name(int sdr) {
	Assert(shaders[sdr].id == sdr);
	return SCP_string(shaders[sdr].vert_name);
}

SCP_string special_shader::get_frag_name(int sdr) {
	Assert(shaders[sdr].id == sdr);
	return SCP_string(shaders[sdr].frag_name);
}

SCP_map<int, SCP_vector<SCP_string> > special_shader::uniforms;
SCP_vector<SCP_string> &special_shader::get_uniform_names() {
	SCP_map<int, SCP_vector<SCP_string> >::iterator itr = uniforms.find(flags_id);
	if (itr == uniforms.end()) {
		SCP_vector<SCP_string> uni;
		for (int i = 0; shaders[flags_id].uniforms[i]; i++)
			uni.push_back(shaders[flags_id].uniforms[i]);
		uniforms[flags_id] = uni;
	}

	return uniforms[flags_id];
}

void special_shader::configure(int pass) {
	in_pass = pass;
}

bool special_shader::start_pass(int pass) {
	configure(pass);
	return apply();
}

bool special_shader::apply() {
	//Assert(state == loaded);

	if (passes.size() < in_pass + 1) {
		SCP_string sflags;

		if (shaders[flags_id].version) {
			sflags += "#version ";
			SCP_stringstream sout;
			sout << shaders[flags_id].version;
			sflags += sout.str();
			sflags += "\n";
		}

		sflags += "#define PASS_";
		SCP_stringstream sout;
		sout << in_pass;
		sflags += sout.str();
		sflags += "\n";

		// append #defines to the shaders source code
		SCP_string frag = sflags;
		frag += fragment_shader;

		SCP_string t_frag;
		t_frag = fragment_shader;
		fragment_shader = frag;
		if (!shader::compile_link())
			return false;
		
		passes.push_back(shader_program);

		fragment_shader = t_frag;

		state = compiled;
	}

	shader_program = passes[in_pass];

	shader::apply();

	return true;
}

void shader_manager::apply_main_shader(int flags) {
	if (current_main && current_main->get_id() == flags) {
		current_main->apply();
		return;
	}

	SCP_map<int, main_shader*>::iterator itr = main_shaders_cache.find(flags);

	if (itr != main_shaders_cache.end()) {
		current_main = (*itr).second;
		current_main->apply();
	} else {
		itr = main_shaders.find(flags);
		if (itr != main_shaders.end()) {
			current_main = (*itr).second;
			main_shaders_cache[flags] = current_main;
			current_main->apply();
		} else {
			current_main = NULL;
			mprintf(("Requested main shader not found: 0x%x.\n", flags));
		}
	}
}

opengl::post_shader *shader_manager::apply_post_shader() {
	// check which effects do we need
	return apply_post_shader(post_shader::choose_post_shader());
}

opengl::post_shader *shader_manager::apply_post_shader(int flags) {
	// check if shader is already loaded
	SCP_map<int, post_shader*>::iterator itr = post_shaders.find(flags);
	if (itr != post_shaders.end())
		current_post = (*itr).second;
	else
		// load shader
		current_post = load_post_shader(flags);

	if (current_post)
		current_post->apply();
	else
		apply_fixed_pipeline();

	return current_post;
}

opengl::special_shader *shader_manager::apply_special_shader(int flags) {
	special_shader *sshader;

	SCP_map<int, special_shader*>::iterator itr = special_shaders.find(flags);
	if (itr == special_shaders.end()) {
		mprintf(("  Compiling special shader ->  %s / %s ... \n", special_shader::get_vert_name(flags).c_str(), special_shader::get_frag_name(flags).c_str()));

		resources::text_file *vert = new resources::text_file(special_shader::get_vert_name(flags).c_str(), resources::text_file::shader_source);
		resources::text_file *frag = new resources::text_file(special_shader::get_frag_name(flags).c_str(), resources::text_file::shader_source);
		sshader = new opengl::special_shader(flags, vert, frag);
		special_shaders[flags] = sshader;

		delete vert;
		delete frag;
	} else
		sshader = (*itr).second;

	if (!sshader->start_pass(0))
		return NULL;

	return sshader;
}

void shader_manager::apply_fixed_pipeline() {
	vglUseProgramObjectARB(0);
}

shader_manager::shader_manager() : current_main(NULL), current_post(NULL) {
	mprintf(("\nInitializing Shaders Manager...\n"));
	load_main_shaders();
//	load_post_shaders(); //we load post-processing shaders on demand
	mprintf(("Shaders Manager initialized.\n\n"));
}

shader_manager::~shader_manager() {
	// delete main shaders
	SCP_map<int, main_shader*>::iterator mitr;
	for (mitr = main_shaders.begin(); mitr != main_shaders.end(); mitr++)
		delete (*mitr).second;

	main_shaders.clear();

	// delete post shaders
	SCP_map<int, post_shader*>::iterator pitr;
	for (pitr = post_shaders.begin(); pitr != post_shaders.end(); pitr++)
		delete (*pitr).second;

	post_shaders.clear();
}

opengl::shader_manager *opengl::shader_manager::instance = NULL;

void shader_manager::load_main_shaders() {
	mprintf(("Loading and compiling main shaders...\n"));

	// check if we can use new main shaders
	bool main_vert = resources::text_file::file_exist("main-v.sdr", resources::text_file::shader_source) != 0;
	bool main_frag = resources::text_file::file_exist("main-f.sdr", resources::text_file::shader_source) != 0;

	// use legacy array with shaders names and flags
	for (int i = 0; i < Num_shader_files; i++) {
		opengl_shader_file_t *shader_file = &GL_shader_file[i];

		// omit shaders that surely won't be used
		if (!config::is_enabled(config::glow) && (shader_file->flags & main_shader::flag_glow_map))
			continue;

		if (!config::is_enabled(config::specular) && (shader_file->flags & main_shader::flag_specular_map))
			continue;

		if (!config::is_enabled(config::env_map) && (shader_file->flags & main_shader::flag_env_map))
			continue;

		if (!config::is_enabled(config::normal_map) && (shader_file->flags & main_shader::flag_normal_map))
			continue;

		if (!config::is_enabled(config::height_map) && (shader_file->flags & main_shader::flag_height_map))
			continue;

		// choose appropiate shaders names
		char *vert_name;
		if (main_vert)
			vert_name = "main-v.sdr";
		else
			vert_name = shader_file->vert;

		char *frag_name;
		if (main_frag)
			frag_name = "main-f.sdr";
		else
			frag_name = shader_file->frag;

		mprintf(("  Compiling main shader ->  %s (%s) / %s (%s) ... \n", vert_name, shader_file->vert, frag_name, shader_file->frag));

		// create new shader program
		resources::text_file *vert = new resources::text_file(vert_name, resources::text_file::shader_source);
		resources::text_file *frag = new resources::text_file(frag_name, resources::text_file::shader_source);
		opengl::main_shader *sdr = new opengl::main_shader(vert, frag);

		// configure shader
		sdr->configure(shader_file->flags);

		// compile shader and add it to shaders list
		if (sdr->compile_link())
			main_shaders[shader_file->flags] = sdr;
		else {
			// if problem caused by height map - disable
			if (shader_file->flags & main_shader::flag_height_map) {
				mprintf(("  Shader in_error!  Disabling height maps!\n"));
				config::disable(config::height_map);
			}

			// if problem caused by normal map - disable
			if (shader_file->flags & main_shader::flag_normal_map) {
				mprintf(("  Shader in_error!  Disabling normal maps and height maps!\n"));
				config::disable(config::height_map);
				config::disable(config::normal_map);
			}

			// there is no way to use glsl
			if (i == 0) {
				mprintf(("  Shader in_error!  Disabling GLSL!\n"));

				config::disable(config::glsl);
				config::disable(config::height_map);
				config::disable(config::normal_map);

				delete sdr;
				delete vert;
				delete frag;

				destroy();
				return;
			}

			delete sdr;
		}
			
		// clean
		delete vert;
		delete frag;
	}
}

opengl::post_shader *shader_manager::load_post_shader(int flags) {
	const char *vert_name = "post-v.sdr";
	const char *frag_name = "post-f.sdr";
	mprintf(("  Compiling post shader (0x%x) ->  %s / %s ... \n", flags, vert_name, frag_name));

	// create new shader program
	resources::text_file *vert = new resources::text_file(vert_name, resources::text_file::shader_source);
	resources::text_file *frag = new resources::text_file(frag_name, resources::text_file::shader_source);
	opengl::post_shader *sdr = new opengl::post_shader(vert, frag);

	// configure shader
	sdr->configure(flags);

	// compile shader and add it to shaders list
	if (sdr->compile_link())
		post_shaders[flags] = sdr;
	else {
		mprintf(("  Post-processing shader in_error!\n"));

		delete sdr;
		sdr = NULL;
	}

	// clean
	delete vert;
	delete frag;

	return sdr;
}

// C wrappers
void gr_clear_shaders_cache() {
	if (opengl::config::is_enabled(opengl::config::glsl))
		opengl::shader_manager::get()->clear_cache();
}
