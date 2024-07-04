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
    vec3 _146;
    if (_66)
    {
        _146 = pow(_48.xyz, vec3(2.2000000476837158203125));
    }
    else
    {
        _146 = _48.xyz;
    }
    vec4 _148;
    if (_66)
    {
        _148 = vec4(pow(fragColor.xyz, vec3(2.2000000476837158203125)), fragColor.w);
    }
    else
    {
        _148 = fragColor;
    }
    fragOut0 = mix(mix(vec4(_146.x, _146.y, _146.z, _48.w) * _148, vec4(_148.xyz, _146.x * _148.w), vec4(float(_39.alphaTexture))), _148, vec4(float(_39.noTexturing))) * _39.intensity;
}

