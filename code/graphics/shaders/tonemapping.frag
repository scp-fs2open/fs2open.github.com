#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "gamma.sdr"

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D sceneTex;

layout(set = 2, binding = 0, std140) uniform genericData {
	float exposure;
	int tonemapper;
	float x0;
	float y0;
	float x1;
	float toe_B;
	float toe_lnA;
	float sh_B;
	float sh_lnA;
	float sh_offsetX;
	float sh_offsetY;
	int linearOut;
};

// Tonemapping operators — matched to OpenGL tonemapping-f.sdr implementations

vec3 linear_tonemap(vec3 color) {
	return clamp(color, 0.0, 1.0);
}

vec3 uc2_tonemap(vec3 color) {
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
	return color;
}

vec3 aces_tonemap(vec3 color) {
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		 1.60475, -0.10208, -0.00327,
		-0.53108,  1.10813, -0.07276,
		-0.07367, -0.00605,  1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

vec3 aces_approx_tonemap(vec3 color) {
	color *= 0.6;
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 cineon_tonemap(vec3 color) {
	// optimized filmic operator by Jim Hejl and Richard Burgess-Dawson
	// linear to sRGB conversion embedded in shader
	color = max(vec3(0.0), color - 0.004);
	return (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
}

vec3 reinhard_jodie_tonemap(vec3 color) {
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return color;
}

vec3 reinhard_extended_tonemap(vec3 color) {
	float max_white = 1.0;
	vec3 numerator = color * (1.0 + (color / vec3(max_white * max_white)));
	return numerator / (1.0 + color);
}

// Piecewise Power Curve helpers — matched to OpenGL shoulder sign convention
float ppc_toe(float x) {
	return exp(toe_lnA + toe_B * log(x));
}

float ppc_linear(float x) {
	return y0 + (x - x0);
}

float ppc_shoulder(float x) {
	// Scale is -1 so reverse subtraction to save a mult
	x = sh_offsetX - x;
	x = exp(sh_lnA + sh_B * log(x));
	x = sh_offsetY - x;
	return x;
}

float ppc_eval(float x_in) {
	if (x_in <= x0) {
		return ppc_toe(x_in);
	} else if (x_in <= x1) {
		return ppc_linear(x_in);
	} else if (x_in < sh_offsetX) {
		return ppc_shoulder(x_in);
	} else {
		return sh_offsetY;
	}
}

vec3 ppc_tonemap(vec3 color) {
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if (luma <= 0.0) return vec3(0.0);
	float luma_tone;
	if (luma <= x0) {
		luma_tone = ppc_toe(luma);
	} else if (luma <= x1) {
		luma_tone = ppc_linear(luma);
	} else if (luma < sh_offsetX) {
		luma_tone = ppc_shoulder(luma);
	} else {
		luma_tone = sh_offsetY;
	}
	return color * luma_tone / luma;
}

vec3 ppc_rgb_tonemap(vec3 color) {
	return vec3(ppc_eval(color.r), ppc_eval(color.g), ppc_eval(color.b));
}

void main()
{
	vec3 color = texture(sceneTex, fragTexCoord).rgb;
	color *= exposure;

	// Apply selected tonemapper
	if (tonemapper == 0) {
		color = linear_tonemap(color);
	} else if (tonemapper == 1) {
		color = uc2_tonemap(color);
	} else if (tonemapper == 2) {
		color = aces_tonemap(color);
	} else if (tonemapper == 3) {
		color = aces_approx_tonemap(color);
	} else if (tonemapper == 4) {
		color = cineon_tonemap(color);
	} else if (tonemapper == 5) {
		color = reinhard_jodie_tonemap(color);
	} else if (tonemapper == 6) {
		color = reinhard_extended_tonemap(color);
	} else if (tonemapper == 7) {
		color = ppc_tonemap(color);
	} else if (tonemapper == 8) {
		color = ppc_rgb_tonemap(color);
	}

	if (linearOut == 0) {
		color = linear_to_srgb(color);
	}

	fragOut0 = vec4(color, 1.0);
}
