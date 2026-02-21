#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.sdr"
#include "gamma.sdr"

layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D sTextures[16];
// sTextures[0] = ColorBuffer
// sTextures[1] = NormalBuffer
// sTextures[2] = PositionBuffer
// sTextures[3] = SpecBuffer

layout(set = 0, binding = 0, std140) uniform lightData {
	vec3 diffuseLightColor;
	float coneAngle;

	vec3 lightDir;
	float coneInnerAngle;

	vec3 coneDir;
	float dualCone;

	vec3 scale;
	float lightRadius;

	int lightType;
	int enable_shadows;
	float sourceRadius;

	float pad0;
};

layout(set = 0, binding = 1, std140) uniform globalDeferredData {
	mat4 shadow_mv_matrix;
	mat4 shadow_proj_matrix[4];

	mat4 inv_view_matrix;

	float veryneardist;
	float neardist;
	float middist;
	float fardist;

	float invScreenWidth;
	float invScreenHeight;

	float nearPlane;

	int use_env_map;
};

layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 3) uniform samplerCube sEnvmap;
layout(set = 0, binding = 4) uniform samplerCube sIrrmap;

layout(set = 2, binding = 1, std140) uniform matrixData {
	mat4 modelViewMatrix;
	mat4 projMatrix;
};

// ===== Variance Shadow Mapping (ported from shadows.sdr) =====

const float VARIANCE_SHADOW_SCALE = 1000000.0;

vec2 sampleShadowMap(vec2 uv, vec2 offset_uv, int cascade, float shadowMapSizeInv)
{
	return texture(shadowMap, vec3(uv + offset_uv * shadowMapSizeInv, float(cascade))).xy;
}

float computeShadowFactor(float shadowDepth, vec2 moments, float bias)
{
	float shadow = 1.0;
	if((moments.x - bias) > shadowDepth)
	{
		float variance = moments.y * VARIANCE_SHADOW_SCALE - moments.x * moments.x;
		float mD = moments.x - bias - shadowDepth;
		shadow = variance / (variance + mD * mD);
		shadow = clamp(shadow, 0.0, 1.0);
	}
	return shadow;
}

float samplePoissonPCF(float shadowDepth, int cascade, vec4 shadowUV[4], bool use_simple_pass)
{
	if(cascade > 3 || cascade < 0) return 1.0;

	vec2 poissonDisc[16] = vec2[](
		vec2(-0.76275, -0.3432573),
		vec2(-0.5226235, -0.8277544),
		vec2(-0.3780261, 0.01528688),
		vec2(-0.7742821, 0.4245702),
		vec2(0.04196143, -0.02622231),
		vec2(-0.2974772, -0.4722782),
		vec2(-0.516093, 0.71495),
		vec2(-0.3257416, 0.3910343),
		vec2(0.2705966, 0.6670476),
		vec2(0.4918377, 0.1853267),
		vec2(0.4428544, -0.6251478),
		vec2(-0.09204347, 0.9267113),
		vec2(0.391505, -0.2558275),
		vec2(0.05605913, -0.7570801),
		vec2(0.81772, -0.02475523),
		vec2(0.6890262, 0.5191521)
	);

	float maxUVOffset[4];
	maxUVOffset[0] = 1.0/300.0;
	maxUVOffset[1] = 1.0/250.0;
	maxUVOffset[2] = 1.0/200.0;
	maxUVOffset[3] = 1.0/200.0;

	if (use_simple_pass) {
		float visibility = 1.0f;
		for (int i=0; i<16; i++) {
			vec2 shadow_sample = sampleShadowMap(shadowUV[cascade].xy, poissonDisc[i], cascade, maxUVOffset[cascade]);
			if( ((shadow_sample.x - 0.002f) > shadowDepth) ) {
				visibility -= (1.0f/16.0f);
			}
		}
		return visibility;
	} else {
		vec2 sum = vec2(0.0f);
		for (int i=0; i<16; i++) {
			sum += sampleShadowMap(shadowUV[cascade].xy, poissonDisc[i], cascade, maxUVOffset[cascade]);
		}
		return computeShadowFactor(shadowDepth, sum*(1.0f/16.0f), 0.1f);
	}
}

float getShadowValue(float depth, float shadowDepth, vec4 shadowUV[4])
{
	int cascade = 4;
	cascade -= int(step(depth, fardist));
	cascade -= int(step(depth, middist));
	cascade -= int(step(depth, neardist));
	cascade -= int(step(depth, veryneardist));
	float cascade_start_dist[5];
	cascade_start_dist[0] = 0.0;
	cascade_start_dist[1] = veryneardist;
	cascade_start_dist[2] = neardist;
	cascade_start_dist[3] = middist;
	cascade_start_dist[4] = fardist;
	if(cascade > 3 || cascade < 0) return 1.0;

	bool use_simple_pass;
	if (fardist < 50.0f) {
		use_simple_pass = true;
	} else {
		use_simple_pass = false;
	}

	float dist_threshold = (cascade_start_dist[cascade+1] - cascade_start_dist[cascade])*0.2;
	if(cascade_start_dist[cascade+1] - dist_threshold > depth)
		return samplePoissonPCF(shadowDepth, cascade, shadowUV, use_simple_pass);
	return mix(samplePoissonPCF(shadowDepth, cascade, shadowUV, use_simple_pass),
		samplePoissonPCF(shadowDepth, cascade+1, shadowUV, use_simple_pass),
		smoothstep(cascade_start_dist[cascade+1] - dist_threshold, cascade_start_dist[cascade+1], depth));
}

vec4 transformToShadowMap(mat4 proj, int i, vec4 pos)
{
	vec4 shadow_proj = proj * pos;
	// Vulkan shadow projection maps to [0,1] depth, but XY is still [-1,1]
	// Transform XY from [-1,1] to [0,1]
	shadow_proj.xy = shadow_proj.xy * 0.5 + 0.5;
	shadow_proj.w = shadow_proj.z;  // depth for shadow comparison
	shadow_proj.z = float(i);       // cascade index for array layer
	return shadow_proj;
}

// ===== Light calculations =====

// Nearest point sphere and tube light calculations taken from
// "Real Shading in Unreal Engine 4" by Brian Karis, Epic Games
// Part of SIGGRAPH 2013 Course: Physically Based Shading in Theory and Practice

vec3 ExpandLightSize(in vec3 lightDirIn, in vec3 reflectDir) {
	vec3 centerToRay = max(dot(lightDirIn, reflectDir),sourceRadius) * reflectDir - lightDirIn;
	return lightDirIn + centerToRay * clamp(sourceRadius/length(centerToRay), 0.0, 1.0);
}

void GetLightInfo(vec3 position, in float alpha, in vec3 reflectDir, out vec3 lightDirOut, out float attenuation, out float area_normalisation)
{
	if (lightType == LT_DIRECTIONAL) {
		lightDirOut = normalize(lightDir);
		attenuation = 1.0;
		area_normalisation = 1.0;
	} else {
		vec3 lightPosition = modelViewMatrix[3].xyz;
		if (lightType == LT_POINT) {
			lightDirOut = lightPosition - position.xyz;
			float dist = length(lightDirOut);

			lightDirOut = ExpandLightSize(lightDirOut, reflectDir);
			dist = length(lightDirOut);
			float alpha_adjust = clamp(alpha + (sourceRadius/(2*dist)), 0.0, 1.0);
			area_normalisation = alpha/alpha_adjust;
			area_normalisation *= area_normalisation;

			if(dist > lightRadius) {
				discard;
			}
			attenuation = 1.0 - clamp(sqrt(dist / lightRadius), 0.0, 1.0);
		}
		else if (lightType == LT_TUBE) {
			vec3 beamVec = vec3(modelViewMatrix * vec4(0.0, 0.0, -scale.z, 0.0));
			vec3 beamDir = normalize(beamVec);
			vec3 adjustedLightPos = lightPosition - (beamDir * lightRadius);
			vec3 adjustedbeamVec = beamVec - 2.0 * lightRadius * beamDir;
			float beamLength = length(adjustedbeamVec);
			vec3 sourceDir = adjustedLightPos - position.xyz;

			vec3 a_t = reflectDir;
			vec3 b_t = beamDir;
			vec3 b_0 = sourceDir;
			vec3 c = cross(a_t, b_t);
			vec3 d = b_0;
			vec3 r = d - a_t * dot(d, a_t) - c * dot(d,c);
			float tubeneardist = dot(r, r)/dot(b_t, r);
			lightDirOut = sourceDir - beamDir * clamp(tubeneardist, 0.0, beamLength);

			lightDirOut = ExpandLightSize(lightDirOut, reflectDir);
			float dist = length(lightDirOut);
			float alpha_adjust = min(alpha + (sourceRadius/(2*dist)), 1.0);
			area_normalisation = alpha/alpha_adjust;

			if(dist > lightRadius) {
				discard;
			}
			attenuation = 1.0 - clamp(sqrt(dist / lightRadius), 0.0, 1.0);
		}
		else if (lightType == LT_CONE) {
			lightDirOut = lightPosition - position.xyz;
			float coneDot = dot(normalize(-lightDirOut), coneDir);
			float dist = length(lightDirOut);
			attenuation = 1.0 - clamp(sqrt(dist / lightRadius), 0.0, 1.0);
			area_normalisation = 1.0;

			if(dualCone > 0.5) {
				if(abs(coneDot) < coneAngle) {
					discard;
				} else {
					attenuation *= smoothstep(coneAngle, coneInnerAngle, abs(coneDot));
				}
			} else {
				if (coneDot < coneAngle) {
					discard;
				} else {
					attenuation *= smoothstep(coneAngle, coneInnerAngle, coneDot);
				}
			}
		}
		attenuation *= attenuation;
		lightDirOut = normalize(lightDirOut);
	}
}

// ===== Environment Map Lighting =====
// Ported from deferred-f.sdr ComputeEnvLight()

void ComputeEnvLight(float alpha, float ao, vec3 light_dir, vec3 eyeDir,
                     vec3 normal, vec4 baseColor, vec4 specColor, out vec3 envLight)
{
	const float ENV_REZ = 512.0;
	const float REZ_BIAS = log2(ENV_REZ * sqrt(3.0));

	float alphaSqr = alpha * alpha;
	float rough_bias = 0.5 * log2(2.0 / alphaSqr - 1.0);
	float mip_bias = REZ_BIAS - rough_bias;

	// Sample specular environment map with roughness-based mip bias
	vec3 env_light_dir = vec3(modelViewMatrix * vec4(light_dir, 0.0));
	vec4 specEnvColour = srgb_to_linear(textureLod(sEnvmap, env_light_dir, mip_bias));

	vec3 halfVec = normal;

	// Fresnel using Schlick approximation
	vec3 fresnel = mix(specColor.rgb, FresnelSchlick(halfVec, eyeDir, specColor.rgb), specColor.a);

	// Pseudo-IBL geometry term (k = alpha^2 / 2)
	float k = alphaSqr / 2.0;
	float NdotL = max(dot(light_dir, normal), 0.0);
	float g1vNL = GeometrySchlickGGX(NdotL, k);

	vec3 specEnvLighting = specEnvColour.rgb * fresnel * g1vNL;

	// Diffuse from irradiance map
	vec3 kD = vec3(1.0) - fresnel;
	kD *= (vec3(1.0) - specColor.rgb);
	vec3 diffEnvColor = srgb_to_linear(texture(sIrrmap, vec3(modelViewMatrix * vec4(normal, 0.0))).rgb);
	vec3 diffEnvLighting = kD * baseColor.rgb * diffEnvColor * ao;

	envLight = (specEnvLighting + diffEnvLighting) * baseColor.a;
}

void main()
{
	vec2 screenPos = gl_FragCoord.xy * vec2(invScreenWidth, invScreenHeight);
	vec4 position_buffer = texture(sTextures[2], screenPos);
	vec3 position = position_buffer.xyz;

	if(abs(dot(position, position)) < nearPlane * nearPlane)
		discard;

	vec4 diffuse = texture(sTextures[0], screenPos);
	vec3 diffColor = diffuse.rgb;
	vec4 normalData = texture(sTextures[1], screenPos);
	vec3 normal = normalize(normalData.xyz);
	float gloss = normalData.a;
	float roughness = clamp(1.0f - gloss, 0.0f, 1.0f);
	float alpha = roughness * roughness;
	vec3 eyeDir = normalize(-position);
	vec3 reflectDir = reflect(-eyeDir, normal);
	vec4 specColor = texture(sTextures[3], screenPos);

	vec4 fragmentColor = vec4(1.0);

	if (lightType == LT_AMBIENT) {
		float ao = position_buffer.w;
		fragmentColor.rgb = diffuseLightColor * diffColor * ao;
		if (use_env_map != 0) {
			vec3 envLight;
			ComputeEnvLight(alpha, ao, reflectDir, eyeDir, normal, diffuse, specColor, envLight);
			fragmentColor.rgb += envLight;
		}
	}
	else {
		float fresnel = specColor.a;

		vec3 lightDirCalc;
		float attenuation;
		float area_normalisation;
		GetLightInfo(position, alpha, reflectDir, lightDirCalc, attenuation, area_normalisation);

		// Shadow attenuation for directional lights
		if (enable_shadows != 0 && lightType == LT_DIRECTIONAL) {
			vec4 fragShadowPos = shadow_mv_matrix * inv_view_matrix * vec4(position, 1.0);
			vec4 fragShadowUV[4];
			for (int i = 0; i < 4; i++) {
				fragShadowUV[i] = transformToShadowMap(shadow_proj_matrix[i], i, fragShadowPos);
			}
			float shadowVal = getShadowValue(-position.z, fragShadowPos.z, fragShadowUV);
			attenuation *= shadowVal;
		}

		vec3 halfVec = normalize(lightDirCalc + eyeDir);
		float NdotL = clamp(dot(normal, lightDirCalc), 0.0, 1.0);
		fragmentColor.rgb = computeLighting(specColor.rgb, diffColor, lightDirCalc, normal.xyz, halfVec, eyeDir, roughness, fresnel, NdotL).rgb * diffuseLightColor * attenuation * area_normalisation;
	}

	fragOut0 = max(fragmentColor, vec4(0.0));
}
