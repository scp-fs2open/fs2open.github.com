#version 450
#extension GL_ARB_separate_shader_objects : enable

// Post-processing effects shader (Vulkan port of post-f.sdr)
// Uses runtime effectFlags instead of compile-time #ifdef flags

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 fragOut0;

layout(set = 1, binding = 1) uniform sampler2D tex;

// Effect flag bits (match post_effect_t index in post_processing.tbl)
const int FLAG_DISTORT_NOISE = (1 << 0);
const int FLAG_SATURATION    = (1 << 1);
const int FLAG_BRIGHTNESS    = (1 << 2);
const int FLAG_CONTRAST      = (1 << 3);
const int FLAG_GRAIN         = (1 << 4);
const int FLAG_STRIPES       = (1 << 5);
const int FLAG_CUTOFF        = (1 << 6);
const int FLAG_DITH          = (1 << 7);
const int FLAG_TINT          = (1 << 8);

layout(std140, set = 2, binding = 0) uniform genericData {
	float timer;
	float noise_amount;
	float saturation;
	float brightness;

	float contrast;
	float film_grain;
	float tv_stripes;
	float cutoff;

	vec3 tint;
	float dither;

	vec3 custom_effect_vec3_a;
	float custom_effect_float_a;

	vec3 custom_effect_vec3_b;
	float custom_effect_float_b;

	int effectFlags;
};

void main()
{
	vec2 distort = vec2(0.0, 0.0);

	// Distort noise
	if ((effectFlags & FLAG_DISTORT_NOISE) != 0) {
		float distort_factor = timer * sin(fragTexCoord.x * fragTexCoord.y * 100.0 + timer);
		distort_factor = mod(distort_factor, 8.0) * mod(distort_factor, 4.0);
		distort = vec2(mod(distort_factor, noise_amount), mod(distort_factor, noise_amount + 0.002));
	}

	vec4 color_in = texture(tex, fragTexCoord.xy + distort);
	vec4 color_out;

	// Saturation
	if ((effectFlags & FLAG_SATURATION) != 0) {
		vec4 color_grayscale = color_in;
		color_grayscale.rgb = vec3(dot(color_in.rgb, vec3(0.299, 0.587, 0.184)));
		color_out = mix(color_in, color_grayscale, 1.0 - saturation);
	} else {
		color_out = color_in;
	}

	// Brightness
	if ((effectFlags & FLAG_BRIGHTNESS) != 0) {
		color_out.rgb = color_out.rgb * vec3(brightness);
	}

	// Contrast
	if ((effectFlags & FLAG_CONTRAST) != 0) {
		color_out.rgb = color_out.rgb + vec3(0.5 - 0.5 * contrast);
	}

	// Film grain
	if ((effectFlags & FLAG_GRAIN) != 0) {
		float x = fragTexCoord.x * fragTexCoord.y * timer * 1000.0;
		x = mod(x, 13.0) * mod(x, 123.0);
		float dx = mod(x, 0.01);
		vec3 result = color_out.rgb + color_out.rgb * clamp(0.1 + dx * 100.0, 0.0, 1.0);
		color_out.rgb = mix(color_out.rgb, result, film_grain);
	}

	// TV stripes
	if ((effectFlags & FLAG_STRIPES) != 0) {
		vec2 sc;
		sc.x = sin(fragTexCoord.y * 2048.0);
		sc.y = cos(fragTexCoord.y * 2048.0);
		vec3 stripes = color_out.rgb + color_out.rgb * vec3(sc.x, sc.y, sc.x) * 0.8;
		color_out.rgb = mix(color_out.rgb, stripes, tv_stripes);
	}

	// Cutoff
	if ((effectFlags & FLAG_CUTOFF) != 0) {
		if (cutoff > 0.0) {
			vec4 color_greyscale;
			color_greyscale.rgb = vec3(dot(color_in.rgb, vec3(0.299, 0.587, 0.184)));
			vec4 normalized_col;
			float col_length = length(color_out.rgb);
			if (col_length > 1.0) {
				normalized_col = color_out / col_length;
			} else {
				normalized_col = color_out;
			}
			vec3 unit_grey = vec3(0.5773);
			float sat = dot(normalized_col.rgb, unit_grey);
			color_out = mix(color_greyscale, color_out, sat * cutoff);
		}
	}

	// Dithering
	if ((effectFlags & FLAG_DITH) != 0) {
		float downsampling_factor = 4.0;
		float bias = 0.5;
		color_out.rgb = floor(color_out.rgb * downsampling_factor + bias) / downsampling_factor;
	}

	// Tint
	if ((effectFlags & FLAG_TINT) != 0) {
		color_out.rgb += tint;
	}

	color_out.a = 1.0;
	fragOut0 = color_out;
}
