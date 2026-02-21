#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D bloomed;

layout(std140, set = 2, binding = 0) uniform genericData {
	float bloom_intensity;
	int levels;
};

void main()
{
	vec4 color_out = vec4(0.0, 0.0, 0.0, 1.0);
	float factor = 0.0;
	for (int mipmap = 0; mipmap < levels; ++mipmap) {
		float scale = 1.0 / exp2(float(mipmap));
		factor += scale;
		color_out.rgb += textureLod(bloomed, fragTexCoord, float(mipmap)).rgb * scale;
	}
	color_out.rgb /= factor;
	color_out.rgb *= bloom_intensity;
	fragOut0 = color_out;
}
