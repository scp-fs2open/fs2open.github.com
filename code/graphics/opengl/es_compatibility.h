#ifdef USE_OPENGL_ES
#include <glad/glad.h>
#include <khr/khrplatform.h>

//Stubs Enums
#define GL_CLIP_DISTANCE0					    0x3000
#define GLAD_GL_ARB_draw_buffers_blend		    1
#define GL_MAP_PERSISTENT_BIT				    0x0040
#define GLAD_GL_ARB_vertex_attrib_binding	    0
#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB	    0x8E8C
#define GL_PROXY_TEXTURE_2D					    GL_TEXTURE_2D // Huge NOPE, needs an alternative way to reeplace proxy
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE	    0x86A0 // another nope, just compile fix
#define GL_FILL                                 0x1B02
#define GL_LINE                                 0x1B01
#define GL_POINT                                0x1B00
#define GLAD_GL_ARB_timer_query                 1
#define GL_TEXTURE_CUBE_MAP_SEAMLESS            0x884F
#define GL_DOUBLE                               0x140A
#define GL_TIMESTAMP							1

//Enums
#define GL_UNSIGNED_INT_8_8_8_8_REV				GL_UNSIGNED_BYTE
#define GL_BGRA									GL_BGRA_EXT // Depends on GL_EXT_texture_format_BGRA8888
#define GL_DEPTH_COMPONENT32					GL_DEPTH_COMPONENT24
#define GL_UNSIGNED_SHORT_1_5_5_5_REV			GL_UNSIGNED_SHORT_5_5_5_1 // Not great changes alpha bit position, may need to convert it
#define GL_RGB5									GL_RGB5_A1 // has extra alpha bit, not sure if it going to work
#define GL_BGR									GL_BGRA_EXT // only if BGRA8 and depends on GL_EXT_texture_format_BGRA8888
#define GLAD_GL_ARB_texture_storage				1 // Part of 3.2
#define GLAD_GL_ARB_texture_compression_bptc    GL_EXT_texture_compression_bptc
#define GL_ARB_gpu_shader5						GL_EXT_gpu_shader5
#define GLAD_GL_ARB_gpu_shader5				    GL_EXT_gpu_shader5
#define GLAD_GL_ARB_get_program_binary		    GL_OES_get_program_binary
#define GL_MULTISAMPLE						    GL_TRUE // Always enabled on ES
#define GLAD_GL_ARB_buffer_storage			    GL_EXT_buffer_storage

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

inline void glDrawBuffer(GLenum data)
{
	const GLenum buffer[] = { data };
	glDrawBuffers(1, buffer);
}

// does not exist on ES, ES uses layout(location=) directly on shader
#define glBindFragDataLocation(program, colorNumber, name) ((void)0)

// glPolygonMode() is not supported on ES,  no wireframe, GL_FILL is default, GL_POINTS, and GL_LINES needs an alternative path
#define glPolygonMode(face, mode) ((void)0)

// glGetCompressedTexImage not present on GLES, stub
static inline void glGetCompressedTexImage(GLenum /*target*/, GLint /*level*/, void* /*img*/) 
{
	
}

// does not exist, needs an alternative path, stub
inline void glGetTexImage(GLenum, GLint, GLenum, GLenum, void*) 
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

// No support for this on ES, it is used for profiling?
inline void glQueryCounter(GLuint id, GLenum target)
{

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