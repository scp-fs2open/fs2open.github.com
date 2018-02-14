in vec4 fragTexCoord;
out vec4 fragOut0;
uniform sampler2D bloomed;
uniform float bloom_intensity;
uniform int levels;
void main() {
	vec4 color_out = vec4(0.0, 0.0, 0.0, 1.0);
	float factor = 0.0;
	for (int mipmap = 0; mipmap < levels; ++mipmap) {
		float scale = 1.0/exp2(float(mipmap));
		factor += scale;
		color_out.rgb += textureLod(bloomed, fragTexCoord.xy, float(mipmap)).rgb * scale;
	}
	color_out.rgb /= factor;
	color_out.rgb *= bloom_intensity;
	fragOut0 = color_out;
}