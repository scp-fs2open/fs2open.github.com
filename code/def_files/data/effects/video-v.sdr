in vec4 vertPosition;
in vec4 vertTexCoord;
out vec4 fragTexCoord;
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;
void main()
{
	fragTexCoord = vertTexCoord;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
}