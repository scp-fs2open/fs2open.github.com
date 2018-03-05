
#include "gamma.sdr"

in vec4 fragImpactUV;
in float fragNormOffset;
uniform vec4 color;
uniform sampler2DArray shieldMap;
uniform int shieldMapIndex;
uniform int srgb;
out vec4 fragOut0;
#define EMISSIVE_GAIN 2.0
void main()
{
	if(fragNormOffset < 0.0) discard;
	if(fragImpactUV.x < 0.0 || fragImpactUV.x > 1.0 || fragImpactUV.y < 0.0 || fragImpactUV.y > 1.0) discard;
	vec4 shieldColor = texture(shieldMap, vec3(fragImpactUV.xy, float(shieldMapIndex)));
	shieldColor.rgb = (srgb == 1) ? srgb_to_linear(shieldColor.rgb) * EMISSIVE_GAIN : shieldColor.rgb;
	vec4 blendColor = color;
	blendColor.rgb = (srgb == 1) ? srgb_to_linear(blendColor.rgb) * EMISSIVE_GAIN : blendColor.rgb;
	fragOut0 = shieldColor * blendColor;
}
