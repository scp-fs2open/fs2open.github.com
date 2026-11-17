#include "gropenglcortao.h"

#include "ShaderProgram.h"
#include "gropengldraw.h"
#include "gropenglshader.h"
#include "gropenglstate.h"
#include "graphics/2d.h"
#include "graphics/cortao_occluder_lut.h"
#include "graphics/cortao_params.h"
#include "graphics/ssao.h"

#include <array>

using namespace graphics;

#ifdef USE_OPENGL_ES

// GLES has no GL_R16 textures and no border clamp. CAPABILITY_SCREEN_SPACE_AO is false there.
GLuint opengl_cortao_execute()
{
	return 0;
}

void opengl_cortao_shutdown() {}

#else

namespace {

GLsizei Cortao_width = 0;
GLsizei Cortao_height = 0;
uint32_t Cortao_mip_levels = 0;

// The packed data (R32F that holds the uint bits) with its mip levels, and one framebuffer for each level
GLuint Cortao_packed_texture = 0;
std::array<GLuint, CORTAO_MAX_MIP_LEVELS> Cortao_packed_framebuffers{};

// The main pass writes the raw occlusion, and the filter pass writes the filtered occlusion.
GLuint Cortao_ao_raw_texture = 0;
GLuint Cortao_ao_raw_framebuffer = 0;
GLuint Cortao_ao_texture = 0;
GLuint Cortao_ao_framebuffer = 0;

GLuint Cortao_lut_texture = 0;

// Some resource could not be made. A size change tries again.
bool Cortao_failed = false;

// A frame counter for the animated noise. The scene and the cockpit passes each count one step.
uint64_t Cortao_frame = 0;

void bind_texture_for_setup(GLuint texture)
{
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(texture);
}

GLuint create_framebuffer(GLuint texture, GLint level, const char* label)
{
	GLuint framebuffer = 0;
	glGenFramebuffers(1, &framebuffer);
	GL_state.BindFrameBuffer(framebuffer);
	opengl_set_object_label(GL_FRAMEBUFFER, framebuffer, label);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, level);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		mprintf(("OpenGL CORTAO: The framebuffer '%s' is not complete\n", label));
		glDeleteFramebuffers(1, &framebuffer);
		return 0;
	}
	return framebuffer;
}

GLuint create_ao_texture(const char* label)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	bind_texture_for_setup(texture);
	opengl_set_object_label(GL_TEXTURE, texture, label);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, Cortao_width, Cortao_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return texture;
}

void delete_resources()
{
	for (auto& framebuffer : Cortao_packed_framebuffers) {
		if (framebuffer) {
			glDeleteFramebuffers(1, &framebuffer);
			framebuffer = 0;
		}
	}
	for (auto* framebuffer : {&Cortao_ao_raw_framebuffer, &Cortao_ao_framebuffer}) {
		if (*framebuffer) {
			glDeleteFramebuffers(1, framebuffer);
			*framebuffer = 0;
		}
	}
	for (auto* texture : {&Cortao_packed_texture, &Cortao_ao_raw_texture, &Cortao_ao_texture, &Cortao_lut_texture}) {
		if (*texture) {
			glDeleteTextures(1, texture);
			*texture = 0;
		}
	}
	Cortao_width = 0;
	Cortao_height = 0;
	Cortao_mip_levels = 0;
}

bool create_resources(GLsizei width, GLsizei height)
{
	Cortao_width = width;
	Cortao_height = height;
	Cortao_mip_levels = cortao_mip_levels(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	GL_state.PushFramebufferState();

	// The packed data. CORTAO requires NEAREST filtering, NEAREST mipmaps and a border of 0.
	glGenTextures(1, &Cortao_packed_texture);
	bind_texture_for_setup(Cortao_packed_texture);
	opengl_set_object_label(GL_TEXTURE, Cortao_packed_texture, "CORTAO packed data");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Cortao_mip_levels) - 1);
	for (uint32_t mip = 0; mip < Cortao_mip_levels; ++mip) {
		glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(mip), GL_R32F, std::max(1, width >> mip),
			std::max(1, height >> mip), 0, GL_RED, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const GLfloat border[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	bool ok = true;
	for (uint32_t mip = 0; mip < Cortao_mip_levels && ok; ++mip) {
		Cortao_packed_framebuffers[mip] =
			create_framebuffer(Cortao_packed_texture, static_cast<GLint>(mip), "CORTAO packed data framebuffer");
		ok = Cortao_packed_framebuffers[mip] != 0;
	}

	if (ok) {
		Cortao_ao_raw_texture = create_ao_texture("CORTAO raw occlusion");
		Cortao_ao_raw_framebuffer = create_framebuffer(Cortao_ao_raw_texture, 0, "CORTAO raw occlusion framebuffer");
		Cortao_ao_texture = create_ao_texture("CORTAO filtered occlusion");
		Cortao_ao_framebuffer = create_framebuffer(Cortao_ao_texture, 0, "CORTAO filtered occlusion framebuffer");
		ok = Cortao_ao_raw_framebuffer != 0 && Cortao_ao_framebuffer != 0;
	}

	if (ok) {
		// The 3D lookup table is stored as a 2D texture with the slices one after the other.
		// See cortao_sampleOccluderLookupTable() in cortao-main-f.sdr.
		glGenTextures(1, &Cortao_lut_texture);
		bind_texture_for_setup(Cortao_lut_texture);
		opengl_set_object_label(GL_TEXTURE, Cortao_lut_texture, "CORTAO occluder lookup table");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		// Each row is 48 bytes, so the default unpack alignment of 4 is correct.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, CORTAO_OCCLUDER_LUT_WIDTH,
			CORTAO_OCCLUDER_LUT_HEIGHT * CORTAO_OCCLUDER_LUT_DEPTH, 0, GL_RED, GL_UNSIGNED_SHORT, Cortao_occluder_lut);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	GL_state.PopFramebufferState();

	if (!ok) {
		delete_resources();
		return false;
	}

	mprintf(("OpenGL CORTAO: Initialized (%dx%d, %u packed mip levels)\n", width, height, Cortao_mip_levels));
	return true;
}

// Binds the shader, or returns false when it could not be compiled
bool set_shader(shader_type type, unsigned int flags)
{
	const int handle = gr_opengl_maybe_create_shader(type, flags);
	if (handle < 0) {
		return false;
	}
	opengl_shader_set_current(handle);
	return true;
}

void draw_pass(GLuint framebuffer, GLsizei width, GLsizei height)
{
	GL_state.BindFrameBuffer(framebuffer);
	glViewport(0, 0, width, height);
	// The shaders use gl_FragCoord, not the texture coordinates.
	opengl_draw_full_screen_textured(0.0f, 0.0f, 1.0f, 1.0f);
}

} // namespace

GLuint opengl_cortao_execute()
{
	if (!ssao_enabled()) {
		return 0;
	}

	const GLsizei width = Scene_texture_width;
	const GLsizei height = Scene_texture_height;
	if (width != Cortao_width || height != Cortao_height) {
		delete_resources();
		Cortao_failed = !create_resources(width, height);
		if (Cortao_failed) {
			// Remember the size, so that only a size change tries again.
			Cortao_width = width;
			Cortao_height = height;
		}
	}
	if (Cortao_failed) {
		return 0;
	}

	GR_DEBUG_SCOPE("CORTAO SSAO");

	// No temporal filter (FSR) runs on OpenGL.
	constexpr bool temporalFilterActive = false;
	const uint32_t seed = cortao_random_seed(ssao_animate_noise(temporalFilterActive), Cortao_frame++);
	const SsaoFilterMode filterMode = ssao_filter_mode(temporalFilterActive);
	const auto w = static_cast<uint32_t>(width);
	const auto h = static_cast<uint32_t>(height);

	GLint savedViewport[4];
	glGetIntegerv(GL_VIEWPORT, savedViewport);
	GL_state.PushFramebufferState();
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	gr_zbuffer_set(GR_ZBUFF_NONE);

	bool ok = true;
	{
		GR_DEBUG_SCOPE("CORTAO prepass");
		ok = set_shader(SDR_TYPE_CORTAO_PREPASS, 0);
		if (ok) {
			Current_shader->program->Uniforms.setTextureUniform("PositionBuffer", 0);
			Current_shader->program->Uniforms.setTextureUniform("NormalBuffer", 1);
			GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_position_texture);
			GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_normal_texture);

			for (uint32_t mip = 0; mip < Cortao_mip_levels; ++mip) {
				opengl_set_generic_uniform_data<cortao_prepass_uniforms>([&](cortao_prepass_uniforms* data) {
					cortao_fill_prepass_uniforms(*data, w, h, seed, mip);
				});
				draw_pass(Cortao_packed_framebuffers[mip], std::max(1, width >> mip), std::max(1, height >> mip));
			}
		}
	}

	if (ok) {
		GR_DEBUG_SCOPE("CORTAO main pass");
		ok = set_shader(SDR_TYPE_CORTAO_MAIN, cortao_main_shader_flags());
		if (ok) {
			Current_shader->program->Uniforms.setTextureUniform("PackedData", 0);
			Current_shader->program->Uniforms.setTextureUniform("NormalBuffer", 1);
			Current_shader->program->Uniforms.setTextureUniform("OccluderLookupTable", 2);
			GL_state.Texture.Enable(0, GL_TEXTURE_2D, Cortao_packed_texture);
			GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_normal_texture);
			GL_state.Texture.Enable(2, GL_TEXTURE_2D, Cortao_lut_texture);

			// The OpenGL G-buffer pass uses the normal viewport, so NDC +Y is the last texel row.
			opengl_set_generic_uniform_data<cortao_main_uniforms>([&](cortao_main_uniforms* data) {
				cortao_fill_main_uniforms(*data, w, h, seed, false);
			});
			draw_pass(Cortao_ao_raw_framebuffer, width, height);
		}
	}

	GLuint result = ok ? Cortao_ao_raw_texture : 0;
	if (ok && filterMode != SsaoFilterMode::Off) {
		GR_DEBUG_SCOPE("CORTAO filter pass");
		const unsigned int flags = (filterMode == SsaoFilterMode::Large) ? SDR_FLAG_CORTAO_LARGE_FILTER : 0;
		if (set_shader(SDR_TYPE_CORTAO_FILTER, flags)) {
			Current_shader->program->Uniforms.setTextureUniform("PackedData", 0);
			Current_shader->program->Uniforms.setTextureUniform("SsaoBuffer", 1);
			GL_state.Texture.Enable(0, GL_TEXTURE_2D, Cortao_packed_texture);
			GL_state.Texture.Enable(1, GL_TEXTURE_2D, Cortao_ao_raw_texture);

			opengl_set_generic_uniform_data<cortao_filter_uniforms>([&](cortao_filter_uniforms* data) {
				cortao_fill_filter_uniforms(*data, w, h);
			});
			draw_pass(Cortao_ao_framebuffer, width, height);
			result = Cortao_ao_texture;
		}
	}

	GL_state.PopFramebufferState();
	glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);

	return result;
}

void opengl_cortao_shutdown()
{
	delete_resources();
	Cortao_failed = false;
}

#endif // USE_OPENGL_ES
