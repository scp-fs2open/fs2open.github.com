
/**
 * @brief Unpacks a DXT5nm encoded normal
 * @param normal_sample The Alpha and Green channel of the DXT5 texture
 * @return The unpacked normal
 */
vec3 unpackNormal(vec2 normal_sample) {
	vec3 normal;
	normal.xy = normal_sample * 2.0 - 1.0;
	normal.z = sqrt(1 - normal_sample.x * normal_sample.x - normal_sample.y * normal_sample.y);
	return normal;
}
