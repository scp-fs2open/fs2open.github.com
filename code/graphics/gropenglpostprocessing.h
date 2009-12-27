
#ifndef _GROPENGLPOSTPROCESSING_H
#define _GROPENGLPOSTPROCESSING_H

#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"

#include "globalincs/pstypes.h"

/* Depth of field implementation should be considered as proof of concept.
 * The effects itself is fully working but there is no way to control it.
 * -Hery
 */
//#define DEPTH_OF_FIELD

/** 3D OpenGL graphics.
 * This is namespace of all wrappers for OpenGL functions that make it easier to maintain
 * FSO 3D graphics code. It includes also additionall effects implemented using OpenGL
 * functions.
 */
namespace opengl {
	/** Simple post-processing effect descriptor. */
	struct post_effect {
		/** Effect name */
		SCP_string name;
		/** Name of %shader uniform variable that stores effect intensity. */
		SCP_string uniform_name;
		/** Name of GLSL preprocessor definition that enables effect. */
		SCP_string define_name;
		/** Determines whether effect is always turned on. */
		bool always_on;
		/** Current effect intensity. */
		float intensity;
		/** Default effect intensity. */
		float default_intensity;
		/** Intensity divisor. */
		float div;
		/** Intensity adder */
		float add;

		/** Determines wheter configuration has changed.
		 * Each function that modifes any field of this structure is obliged tu set this variable to @b true.
		 */
		static bool changed;
	};

	class post_shader;
	/** Simple post-processing effects.
	 * Effect is considered as @e simple when it doesn't require any changes in FSO source code to implement it.
	 * In other words, its full implementation can be placed in @c post-v.sdr and @c post-f.sdr files.
	 */
	class simple_effects {
	private:
		static SCP_vector<post_effect> effects;

	public:
		simple_effects();
		~simple_effects();

		/** Set all uniforms that control simple effects. 
		 * @param sdr currently applied post-processing %shader
		 */
		void set_uniforms(post_shader *sdr);

		/** Read list of effects from post_processing.tbl. */
		static void read_list();
		/** Get list of effects. */
		static SCP_vector<post_effect> &get_effects();

		/** Set effect intensity.
		 * @param name effect name (as in post_effect::name)
		 * @param intensity effect intensity
		 */
		static void set_effect(SCP_string &name, int intensity);

		/** Restore default effects intensities. */
		static void restore_defaults();

		/** Get a list of uniform variables used by simple effects.
		 * The names of uniform variables are appended to the given list.
		 */
		static void get_uniforms(SCP_vector<SCP_string> &uniforms);
	};

	/** Bloom.
	 * Bloom is an effect that together with tone mapping may successfully fake HDR. The general
	 * idea is to blur the brightest parts of the scene. @n
	 * This effect requires @c bloom-f.sdr and @c brightpass-f.sdr shaders apart from @c post-f/v.sdr
	 */
	class bloom {
	public:
		/** Prepare effect.
		 * Performs all actions that are required to properly apply bloom to
		 * the current frame (i.e. calls bloom::blur on scene %texture).
		 * @param image rendered scene
		 * @returns @c true if no error
		 */
		bool begin(texture *image);

		/** Clean up per frame data. */
		void end();

		/** Set all uniforms that control bloom. 
		 * @param sdr currently applied post-processing %shader
		 */
		void set_uniforms(post_shader *sdr);

		/** Cut off dark parts and blur %texture.
		 * High-pass filter is applied to the %texture and the result is blurred by
		 * two pass Gaussian blur implementation with fixed kernel. Blurring is performed
		 * on downscaled textures to improve algorithm performance. Additionally, downscaling
		 * factor varies blur intensity since the kernel is fixed.
		 * @param tex texture to be blurred
		 * @param downscale downscaling factor
		 * @return output texture
		 */
		texture *blur(texture *tex, int downscale);

		/** Get a list of uniform variables used by %bloom.
		 * The names of uniform variables are appended to the given list.
		 */
		static void get_uniforms(SCP_vector<SCP_string> &uniforms) {
			uniforms.push_back("bloomed");
			uniforms.push_back("bloom_intensity");
		}

	private:
		/** @c blur-f.sdr and @c brightpass-f.sdr uniforms IDs */
		enum {
			b_texture, /**< input %texture */
			blur_size /**< %texture size */
		};

		/** @c post-f.sdr bloom specific uniforms IDs */
		enum {
			bloomed, /**< blurred %texture */
			intensity /**< effect intensity */
		};

		texture *blurred;
	};

#ifdef DEPTH_OF_FIELD
	/** Depth of field.
	 * Depth of field consist in blurring objects that are nearer or further
	 * than the object that is in focus. It is achieved by blending normal and
	 * blurred scene depending on information from the depth buffer.
	 */
	class depth_of_field {
	public:
		/** Prepare effect.
		 * Performs all actions that are required to properly apply depth of field to
		 * the current frame (i.e. blurs scene).
		 * @param image rendered scene
		 * @returns @c true if no error
		 */
		bool begin(texture *pp_img);

		/** Clean up per frame data. */
		void end();

		/** Set all uniforms that control depth of field. 
		 * @param sdr currently applied post-processing %shader
		 */
		void set_uniforms(post_shader *sdr);

		/** Blur %texture.
		 * The %texture is blurred by two pass Gaussian blur implementation with fixed kernel.
		 * Blurring is performed on downscaled textures to improve algorithm performance.
		 * Additionally, downscaling factor varies blur intensity since the kernel is fixed.
		 * @param tex texture to be blurred
		 * @param downscale downscaling factor
		 * @return output texture
		 */
		texture *blur(texture *, int);

		/** Get a list of uniform variables used by %bloom.
		 * The names of uniform variables are appended to the given list.
		 */
		static void get_uniforms(SCP_vector<SCP_string> &uniforms) {
			uniforms.push_back("blurred_tex");
			uniforms.push_back("depth_tex");
		}
	private:
		/** @c blur-f.sdr and @c brightpass-f.sdr uniforms IDs */
		enum {
			b_texture, /**< input %texture */
			blur_size /**< %texture size */
		};

		/** @c post-f.sdr depth of field specific uniforms IDs */
		enum {
			blurred_tex, /**< blurred %texture */
			depth_tex /**< depth %texture */
		};

		texture *blurred;
	};
#endif
	/** Post-processing manager.
	 * This is the main manager of all post-processing effects. Since there can be
	 * only one instance of this class it is implemented as a classical singleton.
	 * You shouldn't assume this class to be a facade covering all post-processing
	 * effects, in fact its only a skeleton which allows effects implementation to
	 * perform operations they need.
	 */
	class post_processing {
	private:
		post_processing();
		~post_processing();

		static post_processing *instance;

		render_target *target;
		texture *image;
		texture *depth;

		// Complex effects
		bloom *bloom_eff;
#ifdef DEPTH_OF_FIELD
		depth_of_field *dof_eff;
#endif
		simple_effects *simple_eff;

	public:
		/** Initialize post-processing. */
		static void create();
		/** Destroy post-processing objects. */
		static void remove();

		/** Get post-processing manager. */
		static inline post_processing *get();

		/** Get rendering target. */
		render_target *get_target() {
			return target;
		}

		/** Start frame. 
		 * This member function should be invoked before rendering any objects that
		 * are supposed to be taken into account while applying post-processing effects.
		 */
		void before();

		/** End frame.
		 * This member function should be invoked when all objects have been already rendered.
		 * Everything drawn afterwards will appear directly on the screen.
		 */
		void after();

		/** Turn post-processing off. */
		void disable();
	};
}

// Wrappers
void gr_opengl_set_post_effect(SCP_string &effect, int x);
void gr_opengl_set_default_post_process();
void gr_opengl_post_process_release();
void gr_opengl_post_process_init();
void gr_opengl_save_zbuffer();
void gr_opengl_post_process_before();
void gr_opengl_post_process_after();

#endif
