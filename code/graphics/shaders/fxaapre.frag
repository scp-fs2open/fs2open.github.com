#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D tex;

void main()
{
	vec4 color = texture(tex, fragTexCoord);
	// Store computed luma in alpha channel for FXAA main pass
	fragOut0 = vec4(color.rgb, dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}
