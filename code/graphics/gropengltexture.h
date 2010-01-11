/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#ifndef _GROPENGLTEXTURE_H
#define _GROPENGLTEXTURE_H

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"

/* New code is currently used only by post-processing. It will replace completely the legacy code after
 * bmpman overhaul.
 */

/* Legacy code */
typedef struct tcache_slot_opengl {
	GLuint texture_id;
	GLenum texture_target;
	float u_scale, v_scale;
	int	bitmap_handle;
	int	size;
	ushort w, h;
	ubyte bpp;
	ubyte mipmap_levels;
} tcache_slot_opengl;

extern int GL_min_texture_width;
extern GLint GL_max_texture_width;
extern int GL_min_texture_height;
extern GLint GL_max_texture_height;
extern GLint GL_supported_texture_units;
extern int GL_mipmap_filter;
extern GLenum GL_texture_target;
extern GLenum GL_texture_face;
extern GLfloat GL_anisotropy;
extern bool GL_rendering_to_framebuffer;

void opengl_switch_arb(int unit, int state);
void opengl_tcache_init();
void opengl_free_texture_slot(int n);
void opengl_tcache_flush();
void opengl_tcache_shutdown();
void opengl_tcache_frame();
void opengl_set_additive_tex_env();
void opengl_set_modulate_tex_env();
void opengl_preload_init();
GLfloat opengl_get_max_anisotropy();
//void opengl_set_anisotropy(GLfloat aniso_value = GL_anisotropy);
void opengl_kill_render_target(int slot);
int opengl_make_render_target(int handle, int slot, int *w, int *h, ubyte *bpp, int *mm_lvl, int flags);
int opengl_set_render_target(int slot, int face = -1, int is_static = 0);
int opengl_export_image(int slot, int width, int height, int alpha, int num_mipmaps, ubyte *image_data = NULL);
void opengl_set_texture_target(GLenum target = GL_TEXTURE_2D);
void opengl_set_texture_face(GLenum face = GL_TEXTURE_2D);

int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int stage = 0);
int gr_opengl_preload(int bitmap_num, int is_aabitmap);
void gr_opengl_set_texture_panning(float u, float v, bool enable);
void gr_opengl_set_texture_addressing(int mode);

/* New code */
namespace opengl {
	/** OpenGL %texture representation.
	 * This class is a wrapper for OpenGL functions responsible for textures management.
	 * It covers creating, loading from memory, binding and modyfing the %texture. Loading
	 * from a file should be implemented in separate classes.
	 * @note This class duplicates functionality of legacy texture management functions in @c gropengltexture.h. They
	 * will be merged after @c bmpman overhaul.
	 * @todo Support loading from memory. @n Allow CPU to modify %texture.
	 */
	class texture {
	private:
		int width, height;
		GLuint tex;

		texture() {}
	public:
		/** %Texture format. */
		enum format {
			fmt_color, /**< default colour %texture (@c GL_RGB8) */
			fmt_depth /**< default depth %texture (@c GL_DEPTH_COMPONENT32)*/
		};

		/** Create new texture.
		 * Creates @e w x @e h @c GL_RGB8 %texture.
		 * @param w %texture width
		 * @param h %texture height
		 * @return pointer to the %texture object
		 */
		static texture *create(int w, int h);

		/** Create new texture.
		 * Creates @e w x @e h %texture.
		 * @param w %texture width
		 * @param h %texture height
		 * @param fmt %texture format
		 * @return pointer to the %texture object
		 */
		static texture *create(int w, int h, format fmt);

		~texture();

		/** Get %texture width. */
		int get_width() const {
			return width;
		}

		/** Get %texture height. */
		int get_height() const {
			return height;
		}

		/** Get OpenGL %texture name. */
		GLuint get_id() const {
			return tex;
		}

		/** Bind %texture to the appropriate target. */
		void bind() const {
			// This class currently supports only GL_TEXTURE_2D.
			glBindTexture(GL_TEXTURE_2D, tex);
		}
	};

	/**
	 * Texture pool.
	 * Object pool for temporary textures used mainly in post-processing code. Currently
	 * supports only default color textures (@c GL_RGBA8). It is based on std::multimap
	 * hence acquiring and releasing complexity is logarithmic.
	 * @note Multimap is the easiest and the most flexible way to do this. However, it is O(log n) and it is
	 * possible to make it O(1) using for example vector and base 2 logarithms of the texture size as indexes
	 * (similar approach to the O(1) power-of-2 memory allocators in FreeBSD and Solaris).
	 */
	class texture_pool {
	private:
		static SCP_multimap <std::pair<int, int>, texture*> pool;
	public:
		/** Acquire texture.
		 * Get a %texture from the pool or create a new one if nothing appropriate in the cache.
		 * @param w %texture width
		 * @param h %texture height
		 */
		static texture *acquire(int w, int h) {
			Assert(w > 0 && h > 0);
			SCP_multimap <std::pair<int, int>, texture*>::iterator itr = pool.find(std::pair<int, int>(w, h));
			if (itr != pool.end()) {
				texture *tex = (*itr).second;
				pool.erase(itr);
				return tex;
			} else {
				mprintf(("texture_pool: creating new %dx%d texture\n", w, h));
				return texture::create(w, h);
			}
		}

		/** Release texture.
		 * Put the %texture back in the pool.
		 * @param tex released texture
		 */
		static void release(texture *tex) {
			Assert(tex);
			pool.insert(std::pair<std::pair<int, int>, texture*>(std::pair<int, int>(tex->get_width(), tex->get_height()), tex));
		}
	};

	/** OpenGL render buffer representation.
	 * Instance methods of this class wrap OpenGL functions responsible for render buffers management.
	 * Class methods provide render buffer object pool based on the same algorithm as texture_pool,
	 * thus also O(log N).
	 * @note This class currently supports only depth buffers.
	 */
	class render_buffer {
	private:
		GLuint rb;
		int width, height;

		render_buffer(int w, int h);

		static SCP_multimap <std::pair<int, int>, render_buffer*> pool;
	public:
		~render_buffer();

		/** Get render buffer width. */
		int get_width() const {
			return width;
		}

		/** Get render buffer height. */
		int get_height() const {
			return height;
		}

		/** Get OpenGL render buffer name. */
		GLuint get_id() const {
			return rb;
		}

		/** Acquire render buffer.
		 * Get a render buffer from the pool or create a new one if nothing appropriate in the cache.
		 * @param w render buffer width
		 * @param h render buffer height
		 */
		static render_buffer *acquire(int w, int h) {
			Assert(w > 0 && h > 0);

			SCP_multimap <std::pair<int, int>, render_buffer*>::iterator itr = pool.find(std::pair<int, int>(w, h));
			if (itr != pool.end()) {
				render_buffer *rb = (*itr).second;
				pool.erase(itr);
				return rb;
			} else {
				mprintf(("render_buffer: creating new %dx%d render buffer\n", w, h));
				return new render_buffer(w, h);
			}
		}

		/** Release render buffer.
		 * Put the %render buffer back in the pool.
		 * @param tex released render buffer
		 */
		static void release(render_buffer *tex) {
			Assert(tex);
			pool.insert(std::pair<std::pair<int, int>, render_buffer*>(std::pair<int, int>(tex->get_width(), tex->get_height()), tex));
		}

	};

	/** OpenGL Frame Buffer Object representation.
	 * This class is a wrapper for OpenGL functions responsible for FBO management.
	 * Instance methods cover creation, applying, color and depth texture attachment.
	 * Class methods provide render buffer object pool based on the same algorithm as texture_pool,
	 * thus also O(log N).
	 * @note Each rendering target has to be applied before invoking any other non-constant member function.
	 * @note This class duplicates functionality of legacy rendering target management functions in 
	 * @c gropengltexture.h. They will be merged after @c bmpman overhaul.
	 */
	class render_target {
	private:
		render_buffer *depth_rb;
		texture *depth_t;

		bool use_rb;

		GLuint target;
		int width, height;

		static render_target *current;

		static SCP_multimap <std::pair<int, int>, render_target*> pool;

		render_target(int w, int h);
		~render_target();
	public:
		/** Apply rendering target. */
		void apply() const;

		/** Flags indicating which buffers are to be cleared. */
		enum clr_buffers {
			clr_color = GL_COLOR_BUFFER_BIT, /**< clear back buffer */
			clr_depth = GL_DEPTH_BUFFER_BIT, /**< clear depth buffer */
		};

		/** Clear buffers of the default rendering target.
		 * @param flags buffers to clear 
		 * @param black determines whether explicitly set clear color to black
		 * @see render_target::clr_buffers
		 */
		static inline void clear_default(int flags, bool black = true) {
			if (black)
				glClearColor(0, 0, 0, 0);
			glClear(flags);
		}

		/** Clear buffers.
		 * @param flags buffers to clear 
		 * @param black determines whether explicitly set clear color to black
		 * @see render_target::clr_buffers
		 */
		inline void clear(int flags, bool black = true) {
			Assert(current == this);

			if (black)
				glClearColor(0, 0, 0, 0);
			glClear(flags);
		}

		/** Apply default rendering target. */
		static void apply_default();

		/** Get depth texture. */
		texture *get_depth() const {
			return depth_t;
		}

		/** Use render buffer for depth buffer. */
		void use_depth_rb() {
			Assert(current == this);

			if (use_rb)
				return;

			use_rb = true;

			if (!depth_rb)
				depth_rb = render_buffer::acquire(width, height);
			vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb->get_id());
		}

		/** Set depth texture. */
		void set_depth(texture *tex) {
			Assert(current == this);

			Assert(tex);
			Assert(tex->get_width() == get_width());
			Assert(tex->get_height() == get_height());

			use_rb = false;

			depth_t = tex;
			vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_t->get_id(), 0);
		}

		/** Get rendering target width. */
		int get_width() const {
			return width;
		}

		/** Get rendering target height. */
		int get_height() const {
			return height;
		}

		/** Attach texture as the specified attachment.
		 * @param x attachment number
		 * @param tex texture to attach
		 */
		void attach_texture(unsigned int x, const texture *tex);

		/** Attach texture as the default attachment.
		 * @param tex texture to attach
		 */
		void attach_texture(const texture *tex) {
			attach_texture(0, tex);
		}

		/** Check rendering target status.
		 * @return @c true if no error
		 */
		bool check_status();

		/** Acquire a rendering target.
		 * Get a rendering target from the pool or create a new one if nothing appropriate in the cache.
		 * @param w rendering target width
		 * @param h rendering target height
		 */
		static render_target *acquire(int w, int h) {
			Assert(w > 0 && h > 0);

			SCP_multimap <std::pair<int, int>, render_target*>::iterator itr = pool.find(std::pair<int, int>(w, h));
			if (itr != pool.end()) {
				render_target *rt = (*itr).second;
				pool.erase(itr);
				return rt;
			} else {
				mprintf(("render_target: creating new %dx%d FBO\n", w, h));
				return new render_target(w, h);
			}
		}

		/** Release rendering target.
		 * Put the rendering target back in the pool.
		 * @param rt released rendering target
		 */
		static void release(render_target *rt) {
			Assert(rt);
			pool.insert(std::pair<std::pair<int, int>, render_target*>(std::pair<int, int>(rt->get_width(), rt->get_height()), rt));
		}

		/** Draw a quad. */
		static void draw_texture();
	};
}

#endif	//_GROPENGLTEXTURE_H
