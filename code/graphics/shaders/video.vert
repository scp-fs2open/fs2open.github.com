#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex inputs
layout (location = 0) in vec4 vertPosition;
layout (location = 2) in vec4 vertTexCoord;

// Outputs to fragment shader
layout (location = 0) out vec4 fragTexCoord;

// Uniform buffer: Matrices (binding 1 in PerDraw set)
layout (set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

void main()
{
	fragTexCoord = vertTexCoord;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
}
