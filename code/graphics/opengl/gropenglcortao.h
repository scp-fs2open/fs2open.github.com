#pragma once

// The CORTAO screen-space ambient occlusion passes for the OpenGL backend (see graphics/ssao.h).
// They are the same fragment passes as in the Vulkan backend (VulkanPostProcessingCortao.cpp).

#include "graphics/opengl/gropengl.h"

// The texture unit of SsaoBuffer in the deferred light shader. Units 0 to 6 and
// SHADOW_MAP_RAW_TEXTURE_UNIT (11) are in use there.
constexpr GLuint SSAO_TEXTURE_UNIT = 12;

/**
 * @brief Write the occlusion of the current G-buffer, if SSAO is enabled
 *
 * Call before the deferred light shader is bound. This function changes the shader, the blend mode,
 * the texture units 0 to 2 and the uniform buffer bindings. It restores the framebuffer and the viewport.
 *
 * @return The occlusion texture for the deferred light pass, or 0 when SSAO is not active
 */
GLuint opengl_cortao_execute();

// Delete the CORTAO textures and framebuffers. The next opengl_cortao_execute() creates them again.
void opengl_cortao_shutdown();
