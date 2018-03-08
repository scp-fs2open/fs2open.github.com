in vec4 vertPosition;
in vec4 vertTexCoord;
in vec4 vertColor;
in float vertRadius;
out vec4 fragTexCoord;
out vec4 fragColor;
out float fragOffset;
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;
uniform float use_offset;
void main()
{
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
	fragOffset = vertRadius * use_offset;
	fragTexCoord = vec4(vertTexCoord.xyz, 0.0);
	fragColor = vertColor;
}
