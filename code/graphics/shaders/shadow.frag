#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

void main()
{
	float depth = gl_FragCoord.z;
	// Variance Shadow Mapping: store (depth, depth^2 * scale_inv, 0, 1)
	// VARIANCE_SHADOW_SCALE = 1000000.0 in shadows.sdr
	fragColor = vec4(depth, depth * depth * (1.0 / 1000000.0), 0.0, 1.0);
}
