#version 150

uniform sampler2D srcImage;
layout(location = 0) out vec4 outFragColor;
layout(location = 0) in vec2 vTexcoord;

void main()
{
    outFragColor = vec4(texture(srcImage, vTexcoord).xyz, 1.0);
}
