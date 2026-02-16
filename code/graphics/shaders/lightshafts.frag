#version 450
#extension GL_ARB_separate_shader_objects : enable

// Lightshafts (god rays) post-processing shader
// Raymarches from each fragment toward the sun position,
// accumulating brightness from depth==1.0 (sky) pixels.

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D scene; // Depth texture

const int SAMPLE_NUM = 50;

layout(std140, set = 2, binding = 0) uniform genericData {
	vec2 sun_pos;
	float density;
	float weight;

	float falloff;
	float intensity;
	float cp_intensity;

	float pad0;
};

void main()
{
	vec2 step = fragTexCoord.st - sun_pos.xy;
	vec2 pos = fragTexCoord.st;
	step *= 1.0 / float(SAMPLE_NUM) * density;

	float decay = 1.0;
	vec4 sum = vec4(0.0);

	// Raymarch from fragment toward sun, accumulating bright sky pixels
	for (int i = 0; i < SAMPLE_NUM; i++) {
		pos.st -= step;
		vec4 tex_sample = texture(scene, pos);
		if (tex_sample.r == 1.0) // Depth == 1.0 means far plane (sky)
			sum += decay * weight;
		decay *= falloff;
	}

	fragOut0 = sum * intensity;
	fragOut0.a = 1.0;
}
