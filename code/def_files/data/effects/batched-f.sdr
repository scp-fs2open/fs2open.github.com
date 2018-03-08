
#include "gamma.sdr"

in vec4 fragTexCoord;
in vec4 fragColor;
out vec4 fragOut0;
uniform sampler2DArray baseMap;
uniform float intensity;

void main()
{
	vec4 baseColor = texture(baseMap, fragTexCoord.xyz);

	baseColor.rgb = srgb_to_linear(baseColor.rgb);
	vec4 blendColor = vec4(srgb_to_linear(fragColor.rgb), fragColor.a);

	fragOut0 = baseColor * blendColor * intensity;
}
