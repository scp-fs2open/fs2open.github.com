#version 450
#extension GL_ARB_separate_shader_objects : enable

// Fullscreen triangle vertex shader
// Generates a triangle that covers the entire screen without any vertex input
// Uses gl_VertexIndex to compute positions and UVs

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // Generate fullscreen triangle positions from vertex index
    // Vertex 0: (-1, -1), Vertex 1: (3, -1), Vertex 2: (-1, 3)
    // This creates a triangle that fully covers the [-1,1] clip space
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    // UV coordinates for the fullscreen triangle
    // Maps the triangle to [0,1] texture space
    vec2 texCoords[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragTexCoord = texCoords[gl_VertexIndex];
}
