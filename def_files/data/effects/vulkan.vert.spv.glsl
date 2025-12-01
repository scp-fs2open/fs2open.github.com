#version 150

const vec2 _20[3] = vec2[](vec2(0.0, -0.5), vec2(0.5), vec2(-0.5, 0.5));
const vec3 _29[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

layout(location = 0) out vec3 vColor;

void main()
{
    vec3 _18 = _29[gl_VertexIndex];
    gl_Position = vec4(_20[gl_VertexIndex], 0.0, 1.0);
    vColor = _18;
}
