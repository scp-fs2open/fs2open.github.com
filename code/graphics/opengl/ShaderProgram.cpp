//
//

#include "ShaderProgram.h"

#include "graphics/opengl/gropenglstate.h"

namespace {

/**
 * Retrieve the compilation log for a given shader object, and store it in the GLshader_info_log global variable
 *
 * @param shader_object		OpenGL handle of a shader object
 */
SCP_string get_shader_info_log(GLuint shader_object)
{
	GLint length;
	glGetShaderiv(shader_object, GL_INFO_LOG_LENGTH, &length);

	if (length <= 0) {
		return SCP_string();
	}
	SCP_string info_log;
	info_log.resize(length);

	glGetShaderInfoLog(shader_object, (GLsizei) info_log.size(), nullptr, &info_log[0]);

	// Remove trailing null character
	info_log.resize(info_log.size() - 1);

	return info_log;
}

/**
* Retrieve the compilation log for a given shader object, and store it in the GLshader_info_log global variable
*
* @param program_object		OpenGL handle of a shader object
*/
SCP_string get_program_info_log(GLuint program_object)
{
	GLint length;
	glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &length);

	if (length <= 0) {
		return SCP_string();
	}
	SCP_string info_log;
	info_log.resize(length);

	glGetProgramInfoLog(program_object, (GLsizei) info_log.size(), nullptr, &info_log[0]);

	// Remove trailing null character
	info_log.resize(info_log.size() - 1);

	return info_log;
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
GLuint compile_shader_object(const SCP_vector<SCP_string>& shader_source, GLenum shader_type)
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

	auto info_log = get_shader_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		// basic error check
		mprintf(("%s shader failed to compile:\n%s\n", (shader_type == GL_VERTEX_SHADER) ? "Vertex" : ((shader_type == GL_GEOMETRY_SHADER) ? "Geometry" : "Fragment"), info_log.c_str()));

		// this really shouldn't exist, but just in case
		if (shader_object) {
			glDeleteProgram(shader_object);
		}

		throw std::runtime_error("Failed to compile shader!");
	}

	// we succeeded, maybe output warnings too
	if (info_log.size() > 5) {
		nprintf(("SHADER-DEBUG", "%s shader compiled with warnings:\n%s\n", (shader_type == GL_VERTEX_SHADER) ? "Vertex" : ((shader_type == GL_GEOMETRY_SHADER) ? "Geometry" : "Fragment"), info_log.c_str()));
	}

	return shader_object;
}

void link_program(GLuint program) {
	glLinkProgram(program);

	GLint status;
	// check if the link was successful
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	auto log = get_program_info_log(program);

	// we failed, bail out now...
	if (status == GL_FALSE) {
		mprintf(("Shader failed to link:\n%s\n", log.c_str()));

		throw std::runtime_error("Failed to compile shader!");
	}

	// we succeeded, maybe output warnings too
	if (log.size() > 5) {
		nprintf(("SHADER-DEBUG", "Shader linked with warnings:\n%s\n", log.c_str()));
	}
}

GLenum get_gl_shader_stage(opengl::ShaderStage stage) {
	switch(stage) {
		case opengl::STAGE_VERTEX:
			return GL_VERTEX_SHADER;
		case opengl::STAGE_GEOMETRY:
			return GL_GEOMETRY_SHADER;
		case opengl::STAGE_FRAGMENT:
			return GL_FRAGMENT_SHADER;
		default:
			UNREACHABLE("Unhandled shader type found!");
			return GL_NONE;
	}
}
}

opengl::ShaderProgram::ShaderProgram(const SCP_string& program_name) : _program_id(0), Uniforms(this) {
	_program_id = glCreateProgram();
	opengl_set_object_label(GL_PROGRAM, _program_id, program_name);
}
opengl::ShaderProgram::~ShaderProgram() {
	freeCompiledShaders();

	if (_program_id != 0) {
		// Make sure this program isn't used at the moment.
		GL_state.UseProgram(0);
		glDeleteProgram(_program_id);
	}
}

void opengl::ShaderProgram::use() {
	GL_state.UseProgram(_program_id);
}
GLuint opengl::ShaderProgram::getShaderHandle() {
	return _program_id;
}
void opengl::ShaderProgram::addShaderCode(opengl::ShaderStage stage, const SCP_string& name, const SCP_vector<SCP_string>& codeParts) {
	auto shader_obj = compile_shader_object(codeParts, get_gl_shader_stage(stage));
	opengl_set_object_label(GL_SHADER, shader_obj, name);
	_compiled_shaders.push_back(shader_obj);
	glAttachShader(_program_id, shader_obj);
}
void opengl::ShaderProgram::freeCompiledShaders() {
	for (auto& compiled_shader : _compiled_shaders) {
		glDetachShader(_program_id, compiled_shader);
		glDeleteShader(compiled_shader);
	}
	_compiled_shaders.clear();
}
void opengl::ShaderProgram::linkProgram() {
	link_program(_program_id);

	// We don't need the shaders anymore
	freeCompiledShaders();
}

void opengl::ShaderProgram::initAttribute(const SCP_string& name, const vec4& default_value)
{
	auto attrib_loc = glGetAttribLocation(_program_id, name.c_str());

	if (attrib_loc == -1)
	{
		// Not available or optimized out, ignore
		return;
	}

	// The shader needs to be in use before glVertexAttrib can be used
	use();
	glVertexAttrib4f(
		attrib_loc,
		default_value.xyzw.x,
		default_value.xyzw.y,
		default_value.xyzw.z,
		default_value.xyzw.w
	);
}

opengl::ShaderUniforms::ShaderUniforms(ShaderProgram* shaderProgram) : _program(shaderProgram) {
	Assertion(shaderProgram != nullptr, "Shader program may not be null!");
}

size_t opengl::ShaderUniforms::findUniform(const SCP_string& name)
{
	auto iter = _uniform_lookup.find(name);

	if (iter == _uniform_lookup.end()) {
		return INVALID_SIZE;
	} else {
		return iter->second;
	}
}
void opengl::ShaderUniforms::setTextureUniform(const SCP_string& name, const int texture_unit)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()),
			  "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident        = false;

	if (uniform_index != INVALID_SIZE) {
		Assert(uniform_index < _uniforms.size());

		uniform_bind* bind_info = &_uniforms[uniform_index];

		if (bind_info->value == texture_unit) {
			return;
		}

		bind_info->value = texture_unit;
		resident         = true;
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		uniform_bind new_bind;

		new_bind.name  = name;
		new_bind.value = texture_unit;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform1i(findUniformLocation(name.c_str()), texture_unit);
}

GLint opengl::ShaderUniforms::findUniformLocation(const SCP_string& name) {
	auto iter = _uniform_locations.find(name);

	if (iter == _uniform_locations.end()) {
		// Lazily initialize the uniform locations when required. This avoids keeping a list of all uniforms in the code
		auto location = glGetUniformLocation(_program->getShaderHandle(), name.c_str());

		if (location == -1)
		{
			// This can happen if the uniform has been optimized out by the driver
			mprintf(("WARNING: Failed to find uniform '%s'.\n", name.c_str()));
		}

		_uniform_locations.insert(std::make_pair(name, location));
		return location;
	}
	else {
		return iter->second;
	}
}
