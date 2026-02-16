#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex inputs
layout (location = 0) in vec4 vertPosition;
layout (location = 1) in vec4 vertColor;
layout (location = 2) in vec4 vertTexCoord;

// Outputs to fragment shader
layout (location = 0) out vec4 fragTexCoord;
layout (location = 1) out vec4 fragColor;

// Uniform buffer: Matrices (binding 1 in PerDraw set)
layout (set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

// Uniform buffer: GenericData (binding 0 in PerDraw set)
// Must match the layout used by vulkan_set_default_material_uniforms()
layout (set = 2, binding = 0, std140) uniform genericData {
	mat4 modelMatrix;

	vec4 color;

	vec4 clipEquation;

	int baseMapIndex;
	int alphaTexture;
	int noTexturing;
	int srgb;

	float intensity;
	float alphaThreshold;
	uint clipEnabled;
};

void main()
{
	fragColor = vertColor * color;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
	fragTexCoord = vertTexCoord;
}
