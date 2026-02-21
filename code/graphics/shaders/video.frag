#version 450
#extension GL_ARB_separate_shader_objects : enable

// Inputs from vertex shader
layout (location = 0) in vec4 fragTexCoord;

// Output
layout (location = 0) out vec4 fragOut0;

// YUV textures use the texture array at binding 1 in Material set
// Array indices: 0 = Y, 1 = U, 2 = V
layout (set = 1, binding = 1) uniform sampler2DArray textures[16];

// Uniform buffer: MovieData (binding 4 in PerDraw set)
layout (set = 2, binding = 4, std140) uniform movieData {
	float alpha;
	float pad[3];
};

void main()
{
	// Sample YUV from texture array slots 0, 1, 2
	float y = texture(textures[0], vec3(fragTexCoord.st, 0.0)).r;
	float u = texture(textures[1], vec3(fragTexCoord.st, 0.0)).r;
	float v = texture(textures[2], vec3(fragTexCoord.st, 0.0)).r;
	vec3 val = vec3(y - 0.0625, u - 0.5, v - 0.5);
	fragOut0.r = dot(val, vec3(1.1640625, 0.0, 1.59765625));
	fragOut0.g = dot(val, vec3(1.1640625, -0.390625, -0.8125));
	fragOut0.b = dot(val, vec3(1.1640625, 2.015625, 0.0));
	fragOut0.a = alpha;
}
