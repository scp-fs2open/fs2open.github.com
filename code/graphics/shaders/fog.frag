#version 450
#extension GL_ARB_separate_shader_objects : enable

// Scene fog fragment shader — port of fog-f.sdr to Vulkan
// Applies distance-based exponential fog to the lit composite image.

#include "gamma.sdr"

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D tex;       // composite (lit scene)
layout(set = 1, binding = 4) uniform sampler2D depth_tex;  // scene depth copy

layout(std140, set = 2, binding = 0) uniform genericData {
	vec3 fog_color;
	float fog_start;

	float fog_density;
	float zNear;
	float zFar;

	float pad0;
};

void main()
{
	vec4 color_in = texture(tex, fragTexCoord.xy);

	float depth_val = texture(depth_tex, fragTexCoord.xy).x;
	// Vulkan depth range [0,1] — linearize directly (no 2*d-1 transform)
	float view_depth = zNear * zFar / (zFar - depth_val * (zFar - zNear));

	// Cap infinite depth: Vulkan's formula yields infinity at d=1.0 due to
	// float precision with extreme zFar. OpenGL's formula gives finite zFar
	// instead. Capping to zFar makes both renderers apply full fog to
	// background pixels.
	if (isinf(view_depth)) view_depth = zFar;

	float fog_dist = clamp(1 - pow(fog_density, view_depth - fog_start), 0.0, 1.0);
	vec3 finalFogColor = srgb_to_linear(fog_color);

	fragOut0.rgb = mix(color_in.rgb, finalFogColor, fog_dist);
	fragOut0.a = 1.0;
}
