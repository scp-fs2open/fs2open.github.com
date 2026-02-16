#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vertPosition;
layout(location = 1) in vec4 vertColor;
layout(location = 2) in vec4 vertTexCoord;
layout(location = 6) in float vertRadius;

layout(location = 0) out vec4 fragPosition;
layout(location = 1) out vec4 fragTexCoord;
layout(location = 2) out vec4 fragColor;
layout(location = 3) out float fragRadius;

layout(set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

void main()
{
	fragRadius = vertRadius;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
	fragPosition = modelViewMatrix * vertPosition;
	fragTexCoord = vec4(vertTexCoord.xyz, 0.0);
	fragColor = vertColor;
}
