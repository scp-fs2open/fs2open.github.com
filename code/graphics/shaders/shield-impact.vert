#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vertPosition;
layout(location = 3) in vec3 vertNormal;

layout(location = 0) out vec4 fragImpactUV;
layout(location = 1) out float fragNormOffset;

layout (set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

layout (set = 2, binding = 0, std140) uniform genericData {
	mat4 shieldModelViewMatrix;
	mat4 shieldProjMatrix;

	vec3 hitNormal;
	int srgb;

	vec4 color;

	int shieldMapIndex;
};

void main()
{
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
	fragNormOffset = dot(hitNormal, vertNormal);
	fragImpactUV = shieldProjMatrix * shieldModelViewMatrix * vertPosition;
	fragImpactUV += 1.0;
	fragImpactUV *= 0.5;
}
