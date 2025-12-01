#version 450
#extension GL_ARB_separate_shader_objects : enable

// Blit fragment shader
// Samples the scene texture and outputs to swapchain

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D sceneTexture;

void main() {
    vec4 sceneColor = texture(sceneTexture, fragTexCoord);
    outColor = sceneColor;
}
