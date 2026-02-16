#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D tex;

layout(std140, set = 2, binding = 0) uniform genericData {
	float texSize;
	int level;
	int direction; // 0 = horizontal, 1 = vertical
};

void main()
{
	float BlurWeights[6];
	BlurWeights[0] = 0.1362;
	BlurWeights[1] = 0.1297;
	BlurWeights[2] = 0.1120;
	BlurWeights[3] = 0.0877;
	BlurWeights[4] = 0.0623;
	BlurWeights[5] = 0.0402;

	vec4 sum = textureLod(tex, fragTexCoord, float(level)) * BlurWeights[0];

	for (int i = 1; i < 6; i++) {
		float offset = float(i) * texSize;
		if (direction == 0) {
			sum += textureLod(tex, vec2(clamp(fragTexCoord.x - offset, 0.0, 1.0), fragTexCoord.y), float(level)) * BlurWeights[i];
			sum += textureLod(tex, vec2(clamp(fragTexCoord.x + offset, 0.0, 1.0), fragTexCoord.y), float(level)) * BlurWeights[i];
		} else {
			sum += textureLod(tex, vec2(fragTexCoord.x, clamp(fragTexCoord.y - offset, 0.0, 1.0)), float(level)) * BlurWeights[i];
			sum += textureLod(tex, vec2(fragTexCoord.x, clamp(fragTexCoord.y + offset, 0.0, 1.0)), float(level)) * BlurWeights[i];
		}
	}

	fragOut0 = sum;
}
