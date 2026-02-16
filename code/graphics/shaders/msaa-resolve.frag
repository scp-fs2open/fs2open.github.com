#version 450
#extension GL_ARB_separate_shader_objects : enable

// MSAA resolve fragment shader — depth-weighted resolve from multisampled G-buffer
// to non-MSAA G-buffer. Ported from OpenGL msaa-f.sdr.

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 fragOut0; // color
layout(location = 1) out vec4 fragOut1; // position
layout(location = 2) out vec4 fragOut2; // normal
layout(location = 3) out vec4 fragOut3; // specular
layout(location = 4) out vec4 fragOut4; // emissive

// All 6 MSAA input textures via Material Set (Set 1) Binding 1 texture array.
// Elements 0-5 hold MSAA image views; 6-15 are fallback (unused by this shader).
// [0]=color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=depth
layout(set = 1, binding = 1) uniform sampler2DMS msaaTex[6];

// GenericData UBO at PerDraw Set (Set 2) Binding 0
layout(std140, set = 2, binding = 0) uniform genericData {
	int samples;
	float fov;
};

const float voxelDepth = 2.5;
const float voxelDepthFalloff = 2.5;

// Runtime fallback median distance — simple max loop (no sorting networks needed)
float getMedianDist(ivec2 texel) {
	float minDist = -1000000.0;
	for (int i = 0; i < samples; i++) {
		minDist = max(minDist, texelFetch(msaaTex[1], texel, i).z);
	}
	return minDist;
}

void main()
{
	vec2 texSize = vec2(textureSize(msaaTex[0]));
	ivec2 texel = ivec2(texSize * fragTexCoord);

	float texelWidthFactor = tan(fov / texSize.y);
	float dist = getMedianDist(texel);

	float weight = 0.0;
	vec4 color = vec4(0.0);
	vec4 pos = vec4(0.0);
	vec4 normal = vec4(0.0);
	vec4 specular = vec4(0.0);
	vec4 emissive = vec4(0.0);
	float depth = 0.0;

	for (int i = 0; i < samples; i++) {
		vec4 localPos = texelFetch(msaaTex[1], texel, i);
		// Calculate local weight from distance voxel. If the distance is 0
		// (no model), set weight to 1 to allow background emissive through.
		// If median distance is 0, only process samples that are also 0.
		float localWeight = max(
			step(-0.001, dist) * step(-0.001, localPos.z),
			smoothstep(dist + dist * texelWidthFactor * (voxelDepth + voxelDepthFalloff),
			           dist + dist * texelWidthFactor * voxelDepth, localPos.z) *
			smoothstep(dist - dist * texelWidthFactor * voxelDepth,
			           dist + dist * texelWidthFactor * (voxelDepth + voxelDepthFalloff), localPos.z)
		);

		pos += localPos * localWeight;
		color += texelFetch(msaaTex[0], texel, i) * localWeight;
		normal += texelFetch(msaaTex[2], texel, i) * localWeight;
		specular += texelFetch(msaaTex[3], texel, i) * localWeight;
		emissive += texelFetch(msaaTex[4], texel, i) * localWeight;
		depth += texelFetch(msaaTex[5], texel, i).x * localWeight;
		weight += localWeight;
	}

	fragOut0 = color / weight;
	fragOut1 = pos / weight;
	fragOut2 = vec4(normalize(normal.xyz), normal.a / weight);
	fragOut3 = specular / weight;
	fragOut4 = emissive / weight;
	gl_FragDepth = depth / weight;
}
