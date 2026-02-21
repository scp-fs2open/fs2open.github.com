#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex inputs (note: rocketui uses vec2 for position)
layout (location = 0) in vec2 vertPosition;
layout (location = 1) in vec4 vertColor;
layout (location = 2) in vec2 vertTexCoord;

// Outputs to fragment shader
layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out vec4 fragColor;
layout (location = 2) out vec2 fragScreenPosition;

// Uniform buffer: GenericData/RocketUI (binding 0 in PerDraw set)
layout (set = 2, binding = 0, std140) uniform genericData {
	mat4 projMatrix;

	vec2 offset;
	int textured;
	int baseMapIndex;

	float horizontalSwipeOffset;
	float pad[3];
};

void main()
{
	fragTexCoord = vertTexCoord;
	fragColor = vertColor;

	vec4 position = vec4(vertPosition + offset, 0.0, 1.0);

	fragScreenPosition = position.xy;
	gl_Position = projMatrix * position;
}
