in vec4 vertPosition;
in vec4 vertTexCoord;
in vec4 vertColor;
in float vertRadius;
#ifdef FLAG_EFFECT_GEOMETRY
 in vec3 vertUvec;
 out vec3 geoUvec;
 out float geoRadius;
 out vec4 geoColor;
 out float geoArrayIndex;
#else
 out float fragRadius;
 out vec4 fragPosition;
 out vec4 fragTexCoord;
 out vec4 fragColor;
#endif
#ifdef FLAG_EFFECT_DISTORTION
out float fragOffset;
uniform float use_offset;
#endif
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;
void main()
{
	#ifdef FLAG_EFFECT_GEOMETRY
	 geoRadius = vertRadius;
	 geoUvec = vertUvec;
	 gl_Position = modelViewMatrix * vertPosition;
	 geoColor = vertColor;
	 geoArrayIndex = vertTexCoord.z;
	#else
	 fragRadius = vertRadius;
	 gl_Position = projMatrix * modelViewMatrix * vertPosition;
	 fragPosition = modelViewMatrix * vertPosition;
	 fragTexCoord = vec4(vertTexCoord.xyz, 0.0);
	 fragColor = vertColor;
	#endif
	#ifdef FLAG_EFFECT_DISTORTION
	fragOffset = vertRadius * use_offset;
	#endif
}
