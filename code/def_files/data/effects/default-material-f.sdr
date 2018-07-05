
#include "gamma.sdr"

in vec4 fragTexCoord;
in vec4 fragColor;
out vec4 fragOut0;

uniform sampler2DArray baseMap;
uniform int baseMapIndex;
uniform int alphaTexture;
uniform int noTexturing;
uniform int srgb;
uniform float intensity;
uniform float alphaThreshold;

void main()
{
	vec4 baseColor = texture(baseMap, vec3(fragTexCoord.xy, float(baseMapIndex)));
	if(alphaThreshold > baseColor.a) discard;
	baseColor.rgb = (srgb == 1) ? srgb_to_linear(baseColor.rgb) : baseColor.rgb;
	vec4 blendColor = (srgb == 1) ? vec4(srgb_to_linear(fragColor.rgb), fragColor.a) : fragColor;
	fragOut0 = mix(mix(baseColor * blendColor, vec4(blendColor.rgb, baseColor.r * blendColor.a), float(alphaTexture)), blendColor, float(noTexturing)) * intensity;
}
