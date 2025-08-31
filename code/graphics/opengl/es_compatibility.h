#ifndef ES_COMPATIBILITY_H
#define ES_COMPATIBILITY_H
#ifdef USE_OPENGL_ES
#include <vector>
#include <glad/glad.h>
#include <KHR/khrplatform.h>
//TODO: 
//-Message:GL_INVALID_OPERATION in glBlitFramebuffer(source and destination color buffer cannot be the same), this may not be easy to handle
//-IMGUI crashes on windows + opengl es
//-limited color attachments to 4 in not a good way
//-GL_PROXY_TEXTURE_2D and GL_TEXTURE_COMPRESSED_IMAGE_SIZE

//Stubs Enums, this does not exist on GLES and need to be handled
#define GLAD_GL_ARB_draw_buffers_blend		    1
#define GL_MAP_PERSISTENT_BIT				    0x0040
#define GLAD_GL_ARB_vertex_attrib_binding	    0
#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB	    0x8E8C
#define GL_PROXY_TEXTURE_2D					    GL_TEXTURE_2D // needs an alternative way to reeplace proxy
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE	    0x86A0 // just compile fix
#define GL_FILL                                 0x1B02
#define GL_LINE                                 0x1B01
#define GL_POINT                                0x1B00
#define GLAD_GL_ARB_timer_query                 0
#define GL_DOUBLE                               0x140A
#define GL_TIMESTAMP							1
#define GL_UNSIGNED_SHORT_1_5_5_5_REV			0x1CCC
#define GL_UNSIGNED_INT_8_8_8_8_REV				0xBAAA
#define GL_BGR									0x2CCA

//Enums
#define GL_CLIP_DISTANCE0						GL_CLIP_DISTANCE0_EXT // GL_EXT_clip_cull_distance
#define GL_BGRA									GL_BGRA_EXT // Depends on GL_EXT_texture_format_BGRA8888
#define GL_DEPTH_COMPONENT32					GL_DEPTH_COMPONENT24
#define GL_RGB5									GL_RGB5_A1 // has extra alpha bit, not sure if it going to work
#define GLAD_GL_ARB_texture_storage				1 // Part of 3.2
#define GLAD_GL_ARB_texture_compression_bptc	GLAD_GL_EXT_texture_compression_bptc
#define GL_ARB_gpu_shader5						GL_EXT_gpu_shader5
#define GLAD_GL_ARB_gpu_shader5					GLAD_GL_EXT_gpu_shader5
#define GLAD_GL_ARB_get_program_binary			GLAD_GL_OES_get_program_binary
#define GLAD_GL_ARB_buffer_storage				GLAD_GL_EXT_buffer_storage

//Functions
#define glBlendFunciARB					glBlendFunci
#define glTexImage2DMultisample			glFramebufferTexture2DMultisampleEXT // Depends on GL_EXT_multisampled_render_to_texture
#define glBufferStorage					glBufferStorageEXT
#define glGetDebugMessageLogARB         glGetDebugMessageLog
#define gladLoadGLLoader                gladLoadGLES2Loader
#define glDebugMessageCallbackARB       glDebugMessageCallback
#define glDebugMessageControlARB        glDebugMessageControl
#define glGetDebugMessageLogARB         glGetDebugMessageLog
#define glDepthRange                    glDepthRangef

// Bring internalFormat info for glTexSubImage3D calls
static inline GLint query_internalformat_3d(GLenum target, GLint level)
{
	GLint ifmt = 0;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &ifmt);
	return ifmt;
}

// BGRA 1_5_5_5_REV -> RGBA 5_5_5_1
static inline void convert_BGRA1555_REV_to_RGBA5551(const uint16_t* src, uint16_t* dst, size_t npx)
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
static inline void convert_BGRA1555_REV_to_RGBA8888(const uint16_t* src, uint8_t* dstRGBA8, size_t npx)
{
	for (size_t i = 0; i < npx; ++i) {
		uint16_t s = src[i];
		uint8_t A = (s >> 15) ? 255 : 0;
		uint8_t R5 = (s >> 10) & 0x1F;
		uint8_t G5 = (s >> 5) & 0x1F;
		uint8_t B5 = s & 0x1F;
		// expand 5 to 8 bits: (v<<3)|(v>>2)
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
static inline void convert_BGR_to_RGB(uint8_t* p, size_t npx)
{
	for (size_t i = 0; i < npx; ++i) {
		uint8_t* px = p + 3 * i;
		uint8_t tmp = px[0]; // <- B
		px[0] = px[2];		 // R -> B
		px[2] = tmp;		 // B -> R
	}
}

// BGRA 8888 -> RGBA 8888
static inline void convert_BGRA8888_to_RGBA8888(const uint8_t* src, uint8_t* dst, size_t npx)
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
static inline void convert_BGR_to_RGBA(const uint8_t* src, uint8_t* dst, size_t npx)
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


static inline void glTexSubImage3D(GLenum target, GLint level, GLint xoff, GLint yoff, GLint zoff, GLsizei w, GLsizei h, GLsizei d, GLenum format, GLenum type, const void* data)
{
	if (format == GL_BGRA && type == GL_UNSIGNED_SHORT_1_5_5_5_REV) {
		GLint internalFormat = query_internalformat_3d(target,level);
		const size_t npx = size_t(w) * size_t(h);
		if (internalFormat != GL_RGBA8)
		{
			format = GL_RGBA;
			type = GL_UNSIGNED_SHORT_5_5_5_1;
			if (data != nullptr) {
				std::vector<uint8_t> scratch(npx * 2);
				convert_BGRA1555_REV_to_RGBA5551(reinterpret_cast<const uint16_t*>(data),reinterpret_cast<uint16_t*>(scratch.data()),npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexSubImage3D_glad(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		} else {
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
			if (data != nullptr) {
				std::vector<uint8_t> scratch(npx * 4);
				convert_BGRA1555_REV_to_RGBA8888(reinterpret_cast<const uint16_t*>(data), scratch.data(), npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexSubImage3D_glad(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			} 
			
		}
	}

	if (format == GL_BGR && type == GL_UNSIGNED_BYTE) {
		GLint internalFormat = query_internalformat_3d(target, level);
		if (internalFormat != GL_RGBA8)
		{
			format = GL_RGB;
			if (data != nullptr)
			{
				std::vector<uint8_t> scratch(w * h * 3);
				memcpy(scratch.data(), data, scratch.size());
				convert_BGR_to_RGB(scratch.data(), (size_t)w * (size_t)h);
				glTexSubImage3D_glad(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		}
		else
		{
			format = GL_RGBA;
			if (data != nullptr) 
			{
				const size_t npx = size_t(w) * size_t(h) * size_t(d);
				std::vector<uint8_t> scratch(npx * 4);
				convert_BGR_to_RGBA(static_cast<const uint8_t*>(data), scratch.data(), npx);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexSubImage3D_glad(target, level, xoff, yoff, zoff, w, h, d, format, type, scratch.data());
				return;
			}
		}
	}

	if (format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8_REV) { 
		type = GL_UNSIGNED_BYTE;
		if (data != nullptr /* && !GLAD_GL_EXT_texture_format_BGRA8888 //this check dosent work*/) {
			// do conversion
			const size_t npx = size_t(w) * size_t(h);
			std::vector<uint8_t> scratch(npx * 4);
			convert_BGRA8888_to_RGBA8888(reinterpret_cast<const uint8_t*>(data), scratch.data(), npx);
			glTexSubImage3D_glad(target, level, xoff, yoff, zoff, w, h, d, GL_RGBA, type, scratch.data());
		}
	}

	glTexSubImage3D_glad(target, level, xoff, yoff, zoff, w, h, d, format, type, data);
}

static inline void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{ 
	if (internalformat == GL_RGBA16F) {
		glTexImage2D_glad(target, level, internalformat, width, height, border, GL_RGBA, GL_HALF_FLOAT, data);
	} else if (internalformat == GL_RGBA8) {
		glTexImage2D_glad(target, level, internalformat, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else if (internalformat == GL_DEPTH_COMPONENT24) {
		glTexImage2D_glad(target, level, internalformat, width, height, border, format, GL_UNSIGNED_INT, data);
	} else {
		glTexImage2D_glad(target, level, internalformat, width, height, border, format, type, data);
	}
}

inline void glDrawBuffer(GLenum data)
{
	if (data >= GL_COLOR_ATTACHMENT4)
		return; //limit color attachments to 4
	const GLenum buffer[] = { data };
	glDrawBuffers(1, buffer);
}

// does not exist on ES, ES uses layout(location=) directly on shader
#define glBindFragDataLocation(program, colorNumber, name) ((void)0)

// glPolygonMode() is not supported on ES,  no wireframe, GL_FILL is default, GL_POINTS, and GL_LINES needs an alternative path
#define glPolygonMode(face, mode) ((void)0)

// No support for this on ES, it is used for profiling?
static inline void glQueryCounter(GLuint /*id*/, GLenum /*target*/) 
{
}

// glGetCompressedTexImage not present on GLES and no equivalent
#define glGetCompressedTexImage(target, level, pixels) ((void)0)

// does not exist, needs an alternative path, stub
inline void glGetTexImage(GLenum /*target*/, GLint /*level*/, GLenum /*format*/, GLenum /*type*/, void* /*pixels*/)
{
}

// Convert the call to glMapBufferRange, Untested
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

// Debug Enums
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