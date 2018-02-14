in vec4 fragTexCoord;
out vec4 fragOut0;
uniform sampler2D tex;
void main() {
	vec4 color = texture(tex, fragTexCoord.xy);
	fragOut0 = vec4(color.rgb, dot(color.rgb, vec3(0.299, 0.587, 0.114)) );
}