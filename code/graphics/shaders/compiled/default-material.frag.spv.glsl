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
    vec3 _160;
    if (_66)
    {
        _160 = pow(_48.xyz, vec3(2.2000000476837158203125));
    }
    else
    {
        _160 = _48.xyz;
    }
    vec4 _153 = _48;
    _153.x = _160.x;
    vec4 _155 = _153;
    _155.y = _160.y;
    vec4 _157 = _155;
    _157.z = _160.z;
    vec4 _162;
    if (_66)
    {
        _162 = vec4(pow(fragColor.xyz, vec3(2.2000000476837158203125)), fragColor.w);
    }
    else
    {
        _162 = fragColor;
    }
    fragOut0 = mix(mix(_157 * _162, vec4(_162.xyz, _160.x * _162.w), vec4(float(_39.alphaTexture))), _162, vec4(float(_39.noTexturing))) * _39.intensity;
}

