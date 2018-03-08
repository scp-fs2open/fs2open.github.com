
#include "gamma.sdr"

in vec4 fragTexCoord;
in vec4 fragColor;
out vec4 fragOut0;
uniform sampler2D baseMap;
uniform int noTexturing;
uniform int srgb;

void main()
{
	vec4 baseColor = texture(baseMap, fragTexCoord.xy);

	baseColor.rgb = (srgb == 1) ? srgb_to_linear(baseColor.rgb) : baseColor.rgb;
	vec4 blendColor = (srgb == 1) ? vec4(srgb_to_linear(fragColor.rgb), fragColor.a) : fragColor;
	fragOut0 = mix(baseColor * blendColor, blendColor, float(noTexturing));
}
