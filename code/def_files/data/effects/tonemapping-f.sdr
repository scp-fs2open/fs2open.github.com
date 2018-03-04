
#include "gamma.sdr"

in vec4 fragTexCoord;
out vec4 fragOut0;
uniform sampler2D tex;
uniform float exposure;
vec3 Uncharted2Tonemapping(vec3 hdr_color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((hdr_color*(A*hdr_color+C*B)+D*E)/(hdr_color*(A*hdr_color+B)+D*F))-E/F;
}

void main()
{
	vec4 color = texture(tex, fragTexCoord.xy);
	// Tone mapping using John Hable's Uncharted 2 tonemapping algorithm.
	float whitepoint = 11.2f; // hardcoded whitepoint value from Hable's algo
	color.rgb = Uncharted2Tonemapping(color.rgb * exposure) / Uncharted2Tonemapping(vec3(whitepoint));
	color.rgb = linear_to_srgb(color.rgb); // return from linear color space to SRGB color space
	fragOut0 = vec4(color.rgb, 1.0);
}
