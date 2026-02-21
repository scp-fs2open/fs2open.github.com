#version 450
#extension GL_ARB_separate_shader_objects : enable

// Decal fragment shader — screen-space decal projection into G-buffer
// Port of OpenGL decal-f.sdr to Vulkan
//
// Technique based on:
// http://martindevans.me/game-development/2015/02/27/Drawing-Stuff-On-Other-Stuff-With-Deferred-Screenspace-Decals/

// G-buffer outputs: 6 locations matching the G-buffer render pass
// Attachments 1, 3, 5 are write-masked to 0 by the pipeline blend state
layout (location = 0) out vec4 fragOut0; // [0] Color/Diffuse
layout (location = 1) out vec4 fragOut1; // [1] Position (masked)
layout (location = 2) out vec4 fragOut2; // [2] Normal
layout (location = 3) out vec4 fragOut3; // [3] Specular (masked)
layout (location = 4) out vec4 fragOut4; // [4] Emissive
layout (location = 5) out vec4 fragOut5; // [5] Composite (masked)

layout (location = 0) flat in mat4 invModelMatrix;  // locations 0-3
layout (location = 4) flat in vec3 decalDirection;
layout (location = 5) flat in float normal_angle_cutoff;
layout (location = 6) flat in float angle_fade_start;
layout (location = 7) flat in float alpha_scale;

// Set 1 = Material, Binding 1 = texture array (diffuse/glow/normal in slots 0/1/2)
layout (set = 1, binding = 1) uniform sampler2DArray decalTextures;

// Set 1 = Material, Binding 4 = depth copy (same slot as soft particle depth)
layout (set = 1, binding = 4) uniform sampler2D gDepthBuffer;

// Set 1 = Material, Binding 6 = normal copy (distortion map slot, unused during G-buffer pass)
layout (set = 1, binding = 6) uniform sampler2D gNormalBuffer;

// Set 1 = Material, Binding 2 = DecalGlobals UBO
layout (set = 1, binding = 2, std140) uniform decalGlobalData {
	mat4 viewMatrix;
	mat4 projMatrix;
	mat4 invViewMatrix;
	mat4 invProjMatrix;

	vec2 viewportSize;
};

// Set 2 = PerDraw, Binding 3 = DecalInfo UBO
layout (set = 2, binding = 3, std140) uniform decalInfoData {
	int diffuse_index;
	int glow_index;
	int normal_index;
	int diffuse_blend_mode;

	int glow_blend_mode;
};

#include "gamma.sdr"
#include "lighting.sdr"
#include "normals.sdr"

vec3 computeViewPosition(vec2 textureCoord) {
	vec4 clipSpaceLocation;
	vec2 normalizedCoord = textureCoord / viewportSize;

	clipSpaceLocation.x = normalizedCoord.x * 2.0 - 1.0;
	// Vulkan negative viewport (y=H, height=-H) inverts the Y mapping:
	// pixel_y = H/2 * (1 - NDC_y), so NDC_y = 1 - 2*pixel_y/H
	clipSpaceLocation.y = 1.0 - normalizedCoord.y * 2.0;
	// Vulkan depth is [0,1] — use directly (no *2-1 like OpenGL)
	clipSpaceLocation.z = texelFetch(gDepthBuffer, ivec2(textureCoord), 0).r;
	clipSpaceLocation.w = 1.0;

	vec4 homogenousLocation = invProjMatrix * clipSpaceLocation;

	return homogenousLocation.xyz / homogenousLocation.w;
}

vec3 getPixelNormal(vec3 frag_position, vec2 tex_coord, inout float alpha, out vec3 binormal, out vec3 tangent) {
	vec3 normal;

	if (normal_index < 0) {
		// No decal normal map: read scene normal from the copy texture
		// for more accurate angle rejection (matching OpenGL USE_NORMAL_MAP path)
		normal = texelFetch(gNormalBuffer, ivec2(tex_coord), 0).xyz;
		binormal = vec3(0.0);
		tangent = vec3(0.0);
	} else {
		// Has decal normal map: use screen-space derivatives for tangent frame
		vec3 pos_dx = dFdx(frag_position);
		vec3 pos_dy = dFdy(frag_position);
		normal = normalize(cross(pos_dx, pos_dy));
		binormal = normalize(pos_dx);
		tangent = normalize(pos_dy);
	}

	// Calculate angle between surface normal and decal direction
	float angle = acos(clamp(dot(normal, decalDirection), -1.0, 1.0));

	if (angle > normal_angle_cutoff) {
		discard;
	}

	// Smooth alpha transition near edges
	alpha = alpha * (1.0 - smoothstep(angle_fade_start, normal_angle_cutoff, angle));

	return normal;
}

vec2 getDecalTexCoord(vec3 view_pos, inout float alpha) {
	vec4 object_pos = invModelMatrix * invViewMatrix * vec4(view_pos, 1.0);

	bvec3 invalidComponents = greaterThan(abs(object_pos.xyz), vec3(0.5));
	bvec4 nanComponents = isnan(object_pos);

	if (any(invalidComponents) || any(nanComponents)) {
		discard;
	}

	// Fade out near top/bottom of decal box
	alpha = alpha * (1.0 - smoothstep(0.4, 0.5, abs(object_pos.z)));

	return object_pos.xy + 0.5;
}

void main() {
	vec3 frag_position = computeViewPosition(gl_FragCoord.xy);

	float alpha = alpha_scale;

	vec2 tex_coord = getDecalTexCoord(frag_position, alpha);

	vec3 binormal;
	vec3 tangent;
	vec3 normal = getPixelNormal(frag_position, gl_FragCoord.xy, alpha, binormal, tangent);

	vec4 diffuse_out = vec4(0.0);
	vec4 emissive_out = vec4(0.0);
	vec3 normal_out = vec3(0.0);

	if (diffuse_index >= 0) {
		vec4 color = texture(decalTextures, vec3(tex_coord, float(diffuse_index)));
		color.rgb = srgb_to_linear(color.rgb);

		if (diffuse_blend_mode == 0) {
			diffuse_out = vec4(color.rgb, color.a * alpha);
		} else {
			diffuse_out = vec4(color.rgb * alpha, 1.0);
		}
	}

	if (glow_index >= 0) {
		vec4 color = texture(decalTextures, vec3(tex_coord, float(glow_index)));
		color.rgb = srgb_to_linear(color.rgb) * GLOW_MAP_SRGB_MULTIPLIER;
		color.rgb *= GLOW_MAP_INTENSITY;

		if (glow_blend_mode == 0) {
			emissive_out = vec4(color.rgb + emissive_out.rgb * emissive_out.a, color.a * alpha);
		} else {
			emissive_out.rgb += color.rgb * alpha;
		}
	}

	if (normal_index >= 0) {
		vec3 decalNormal = unpackNormal(texture(decalTextures, vec3(tex_coord, float(normal_index))).ag);

		mat3 tangentToView;
		tangentToView[0] = tangent;
		tangentToView[1] = binormal;
		tangentToView[2] = normal;

		normal_out = tangentToView * decalNormal * alpha;
	}

	// Active outputs (blend enabled by pipeline)
	fragOut0 = diffuse_out;   // [0] Color
	fragOut2 = vec4(normal_out, 0.0); // [2] Normal
	fragOut4 = emissive_out;  // [4] Emissive

	// Masked outputs (write mask = 0 in pipeline, zero cost)
	fragOut1 = vec4(0.0);     // [1] Position
	fragOut3 = vec4(0.0);     // [3] Specular
	fragOut5 = vec4(0.0);     // [5] Composite
}
