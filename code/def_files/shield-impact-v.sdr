in vec4 vertPosition;
in vec3 vertNormal;
out vec4 fragImpactUV;
out float fragNormOffset;
uniform mat4 modelViewMatrix;
uniform mat4 projMatrix;
uniform vec3 hitNormal;
uniform mat4 shieldModelViewMatrix;
uniform mat4 shieldProjMatrix;
void main()
{
	gl_Position = projMatrix * modelViewMatrix * vertPosition;
	//vec3 normal = normalize(mat3(modelViewMatrix) * vertNormal);
	fragNormOffset = dot(hitNormal, vertNormal);
	fragImpactUV = shieldProjMatrix * shieldModelViewMatrix * vertPosition;
	fragImpactUV += 1.0f;
	fragImpactUV *= 0.5f;
}