#version 450
#extension GL_ARB_separate_shader_objects : enable

// Blit fragment shader
// Samples the scene texture and outputs to swapchain
// Can be extended later for tonemapping/post-processing

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D sceneTexture;

void main() {
    // Sample the scene texture and pass through unchanged
    // Textures use BGRA format, vertex colors use RGBA format - both are correct
    // Swapchain is BGRA but Vulkan handles the component mapping automatically
    outColor = texture(sceneTexture, fragTexCoord);
}

