#version 450
#extension GL_ARB_separate_shader_objects : enable

// Fullscreen triangle vertex shader for post-processing passes.
// Uses gl_VertexIndex to generate a single triangle covering the entire screen.
// No vertex buffer required — draw with vkCmdDraw(3, 1, 0, 0).

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	// Generate fullscreen triangle vertices from vertex index:
	//   0: (-1, -1) uv (0, 0)  — top-left in Vulkan NDC
	//   1: ( 3, -1) uv (2, 0)  — oversize right
	//   2: (-1,  3) uv (0, 2)  — oversize bottom
	// After viewport clipping, UV [0,1] maps to screen corners.
	vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	fragTexCoord = pos;
	gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
