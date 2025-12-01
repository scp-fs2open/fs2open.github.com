#version 150

layout(std140) uniform SceneLightingData
{
    vec3 directionalLightColor;
    vec3 directionalLightDir;
    vec3 ambientLight;
    vec4 sceneColorScale;
} _6;

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
} _17;

layout(std140) uniform LightingValues
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 diffuseLightColor;
    vec4 specularLightColor;
    float ambientFactor;
    float diffuseFactor;
    float specFactor;
    float shininess;
    float specGlossExponent;
    uint diffuseLightCount;
    uint specLightCount;
} _28;

uniform sampler2D sBaseMap;

layout(location = 0) in vec3 P;
layout(location = 1) in vec3 N;
layout(location = 2) in vec4 unmodVertColor;
layout(location = 3) in vec2 texCoords;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 _56 = texture(sBaseMap, texCoords);
    vec3 _59 = normalize(N);
    FragColor = (((vec4(vec3(dot(_6.directionalLightDir, _59) * clamp(_28.diffuseFactor, 0.0, 1.0)), 0.0) * vec4(_6.directionalLightColor, 1.0)) + vec4(_6.ambientLight, 1.0)) * _28.ambientColor) * vec4(_6.sceneColorScale.xyz, 1.0);
    vec4 _97;
    if (_17.noTexturing != 0)
    {
        _97 = _17.color * _6.sceneColorScale;
    }
    else
    {
        _97 = _6.sceneColorScale * vec4(vec3(_56) * vec3(unmodVertColor), _56.w) * _17.color;
    }
    FragColor *= _97;
}
