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
			Assertion(false, "Unhandled shader type found!");
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

opengl::ShaderProgram::ShaderProgram(ShaderProgram&& other): _program_id(0), Uniforms(this) {
	*this = std::move(other);
}

opengl::ShaderProgram& opengl::ShaderProgram::operator=(ShaderProgram&& other) {
	std::swap(_program_id, other._program_id);
	std::swap(Uniforms, other.Uniforms);

	return *this;
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
		// Not available, ignore
		return;
	}

	_attribute_locations.insert(std::make_pair(name, attrib_loc));

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
GLint opengl::ShaderProgram::getAttributeLocation(const SCP_string& name) {
	auto iter = _attribute_locations.find(name);
	if (iter == _attribute_locations.end()) {
		return -1;
	} else {
		return iter->second;
	}
}

opengl::ShaderUniforms::ShaderUniforms(ShaderProgram* shaderProgram) : _program(shaderProgram) {
	Assertion(shaderProgram != nullptr, "Shader program may not be null!");
}

size_t opengl::ShaderUniforms::findUniform(const SCP_string& name)
{
	auto iter = _uniform_lookup.find(name);

	if (iter == _uniform_lookup.end()) {
		return INVALID_SIZE;
	}
	else {
		return iter->second;
	}
}
void opengl::ShaderUniforms::setUniformi(const SCP_string &name, const int val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert(uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::INT) {
			if (_uniform_data_ints[bind_info->index] == val) {
				return;
			}

			_uniform_data_ints[bind_info->index] = val;
			resident = true;
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		_uniform_data_ints.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = _uniform_data_ints.size() - 1;
		new_bind.type = uniform_bind::INT;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform1i(findUniformLocation(name.c_str()), val);
}

void opengl::ShaderUniforms::setUniform1iv(const SCP_string &name, const int count, const int *val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	auto uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != INVALID_SIZE) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::INT && bind_info->count == count) {
			bool equal = true;

			// if the values are close enough, pass.
			for (int i = 0; i < count; ++i) {
				if (val[i] != _uniform_data_ints[bind_info->index + i]) {
					equal = false;
					break;
				}
			}

			if (equal) {
				return;
			}

			resident = true;
			for (int i = 0; i < count; ++i) {
				_uniform_data_ints[bind_info->index + i] = val[i];
			}
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		for (int i = 0; i < count; ++i) {
			_uniform_data_ints.push_back(val[i]);
		}

		uniform_bind new_bind;
		new_bind.count = count;
		new_bind.index = _uniform_data_ints.size() - count;
		//	new_bind.index = num_matrix_uniforms - count;
		new_bind.type = uniform_bind::INT;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform1iv(findUniformLocation(name.c_str()), count, (const GLint*)val);
}

void opengl::ShaderUniforms::setUniformf(const SCP_string &name, const float val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::FLOAT) {
			if (fl_equal(_uniform_data_floats[bind_info->index], val)) {
				return;
			}

			_uniform_data_floats[bind_info->index] = val;
			resident = true;
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		_uniform_data_floats.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = _uniform_data_floats.size() - 1;
		new_bind.type = uniform_bind::FLOAT;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform1f(findUniformLocation(name.c_str()), val);
}

void opengl::ShaderUniforms::setUniform2f(const SCP_string &name, const float x, const float y)
{
	vec2d temp;

	temp.x = x;
	temp.y = y;

	setUniform2f(name, temp);
}

void opengl::ShaderUniforms::setUniform2f(const SCP_string &name, const vec2d &val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::VEC2) {
			if (vm_vec_equal(_uniform_data_vec2d[bind_info->index], val)) {
				return;
			}

			_uniform_data_vec2d[bind_info->index] = val;
			resident = true;
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		_uniform_data_vec2d.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = _uniform_data_vec2d.size() - 1;
		new_bind.type = uniform_bind::VEC2;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform2f(findUniformLocation(name.c_str()), val.x, val.y);
}

void opengl::ShaderUniforms::setUniform3f(const SCP_string &name, const float x, const float y, const float z)
{
	vec3d temp;

	temp.xyz.x = x;
	temp.xyz.y = y;
	temp.xyz.z = z;

	setUniform3f(name, temp);
}

void opengl::ShaderUniforms::setUniform3f(const SCP_string &name, const vec3d &val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::VEC3) {
			if (vm_vec_equal(_uniform_data_vec3d[bind_info->index], val)) {
				return;
			}

			_uniform_data_vec3d[bind_info->index] = val;
			resident = true;
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		_uniform_data_vec3d.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = _uniform_data_vec3d.size() - 1;
		new_bind.type = uniform_bind::VEC3;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform3f(findUniformLocation(name.c_str()), val.a1d[0], val.a1d[1], val.a1d[2]);
}

void opengl::ShaderUniforms::setUniform4f(const SCP_string &name, const float x, const float y, const float z, const float w)
{
	vec4 temp;

	temp.xyzw.x = x;
	temp.xyzw.y = y;
	temp.xyzw.z = z;
	temp.xyzw.w = w;

	setUniform4f(name, temp);
}

void opengl::ShaderUniforms::setUniform4f(const SCP_string &name, const vec4 &val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::VEC4) {
			if (vm_vec_equal(_uniform_data_vec4[bind_info->index], val)) {
				// if the values are close enough, pass.
				return;
			}

			_uniform_data_vec4[bind_info->index] = val;
			resident = true;
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		_uniform_data_vec4.push_back(val);

		uniform_bind new_bind;

		new_bind.count = 1;
		new_bind.index = _uniform_data_vec4.size() - 1;
		new_bind.type = uniform_bind::VEC4;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform4f(findUniformLocation(name.c_str()), val.a1d[0], val.a1d[1], val.a1d[2], val.a1d[3]);
}

void opengl::ShaderUniforms::setUniform1fv(const SCP_string &name, const int count, const float *val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	auto uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != INVALID_SIZE) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::FLOAT && bind_info->count == count) {
			bool equal = true;

			// if the values are close enough, pass.
			for (int i = 0; i < count; ++i) {
				if (!fl_equal(val[i], _uniform_data_floats[bind_info->index + i])) {
					equal = false;
					break;
				}
			}

			if (equal) {
				return;
			}

			resident = true;
			for (int i = 0; i < count; ++i) {
				_uniform_data_floats[bind_info->index + i] = val[i];
			}
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		for (int i = 0; i < count; ++i) {
			_uniform_data_floats.push_back(val[i]);
		}

		uniform_bind new_bind;
		new_bind.count = count;
		new_bind.index = _uniform_data_floats.size() - count;
		//	new_bind.index = num_matrix_uniforms - count;
		new_bind.type = uniform_bind::FLOAT;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform1fv(findUniformLocation(name.c_str()), count, (const GLfloat*)val);
}

void opengl::ShaderUniforms::setUniform3fv(const SCP_string &name, const int count, const vec3d *val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	auto uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != INVALID_SIZE) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::VEC3 && bind_info->count == count) {
			bool equal = true;

			// if the values are close enough, pass.
			for (int i = 0; i < count; ++i) {
				if (!vm_vec_equal(val[i], _uniform_data_vec3d[bind_info->index + i])) {
					equal = false;
					break;
				}
			}

			if (equal) {
				return;
			}

			resident = true;
			for (int i = 0; i < count; ++i) {
				_uniform_data_vec3d[bind_info->index + i] = val[i];
			}
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		for (int i = 0; i < count; ++i) {
			_uniform_data_vec3d.push_back(val[i]);
		}

		uniform_bind new_bind;
		new_bind.count = count;
		new_bind.index = _uniform_data_vec3d.size() - count;
		//	new_bind.index = num_matrix_uniforms - count;
		new_bind.type = uniform_bind::VEC3;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform3fv(findUniformLocation(name.c_str()), count, (const GLfloat*)val);
}

void opengl::ShaderUniforms::setUniform4fv(const SCP_string &name, const int count, const vec4 *val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	auto uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != INVALID_SIZE) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::VEC4 && bind_info->count == count) {
			bool equal = true;

			// if the values are close enough, pass.
			for (int i = 0; i < count; ++i) {
				if (!vm_vec_equal(val[i], _uniform_data_vec4[bind_info->index + i])) {
					equal = false;
					break;
				}
			}

			if (equal) {
				return;
			}

			resident = true;
			for (int i = 0; i < count; ++i) {
				_uniform_data_vec4[bind_info->index + i] = val[i];
			}
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		for (int i = 0; i < count; ++i) {
			_uniform_data_vec4.push_back(val[i]);
		}

		uniform_bind new_bind;
		new_bind.count = count;
		new_bind.index = _uniform_data_vec4.size() - count;
		//	new_bind.index = num_matrix_uniforms - count;
		new_bind.type = uniform_bind::VEC4;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniform4fv(findUniformLocation(name.c_str()), count, (const GLfloat*)val);
}

void opengl::ShaderUniforms::setUniformMatrix4f(const SCP_string &name, const matrix4 &val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::MATRIX4 && bind_info->count == 1) {
			if (vm_matrix_equal(_uniform_data_matrix4[bind_info->index], val)) {
				return;
			}

			_uniform_data_matrix4[bind_info->index] = val;
			resident = true;
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		//matrix_uniform_data[num_matrix_uniforms] = val;
		//memcpy(&(matrix_uniform_data[num_matrix_uniforms]), &val, sizeof(matrix4));
		_uniform_data_matrix4.push_back(val);
		//	num_matrix_uniforms += 1;

		uniform_bind new_bind;
		new_bind.count = 1;
		new_bind.index = _uniform_data_matrix4.size() - 1;
		//	new_bind.index = num_matrix_uniforms - 1;
		new_bind.type = uniform_bind::MATRIX4;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniformMatrix4fv(findUniformLocation(name.c_str()), 1, GL_FALSE, (const GLfloat*)&val);
}

void opengl::ShaderUniforms::setUniformMatrix4fv(const SCP_string &name, const int count, const matrix4 *val)
{
	Assertion(GL_state.IsCurrentProgram(_program->getShaderHandle()), "The program must be current before setting uniforms!");

	size_t uniform_index = findUniform(name);
	bool resident = false;

	if (uniform_index != (size_t)-1) {
		Assert((size_t)uniform_index < _uniforms.size());

		uniform_bind *bind_info = &_uniforms[uniform_index];

		if (bind_info->type == uniform_bind::MATRIX4 && bind_info->count == count) {
			bool equal = true;

			// if the values are close enough, pass.
			for (int i = 0; i < count; ++i) {
				if (!vm_matrix_equal(val[i], _uniform_data_matrix4[bind_info->index + i])) {
					equal = false;
					break;
				}
			}

			if (equal) {
				return;
			}

			resident = true;
			for (int i = 0; i < count; ++i) {
				_uniform_data_matrix4[bind_info->index + i] = val[i];
			}
		}
	}

	if (!resident) {
		// uniform doesn't exist in our previous uniform block so queue this new value
		for (int i = 0; i < count; ++i) {
			_uniform_data_matrix4.push_back(val[i]);
		}

		uniform_bind new_bind;
		new_bind.count = count;
		new_bind.index = _uniform_data_matrix4.size() - count;
		//	new_bind.index = num_matrix_uniforms - count;
		new_bind.type = uniform_bind::MATRIX4;
		new_bind.name = name;

		_uniforms.push_back(new_bind);

		_uniform_lookup[name] = _uniforms.size() - 1;
	}

	glUniformMatrix4fv(findUniformLocation(name.c_str()), count, GL_FALSE, (const GLfloat*)val);
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
