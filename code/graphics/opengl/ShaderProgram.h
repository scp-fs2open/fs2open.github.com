
#ifndef OPENGL_SHADER_PROGRAM_H
#define OPENGL_SHADER_PROGRAM_H
#pragma once

#include "globalincs/pstypes.h"
#include "gropenglshader.h"

#include <glad/glad.h>

namespace opengl {

class ShaderProgram;
class ShaderUniforms {
	struct uniform_bind
	{
		SCP_string name;

		enum data_type {
			INT,
			FLOAT,
			VEC2,
			VEC3,
			VEC4,
			MATRIX4
		};

		uniform_bind::data_type type;
		size_t index;

		int count;
		int tranpose;
	};

	ShaderProgram* _program;

	SCP_vector<uniform_bind> _uniforms;

	SCP_vector<int> _uniform_data_ints;
	SCP_vector<float> _uniform_data_floats;
	SCP_vector<vec2d> _uniform_data_vec2d;
	SCP_vector<vec3d> _uniform_data_vec3d;
	SCP_vector<vec4> _uniform_data_vec4;
	SCP_vector<matrix4> _uniform_data_matrix4;

	SCP_unordered_map<SCP_string, size_t> _uniform_lookup;

	SCP_unordered_map<SCP_string, GLint> _uniform_locations;

	size_t findUniform(const SCP_string &name);
	GLint findUniformLocation(const SCP_string& name);
 public:
	explicit ShaderUniforms(ShaderProgram* shaderProgram);

	void setUniformi(const SCP_string &name, const int value);
	void setUniform1iv(const SCP_string &name, const int count, const int *val);
	void setUniformf(const SCP_string &name, const float value);
	void setUniform2f(const SCP_string &name, const float x, const float y);
	void setUniform2f(const SCP_string &name, const vec2d &val);
	void setUniform3f(const SCP_string &name, const float x, const float y, const float z);
	void setUniform3f(const SCP_string &name, const vec3d &value);
	void setUniform4f(const SCP_string &name, const float x, const float y, const float z, const float w);
	void setUniform4f(const SCP_string &name, const vec4 &val);
	void setUniform1fv(const SCP_string &name, const int count, const float *val);
	void setUniform3fv(const SCP_string &name, const int count, const vec3d *val);
	void setUniform4fv(const SCP_string &name, const int count, const vec4 *val);
	void setUniformMatrix4fv(const SCP_string &name, const int count, const matrix4 *value);
	void setUniformMatrix4f(const SCP_string &name, const matrix4 &val);
};

enum ShaderStage {
	STAGE_VERTEX,
	STAGE_GEOMETRY,
	STAGE_FRAGMENT
};

class ShaderProgram {
	GLuint _program_id;

	SCP_vector<GLuint> _compiled_shaders;

	SCP_unordered_map<opengl_vert_attrib::attrib_id, GLint> _attribute_locations;

	void freeCompiledShaders();
 public:
	explicit ShaderProgram(const SCP_string& program_name);
	~ShaderProgram();

	ShaderUniforms Uniforms;

	ShaderProgram(const ShaderProgram&) SCP_DELETED_FUNCTION;
	ShaderProgram& operator=(const ShaderProgram&) SCP_DELETED_FUNCTION;

	ShaderProgram(ShaderProgram&& other);
	ShaderProgram& operator=(ShaderProgram&& other);

	void use();

	void addShaderCode(ShaderStage stage, const SCP_string& name, const SCP_vector<SCP_string>& codeParts);

	void linkProgram();

	void initAttribute(const SCP_string& name, opengl_vert_attrib::attrib_id attr_id, const vec4& default_value);

	GLint getAttributeLocation(opengl_vert_attrib::attrib_id attribute);

	GLuint getShaderHandle();
};

}

#endif // OPENGL_SHADER_PROGRAM_H
