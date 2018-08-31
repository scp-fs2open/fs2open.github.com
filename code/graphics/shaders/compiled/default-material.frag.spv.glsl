#version 150

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
} _39;

uniform sampler2DArray baseMap;

in vec4 fragTexCoord;
in vec4 fragColor;
out vec4 fragOut0;

void main()
{
    vec4 _48 = texture(baseMap, vec3(fragTexCoord.xy, float(_39.baseMapIndex)));
    if (_39.alphaThreshold > _48.w)
    {
        discard;
    }
    bool _66 = _39.srgb == 1;
    vec3 _148;
    if (_66)
    {
        _148 = pow(_48.xyz, vec3(2.2000000476837158203125));
    }
    else
    {
        _148 = _48.xyz;
    }
    vec4 _150;
    if (_66)
    {
        _150 = vec4(pow(fragColor.xyz, vec3(2.2000000476837158203125)), fragColor.w);
    }
    else
    {
        _150 = fragColor;
    }
    fragOut0 = mix(mix(vec4(_148.x, _148.y, _148.z, _48.w) * _150, vec4(_150.xyz, _148.x * _150.w), vec4(float(_39.alphaTexture))), _150, vec4(float(_39.noTexturing))) * _39.intensity;
}

