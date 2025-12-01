#version 150

layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 outFragColor;

void main()
{
    outFragColor = vec4(vColor, 1.0);
}
