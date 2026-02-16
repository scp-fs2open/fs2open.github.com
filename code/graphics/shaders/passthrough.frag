#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "gamma.sdr"

// Inputs from vertex shader
layout (location = 0) in vec4 fragTexCoord;
layout (location = 1) in vec4 fragColor;

// Output
layout (location = 0) out vec4 fragOut0;

// Texture sampler (binding 1 in Material set texture array)
layout (set = 1, binding = 1) uniform sampler2DArray baseMap;

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
	vec4 baseColor = texture(baseMap, vec3(fragTexCoord.xy, float(baseMapIndex)));

	baseColor.rgb = (srgb == 1) ? srgb_to_linear(baseColor.rgb) : baseColor.rgb;
	vec4 blendColor = (srgb == 1) ? vec4(srgb_to_linear(fragColor.rgb), fragColor.a) : fragColor;
	fragOut0 = mix(baseColor * blendColor, blendColor, float(noTexturing));
}
