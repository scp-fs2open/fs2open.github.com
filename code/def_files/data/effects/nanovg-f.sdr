
// Enabled at all times but the define is kept here in case this should be configurable in the future
#define EDGE_AA

layout(std140) uniform NanoVGUniformData {
    mat3 scissorMat;

    mat3 paintMat;

    vec4 innerCol;

    vec4 outerCol;

    vec2 scissorExt;
    vec2 scissorScale;

    vec2 extent;
    float radius;
    float feather;

    float strokeMult;
    float strokeThr;
    int texType;
    int type;

    vec2 viewSize;
    int texArrayIndex;
};

uniform sampler2DArray nvg_tex;

in vec2 ftcoord;
in vec2 fpos;
out vec4 outColor;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
	vec2 ext2 = ext - vec2(rad,rad);
	vec2 d = abs(pt) - ext2;
	return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;
}

// Scissoring
float scissorMask(vec2 p) {
	vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);
	sc = vec2(0.5,0.5) - sc * scissorScale;
	return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);
}
#ifdef EDGE_AA
// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() {
	return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);
}
#endif

void main(void) {
   vec4 result;
	float scissor = scissorMask(fpos);
#ifdef EDGE_AA
	float strokeAlpha = strokeMask();
#else
	float strokeAlpha = 1.0;
#endif
#ifdef EDGE_AA
	if (strokeAlpha < strokeThr) {
		discard;
	}
#endif
	if (type == 0) {			// Gradient
		// Calculate gradient color using box gradient
		vec2 pt = (paintMat * vec3(fpos,1.0)).xy;
		float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);
		vec4 color = mix(innerCol,outerCol,d);
		// Combine alpha
		color *= strokeAlpha * scissor;
		result = color;
	} else if (type == 1) {		// Image
		// Calculate color fron texture
		vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;
		vec4 color = texture(nvg_tex, vec3(pt, float(texArrayIndex)));
		// Apply color tint and alpha.
		if (texType == 1) color = vec4(color.xyz*color.w,color.w);
		if (texType == 2) color = vec4(color.r);
		color *= innerCol;
		// Combine alpha
		color *= strokeAlpha * scissor;
		result = color;
	} else if (type == 2) {		// Stencil fill
		result = vec4(1,1,1,1);
	} else if (type == 3) {		// Textured tris
		vec4 color = texture(nvg_tex, vec3(ftcoord, float(texArrayIndex)));
		if (texType == 1) color = vec4(color.xyz*color.w,color.w);		if (texType == 2) color = vec4(color.x);		color *= scissor;
		result = color * innerCol;
	}
	outColor = result;
}
