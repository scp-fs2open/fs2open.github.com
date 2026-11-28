// CORTAO Reference Implementation v1.0.3
//
// MIT License
//
// Copyright (c) 2026 Daniel Isheden
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "CORTAOBase.h"


#ifndef CORTAO_FILTER_BENT_NORMAL
	#define CORTAO_FILTER_BENT_NORMAL 0
#endif

#ifndef CORTAO_LARGE_FILTER
	#define CORTAO_LARGE_FILTER 0
#endif

#ifndef CORTAO_FILTER_SYMMETRY
	#define CORTAO_FILTER_SYMMETRY 0
#endif

cortao_uint4 cortao_gatherPackedData(cortao_float2 texCoords, cortao_int2 offset);

#if CORTAO_FILTER_BENT_NORMAL
	cortao_half4x4 cortao_gatherSSAO(cortao_float2 texCoords, cortao_int2 offset);
	void cortao_writeFilterOutput(cortao_int2 pixelCoords, cortao_half4 ssao);
#else
	cortao_half4 cortao_gatherSSAO(cortao_float2 texCoords, cortao_int2 offset);
	void cortao_writeFilterOutput(cortao_int2 pixelCoords, cortao_half ssao);
#endif

struct CORTAOFilterPassParameters {
	cortao_uint packedResolution;
	cortao_float depthSensitivity;
	cortao_float normalSensitivity;
	cortao_uint _padding0;
};

struct CORTAOWeightParams {

	cortao_float4 depthMul;
	cortao_float depthOffset;
	cortao_half4 deltaX;
	cortao_half4 deltaY;
	
	cortao_short4 centerNormalX;
	cortao_short4 centerNormalY;
	
	cortao_half normalErrorMul;
};


cortao_short4 cortao_unpackNormalX(cortao_uint4 packedData) {
#if CORTAO_16_BIT
	cortao_ushort bits = cortao_ushort(16);
#else
	cortao_ushort bits = cortao_ushort(32);
#endif
	cortao_short normalBits = cortao_short(CORTAO_PACKED_NORMAL_BITS/2);
	
	return (cortao_short4(packedData) << (bits - normalBits)) >> (bits - normalBits);
}

cortao_short4 cortao_unpackNormalY(cortao_uint4 packedData) {
#if CORTAO_16_BIT
	cortao_ushort bits = cortao_ushort(16);
#else
	cortao_ushort bits = cortao_ushort(32);
#endif
	cortao_short normalBits = cortao_short(CORTAO_PACKED_NORMAL_BITS/2);
	
	return (cortao_short4(packedData) << (bits - normalBits*2)) >> (bits - normalBits);
}

cortao_half4 calculateWeight(CORTAOWeightParams params, cortao_uint4 packedData, cortao_int2 offset) {
	cortao_half4 sampleDepth = cortao_half4(cortao_fma(cortao_unpackDepth(packedData), params.depthMul, params.depthOffset.xxxx));
	cortao_half4 error = abs(cortao_fma(params.deltaX, cortao_half4(offset.xxxx), cortao_fma(params.deltaY, cortao_half4(offset.yyyy), sampleDepth)));

	cortao_short4 normalX = cortao_unpackNormalX(packedData) - params.centerNormalX;
	cortao_short4 normalY = cortao_unpackNormalY(packedData) - params.centerNormalY;
	cortao_half4 normalError = cortao_half4(normalX*normalX + normalY*normalY); //can't use FMA here...
	error = cortao_fma(normalError, params.normalErrorMul.xxxx, error);
	
	return exp2(-error);
}

#if CORTAO_FILTER_BENT_NORMAL

	cortao_half4x4 cortao_filterAccumulate(cortao_half4x4 ssaoSum, cortao_float2 texCoords, cortao_int2 offset, cortao_half4 weight)
	{
		cortao_half4x4 data = cortao_gatherSSAO(texCoords, offset);
		cortao_unroll for(cortao_uint i = 0u; i < 4u; i++) {
			ssaoSum[i] = cortao_fma(data[i], weight[i].xxxx, ssaoSum[i]);
		}
		return ssaoSum;
	}
#else

	cortao_half4 cortao_filterAccumulate(cortao_half4 ssaoSum, cortao_float2 texCoords, cortao_int2 offset, cortao_half4 weight)
	{
		return cortao_fma(cortao_gatherSSAO(texCoords, offset), weight, ssaoSum);
	}
#endif

void cortao_filterPass(cortao_uint2 threadCoords, CORTAOFilterPassParameters parameters) {
	cortao_uint2 resolution = cortao_getResolution(parameters.packedResolution);
	
	cortao_uint2 pixelCoords = (threadCoords << cortao_uint2(1, 1)) | cortao_uint2(1, 1);
	
	//">" is correct here, as we write to pixelCoords.x-1 as well.
	if(pixelCoords.x > resolution.x || pixelCoords.y > resolution.y) {
		return;
	}
	
	cortao_float2 texCoords = cortao_float2(pixelCoords) / cortao_float2(resolution);
	
		
	cortao_uint4 centerPackedData = cortao_gatherPackedData(texCoords, cortao_int2(0, 0));

	if(centerPackedData.x == 0u && centerPackedData.y == 0u && centerPackedData.z == 0u && centerPackedData.w == 0u) {
		return;
	}

	cortao_float4 centerDepth = cortao_unpackDepth(centerPackedData);
	cortao_float4 inverseCenterDepth = 1.0 / centerDepth;
	
	cortao_half4 deltaX = cortao_half4((centerDepth.xxww - centerDepth.yyzz) * inverseCenterDepth);
	cortao_half4 deltaY = cortao_half4((centerDepth.wzzw - centerDepth.xyyx) * inverseCenterDepth);
	
	CORTAOWeightParams weightParams;
	
	weightParams.depthMul = parameters.depthSensitivity * inverseCenterDepth;
	weightParams.depthOffset = -parameters.depthSensitivity;
	weightParams.deltaX = deltaX * cortao_half(parameters.depthSensitivity);
	weightParams.deltaY = deltaY * cortao_half(parameters.depthSensitivity);
	
	weightParams.centerNormalX = cortao_unpackNormalX(centerPackedData);
	weightParams.centerNormalY = cortao_unpackNormalY(centerPackedData);
	
	cortao_ushort normalBits = cortao_ushort(CORTAO_PACKED_NORMAL_BITS/2);
	weightParams.normalErrorMul = cortao_half(parameters.normalSensitivity * (1.0 / cortao_float(1u << normalBits)));
	
	
	#if CORTAO_FILTER_BENT_NORMAL
		cortao_half4x4 ssaoSum = cortao_half4x4(
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		);
	#else
		cortao_half4 ssaoSum = cortao_half4(0, 0, 0, 0);
	#endif
	
	
	
	ssaoSum = cortao_filterAccumulate(ssaoSum, texCoords, cortao_int2(0, 0), cortao_half4(1, 1, 1, 1));
	cortao_half4 weightSum = cortao_half4(1, 1, 1, 1);
	
	
	
	#if CORTAO_LARGE_FILTER
		cortao_uint sampleCount = 6;
		cortao_int2 offsets[6];
		offsets[0] = cortao_int2(3, 0);
		offsets[1] = cortao_int2(6, 0);
		offsets[2] = cortao_int2(3, 3);
		offsets[3] = cortao_int2(0, 3);
		offsets[4] = cortao_int2(0, 6);
		offsets[5] = cortao_int2(3, -3);
		
	#else
		cortao_uint sampleCount = 2;
		cortao_int2 offsets[2];
		offsets[0] = cortao_int2(3, 0);
		offsets[1] = cortao_int2(0, 3);
	#endif
	
	 cortao_unroll for(cortao_uint i = 0u; i < sampleCount; i++) {
	 	
		cortao_int2 offset = offsets[i];
		
		cortao_half4 weightA = calculateWeight(weightParams, cortao_gatherPackedData(texCoords, -offset), -offset);
		cortao_half4 weightB = calculateWeight(weightParams, cortao_gatherPackedData(texCoords, +offset), +offset);
		
		#if CORTAO_FILTER_SYMMETRY
			cortao_half4 minWeight = min(weightA, weightB);
			weightA = minWeight;
			weightB = minWeight;
			weightSum = cortao_fma(minWeight, cortao_half(2.0).xxxx, weightSum);
		#else
			weightSum += weightA + weightB;
		#endif
		
		
		ssaoSum = cortao_filterAccumulate(ssaoSum, texCoords, -offset, weightA);
		ssaoSum = cortao_filterAccumulate(ssaoSum, texCoords, +offset, weightB);
	}
	
	cortao_half4 inverseWeight = cortao_half(1.0) / cortao_half4(weightSum);
	cortao_unroll for(cortao_uint i = 0u; i < 4u; i++) {
		ssaoSum[i] *= inverseWeight[i];
	}
	
	cortao_uint4 coords = cortao_uint4(pixelCoords - 1u, pixelCoords);	
	cortao_uint2 outputCoordArray[4];
	outputCoordArray[0] = coords.xw;
	outputCoordArray[1] = coords.zw;
	outputCoordArray[2] = coords.zy;
	outputCoordArray[3] = coords.xy;
	cortao_unroll for(cortao_uint i = 0u; i < 4u; i++) {
		
		cortao_uint2 outputCoords = outputCoordArray[i];
		
		if(centerPackedData[i] != 0u && outputCoords.x < resolution.x && outputCoords.y < resolution.y) {
			cortao_writeFilterOutput(cortao_int2(outputCoords), ssaoSum[i]);
		}
	}
}