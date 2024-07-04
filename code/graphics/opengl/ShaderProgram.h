
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

		int value;
	};

	ShaderProgram* _program;

	SCP_vector<uniform_bind> _uniforms;

	SCP_unordered_map<SCP_string, size_t> _uniform_lookup;

	SCP_unordered_map<SCP_string, GLint> _uniform_locations;

	size_t findUniform(const SCP_string &name);
	GLint findUniformLocation(const SCP_string& name);
 public:
	explicit ShaderUniforms(ShaderProgram* shaderProgram);

	void setTextureUniform(const SCP_string &name, const int texture_unit);
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

	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	ShaderProgram(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;

	void use();

	void addShaderCode(ShaderStage stage, const SCP_string& name, const SCP_vector<SCP_string>& codeParts);

	void linkProgram();

	void initAttribute(const SCP_string& name, opengl_vert_attrib::attrib_id attr_id, const vec4& default_value);

	GLint getAttributeLocation(opengl_vert_attrib::attrib_id attribute);

	GLuint getShaderHandle();
};

}

#endif // OPENGL_SHADER_PROGRAM_H
