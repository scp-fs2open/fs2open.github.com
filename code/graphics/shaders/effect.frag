#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "gamma.sdr"

layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec4 fragTexCoord;
layout(location = 2) in vec4 fragColor;
layout(location = 3) in float fragRadius;

layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2DArray baseMap;
layout(set = 1, binding = 4) uniform sampler2D depthMap;

layout(set = 2, binding = 0, std140) uniform genericData {
	float window_width;
	float window_height;
	float nearZ;
	float farZ;
	int linear_depth;
	int srgb;
	int blend_alpha;
};

void main()
{
	vec4 fragmentColor = texture(baseMap, fragTexCoord.xyz);
	fragmentColor.rgb = mix(fragmentColor.rgb, srgb_to_linear(fragmentColor.rgb), float(srgb));
	fragmentColor *= mix(fragColor, vec4(srgb_to_linear(fragColor.rgb), fragColor.a), float(srgb));
	vec2 offset = vec2(fragRadius * abs(0.5 - fragTexCoord.x) * 2.0, fragRadius * abs(0.5 - fragTexCoord.y) * 2.0);
	float offset_len = length(offset);
	if ( offset_len > fragRadius ) {
		fragOut0 = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}
	vec2 depthCoord = vec2(gl_FragCoord.x / window_width, gl_FragCoord.y / window_height );
	vec4 sceneDepth = texture(depthMap, depthCoord);
	float sceneDepthLinear;
	float fragDepthLinear;
	if ( linear_depth == 1 ) {
		// Background pixels have position (0,0,0) from G-buffer clear;
		// treat as infinitely far so particles remain visible against background
		sceneDepthLinear = sceneDepth.z != 0.0 ? -sceneDepth.z : farZ;
		fragDepthLinear = -fragPosition.z;
	} else {
		sceneDepthLinear = ( 2.0 * farZ * nearZ ) / ( farZ + nearZ - sceneDepth.x * (farZ-nearZ) );
		fragDepthLinear = ( 2.0 * farZ * nearZ ) / ( farZ + nearZ - gl_FragCoord.z * (farZ-nearZ) );
	}
	// assume UV of 0.5, 0.5 is the centroid of this sphere volume
	float depthOffset = sqrt((fragRadius*fragRadius) - (offset_len*offset_len));
	float frontDepth = fragDepthLinear - depthOffset;
	float backDepth = fragDepthLinear + depthOffset;
	float intensity = smoothstep(max(nearZ, frontDepth), backDepth, sceneDepthLinear);
	fragmentColor.rgb *= (srgb == 1) ? 1.5 : 1.0;
	fragmentColor = (blend_alpha == 1) ? vec4(fragmentColor.rgb, fragmentColor.a * intensity) : vec4(fragmentColor.rgb * intensity, fragmentColor.a);
	fragOut0 = max(fragmentColor, vec4(0.0));
}
