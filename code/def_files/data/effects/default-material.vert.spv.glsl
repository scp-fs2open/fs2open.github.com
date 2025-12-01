#version 150

out float gl_ClipDistance[1];

layout(std140) uniform genericData
{
    mat4 modelMatrix;
    vec4 color;
    vec4 clipEquation;
    int baseMapIndex;
    int alphaTexture;
    int noTexturing;
    int srgb;
    float intensity;
    float alphaThreshold;
    uint clipEnabled;
} _22;

layout(std140) uniform matrixData
{
    mat4 projViewMatrix;
    mat4 projMatrix;
    mat4 viewMatrix;
    mat4 invViewMatrix;
    mat4 viewProjectionMatrix;
    vec2 clipDistances;
    vec2 screenWidthAndHeight;
    vec3 eyePosition;
} _31;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uTexCoord;
layout(location = 4) in vec4 vertexColor;
layout(location = 0) out vec3 P;
layout(location = 1) out vec3 N;
layout(location = 2) out vec4 unmodVertColor;
layout(location = 3) out vec2 texCoords;

void main()
{
    vec4 v_42 = vec4(position, 1.0) * _22.modelMatrix;
    gl_Position = v_42 * _31.viewProjectionMatrix;
    N = mat3(_22.modelMatrix) * normal;
    P = v_42.xyz;
    unmodVertColor = vertexColor;
    texCoords = uTexCoord;
    gl_ClipDistance[0] = _22.clipEnabled != 0u ? dot(v_42, _22.clipEquation) : 1.0;
}
