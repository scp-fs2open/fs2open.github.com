#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "gamma.sdr"

const float EMISSIVE_GAIN = 2.0;

layout(location = 0) in vec4 fragImpactUV;
layout(location = 1) in float fragNormOffset;

layout(location = 0) out vec4 fragOut0;

layout (set = 1, binding = 1) uniform sampler2DArray shieldMap;

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
	if (fragNormOffset < 0.0) discard;
	if (fragImpactUV.x < 0.0 || fragImpactUV.x > 1.0 || fragImpactUV.y < 0.0 || fragImpactUV.y > 1.0) discard;
	vec4 shieldColor = texture(shieldMap, vec3(fragImpactUV.xy, float(shieldMapIndex)));
	shieldColor.rgb = (srgb == 1) ? srgb_to_linear(shieldColor.rgb) * EMISSIVE_GAIN : shieldColor.rgb;
	vec4 blendColor = color;
	blendColor.rgb = (srgb == 1) ? srgb_to_linear(blendColor.rgb) * EMISSIVE_GAIN : blendColor.rgb;
	fragOut0 = shieldColor * blendColor;
}
