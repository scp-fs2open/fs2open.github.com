#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec4 vertColor;
layout(location = 2) in vec4 vertTexCoord;
layout(location = 6) in float vertRadius;

layout(location = 0) out vec4 fragTexCoord;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out float fragOffset;

layout(set = 2, binding = 0, std140) uniform GenericData {
	float window_width;
	float window_height;
	float use_offset;
	float pad;
};

layout(set = 2, binding = 1, std140) uniform Matrices {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

void main()
{
	fragTexCoord = vertTexCoord;
	fragColor = vertColor;
	fragOffset = vertRadius * use_offset;
	gl_Position = projMatrix * modelViewMatrix * vec4(vertPosition, 1.0);
}
