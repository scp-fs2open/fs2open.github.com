#version 150

void main()
{
    vec2 v1 = vec2(gl_VertexIndex != 1 ? -1.0 : 3.0, gl_VertexIndex != 2 ? 1.0 : -3.0);
    gl_Position = vec4(v1, 1.0, 1.0);
}
