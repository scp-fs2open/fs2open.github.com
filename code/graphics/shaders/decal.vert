#version 450
#extension GL_ARB_separate_shader_objects : enable

// Decal vertex shader — screen-space decal projection
// Port of OpenGL decal-v.sdr to Vulkan

// Binding 0: box vertex positions
layout (location = 0) in vec4 vertPosition;

// Binding 1: per-instance model matrix (mat4 = 4 vec4s at locations 8-11)
layout (location = 8) in vec4 vertModelMatrix0;
layout (location = 9) in vec4 vertModelMatrix1;
layout (location = 10) in vec4 vertModelMatrix2;
layout (location = 11) in vec4 vertModelMatrix3;

layout (location = 0) flat out mat4 invModelMatrix;  // locations 0-3
layout (location = 4) flat out vec3 decalDirection;
layout (location = 5) flat out float normal_angle_cutoff;
layout (location = 6) flat out float angle_fade_start;
layout (location = 7) flat out float alpha_scale;

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

void main() {
	// Reconstruct per-instance model matrix from 4 vec4 columns
	mat4 vertModelMatrix = mat4(vertModelMatrix0, vertModelMatrix1, vertModelMatrix2, vertModelMatrix3);

	// Extract packed data from matrix column w-components
	normal_angle_cutoff = vertModelMatrix[0][3];
	angle_fade_start = vertModelMatrix[1][3];
	alpha_scale = vertModelMatrix[2][3];

	// Clean the matrix (zero out the packed w-components)
	mat4 modelMatrix = vertModelMatrix;
	modelMatrix[0][3] = 0.0;
	modelMatrix[1][3] = 0.0;
	modelMatrix[2][3] = 0.0;

	invModelMatrix = inverse(modelMatrix);
	decalDirection = mat3(viewMatrix) * modelMatrix[2].xyz;
	gl_Position = projMatrix * viewMatrix * modelMatrix * vertPosition;
}
