#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragTexCoord;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in float fragOffset;

layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2DArray baseMap;
layout(set = 1, binding = 5) uniform sampler2D frameBuffer;
layout(set = 1, binding = 6) uniform sampler2D distMap;

layout(set = 2, binding = 0, std140) uniform GenericData {
	float window_width;
	float window_height;
	float use_offset;
	float pad;
};

void main()
{
	vec2 depthCoord = vec2(gl_FragCoord.x / window_width, gl_FragCoord.y / window_height);

	// Sample distortion offset from ping-pong distortion texture
	vec2 distortion = texture(distMap, fragTexCoord.xy + vec2(0.0, fragOffset)).rg;

	// Get particle alpha from base texture (multiply by vertex alpha only, not RGB)
	vec4 fragmentColor = texture(baseMap, fragTexCoord.xyz) * fragColor.a;

	// Scale distortion by particle luminance
	float alpha = clamp(dot(fragmentColor.rgb, vec3(0.3333)) * 10.0, 0.0, 1.0);
	distortion = ((distortion - 0.5) * 0.01) * alpha;

	// Sample scene color at distorted UV, blend via particle alpha
	fragOut0 = texture(frameBuffer, depthCoord + distortion);
	fragOut0.a = alpha;
}
