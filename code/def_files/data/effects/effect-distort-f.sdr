in vec4 fragTexCoord;
in vec4 fragColor;
in float fragOffset;
out vec4 fragOut0;
uniform sampler2DArray baseMap;
uniform sampler2D depthMap;
uniform float window_width;
uniform float window_height;
uniform sampler2D distMap;
uniform sampler2D frameBuffer;
void main()
{
	vec2 depthCoord = vec2(gl_FragCoord.x / window_width, gl_FragCoord.y / window_height);
	vec4 fragmentColor = texture(baseMap, fragTexCoord.xyz) * fragColor.a;
	vec2 distortion = texture(distMap, fragTexCoord.xy+vec2(0.0, fragOffset)).rg;
	float alpha = clamp(dot(fragmentColor.rgb,vec3(0.3333))*10.0,0.0,1.0);
	distortion = ((distortion - 0.5) * 0.01) * alpha;
	fragOut0 = texture(frameBuffer,depthCoord+distortion);
	fragOut0.a = alpha;
}
