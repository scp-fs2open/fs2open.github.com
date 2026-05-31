#ifndef ES_COMPATIBILITY_H
#define ES_COMPATIBILITY_H
#ifdef USE_OPENGL_ES
#include <vector>
#include <glad/glad.h>
#include <KHR/khrplatform.h>
/*
	OpenGL ES 3.2 compatibility layer
	---------------------------------
	TODO/Good to have: 
	-MSAA causes ASSERTION: "GL_state.ValidForFlip()" at gropengl.cpp:141
	-The entire MSAA path is broken in ES on multiple places
	-GL_PROXY_TEXTURE_2D and GL_TEXTURE_COMPRESSED_IMAGE_SIZE solution if possible
	-Screenshoots do not work.
*/

//Stubs Enums, this does not exist on GLES and need to be handled
#define GL_MAP_PERSISTENT_BIT				    0x0040
#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB	    0x8E8C
#define GL_PROXY_TEXTURE_2D					    GL_TEXTURE_2D // needs an alternative way to reeplace proxy
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE	    0x86A0 // just compile fix
#define GL_FILL                                 0x1B02
#define GL_LINE                                 0x1B01
#define GL_POINT                                0x1B00
#define GL_DOUBLE                               0x140A
#define GL_UNSIGNED_SHORT_1_5_5_5_REV			0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV				0x8367
#define GL_BGR									0x80E0
#define GL_MULTISAMPLE							0x809D

//Enums Redefinitions
#define GL_CLIP_DISTANCE0						GL_CLIP_DISTANCE0_EXT // GL_EXT_clip_cull_distance
#define GL_BGRA									GL_BGRA_EXT // Depends on GL_EXT_texture_format_BGRA8888
#define GL_DEPTH_COMPONENT32					GL_DEPTH_COMPONENT24
#define GL_RGB5									GL_RGB5_A1 // has extra alpha bit
#define GL_ARB_gpu_shader5						GL_EXT_gpu_shader5 // Optional on ES 3.1, guarranted on ES 3.2
#define GL_TIMESTAMP							GL_TIMESTAMP_EXT // Depends on GL_EXT_disjoint_timer_query
#define GLAD_GL_ARB_texture_compression_bptc	GLAD_GL_EXT_texture_compression_bptc // Optional and unlikely
#define GLAD_GL_ARB_gpu_shader5					GLAD_GL_EXT_gpu_shader5 // Optional on ES 3.1, guarranted on ES 3.2
#define GLAD_GL_ARB_get_program_binary			GLAD_GL_OES_get_program_binary // ES 2.0
#define GLAD_GL_ARB_buffer_storage				GLAD_GL_EXT_buffer_storage // ES 2.0
#define GLAD_GL_ARB_timer_query					GLAD_GL_EXT_disjoint_timer_query // Optional
#define GLAD_GL_ARB_texture_storage				GL_TRUE // ES 3.2
#define GLAD_GL_ARB_draw_buffers_blend			GL_TRUE // ES 3.2 (glBlendFunci and glBlendEquationi)
#define GLAD_GL_ARB_vertex_attrib_binding		GL_TRUE // ES 3.1

//Functions Redefinitions
#define glBlendFunciARB					glBlendFunci // ES 3.2
#define glTexImage2DMultisample			glFramebufferTexture2DMultisampleEXT // Depends on GL_EXT_multisampled_render_to_texture
#define glBufferStorage					glBufferStorageEXT // Depends on GLAD_GL_EXT_buffer_storage
#define gladLoadGLLoader                gladLoadGLES2Loader
#define glDebugMessageCallbackARB       glDebugMessageCallback // ES 3.2
#define glDebugMessageControlARB        glDebugMessageControl // ES 3.2
#define glGetDebugMessageLogARB         glGetDebugMessageLog // ES 3.2
#define glDepthRange                    glDepthRangef // ES 3.0

/* 
	Pixel Conversion for unsupported uncompressed formats 
*/

// BGRA 1_5_5_5_REV -> RGBA 5_5_5_1
static inline void convert_BGRA1555_REV_to_RGBA5551(const uint16_t* __restrict src, uint16_t* __restrict dst, size_t npx)
{
	for (size_t i = 0; i < npx; i++) {
		const uint16_t s = src[i];
		uint16_t a = (s >> 15) & 0x1;
		uint16_t r = (s >> 10) & 0x1F;
		uint16_t g = (s >> 5) & 0x1F;
		uint16_t b = (s >> 0) & 0x1F;
		dst[i] = static_cast<uint16_t>((r << 11) | (g << 6) | (b << 1) | a);
	}
}

// BGRA1555_REV -> RGBA8888
static inline void convert_BGRA1555_REV_to_RGBA8888(const uint16_t* __restrict src, uint8_t* __restrict dstRGBA8, size_t npx)
{
	for (size_t i = 0; i < npx; ++i) {
		uint16_t s = src[i];
		uint8_t A = (s >> 15) ? 255 : 0;
		uint8_t R5 = (s >> 10) & 0x1F;
		uint8_t G5 = (s >> 5) & 0x1F;
		uint8_t B5 = s & 0x1F;
		// expand 5 to 8 bits
		uint8_t R = (R5 << 3) | (R5 >> 2);
		uint8_t G = (G5 << 3) | (G5 >> 2);
		uint8_t B = (B5 << 3) | (B5 >> 2);
		size_t o = 4 * i;
		dstRGBA8[o + 0] = R;
		dstRGBA8[o + 1] = G;
		dstRGBA8[o + 2] = B;
		dstRGBA8[o + 3] = A;
	}
}

// BGR -> RGB
static inline void convert_BGR_to_RGB(const uint8_t* __restrict src, uint8_t* __restrict dst, size_t npx)
{
	for (size_t i = 0, s = 0, t = 0; i < npx; ++i, s += 3, t += 3) {
		uint8_t b = src[s + 0];
		uint8_t g = src[s + 1];
		uint8_t r = src[s + 2];
		dst[t + 0] = r;
		dst[t + 1] = g;
		dst[t + 2] = b;
	}
}

// BGRA 8888 -> RGBA 8888
static inline void convert_BGRA8888_to_RGBA8888(const uint8_t* __restrict src, uint8_t* __restrict dst, size_t npx)
{
	for (size_t i = 0; i < npx; i++) {
		const uint8_t b = src[4 * i + 0];
		const uint8_t g = src[4 * i + 1];
		const uint8_t r = src[4 * i + 2];
		const uint8_t a = src[4 * i + 3];
		dst[4 * i + 0] = r;
		dst[4 * i + 1] = g;
		dst[4 * i + 2] = b;
		dst[4 * i + 3] = a;
	}
}

// BGR 3B -> RGBA 4B
static inline void convert_BGR_to_RGBA(const uint8_t* __restrict src, uint8_t* __restrict dst, size_t npx)
{
	for (size_t i = 0; i < npx; ++i) {
		const size_t s = i * 3;
		const size_t t = i * 4;
		dst[t + 0] = src[s + 2]; // R <- B
		dst[t + 1] = src[s + 1]; // G
		dst[t + 2] = src[s + 0]; // B <- R
		dst[t + 3] = 255;        // A
	}
}

// BGRA1555_REV -> RGB888
static inline void convert_BGRA1555_REV_to_RGB888(const uint16_t* __restrict src, uint8_t* __restrict dstRGB8, size_t npx)
{
	for (size_t i = 0; i < npx; ++i) {
		uint16_t s = src[i];
		uint8_t R5 = (s >> 10) & 0x1F;
		uint8_t G5 = (s >> 5) & 0x1F;
		uint8_t B5 = s & 0x1F;

		// expand 5 to 8 bits
		uint8_t R = (R5 << 3) | (R5 >> 2);
		uint8_t G = (G5 << 3) | (G5 >> 2);
		uint8_t B = (B5 << 3) | (B5 >> 2);

		size_t o = 3 * i;
		dstRGB8[o + 0] = R;
		dstRGB8[o + 1] = G;
		dstRGB8[o + 2] = B;
	}
}

/* 
	Intercept calls to do pixel conversion and format adjustments to something the driver can work with
	Note: This is only needed for uncompressed texture formats
*/

// Bring internalFormat info for glTexSubImage3D calls
static inline GLint query_internalformat_3d(GLenum target, GLint level)
{
	GLint ifmt = 0;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &ifmt);
	return ifmt;
}

#ifdef glTexSubImage3D
#undef glTexSubImage3D
#endif
static inline void glTexSubImage3D(GLenum target, GLint level, GLint xoff, GLint yoff, GLint zoff, GLsizei w, GLsizei h, GLsizei d, GLenum format, GLenum type, const void* data)
{
	if (format == GL_BGRA && type == GL_UNSIGNED_SHORT_1_5_5_5_REV) 
	{
		const size_t npx = size_t(w) * size_t(h) * size_t(d);
		GLint internalFormat = query_internalformat_3d(target,level);
		if (internalFormat == GL_RGBA8)
		{
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
			if (data != nullptr) 
			{
				std::vector<uint8_t> scratch(npx * 4); // RGBA8888 = 4 BPP
				convert_BGRA1555_REV_to_RGBA8888(reinterpret_cast<const uint16_t*>(data), scratch.data(), npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			} 
		} else if (internalFormat == GL_RGB8) {
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
			if (data != nullptr)
			{
				std::vector<uint8_t> scratch(npx * 3); // RGB888 = 3 BPP
				convert_BGRA1555_REV_to_RGB888(reinterpret_cast<const uint16_t*>(data), scratch.data(), npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		} else {
			format = GL_RGBA;
			type = GL_UNSIGNED_SHORT_5_5_5_1;
			if (data != nullptr) {
				std::vector<uint8_t> scratch(npx * 2); // RGBA5551 = 2 BPP
				convert_BGRA1555_REV_to_RGBA5551(reinterpret_cast<const uint16_t*>(data),
					reinterpret_cast<uint16_t*>(scratch.data()),
					npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		}
	} 
	else if (format == GL_BGR && type == GL_UNSIGNED_BYTE) 
	{
		const size_t npx = size_t(w) * size_t(h) * size_t(d);
		GLint internalFormat = query_internalformat_3d(target, level);
		if (internalFormat == GL_RGBA8)
		{
			format = GL_RGBA;
			if (data != nullptr) {
				std::vector<uint8_t> scratch(npx * 4); // RGBA8888 = 4 BPP
				convert_BGR_to_RGBA(static_cast<const uint8_t*>(data), scratch.data(), npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		}
		else
		{
			format = GL_RGB;
			if (data != nullptr) {
				std::vector<uint8_t> scratch(npx * 3); // RGB888 = 3 BPP
				convert_BGR_to_RGB(static_cast<const uint8_t*>(data), scratch.data(), npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		}
	} 
	else if (format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8_REV) 
	{ 
		const size_t npx = size_t(w) * size_t(h) * size_t(d);
		type = GL_UNSIGNED_BYTE;
		if (data != nullptr /* && !GLAD_GL_EXT_texture_format_BGRA8888*/) {
			// Conversion forced on because the check either does not work or buggy impl on Mali
			std::vector<uint8_t> scratch(npx * 4);
			convert_BGRA8888_to_RGBA8888(reinterpret_cast<const uint8_t*>(data), scratch.data(), npx);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, GL_RGBA, type, scratch.data());
			return;
		}
	}

	glad_glTexSubImage3D(target, level, xoff, yoff, zoff, w, h, d, format, type, data);
}

#ifdef glTexImage2D
#undef glTexImage2D
#endif
static inline void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{ 
	if (internalformat == GL_RGBA16F) {
		glad_glTexImage2D(target, level, internalformat, width, height, border, GL_RGBA, GL_HALF_FLOAT, data);
	} else if (internalformat == GL_RGBA8) {
		glad_glTexImage2D(target, level, internalformat, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else if (internalformat == GL_DEPTH_COMPONENT24) {
		glad_glTexImage2D(target, level, internalformat, width, height, border, format, GL_UNSIGNED_INT, data);
	} else {
		glad_glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
	}
}

#ifdef glTexImage3D
#undef glTexImage3D
#endif
static inline void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *data)
{ 
	if (internalformat == GL_DEPTH_COMPONENT24) {
		glad_glTexImage3D(target, level, internalformat, width, height, depth, border, format, GL_UNSIGNED_INT, data);
	} else if (type == GL_UNSIGNED_INT_8_8_8_8_REV) {
		if (internalformat == GL_RGBA16F) {
			glad_glTexImage3D(target, level, internalformat, width, height, depth, border, format, GL_HALF_FLOAT, data);
		} if (internalformat == GL_RGBA32F) {
			glad_glTexImage3D(target, level, internalformat, width, height, depth, border, format, GL_FLOAT, data);
		} else {
			glad_glTexImage3D(target, level, internalformat, width, height, depth, border, format, GL_UNSIGNED_BYTE, data);
		}
	} else {
		glad_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, data);
	}
}

#ifdef glReadPixels
#undef glReadPixels
#endif
static inline void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data)
{
	if (type == GL_UNSIGNED_INT_8_8_8_8_REV)
		type = GL_UNSIGNED_BYTE;
	glad_glReadPixels(x, y, width, height, format, type, data);
}

/* 
	Missing functions for ES 3.2 compatibility 
*/

// Not present on ES, emulating with glDrawBuffers
// Positional mapping: element i must be COLOR_ATTACHMENTi o NONE.
// ONLY for blits/clears. For draw calls the location of frag shader
// must be equal to the attachment index
inline void glDrawBuffer(GLenum buf)
{
	if (buf >= GL_COLOR_ATTACHMENT0 && buf <= GL_COLOR_ATTACHMENT15) {
		const unsigned idx = buf - GL_COLOR_ATTACHMENT0;
		GLenum buffers[16] = {GL_NONE}; 
		buffers[idx] = buf;
		glDrawBuffers(static_cast<GLsizei>(idx + 1), buffers);
	} else {
		// GL_NONE / GL_BACK
		glDrawBuffers(1, &buf);
	}
}

// glPolygonMode() is not supported on ES,  no wireframe, GL_FILL is default, GL_POINTS, and GL_LINES needs an alternative path
// Notes: 
// -This function is not required to run.
// -Causes wireframes to be render as solid, like WCS target monitor.
#define glPolygonMode(face, mode) ((void)0)

// try to use GLAD_GL_EXT_disjoint_timer_query
static inline void glQueryCounter(GLuint id, GLenum target) 
{
	#ifdef GL_EXT_disjoint_timer_query
		if (GLAD_GL_EXT_disjoint_timer_query)
			glQueryCounterEXT(id, GL_TIMESTAMP_EXT);
	#else
		(void)id;
	#endif
		(void)target;
}

// glGetCompressedTexImage not present on GLES and no equivalent
// Notes: 
// -This function is not required to run.
// -There is not way to emulate
#define glGetCompressedTexImage(target, level, pixels) ((void)0)

// does not exist, try to emulate for target = GL_TEXTURE_2D
// Notes: 
// -glGetTexLevelParameteriv is ES 3.1
// -This function is not required to run.
inline void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	if (target != GL_TEXTURE_2D || pixels == nullptr)
		return;

	// Requested size
	GLint width = 0, height = 0;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
	if (width <= 0 || height <= 0)
		return;

	// Texture is binded to active unit
	GLint tex = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex);
	if (tex == 0)
		return;

	// Save FBO to restore later
	GLint prevReadFbo = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFbo);

	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, static_cast<GLuint>(tex), level);

	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, width, height, format, type, pixels);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevReadFbo));
	glDeleteFramebuffers(1, &fbo);
}

// Convert the call to glMapBufferRange
inline void* glMapBuffer(GLenum target, GLenum access)
{
	GLbitfield flags = 0;

	switch (access) {
		case GL_READ_ONLY:
			flags = GL_MAP_READ_BIT;
			break;
		case GL_WRITE_ONLY:
			flags = GL_MAP_WRITE_BIT;
			break;
		case GL_READ_WRITE:
			flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
			break;
		default:
			return nullptr;
	}

	GLint bufSize = 0;
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufSize);

	if (bufSize <= 0) return nullptr;

	return glMapBufferRange(target, 0, bufSize, flags);
}

// No 64 bit version on ES, ill call the 32bit version
inline void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params)
{
	GLuint available32 = 0;
	glGetQueryObjectuiv(id, pname, &available32);
	*params = static_cast<GLuint64>(available32);
}

/*
	Additional Debug enum declarations
*/

// glEnable flags
#define GL_DEBUG_OUTPUT                     0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS         0x8242

// Sources
#define GL_DEBUG_SOURCE_API                 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM       0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER     0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY         0x8249
#define GL_DEBUG_SOURCE_APPLICATION         0x824A
#define GL_DEBUG_SOURCE_OTHER               0x824B

// Types
#define GL_DEBUG_TYPE_ERROR                 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR   0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR    0x824E
#define GL_DEBUG_TYPE_PORTABILITY           0x824F
#define GL_DEBUG_TYPE_PERFORMANCE           0x8250
#define GL_DEBUG_TYPE_OTHER                 0x8251
#define GL_DEBUG_TYPE_MARKER                0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP            0x8269
#define GL_DEBUG_TYPE_POP_GROUP             0x826A

// Severities
#define GL_DEBUG_SEVERITY_HIGH              0x9146
#define GL_DEBUG_SEVERITY_MEDIUM            0x9147
#define GL_DEBUG_SEVERITY_LOW               0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION      0x826B

// Messages
#define GL_DEBUG_LOGGED_MESSAGES_ARB        0x9145
#define GL_DEBUG_LOGGED_MESSAGES            0x9145

#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB    0x9144
#define GL_MAX_DEBUG_LOGGED_MESSAGES        0x9144

#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB     0x9143
#define GL_MAX_DEBUG_MESSAGE_LENGTH         0x9143

#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB 0x8243
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH     0x8243

// ARB aliases
#define GL_DEBUG_SOURCE_API_ARB                 GL_DEBUG_SOURCE_API
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB       GL_DEBUG_SOURCE_WINDOW_SYSTEM
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB     GL_DEBUG_SOURCE_SHADER_COMPILER
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB         GL_DEBUG_SOURCE_THIRD_PARTY
#define GL_DEBUG_SOURCE_APPLICATION_ARB         GL_DEBUG_SOURCE_APPLICATION
#define GL_DEBUG_SOURCE_OTHER_ARB               GL_DEBUG_SOURCE_OTHER
#define GLAD_GL_ARB_debug_output                GLAD_GL_KHR_debug

#define GL_DEBUG_TYPE_ERROR_ARB                 GL_DEBUG_TYPE_ERROR
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB   GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB    GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#define GL_DEBUG_TYPE_PORTABILITY_ARB           GL_DEBUG_TYPE_PORTABILITY
#define GL_DEBUG_TYPE_PERFORMANCE_ARB           GL_DEBUG_TYPE_PERFORMANCE
#define GL_DEBUG_TYPE_OTHER_ARB                 GL_DEBUG_TYPE_OTHER

#define GL_DEBUG_SEVERITY_HIGH_ARB              GL_DEBUG_SEVERITY_HIGH
#define GL_DEBUG_SEVERITY_MEDIUM_ARB            GL_DEBUG_SEVERITY_MEDIUM
#define GL_DEBUG_SEVERITY_LOW_ARB               GL_DEBUG_SEVERITY_LOW
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB         GL_DEBUG_OUTPUT_SYNCHRONOUS
#endif

#endif