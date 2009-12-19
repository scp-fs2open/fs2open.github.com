/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef _GROPENGLSHADER_H
#define _GROPENGLSHADER_H

#include "graphics/gropengl.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglextension.h"
#include "globalincs/pstypes.h"

#include "cmdline/cmdline.h"

// Legacy stuff
#define MAX_SHADER_UNIFORMS		15

#define SDR_FLAG_LIGHT			opengl::main_shader::flag_light
#define SDR_FLAG_FOG			opengl::main_shader::flag_fog
#define SDR_FLAG_DIFFUSE_MAP	opengl::main_shader::flag_diffuse_map
#define SDR_FLAG_GLOW_MAP		opengl::main_shader::flag_glow_map
#define SDR_FLAG_SPEC_MAP		opengl::main_shader::flag_specular_map
#define SDR_FLAG_NORMAL_MAP		opengl::main_shader::flag_normal_map
#define SDR_FLAG_HEIGHT_MAP		opengl::main_shader::flag_height_map
#define SDR_FLAG_ENV_MAP		opengl::main_shader::flag_env_map

namespace resources {
	/** Resource: Text file (stub).
	 * @todo Resedsign this class.
	 */
	class text_file /*: resource*/ {
	public:
		enum txt_file_type {
			shader_source,
			table_file
		};

		text_file(const char *, txt_file_type ftype);

		const SCP_string &read() const;

		static bool file_exist(const char *, txt_file_type ftype);
	protected:
		SCP_string data;

		static int get_cfile_type(txt_file_type);
	};
}

namespace opengl {
	/** Configuration manager (stub).
	 * @todo Redesign this class.
	 */
	class config {
	public:
		enum feature {
			bloom_int,		// 1
			env_map,
			no_fbo,
			glow,
			glsl,			// 5
			height_map,
			normal_map,
			post_process,
			specular
		};
	private:
		/** Gets pointer to variable that denotes if specified feature is enabled. */
		static int *get_switch_int(feature fconf) {
			switch (fconf) {
				case bloom_int:
					return &Cmdline_bloom_intensity;
				case env_map:
					return &Cmdline_env;
				case no_fbo:
					return &Cmdline_no_fbo;
				case glow:
					return &Cmdline_glow;
				case glsl:
					return &Use_GLSL;
				case height_map:
					return &Cmdline_height;
				case normal_map:
					return &Cmdline_normal;
				case post_process:
					return &Cmdline_postprocess;
				case specular:
					return &Cmdline_spec;
				default:
					mprintf(("ERROR: Unknown feature: 0x%x!\n", fconf));
					Int3();
					return NULL; // get rid of compiler warning
			}			
		}
	public:
		/** Checks if specified feature is enabled. */
		static bool is_enabled(feature fconf) {
			return (*get_switch_int(fconf)) != 0;
		}

		/** Disables specified feature. */
		static void disable(feature fconf) {
			*get_switch_int(fconf) = 0;
		}

		static int get_integer(feature fconf) {
			return *get_switch_int(fconf);
		}
	};

	/** Base class for shaders.
	 * This class is a wrapper for low-level OpenGL operations that are essential to
	 * correctly support shaders independently from their type and purpose. These includes:
	 * loading, compiling, linking, applying and parts of uniform variables management.
	 */
	class shader {
	private:
		int last_texture;

	public:
		/** Create shader from two source files.
		 * @param v vertex porogram GLSL source
		 * @param f fragment program GLSL source
		 */
		shader(resources::text_file *v, resources::text_file *f);
		~shader();

		/** Configures %shader. */
		virtual void configure(int) = 0;

		/** Returns list of used uniform names. */
		virtual SCP_vector<SCP_string> &get_uniform_names() = 0;

		/** Returns %shader ID. */
		int get_id() const {
			return id_flags;
		}

		/** Compiles and links %shader. 
		 * @return @c true if compilation and linking were successful
		 */
		bool compile_link();

		/** Applies shader. */
		inline void apply();

		/** Set uniform variables.
		 * @param id uniform variable id
		 * @param value integer value
		 */
		void set_uniform(unsigned int id, GLint value);

		/** Set uniform variables.
		 * @param id uniform variable id
		 * @param value float value
		 */
		void set_uniform(unsigned int id, GLfloat value);

		/** Set uniform variables.
		 * @param id uniform variable id
		 * @param matrix matrix
		 */
		void set_uniformMatrix4f(unsigned int id, GLfloat *matrix);

		/** Set %texture.
		 * @param id uniform id
		 * @param tex texture instance
		 */
		void set_texture(unsigned int id, const texture *tex) {
			vglActiveTextureARB(GL_TEXTURE0_ARB + last_texture);
			tex->bind();
			set_uniform(id, last_texture);
			last_texture++;
		}

	protected:
		enum sdr_state {
			unknown,
			loaded,
			compiled,
			applied
		};

		sdr_state state;
		int id_flags;

		SCP_string vertex_shader;
		SCP_string fragment_shader;

		SCP_vector<GLint> uniforms;

		GLhandleARB shader_program;

		/** Return location of speicifed uniform.
		 * This function returns location of specified uniform variable. Locations are
		 * cached hence there is no performance hit when calling this member function every
		 * frame.
		 * @param id uniform variable ID
		 * @return uniform variable location
		 */
		inline GLint get_uniform_location(unsigned int id);

		/** Compile object.
		 * This member function compiles given GLSL source code, performs all require error
		 * checks and returns OpenGL name of compiled object.
		 * @note This is an low-level OpenGL function wrapper.
		 * @param shader_source %shader source code
		 * @param shader_type type of %shader program (fragment or vertex)
		 * @return OpenGL name of compiled object
		 */
		GLhandleARB compile_object(const GLcharARB *shader_source, GLenum shader_type);

		/** Link fragment and vertex programs.
		 * This member function links two given compiled objects, performs all require error
		 * checks and returns OpenGL name of linked object.
		 * @note This is an low-level OpenGL function wrapper.
		 * @param vertex_object compiled vertex object
		 * @param fragment_object compiled fragment object
		 * @return OpenGL name of linked object
		 */
		GLhandleARB link_objects(GLhandleARB vertex_object, GLhandleARB shader_object);

		/** Check for compilation and linking errors.
		 * This member function is an wrapper for OpenGL functions responsible for
		 * retriving error log after shaders compilation or linking.
		 * @note This is an low-level OpenGL function wrapper.
		 * @param shader_object OpenGL shader object
		 * @return error log
		 */
		SCP_string check_info_log(GLhandleARB shader_object);
	};

	/** Main shader.
	 * This is a representation of shaders used to render all 3D objects (both opaque and
	 * translucent).
	 */
	class main_shader : public shader {
	private:
		static const char *uniform_names[];
		static SCP_vector<SCP_string> uninames;
	public:
		/** Shader features flags. */
		enum config {
			flag_light = 1,				/**< enable lightning */
			flag_fog = 2,				/**< enable fog */
			flag_diffuse_map = 4,		/**< enable diffuse maps */
			flag_glow_map = 8,			/**< enable glow maps */
			flag_specular_map = 0x10,	/**< enable specular maps */
			flag_normal_map = 0x20,		/**< enable normal maps */
			flag_height_map = 0x40,		/**< enable height maps */
			flag_env_map = 0x80			/**< enable environment maps */
		};

		enum uniform {
			n_lights,		// 1
			sBasemap,
			sGlowmap,
			sSpecmap,
			alpha_spec,		// 5
			envMatrix,
			sEnvmap,
			sNormalmap,
			sHeightmap
		};

		main_shader(resources::text_file *v, resources::text_file *f) : shader(v, f) { }
		virtual ~main_shader() { }

		/** Configure %shader
		 * Enables required features in %shader code.
		 * @param flags bitfield of main_shader::config flags
		 */
		void configure(int flags);

		/** Get uniform variables names. */
		SCP_vector<SCP_string> &get_uniform_names();
	};
	
	/** Post-processing shader.
	 * This a representation of the main post-processing shader, that renders the final scene to
	 * the back buffer. @n @n There is a different method of managing uniform variables in post-processing
	 * shaders since it is impossible to predict what each effect would require. In order to keep it flexible
	 * and elegant @e classes of uniforms were introduced where each class is realted to different effect. Then
	 * in order to access a specific uniform one has to provide its class and internal managed by effect itself
	 * uniform id (unless it is an obligatory uniform variable that doesn't belong to any class).
	 */
	class post_shader : public shader {
	private:
		static int last_one;
		static SCP_vector<int> unif_classes;

	public:
		/** Obligatory uniform variables */
		enum uniform {
			texture, /**< scene color texture */
			timer /**< current time */
		};

		/** Classes of post-processing effects. */
		enum uniform_classes {
			bloom, /**< %bloom @see bloom */
#ifdef DEPTH_OF_FIELD
			dof, /**< depth of field @see depth_of_field */
#endif
			simple /**< simple effects @see simple_effects */
		};

		post_shader(resources::text_file *v, resources::text_file *f) : shader(v, f) { }
		virtual ~post_shader() {}

		/** Configure post-processing shader.
		 * @param flags bitfield containing information on enabled effects. If bit @e n is set
		 * that means effect with index @e n is enabled. The index of effect depends on its position
		 * in @c post_processing.tbl.
		 */
		void configure(int flags);

		/** Set uniform variables.
		 * @param id uniform variable id
		 * @param value integer value
		 */
		void set_uniform(unsigned int uni, GLfloat value) {
			shader::set_uniform(uni, value);
		}

		/** Set uniform variables.
		 * @param cls effect class @see post_shader::uniform_classes
		 * @param id uniform variable id
		 * @param value integer value
		 */
		void set_uniform(unsigned int cls, unsigned int uni, GLfloat value) {
			Assert(cls < unif_classes.size());

			shader::set_uniform(unif_classes[cls] + uni, value);
		}

		/** Set %texture.
		 * @param id uniform id
		 * @param tex texture instance
		 */
		void set_texture(unsigned int x, const opengl::texture *t) {
			shader::set_texture(x, t);
		}

		/** Set %texture.
		 * @param cls effect class @see post_shader::uniform_classes
		 * @param id uniform id
		 * @param tex texture instance
		 */
		void set_texture(unsigned int cls, unsigned int uni, const opengl::texture *tex) {
			Assert(cls < unif_classes.size());

			shader::set_texture(unif_classes[cls] + uni, tex);
		}

		static SCP_vector<SCP_string> uniform_names;
		SCP_vector<SCP_string> &get_uniform_names();

		/** Return post-processing effects list */
		static SCP_vector<post_effect> &get_effects() {
			return simple_effects::get_effects();
		}

		/** Return id of appropiate shader.
		 * This function alayses current post-processing effects settings and
		 * chooses which of them should be enabled.
		 * @return information on effects to be enabled in a format comperhensible for
		 * post_shader::configure
		 */
		static inline int choose_post_shader();
	};

	/** Special shader.
	 * This class represents special shaders, which are used to perform single operations on
	 * input texture in one or many passes. They are mostly used to prepare textures to be
	 * used by post-processing shader.
	 */
	class special_shader : public shader {
	private:
		SCP_vector<GLhandleARB> passes;
		unsigned int in_pass;
		unsigned int flags_id;

		/** Internal special shader description.
		 * This structure stores all information required to correctly support special shaders.
		 * Data are hardcoded since they are mostly used by hardcoded effects and there won't be any
		 * gain in reading such information from a table.
		 */
		struct shader_data {
			/** Shader ID, should equals its index in special_shader::shaders. */
			int id;
			/** Required GLSL version, 0 for default. */
			int version;
			/** Name of vertex program source code. */
			const char *vert_name;
			/** Name of fragment program source code. */
			const char *frag_name;
			/** An array of uniform variables. */
			const char *uniforms[MAX_SHADER_UNIFORMS];
		};
		static const shader_data shaders[];
		static SCP_map<int, SCP_vector<SCP_string> > uniforms;

	public:
		special_shader(int id, resources::text_file *v, resources::text_file *f) : shader(v, f), flags_id(id) { }
		virtual ~special_shader() { }

		enum {
			blur,
			bright_pass
		};

		/** Configure %shader.
		 * Configure %shader by choosing appropriate pass.
		 * @param pass pass to apply
		 */
		void configure(int pass);
		/** Start new pass.
		 * This functions configures %shader to a given pass and applies it.
		 * @param pass pass number
		 */
		bool start_pass(int pass);

		/** Apply %shader. */
		bool apply();

		SCP_vector<SCP_string> &get_uniform_names();

		/** Get vertex fragment source file name.
		 * @param sdr %shader index
		 * @return filename
		 */
		static SCP_string get_vert_name(int sdr);

		/** Get fragment fragment source file name.
		 * @param sdr %shader index
		 * @return filename
		 */
		static SCP_string get_frag_name(int sdr);
	};

	/** Shaders Manager.
	 * This is the main part of shaders system, which manages all shaders used
	 * in FSO independently from their purpose. It is responsible for storing,
	 * choosing and applying appropriate shader.
	 */
	class shader_manager {
	public:
		/** Applies main %shader.
		 * @param flags flags that identify the %shader
		 */
		void apply_main_shader(int flags);

		/** Applies post shader.
		 * Chooses apropriate post-processing %shader and applies it.
		 */
		post_shader *apply_post_shader();

		/** Applies specific post %shader.
		 * @param flags flags that identify the %shader
		 */
		post_shader *apply_post_shader(int flags);

		/** Applies specific special %shader.
		 * @param flags special %shader index
		 */
		special_shader *apply_special_shader(int flags);

		/** Turns main %shader off. */
		void apply_fixed_pipeline();

		/** Returns current main %shader. */
		shader *get_main_shader() {
			return current_main;
		}

		/** Clears cache.
		 * @note This method should be invoked when completely different
		 * objects are going to be rendered. For example, before new mission. */
		void clear_cache() {
			main_shaders_cache.clear();
		}

		/** Get shader_manager instance. */
		static shader_manager *get() {
			Assert(instance != NULL);

			return instance;
		}

		/** Initialize shader_manager. */
		static void create() {
			if (config::is_enabled(config::glsl))
				instance = new shader_manager;
		}

		/** Destroy shaders manager objects. */
		static void destroy() {
			if (instance)
				delete instance;
			instance = NULL;
		}
	private:
		// Singleton
		shader_manager();
		~shader_manager();

		static shader_manager *instance;

		/** Loads all main shaders. */
		void load_main_shaders();

		/** Loads post shader.
		 * @param flags value that specified shader that is to be loaded
		 */
		opengl::post_shader *load_post_shader(int flag);
		
		/** Shaders cache.
		 * @note It appears that this cache can bring significant performance gain.
		 * That's why it may be good idea to divide main_shaders into classes
		 * (the one used to render preview in hud, etc) what will reduce time
		 * needed to find proper shader. However such improvement will require
		 * major changes in rendering engine.
		 */
		SCP_map<int, main_shader*> main_shaders_cache;
		SCP_map<int, main_shader*> main_shaders;

		SCP_map<int, post_shader*> post_shaders;

		SCP_map<int, special_shader*> special_shaders;

		main_shader *current_main;
		post_shader *current_post;
	};
}

#endif	// _GROPENGLSHADER_H
