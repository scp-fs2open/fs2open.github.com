#version 450
#extension GL_ARB_separate_shader_objects : enable

// FXAA 3.11 - Medium Quality (Preset 26)
// Ported from NVIDIA FXAA 3.11 by Timothy Lottes
// Uses pre-computed luma in alpha channel (from FXAA prepass)

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D tex;

layout(std140, set = 2, binding = 0) uniform genericData {
	float rt_w;
	float rt_h;
	float pad0;
	float pad1;
};

// Quality parameters for Medium preset
const float EDGE_THRESHOLD     = 1.0 / 12.0;
const float EDGE_THRESHOLD_MIN = 1.0 / 24.0;
const float SUBPIX             = 0.33;

// Search step offsets for preset 26 (9 steps)
const float QUALITY_P[9] = float[9](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);

float FxaaLuma(vec4 rgba) { return rgba.y; }

void main()
{
	vec2 rcpFrame = vec2(1.0 / rt_w, 1.0 / rt_h);
	vec2 posM = fragTexCoord;

	// Sample center pixel (luma pre-computed in alpha by prepass)
	vec4 rgbyM = textureLod(tex, posM, 0.0);
	float lumaM = rgbyM.w;

	// Sample 4-connected neighbors (use green channel as luma approximation)
	float lumaS = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 0, 1)));
	float lumaE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 1, 0)));
	float lumaN = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 0,-1)));
	float lumaW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, 0)));

	// Local contrast range
	float rangeMax = max(max(max(lumaS, lumaE), max(lumaN, lumaW)), lumaM);
	float rangeMin = min(min(min(lumaS, lumaE), min(lumaN, lumaW)), lumaM);
	float range = rangeMax - rangeMin;

	// Early exit for low-contrast regions
	if (range < max(EDGE_THRESHOLD_MIN, rangeMax * EDGE_THRESHOLD)) {
		fragOut0 = rgbyM;
		return;
	}

	// Sample diagonal neighbors
	float lumaNW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1,-1)));
	float lumaSE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 1, 1)));
	float lumaNE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 1,-1)));
	float lumaSW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, 1)));

	// Subpixel blending factor
	float lumaNS = lumaN + lumaS;
	float lumaWE = lumaW + lumaE;
	float subpixNSWE = lumaNS + lumaWE;
	float subpixNWSWNESE = (lumaNW + lumaSW) + (lumaNE + lumaSE);
	float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
	float subpixB = (subpixA * (1.0 / 12.0)) - lumaM;
	float subpixC = clamp(abs(subpixB) / range, 0.0, 1.0);
	float subpixD = ((-2.0) * subpixC) + 3.0;
	float subpixE = subpixC * subpixC;
	float subpixF = subpixD * subpixE;
	float subpixH = subpixF * subpixF * SUBPIX;

	// Edge orientation detection (horizontal vs vertical)
	float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
	float edgeVert1 = (-2.0 * lumaM) + lumaWE;
	float edgeHorz2 = (-2.0 * lumaE) + (lumaNE + lumaSE);
	float edgeVert2 = (-2.0 * lumaN) + (lumaNW + lumaNE);
	float edgeHorz3 = (-2.0 * lumaW) + (lumaNW + lumaSW);
	float edgeVert3 = (-2.0 * lumaS) + (lumaSW + lumaSE);
	float edgeHorz = abs(edgeHorz3) + (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
	float edgeVert = abs(edgeVert3) + (abs(edgeVert1) * 2.0) + abs(edgeVert2);
	bool horzSpan = edgeHorz >= edgeVert;

	// Select edge perpendicular direction
	float lengthSign = horzSpan ? rcpFrame.y : rcpFrame.x;
	float lumaN2 = horzSpan ? lumaN : lumaW;
	float lumaS2 = horzSpan ? lumaS : lumaE;

	float gradientN = lumaN2 - lumaM;
	float gradientS = lumaS2 - lumaM;
	float lumaNN = lumaN2 + lumaM;
	float lumaSS = lumaS2 + lumaM;
	bool pairN = abs(gradientN) >= abs(gradientS);
	float gradient = max(abs(gradientN), abs(gradientS));
	if (pairN) lengthSign = -lengthSign;

	// Setup search along the edge
	vec2 posB = posM;
	vec2 offNP;
	offNP.x = (!horzSpan) ? 0.0 : rcpFrame.x;
	offNP.y = ( horzSpan) ? 0.0 : rcpFrame.y;
	if (!horzSpan) posB.x += lengthSign * 0.5;
	if ( horzSpan) posB.y += lengthSign * 0.5;

	vec2 posN = posB - offNP * QUALITY_P[0];
	vec2 posP = posB + offNP * QUALITY_P[0];

	float lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0));
	float lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0));

	if (!pairN) lumaNN = lumaSS;
	float gradientScaled = gradient * 0.25;
	bool lumaMLTZero = (lumaM - lumaNN * 0.5) < 0.0;
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;

	bool doneN = abs(lumaEndN) >= gradientScaled;
	bool doneP = abs(lumaEndP) >= gradientScaled;

	// Search loop (preset 26: 9 steps)
	for (int i = 1; i < 9 && (!doneN || !doneP); i++) {
		if (!doneN) {
			posN -= offNP * QUALITY_P[i];
			lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5;
			doneN = abs(lumaEndN) >= gradientScaled;
		}
		if (!doneP) {
			posP += offNP * QUALITY_P[i];
			lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5;
			doneP = abs(lumaEndP) >= gradientScaled;
		}
	}

	// Compute final pixel offset
	float dstN = horzSpan ? (posM.x - posN.x) : (posM.y - posN.y);
	float dstP = horzSpan ? (posP.x - posM.x) : (posP.y - posM.y);
	bool directionN = dstN < dstP;
	float dst = min(dstN, dstP);
	float spanLength = dstP + dstN;

	bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
	bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
	bool goodSpan = directionN ? goodSpanN : goodSpanP;
	float pixelOffset = goodSpan ? ((dst * (-1.0 / spanLength)) + 0.5) : 0.0;
	float pixelOffsetSubpix = max(pixelOffset, subpixH);

	// Apply offset and sample
	vec2 finalPos = posM;
	if (!horzSpan) finalPos.x += pixelOffsetSubpix * lengthSign;
	if ( horzSpan) finalPos.y += pixelOffsetSubpix * lengthSign;

	fragOut0 = textureLod(tex, finalPos, 0.0);
}
