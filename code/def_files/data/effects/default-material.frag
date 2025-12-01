#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "gamma.sdr"

layout (location = 0) in vec4 fragTexCoord;
layout (location = 1) in vec4 fragColor;

layout (location = 0) out vec4 fragOut0;

// Set 0: Uniform buffers (bindings match uniform_block_type enum)
// GenericData = 8
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

// Set 1: Material textures
// Using sampler2D for now (not sampler2DArray) until texture array support is added
layout(set = 1, binding = 0) uniform sampler2D baseMap;

void main()
{
	// Note: baseMapIndex is ignored for now - texture arrays not yet implemented
	vec4 baseColor = texture(baseMap, fragTexCoord.xy);
	if(alphaThreshold > baseColor.a) discard;
	baseColor.rgb = (srgb == 1) ? srgb_to_linear(baseColor.rgb) : baseColor.rgb;
	vec4 blendColor = (srgb == 1) ? vec4(srgb_to_linear(fragColor.rgb), fragColor.a) : fragColor;
	fragOut0 = mix(mix(baseColor * blendColor, vec4(blendColor.rgb, baseColor.r * blendColor.a), float(alphaTexture)), blendColor, float(noTexturing)) * intensity;
}
