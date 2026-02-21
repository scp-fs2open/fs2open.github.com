#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.sdr"

layout(location = 0) in vec4 vertPosition;

layout(set = 0, binding = 0, std140) uniform lightData {
	vec3 diffuseLightColor;
	float coneAngle;

	vec3 lightDir;
	float coneInnerAngle;

	vec3 coneDir;
	float dualCone;

	vec3 scale;
	float lightRadius;

	int lightType;
	int enable_shadows;
	float sourceRadius;

	float pad0;
};

layout(set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

void main()
{
	if (lightType == LT_DIRECTIONAL || lightType == LT_AMBIENT) {
		// Fullscreen triangle from gl_VertexIndex (same as postprocess.vert)
		vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
		gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
	} else {
		gl_Position = projMatrix * modelViewMatrix * vec4(vertPosition.xyz * scale, 1.0);
	}
}
