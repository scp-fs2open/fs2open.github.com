in vec4 fragTexCoord;
out vec4 fragOut0;
uniform sampler2DArray shadow_map;
//uniform sampler2D shadow_map;
uniform int index;
void main()
{
	vec3 texcoord = vec3(fragTexCoord.xy, float(index));
	fragOut0 = vec4(texture(shadow_map, texcoord).rgb,1.0);
}