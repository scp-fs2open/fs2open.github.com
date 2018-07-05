
const float SRGB_GAMMA = 2.2;
const float SRGB_GAMMA_INVERSE = 1.0 / SRGB_GAMMA;

float srgb_to_linear(float val) {
	return pow(val, SRGB_GAMMA);
}
vec2 srgb_to_linear(vec2 val) {
	return pow(val, vec2(SRGB_GAMMA));
}
vec3 srgb_to_linear(vec3 val) {
	return pow(val, vec3(SRGB_GAMMA));
}
vec4 srgb_to_linear(vec4 val) {
	return pow(val, vec4(SRGB_GAMMA));
}

float linear_to_srgb(float val) {
	return pow(val, SRGB_GAMMA_INVERSE);
}
vec2 linear_to_srgb(vec2 val) {
	return pow(val, vec2(SRGB_GAMMA_INVERSE));
}
vec3 linear_to_srgb(vec3 val) {
	return pow(val, vec3(SRGB_GAMMA_INVERSE));
}
vec4 linear_to_srgb(vec4 val) {
	return pow(val, vec4(SRGB_GAMMA_INVERSE));
}
