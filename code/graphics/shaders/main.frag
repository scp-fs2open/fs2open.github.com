#version 450
#extension GL_ARB_separate_shader_objects : enable

// Include shared lighting and gamma functions from legacy shader directory
#include "lighting.sdr"
#include "gamma.sdr"

// Model shader flags (from model_shader_flags.h)
const int MODEL_SDR_FLAG_LIGHT       = (1 << 0);
const int MODEL_SDR_FLAG_DEFERRED    = (1 << 1);
const int MODEL_SDR_FLAG_HDR         = (1 << 2);
const int MODEL_SDR_FLAG_DIFFUSE     = (1 << 3);
const int MODEL_SDR_FLAG_GLOW        = (1 << 4);
const int MODEL_SDR_FLAG_SPEC        = (1 << 5);
const int MODEL_SDR_FLAG_NORMAL      = (1 << 6);
const int MODEL_SDR_FLAG_AMBIENT     = (1 << 7);
const int MODEL_SDR_FLAG_MISC        = (1 << 8);
const int MODEL_SDR_FLAG_TEAMCOLOR   = (1 << 9);
const int MODEL_SDR_FLAG_FOG         = (1 << 10);
const int MODEL_SDR_FLAG_SHADOWS     = (1 << 12);
const int MODEL_SDR_FLAG_ALPHA_MULT  = (1 << 14);

#define MAX_LIGHTS 8

struct model_light {
	vec4 position;

	vec3 diffuse_color;
	int light_type;

	vec3 direction;
	float attenuation;

	float ml_sourceRadius;
};

layout(set = 1, binding = 0, std140) uniform modelData {
	mat4 modelViewMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 textureMatrix;
	mat4 shadow_mv_matrix;
	mat4 shadow_proj_matrix[4];

	vec4 color;

	model_light lights[MAX_LIGHTS];

	float outlineWidth;
	float fogStart;
	float fogScale;
	int buffer_matrix_offset;

	vec4 clip_equation;

	float thruster_scale;
	int use_clip_plane;
	int n_lights;
	float defaultGloss;

	vec3 ambientFactor;
	int desaturate;

	vec3 diffuseFactor;
	int blend_alpha;

	vec3 emissionFactor;
	int alphaGloss;

	int gammaSpec;
	int envGloss;
	int effect_num;
	int sBasemapIndex;

	vec4 fogColor;

	vec3 base_color;
	float anim_timer;

	vec3 stripe_color;
	float vpwidth;

	float vpheight;
	int team_glow_enabled;
	float znear;
	float zfar;

	float veryneardist;
	float neardist;
	float middist;
	float fardist;

	int sGlowmapIndex;
	int sSpecmapIndex;
	int sNormalmapIndex;
	int sAmbientmapIndex;

	int sMiscmapIndex;
	float alphaMult;
	int flags;
	float _pad0;
};

// Textures - Material set (set 1), binding 1 as descriptor array
// Indices: 0=Base, 1=Glow, 2=Spec, 3=Normal, 4=Height, 5=Ambient, 6=Misc
layout(set = 1, binding = 1) uniform sampler2DArray materialTextures[16];

// Inputs from vertex shader
layout(location = 0) in vec4 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragTexCoord;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragTangentNormal;
layout(location = 6) in float fragFogDist;

// Output
layout(location = 0) out vec4 fragOut0;
layout(location = 1) out vec4 fragOut1;
layout(location = 2) out vec4 fragOut2;
layout(location = 3) out vec4 fragOut3;
layout(location = 4) out vec4 fragOut4;

vec3 FresnelLazarovEnv(vec3 specColor, vec3 view, vec3 normal, float gloss)
{
	return specColor + (vec3(1.0) - specColor) * pow(1.0 - clamp(dot(view, normal), 0.0, 1.0), 5.0) / (4.0 - 3.0 * gloss);
}

void GetLightInfo(int i, out vec3 lightDir, out float attenuation)
{
	lightDir = normalize(lights[i].position.xyz);
	attenuation = 1.0;
	if (lights[i].light_type != LT_DIRECTIONAL) {
		// Positional light source
		float dist = distance(lights[i].position.xyz, fragPosition.xyz);
		lightDir = (lights[i].position.xyz - fragPosition.xyz);

		if (lights[i].light_type == LT_TUBE) {  // Tube light
			float beamlength = length(lights[i].direction);
			vec3 beamDir = normalize(lights[i].direction);
			// Get nearest point on line
			float neardist = dot(fragPosition.xyz - lights[i].position.xyz, beamDir);
			// Move back from the endpoint of the beam along the beam by the distance we calculated
			vec3 nearest = lights[i].position.xyz - beamDir * abs(neardist);
			lightDir = nearest - fragPosition.xyz;
			dist = length(lightDir);
		}

		lightDir = normalize(lightDir);
		attenuation = 1.0 / (1.0 + lights[i].attenuation * dist);
	}
}

vec3 CalculateLighting(vec3 normal, vec3 diffuseMaterial, vec3 specularMaterial, float gloss, float fresnel, float shadow, float aoFactor)
{
	vec3 eyeDir = normalize(-fragPosition.xyz);
	vec3 lightAmbient = ambientFactor * aoFactor;
	vec3 lightDiffuse = vec3(0.0, 0.0, 0.0);
	vec3 lightSpecular = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < n_lights; ++i) {
		if (i > 0) {
			shadow = 1.0;
		}
		float roughness = clamp(1.0f - gloss, 0.0f, 1.0f);
		float alpha = roughness * roughness;
		vec3 lightDir;
		float attenuation;
		// gather light params
		GetLightInfo(i, lightDir, attenuation);
		vec3 halfVec = normalize(lightDir + eyeDir);
		float NdotL = clamp(dot(normal, lightDir), 0.0f, 1.0f);
		// Ambient, Diffuse, and Specular
		lightDiffuse += (lights[i].diffuse_color.rgb * diffuseFactor * NdotL * attenuation) * shadow;
		lightSpecular += lights[i].diffuse_color.rgb * computeLighting(specularMaterial, diffuseMaterial, lightDir, normal, halfVec, eyeDir, roughness, fresnel, NdotL) * attenuation * shadow;
	}
	return diffuseMaterial * lightAmbient + lightSpecular;
}

void main()
{
	vec3 eyeDir = normalize(-fragPosition.xyz);
	vec2 texCoord = fragTexCoord.xy;
	mat3 tangentMatrix = mat3(fragTangent, fragBitangent, fragTangentNormal);

	// setup our baseline values for base, emissive, fresnel, gloss, AO and normal
	vec4 baseColor = color;
	vec4 emissiveColor = vec4(0.0, 0.0, 0.0, 1.0);
	float fresnelFactor = 0.0;
	float glossData = defaultGloss;
	vec2 aoFactors = vec2(1.0, 1.0);
	vec3 unitNormal = normalize(fragNormal);
	vec3 normal = unitNormal;

	// Ambient occlusion map
	if ((flags & MODEL_SDR_FLAG_AMBIENT) != 0) {
		// red channel is ambient occlusion factor, green is cavity occlusion factor
		aoFactors = texture(materialTextures[5], vec3(texCoord, float(sAmbientmapIndex))).xy;
	}

	// Normal map - convert from DXT5nm
	if ((flags & MODEL_SDR_FLAG_NORMAL) != 0) {
		vec2 normalSample;
		normal.rg = normalSample = (texture(materialTextures[3], vec3(texCoord, float(sNormalmapIndex))).ag * 2.0) - 1.0;
		normal.b = clamp(sqrt(1.0 - dot(normal.rg, normal.rg)), 0.0001, 1.0);
		normal = tangentMatrix * normal;
		float norm = length(normal);
		// prevent breaking of normal maps
		if (norm > 0.0)
			normal /= norm;
		else
			normal = unitNormal;
	}

	vec2 distort = vec2(0.0, 0.0);

	if (effect_num >= 0) {
		distort = vec2(cos(fragPosition.x*fragPosition.w*0.005+anim_timer*20.0)*sin(fragPosition.y*fragPosition.w*0.005),sin(fragPosition.x*fragPosition.w*0.005+anim_timer*20.0)*cos(fragPosition.y*fragPosition.w*0.005))*0.03;
	}

	// Diffuse map
	if ((flags & MODEL_SDR_FLAG_DIFFUSE) != 0) {
		vec2 diffuseTexCoord = texCoord;
		if (effect_num == 2) {
			diffuseTexCoord = texCoord + distort*(1.0-anim_timer);
		}
		baseColor = texture(materialTextures[0], vec3(diffuseTexCoord, float(sBasemapIndex)));

		if ((flags & MODEL_SDR_FLAG_HDR) != 0) {
			baseColor.rgb = srgb_to_linear(baseColor.rgb);
		}

		if ((flags & MODEL_SDR_FLAG_ALPHA_MULT) != 0) {
			baseColor.a *= alphaMult;
		}

		if (blend_alpha == 0 && baseColor.a < 0.95) discard; // if alpha blending is not on, discard transparent pixels
		// premultiply alpha if blend_alpha is 1. assume that our blend function is srcColor + (1-Alpha)*destColor.
		// if blend_alpha is 2, assume blend func is additive and don't modify color
		if (blend_alpha == 1) baseColor.rgb = baseColor.rgb * baseColor.a;
	}

	// Anti-glint "trick" based on Valve's "Advanced VR Rendering" talk at GDC2015
	vec2 normDx = dFdx(unitNormal.xy);
	vec2 normDy = dFdy(unitNormal.xy);
	float glossGeo = 1.0f - pow(clamp(max(dot(normDx,normDx), dot(normDy,normDy)),0.0,1.0),0.33);
	glossData = min(glossData, glossGeo);

	// Now that we have a base color and min gloss value, compute the spec color
	vec4 specColor = vec4(baseColor.rgb * SPEC_FACTOR_NO_SPEC_MAP, glossData);

	if ((flags & MODEL_SDR_FLAG_SPEC) != 0) {
		specColor = texture(materialTextures[2], vec3(texCoord, float(sSpecmapIndex)));
		if ((flags & MODEL_SDR_FLAG_ALPHA_MULT) != 0) {
			specColor *= alphaMult;
		}

		if (alphaGloss != 0) glossData = specColor.a;
		if (gammaSpec != 0) {
			specColor.rgb = max(specColor.rgb, vec3(0.03f));
			fresnelFactor = 1.0;
		}

		if ((flags & MODEL_SDR_FLAG_HDR) != 0) {
			specColor.rgb = srgb_to_linear(specColor.rgb);
		}
	}

	baseColor.rgb *= aoFactors.y;
	specColor.rgb *= aoFactors.y;

	vec4 teamMask = vec4(0.0);
	vec3 team_color_glow = vec3(0.0);

	// Misc map / team colors
	if ((flags & MODEL_SDR_FLAG_MISC) != 0) {
		if ((flags & MODEL_SDR_FLAG_TEAMCOLOR) != 0) {
			teamMask = texture(materialTextures[6], vec3(texCoord, float(sMiscmapIndex)));

			vec3 color_offset = vec3(-0.5) * (teamMask.x + teamMask.y);

			vec3 team_color = base_color * teamMask.x + stripe_color * teamMask.y + color_offset;
			team_color_glow = (base_color * teamMask.b) + (stripe_color * teamMask.a);

			if ((flags & MODEL_SDR_FLAG_HDR) != 0) {
				baseColor.rgb = linear_to_srgb(baseColor.rgb);
				specColor.rgb = linear_to_srgb(specColor.rgb);
			}

			baseColor.rgb += team_color;
			baseColor.rgb = max(baseColor.rgb, vec3(0.0));
			specColor.rgb += team_color;
			specColor.rgb = max(specColor.rgb, vec3(0.03));

			if ((flags & MODEL_SDR_FLAG_HDR) != 0) {
				baseColor.rgb = srgb_to_linear(baseColor.rgb);
				specColor.rgb = srgb_to_linear(specColor.rgb);
			}
		}
	}

	// Lights aren't applied when we are rendering to the G-buffers since that gets handled later
	if ((flags & MODEL_SDR_FLAG_DEFERRED) == 0) {
		if ((flags & MODEL_SDR_FLAG_LIGHT) != 0) {
			float shadow = 1.0;
			// TODO: Shadow mapping support via shadow_map texture
			baseColor.rgb = CalculateLighting(normal, baseColor.rgb, specColor.rgb, glossData, fresnelFactor, shadow, aoFactors.x);
		} else {
			if ((flags & MODEL_SDR_FLAG_SPEC) != 0) {
				baseColor.rgb += pow(1.0 - clamp(dot(eyeDir, normal), 0.0, 1.0), 5.0 * clamp(glossData, 0.01, 1.0)) * specColor.rgb;
			}
		}
	}

	// Glow map
	if ((flags & MODEL_SDR_FLAG_GLOW) != 0) {
		vec3 glowColor = texture(materialTextures[1], vec3(texCoord, float(sGlowmapIndex))).rgb;
		if ((flags & MODEL_SDR_FLAG_MISC) != 0) {
			if ((flags & MODEL_SDR_FLAG_TEAMCOLOR) != 0) {
				float glowColorLuminance = dot(glowColor, vec3(0.299, 0.587, 0.114));
				glowColor = (team_glow_enabled != 0) ? mix(max(team_color_glow, vec3(0.0)), glowColor, clamp(glowColorLuminance - teamMask.b - teamMask.a, 0.0, 1.0)) : glowColor;
			}
		}
		if ((flags & MODEL_SDR_FLAG_HDR) != 0) {
			glowColor = srgb_to_linear(glowColor) * GLOW_MAP_SRGB_MULTIPLIER;
		}
		emissiveColor.rgb += glowColor * GLOW_MAP_INTENSITY;
	}

	if ((flags & MODEL_SDR_FLAG_ALPHA_MULT) != 0) {
		emissiveColor *= alphaMult;
	}

	// Fog
	if ((flags & MODEL_SDR_FLAG_FOG) != 0) {
		vec3 finalFogColor = fogColor.rgb;
		if ((flags & MODEL_SDR_FLAG_HDR) != 0) {
			finalFogColor = srgb_to_linear(finalFogColor);
		}
		if ((flags & MODEL_SDR_FLAG_DIFFUSE) != 0) {
			if (blend_alpha == 1) finalFogColor *= baseColor.a;
		}
		// Apply fog to both emissive and base color for forward rendering
		baseColor.rgb = mix(emissiveColor.rgb + baseColor.rgb, finalFogColor, fragFogDist);
		emissiveColor.rgb = vec3(0.0);
		specColor.rgb *= fragFogDist;
	}

	// Desaturation
	if ((flags & MODEL_SDR_FLAG_DIFFUSE) != 0) {
		if (desaturate == 1) {
			baseColor.rgb = color.rgb * dot(vec3(1.0), baseColor.rgb) * 0.3333333;
		}
	}

	// Ship effects
	if (effect_num == 0) {
		float shinefactor = 1.0/(1.0 + pow(abs((fract(abs(texCoord.x))-anim_timer) * 1000.0), 2.0)) * 1000.0;
		emissiveColor.rgb += vec3(shinefactor);
		baseColor.a = baseColor.a * clamp(shinefactor * (fract(abs(texCoord.x))-anim_timer) * -10000.0,0.0,1.0);
	} else if (effect_num == 1) {
		float shinefactor = 1.0/(1.0 + pow(abs(fragPosition.y-anim_timer), 2.0));
		emissiveColor.rgb += vec3(shinefactor);
		if ((flags & MODEL_SDR_FLAG_LIGHT) == 0) {
			baseColor.a = clamp((fragPosition.y-anim_timer) * 10000.0,0.0,1.0);
		}
	} else if (effect_num == 2) {
		vec2 screenPos = gl_FragCoord.xy * vec2(vpwidth,vpheight);
		baseColor.a = baseColor.a;
		float cloak_interp = (sin(fragPosition.x*fragPosition.w*0.005+anim_timer*20.0)*sin(fragPosition.y*fragPosition.w*0.005)*0.5)-0.5;
		// Note: framebuffer sampling not yet implemented for Vulkan cloaking effect
	}

	// emissive colors won't be added later when we are using forward rendering so we need to do that here
	if ((flags & MODEL_SDR_FLAG_DEFERRED) == 0) {
		baseColor.rgb += emissiveColor.rgb;
	}

	fragOut0 = baseColor;

	if ((flags & MODEL_SDR_FLAG_DEFERRED) != 0) {
		fragOut1 = vec4(fragPosition.xyz, aoFactors.x);
		fragOut2 = vec4(normal, glossData);
		fragOut3 = vec4(specColor.rgb, fresnelFactor);
		fragOut4 = emissiveColor;
	}
}
