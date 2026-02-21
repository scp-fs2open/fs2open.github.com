#version 450
#extension GL_ARB_separate_shader_objects : enable

// Volumetric nebula fragment shader — port of volumetric-f.sdr to Vulkan
// Raymarches through a 3D volume texture to render volumetric nebulae.
// #ifdef variants replaced with runtime UBO flags (doEdgeSmoothing, useNoise).

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

// Binding 1 is a 16-element sampler2D array in the Material descriptor set layout.
// We use elements [0]=composite and [1]=emissive.
layout(set = 1, binding = 1) uniform sampler2D tex2D[16];
#define composite tex2D[0]
#define emissive  tex2D[1]
layout(set = 1, binding = 4) uniform sampler2D depth;        // scene depth copy
layout(set = 1, binding = 5) uniform sampler3D volume_tex;   // 3D nebula volume
layout(set = 1, binding = 6) uniform sampler3D noise_volume_tex; // 3D noise volume

layout(std140, set = 2, binding = 0) uniform genericData {
	mat4 p_inv;
	mat4 v_inv;
	vec3 camera;
	float zNear;
	vec3 globalLightDirection;
	float zFar;
	vec3 globalLightDiffuse;
	float stepsize;
	vec3 nebPos;
	float opacitydistance;
	vec3 nebSize;
	float alphalim;
	vec3 nebulaColor;
	float udfScale;
	float emissiveSpreadFactor;
	float emissiveIntensity;
	float emissiveFalloff;
	float henyeyGreensteinCoeff;
	vec3 noiseColor;
	int directionalLightSampleSteps;
	float directionalLightStepSize;
	float noiseColorScale1;
	float noiseColorScale2;
	float noiseIntensity;
	float aspect;
	float fov;
	int doEdgeSmoothing;
	int useNoise;
};

const float sqrt4pi_inv = inversesqrt(4.0 * 3.14159);
const float beer_powder_norm = 3.0/2.0 * sqrt(3.0);

// Henyey-Greenstein phase function (assumes scatter vectors both point away from scatter point)
float henyey_greenstein(float cosTheta) {
	float radicant = 1.0 + henyeyGreensteinCoeff * henyeyGreensteinCoeff + 2.0 * henyeyGreensteinCoeff * cosTheta;
	return sqrt4pi_inv * (1.0 - henyeyGreensteinCoeff * henyeyGreensteinCoeff) / pow(radicant, 3.0 / 2.0);
}

void main()
{
	vec4 eyeDirection = p_inv * vec4(fragTexCoord.xy * 2.0 - 1.0, -1, 1);
	eyeDirection.w = 0;
	vec3 rayDirection = normalize((v_inv * eyeDirection).xyz);

	vec4 color_in = texture(composite, fragTexCoord.xy);

	vec3 lCorner = nebPos - nebSize * 0.5;
	vec3 rCorner = nebPos + nebSize * 0.5;

	vec3 t1 = (lCorner - camera) / rayDirection;
	vec3 t2 = (rCorner - camera) / rayDirection;

	vec3 tMin = min(t1, t2);
	vec3 tMax = max(t1, t2);

	vec2 fragcoordAngle = (fragTexCoord.xy - 0.5) * fov;
	fragcoordAngle.x *= aspect;
	// Vulkan depth range [0,1] — linearize directly (no 2*d-1 transform)
	float depth_val = texture(depth, fragTexCoord.xy).x;
	float linearDepth = zNear * zFar / (zFar - depth_val * (zFar - zNear));
	float fragDepth = linearDepth * sqrt(1.0 + tan(fragcoordAngle.x) * tan(fragcoordAngle.x) + tan(fragcoordAngle.y) * tan(fragcoordAngle.y));

	// t at which the ray enters/leaves the nebula cube
	float maxtMin = max(0, max(tMin.x, max(tMin.y, tMin.z)));
	float mintMax = min(fragDepth, min(tMax.x, min(tMax.y, tMax.z)));

	// Cumulative one-minus-alpha, distance, and color
	float cumOMAlpha = 1;
	float cumnebdist = 0;
	vec3 cumcolor = vec3(0, 0, 0);

	// Pre-compute texture gradients (approximate, shared for all steps)
	vec3 initialPos = (camera + rayDirection * maxtMin) / nebSize + 0.5;
	vec3 gradX = dFdx(initialPos);
	vec3 gradY = dFdy(initialPos);

	vec3 sidestep = 1.0 / vec3(textureSize(volume_tex, 0));

	for (float stept = maxtMin; stept < mintMax;) {
		// Step setup
		vec3 position = camera + rayDirection * stept - nebPos;
		vec3 sampleposition = position / nebSize + 0.5;
		vec4 volume_sample = textureGrad(volume_tex, sampleposition, gradX, gradY);

		float stepcolor_alpha = volume_sample.a;

		// Edge smoothing: average 3D texel with corner neighbors to reduce jaggies
		if (doEdgeSmoothing != 0 && cumOMAlpha > 0.8) {
			stepcolor_alpha = stepcolor_alpha / 2.0 + (
				textureGrad(volume_tex, sampleposition + vec3(sidestep.x, sidestep.y, sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(sidestep.x, sidestep.y, -sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(sidestep.x, -sidestep.y, sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(sidestep.x, -sidestep.y, -sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(-sidestep.x, sidestep.y, sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(-sidestep.x, sidestep.y, -sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(-sidestep.x, -sidestep.y, sidestep.z), gradX, gradY).a +
				textureGrad(volume_tex, sampleposition + vec3(-sidestep.x, -sidestep.y, -sidestep.z), gradX, gradY).a) / 16.0;
		}

		float stepsize_current = min(max(stepsize, step(stepcolor_alpha, 0.01) * volume_sample.x * udfScale), mintMax - stept);

		float stepalpha = -(pow(alphalim, 1.0 / (opacitydistance / stepsize_current)) - 1.0) * stepcolor_alpha;
		// All following computations only needed if alpha is non-zero
		if (stepcolor_alpha > 0.01) {
			// Diffuse color (with optional noise mixing)
			vec3 stepcolor_neb;
			if (useNoise != 0) {
				stepcolor_neb = mix(nebulaColor, noiseColor,
					smoothstep(0, 1, (textureGrad(noise_volume_tex, position / noiseColorScale1, gradX, gradY).r + textureGrad(noise_volume_tex, position / noiseColorScale2, gradX, gradY).g) / 2.0 * noiseIntensity));
			} else {
				stepcolor_neb = nebulaColor;
			}
			vec3 stepcolor_diffuse = stepcolor_neb * henyey_greenstein(dot(rayDirection, globalLightDirection));
			float directionalLightStep = 4.0 / float(directionalLightSampleSteps);
			float directionalLightDepth = 0.1;
			// Sample toward sun to determine lighting
			for (int dlstep = 1; dlstep <= directionalLightSampleSteps; dlstep++) {
				vec3 dlsteppos = (position - globalLightDirection * (dlstep * directionalLightStepSize)) / nebSize + 0.5;
				float dlstepalpha = textureGrad(volume_tex, dlsteppos, gradX, gradY).a * step(0, dlsteppos.x) * step(dlsteppos.x, 1) * step(0, dlsteppos.y) * step(dlsteppos.y, 1) * step(0, dlsteppos.z) * step(dlsteppos.z, 1);
				directionalLightDepth += dlstepalpha * directionalLightStep;
			}
			stepcolor_diffuse *= beer_powder_norm * (1 - exp(-directionalLightDepth * 2.0)) * exp(-directionalLightDepth);

			// Emissive contribution (LOD based on cumulative nebula distance)
			cumnebdist += stepcolor_alpha * stepsize_current;
			vec3 emissive_lod = textureLod(emissive, fragTexCoord.xy, clamp(cumnebdist * emissiveSpreadFactor, 0, float(textureQueryLevels(emissive) - 1))).rgb;
			vec3 stepcolor_emissive = clamp(emissive_lod.rgb * pow(alphalim, 1.0 / (opacitydistance / ((fragDepth - stept) * emissiveFalloff + 0.01))) * emissiveIntensity, 0, 1);

			// Combine diffuse and emissive
			vec3 stepcolor = clamp(stepcolor_diffuse + stepcolor_emissive, 0, 1);
			cumcolor += stepalpha * cumOMAlpha * stepcolor;
		}

		cumOMAlpha *= 1.0 - stepalpha;
		stept += stepsize_current;

		if (cumOMAlpha < alphalim)
			break;
	}

	fragOut0 = vec4(cumOMAlpha * color_in.rgb + ((1.0 - cumOMAlpha) * cumcolor), 1);
}
