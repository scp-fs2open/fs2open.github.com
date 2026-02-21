#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 vertPosition;
layout (location = 1) in vec4 vertColor;
layout (location = 2) in vec4 vertTexCoord;

layout (location = 0) out vec4 fragTexCoord;
layout (location = 1) out vec4 fragColor;

layout (set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

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
	fragTexCoord = vertTexCoord;
	fragColor = vertColor * color;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;

	// Vulkan has no glEnable(GL_CLIP_DISTANCE0) equivalent — clip distances are
	// always evaluated when declared in the shader.  Must write a positive value
	// when clipping is disabled to prevent undefined-value vertex culling.
	if (clipEnabled != 0u) {
		gl_ClipDistance[0] = dot(clipEquation, modelMatrix * vertPosition);
	} else {
		gl_ClipDistance[0] = 1.0;
	}
}
