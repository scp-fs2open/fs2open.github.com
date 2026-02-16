#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D tex;

void main()
{
	vec4 color = texture(tex, fragTexCoord);
	fragOut0 = vec4(max(vec3(0.0), color.rgb - vec3(1.0)), 1.0);
}
