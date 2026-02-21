#version 450
#extension GL_ARB_separate_shader_objects : enable

// Fullscreen triangle vertex shader for irradiance map generation.
// Identical to postprocess.vert — draw with vkCmdDraw(3, 1, 0, 0).

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	fragTexCoord = pos;
	gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
