#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex inputs - only position and texcoord are required
// Color comes from uniform to support all vertex layouts
layout (location = 0) in vec4 vertPosition;
layout (location = 2) in vec4 vertTexCoord;

layout (location = 0) out vec4 fragTexCoord;
layout (location = 1) out vec4 fragColor;

// Set 0: Uniform buffers (bindings match uniform_block_type enum)
// Matrices = 6, GenericData = 8
layout (set = 0, binding = 6, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

layout (set = 0, binding = 8, std140) uniform genericData {
	mat4 modelMatrix;

	vec4 color;

	vec4 clipEquation;

	int baseMapIndex;
	int alphaTexture;
	int noTexturing;
	int srgb;

	float intensity;
	float alphaThreshold;
	bool clipEnabled;
};

void main()
{
	fragTexCoord = vertTexCoord;
	// Use uniform color since not all vertex layouts provide vertex colors
	fragColor = color;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;

	if (clipEnabled) {
		gl_ClipDistance[0] = dot(clipEquation, modelMatrix * vertPosition);
	}
}
