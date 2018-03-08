in vec4 vertPosition;
in vec4 vertTexCoord;
in vec4 vertColor;
out vec4 fragTexCoord;
out vec4 fragColor;
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;
uniform vec4 color;

uniform mat4 modelMatrix;
uniform bool clipEnabled;
uniform vec4 clipEquation;

void main()
{
	fragTexCoord = vertTexCoord;
	fragColor = vertColor * color;
	gl_Position = projMatrix * modelViewMatrix * vertPosition;

	if (clipEnabled) {
		gl_ClipDistance[0] = dot(clipEquation, modelMatrix * vertPosition);
	}
}
