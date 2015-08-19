
#include "cmdline/cmdline.h"
#include "freespace2/freespace.h"
#include "globalincs/def_files.h"
#include "graphics/gropengl.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"
#include "nebula/neb.h"
#include "parse/parselo.h"
#include "ship/ship.h"


extern bool PostProcessing_override;
extern int opengl_check_framebuffer();

//Needed to track where the FXAA shaders are
size_t fxaa_shader_id;
//In case we don't find the shaders at all, this override is needed
bool fxaa_unavailable = false;
int Fxaa_preset_last_frame;
bool zbuffer_saved = false;

// lightshaft parameters
bool ls_on = false;
bool ls_force_off = false;
float ls_density = 0.5f;
float ls_weight = 0.02f;
float ls_falloff = 1.0f;
float ls_intensity = 0.5f;
float ls_cpintensity = 0.5f * 50 * 0.02f;
int ls_samplenum = 50;

typedef struct post_effect_t {
	SCP_string name;
	SCP_string uniform_name;
	SCP_string define_name;

	float intensity;
	float default_intensity;
	float div;
	float add;

	bool always_on;

	post_effect_t() :
		intensity(0.0f), default_intensity(0.0f), div(1.0f), add(0.0f),
		always_on(false)
	{
	}
} post_effect_t;

SCP_vector<post_effect_t> Post_effects;

static int Post_initialized = 0;

bool Post_in_frame = false;

static int Post_active_shader_index = -1;

static GLuint Post_framebuffer_id[2] = { 0 };
static GLuint Post_bloom_texture_id[3] = { 0 };
static GLuint Post_shadow_framebuffer_id = 0;
static GLuint Post_shadow_texture_id = 0;
static GLuint Post_shadow_depth_texture_id = 0;

static int Post_texture_width = 0;
static int Post_texture_height = 0;

static bool opengl_post_pass_bloom()
{
	if (Cmdline_bloom_intensity <= 0) {
		return false;
	}

	// we need the scissor test disabled
	GLboolean scissor_test = GL_state.ScissorTest(GL_FALSE);

	// ------  begin bright pass ------

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Post_framebuffer_id[0]);

	// width and height are 1/2 for the bright pass
	int width = Post_texture_width >> 1;
	int height = Post_texture_height >> 1;

	glViewport(0, 0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BRIGHTPASS, 0) );

	GL_state.Uniform.setUniformi( "tex", 0 );

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	GL_state.Texture.Disable();

	// ------ end bright pass ------


	// ------ begin blur pass ------

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);

	// drop width and height once more for the blur passes
	width >>= 1;
	height >>= 1;

	glViewport(0, 0, width, height);

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Post_framebuffer_id[1]);

	for (int pass = 0; pass < 2; pass++) {
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Post_bloom_texture_id[1+pass], 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if ( pass ) {
			opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_VERTICAL) );
		} else {
			opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_HORIZONTAL) );
		}

		GL_state.Uniform.setUniformi( "tex", 0 );
		GL_state.Uniform.setUniformf( "bsize", (pass) ? (float)width : (float)height );

		GL_state.Texture.Enable(Post_bloom_texture_id[pass]);

		opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	GL_state.Texture.Disable();

	// ------ end blur pass --------

	// reset viewport, scissor test and exit
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);
	GL_state.ScissorTest(scissor_test);

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, opengl_get_rtt_framebuffer());

	return true;
}

void gr_opengl_post_process_begin()
{
	if ( !Post_initialized ) {
		return;
	}

	if (Post_in_frame) {
		return;
	}

	if (PostProcessing_override) {
		return;
	}

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Post_framebuffer_id[0]);

//	vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, Post_renderbuffer_id);

//	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Post_screen_texture_id, 0);

//	Assert( !opengl_check_framebuffer() );

	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Post_in_frame = true;
}

void recompile_fxaa_shader() {

	mprintf(("Recompiling FXAA shader with preset %d\n", Cmdline_fxaa_preset));

	// start recompile by grabbing deleting the current shader we have, assuming it's already created
	opengl_delete_shader( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA, 0) );

	// then recreate it again. shader loading code will be updated with the new FXAA presets
	gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA, 0);

	Fxaa_preset_last_frame = Cmdline_fxaa_preset;
}

void opengl_post_pass_fxaa() {

	//If the preset changed, recompile the shader
	if (Fxaa_preset_last_frame != Cmdline_fxaa_preset) {
		recompile_fxaa_shader();
	}

	// We only want to draw to ATTACHMENT0
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	// Do a prepass to convert the main shaders' RGBA output into RGBL
	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA_PREPASS, 0) );

	// basic/default uniforms
	GL_state.Uniform.setUniformi( "tex", 0 );

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Scene_luminance_texture, 0);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	GL_state.Texture.Disable();

	// set and configure post shader ..
	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA, 0) );

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Scene_color_texture, 0);

	// basic/default uniforms
	GL_state.Uniform.setUniformi( "tex0", 0 );
	GL_state.Uniform.setUniformf( "rt_w", static_cast<float>(Post_texture_width));
	GL_state.Uniform.setUniformf( "rt_h", static_cast<float>(Post_texture_height));

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_luminance_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	GL_state.Texture.Disable();

	opengl_shader_set_current();
}

extern GLuint Shadow_map_depth_texture;
extern GLuint Scene_depth_texture;
extern GLuint Cockpit_depth_texture;
extern GLuint Scene_position_texture;
extern GLuint Scene_normal_texture;
extern GLuint Scene_specular_texture;
extern bool stars_sun_has_glare(int index);
extern float Sun_spot;
void gr_opengl_post_process_end()
{
	// state switch just the once (for bloom pass and final render-to-screen)
	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean lighting = GL_state.Lighting(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	GL_state.Texture.SetShaderMode(GL_TRUE);

	// Do FXAA
	if (Cmdline_fxaa && !fxaa_unavailable && !GL_rendering_to_texture) {
		opengl_post_pass_fxaa();
	}
	
	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, 0) );
	float x,y;
	// should we even be here?
	if (!Game_subspace_effect && ls_on && !ls_force_off)
	{	
		int n_lights = light_get_global_count();
		
		for(int idx=0; idx<n_lights; idx++)
		{
			vec3d light_dir;
			vec3d local_light_dir;
			light_get_global_dir(&light_dir, idx);
			vm_vec_rotate(&local_light_dir, &light_dir, &Eye_matrix);
			if (!stars_sun_has_glare(idx))
				continue;
			float dot;
			if((dot=vm_vec_dot( &light_dir, &Eye_matrix.vec.fvec )) > 0.7f)
			{
				
				x = asin(vm_vec_dot( &light_dir, &Eye_matrix.vec.rvec ))/PI*1.5f+0.5f; //cant get the coordinates right but this works for the limited glare fov
				y = asin(vm_vec_dot( &light_dir, &Eye_matrix.vec.uvec ))/PI*1.5f*gr_screen.clip_aspect+0.5f;
				GL_state.Uniform.setUniform2f( "sun_pos", x, y);
				GL_state.Uniform.setUniformi( "scene", 0);
				GL_state.Uniform.setUniformi( "cockpit", 1);
				GL_state.Uniform.setUniformf( "density", ls_density);
				GL_state.Uniform.setUniformf( "falloff", ls_falloff);
				GL_state.Uniform.setUniformf( "weight", ls_weight);
				GL_state.Uniform.setUniformf( "intensity", Sun_spot * ls_intensity);
				GL_state.Uniform.setUniformf( "cp_intensity", Sun_spot * ls_cpintensity);

				GL_state.Texture.SetActiveUnit(0);
				GL_state.Texture.SetTarget(GL_TEXTURE_2D);
				GL_state.Texture.Enable(Scene_depth_texture);
				GL_state.Texture.SetActiveUnit(1);
				GL_state.Texture.SetTarget(GL_TEXTURE_2D);
				GL_state.Texture.Enable(Cockpit_depth_texture);
				GL_state.Color(255, 255, 255, 255);
				GL_state.Blend(GL_TRUE);
				GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);
				
				opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

				GL_state.Blend(GL_FALSE);
				break;
			}
		}
	}
	if(zbuffer_saved)
	{
		zbuffer_saved = false;
		gr_zbuffer_set(GR_ZBUFF_FULL);
		glClear(GL_DEPTH_BUFFER_BIT);
		gr_zbuffer_set(GR_ZBUFF_NONE);
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, Scene_depth_texture, 0);
	}
	
	// Bind the correct framebuffer. opengl_get_rtt_framebuffer returns 0 if not doing RTT
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, opengl_get_rtt_framebuffer());	

	// do bloom, hopefully ;)
	bool bloomed = opengl_post_pass_bloom();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GL_state.Color(255, 255, 255, 255);

	// set and configure post shader ...
	int flags = 0;
	for ( int i = 0; i < (int)Post_effects.size(); i++) {
		if (Post_effects[i].always_on) {
			flags |= (1 << i);
		}
	}

	int post_sdr_handle = Post_active_shader_index;

	if ( post_sdr_handle < 0 ) {
		// no active shader index? use the always on shader.
		post_sdr_handle = gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_MAIN, flags);
	}

	opengl_shader_set_current(post_sdr_handle);

	// basic/default uniforms
	GL_state.Uniform.setUniformi( "tex", 0 );
	GL_state.Uniform.setUniformi( "depth_tex", 2);
	GL_state.Uniform.setUniformf( "timer", static_cast<float>(timer_get_milliseconds() % 100 + 1) );

	for (size_t idx = 0; idx < Post_effects.size(); idx++) {
		if ( GL_shader[post_sdr_handle].flags & (1<<idx) ) {
			const char *name = Post_effects[idx].uniform_name.c_str();
			float value = Post_effects[idx].intensity;

			GL_state.Uniform.setUniformf( name, value);
		}
	}

	// bloom uniforms, but only if we did the bloom
	if (bloomed) {
		float intensity = MIN((float)Cmdline_bloom_intensity, 200.0f) * 0.01f;

		if (Neb2_render_mode != NEB2_RENDER_NONE) {
			// we need less intensity for full neb missions, so cut it by 30%
			intensity /= 3.0f;
		}

		GL_state.Uniform.setUniformf( "bloom_intensity", intensity );

		GL_state.Uniform.setUniformi( "bloomed", 1 );

		GL_state.Texture.SetActiveUnit(1);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Post_bloom_texture_id[2]);
	}
	else
		GL_state.Uniform.setUniformf( "bloom_intensity", 0.0f );

	// now render it to the screen ...
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	GL_state.Texture.SetActiveUnit(2);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_depth_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	//Shadow Map debug window
//#define SHADOW_DEBUG
#ifdef SHADOW_DEBUG
	opengl_shader_set_current( &GL_post_shader[7] );	
	GL_state.Texture.SetActiveUnit(0);
//	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
//	GL_state.Texture.Enable(Shadow_map_depth_texture);
	extern GLuint Shadow_map_texture;
	extern GLuint Post_shadow_texture_id;
	GL_state.Texture.Enable(Shadow_map_texture);
	vglUniform1iARB( opengl_shader_get_uniform("shadow_map"), 0);
	vglUniform1iARB( opengl_shader_get_uniform("index"), 0);
	//opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, Scene_texture_u_scale, Scene_texture_u_scale);
	//opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.5f);
	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 1.0f, 1.0f);
	vglUniform1iARB( opengl_shader_get_uniform("index"), 1);
	//opengl_draw_textured_quad(-1.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.0f, 0.75f, 0.25f);
	opengl_draw_textured_quad(-1.0f, -0.5f, 0.0f, 0.0f, -0.5f, 0.0f, 1.0f, 1.0f);
	vglUniform1iARB( opengl_shader_get_uniform("index"), 2);
	opengl_draw_textured_quad(-0.5f, -1.0f, 0.0f, 0.0f, 0.0f, -0.5f, 1.0f, 1.0f);
	vglUniform1iARB( opengl_shader_get_uniform("index"), 3);
	opengl_draw_textured_quad(-0.5f, -1.0f, 0.0f, 0.0f, 0.0f, -0.5f, 1.0f, 1.0f);
	opengl_shader_set_current();
#endif

	/*GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_depth_texture);

	
	*/
	// Done!
	/*GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_effect_texture);

	opengl_draw_textured_quad(0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_normal_texture);

	opengl_draw_textured_quad(-1.0f, -0.0f, 0.0f, 0.0f, 0.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_specular_texture);

	opengl_draw_textured_quad(0.0f, -0.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);
	*/
	GL_state.Texture.SetActiveUnit(2);
	GL_state.Texture.Disable();
	GL_state.Texture.SetActiveUnit(1);
	GL_state.Texture.Disable();
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.Disable();

	GL_state.Texture.SetShaderMode(GL_FALSE);

	// reset state
	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Lighting(lighting);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);

	opengl_shader_set_current();

	Post_in_frame = false;
}

void get_post_process_effect_names(SCP_vector<SCP_string> &names) 
{
	size_t idx;

	for (idx = 0; idx < Post_effects.size(); idx++) {
		names.push_back(Post_effects[idx].name);
	}
}

void opengl_post_init_uniforms(int flags)
{
	for (int idx = 0; idx < (int)Post_effects.size(); idx++) {
		if (flags & (1 << idx)) {
			opengl_shader_init_uniform(Post_effects[idx].uniform_name.c_str());
		}
	}
}

void gr_opengl_post_process_set_effect(const char *name, int value)
{
	if ( !Post_initialized ) {
		return;
	}

	if (name == NULL) {
		return;
	}

	size_t idx;
	int sflags = 0;

	if(!stricmp("lightshafts",name))
	{
		ls_intensity = value / 100.0f;
		ls_on = !!value;
		return;
	}

	for (idx = 0; idx < Post_effects.size(); idx++) {
		const char *eff_name = Post_effects[idx].name.c_str();

		if ( !stricmp(eff_name, name) ) {
			Post_effects[idx].intensity = (value / Post_effects[idx].div) + Post_effects[idx].add;
			break;
		}
	}

	// figure out new flags
	for (idx = 0; idx < Post_effects.size(); idx++) {
		if ( Post_effects[idx].always_on || (Post_effects[idx].intensity != Post_effects[idx].default_intensity) ) {
			sflags |= (1<<idx);
		}
	}

	Post_active_shader_index = gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_MAIN, sflags);
}

void gr_opengl_post_process_set_defaults()
{
	size_t idx;

	if ( !Post_initialized ) {
		return;
	}

	// reset all effects to their default values
	for (idx = 0; idx < Post_effects.size(); idx++) {
		Post_effects[idx].intensity = Post_effects[idx].default_intensity;
	}

	Post_active_shader_index = -1;
}

extern GLuint Cockpit_depth_texture;
void gr_opengl_post_process_save_zbuffer()
{
	if (Post_initialized)
	{
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, Cockpit_depth_texture, 0);
		gr_zbuffer_clear(TRUE);
		zbuffer_saved = true;
	}
	else
	{
		// If we can't save the z-buffer then just clear it so cockpits are still rendered correctly when
		// post-processing isn't available/enabled.
		gr_zbuffer_clear(TRUE);
	}
}


static bool opengl_post_init_table()
{
	bool warned = false;

	try
	{
		if (cf_exists_full("post_processing.tbl", CF_TYPE_TABLES))
			read_file_text("post_processing.tbl", CF_TYPE_TABLES);
		else
			read_file_text_from_array(defaults_get_file("post_processing.tbl"));

		reset_parse();


		if (optional_string("#Effects")) {
			while (!required_string_one_of(3, "$Name:", "#Ship Effects", "#End")) {
				char tbuf[NAME_LENGTH + 1] = { 0 };
				post_effect_t eff;

				required_string("$Name:");
				stuff_string(tbuf, F_NAME, NAME_LENGTH);
				eff.name = tbuf;

				required_string("$Uniform:");
				stuff_string(tbuf, F_NAME, NAME_LENGTH);
				eff.uniform_name = tbuf;

				required_string("$Define:");
				stuff_string(tbuf, F_NAME, NAME_LENGTH);
				eff.define_name = tbuf;

				required_string("$AlwaysOn:");
				stuff_boolean(&eff.always_on);

				required_string("$Default:");
				stuff_float(&eff.default_intensity);
				eff.intensity = eff.default_intensity;

				required_string("$Div:");
				stuff_float(&eff.div);

				required_string("$Add:");
				stuff_float(&eff.add);

				// Post_effects index is used for flag checks, so we can't have more than 32
				if (Post_effects.size() < 32) {
					Post_effects.push_back(eff);
				}
				else if (!warned) {
					mprintf(("WARNING: post_processing.tbl can only have a max of 32 effects! Ignoring extra...\n"));
					warned = true;
				}
			}
		}

		//Built-in per-ship effects
		ship_effect se1;
		strcpy_s(se1.name, "FS1 Ship select");
		se1.shader_effect = 0;
		se1.disables_rendering = false;
		se1.invert_timer = false;
		Ship_effects.push_back(se1);

		if (optional_string("#Ship Effects")) {
			while (!required_string_one_of(3, "$Name:", "#Light Shafts", "#End")) {
				ship_effect se;
				char tbuf[NAME_LENGTH] = { 0 };

				required_string("$Name:");
				stuff_string(tbuf, F_NAME, NAME_LENGTH);
				strcpy_s(se.name, tbuf);

				required_string("$Shader Effect:");
				stuff_int(&se.shader_effect);

				required_string("$Disables Rendering:");
				stuff_boolean(&se.disables_rendering);

				required_string("$Invert timer:");
				stuff_boolean(&se.invert_timer);

				Ship_effects.push_back(se);
			}
		}

		if (optional_string("#Light Shafts")) {
			required_string("$AlwaysOn:");
			stuff_boolean(&ls_on);
			required_string("$Density:");
			stuff_float(&ls_density);
			required_string("$Falloff:");
			stuff_float(&ls_falloff);
			required_string("$Weight:");
			stuff_float(&ls_weight);
			required_string("$Intensity:");
			stuff_float(&ls_intensity);
			required_string("$Sample Number:");
			stuff_int(&ls_samplenum);

			ls_cpintensity = ls_weight;
			for (int i = 1; i < ls_samplenum; i++)
				ls_cpintensity += ls_weight * pow(ls_falloff, i);
			ls_cpintensity *= ls_intensity;
		}

		required_string("#End");

		return true;
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("Unable to parse 'post_processing.tbl'!  Error message = %s.\n", e.what()));
		return false;
	}
}

void opengl_post_load_shader(SCP_string &sflags, shader_type shader_t, int flags)
{
	if ( shader_t == SDR_TYPE_POST_PROCESS_MAIN ) {
		for (size_t idx = 0; idx < Post_effects.size(); idx++) {
			if (flags & (1 << idx)) {
				sflags += "#define ";
				sflags += Post_effects[idx].define_name.c_str();
				sflags += "\n";
			}
		}
	} else if ( shader_t == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS ) {
		char temp[64];
		sprintf(temp, "#define SAMPLE_NUM %d\n", ls_samplenum);
		sflags += temp;
	} else if ( shader_t == SDR_TYPE_POST_PROCESS_FXAA ) {
		switch (Cmdline_fxaa_preset) {
		case 0:
			sflags += "#define FXAA_QUALITY_PRESET 10\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/6.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/12.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 1:
			sflags += "#define FXAA_QUALITY_PRESET 11\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/7.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/14.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 2:
			sflags += "#define FXAA_QUALITY_PRESET 12\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/8.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/16.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 3:
			sflags += "#define FXAA_QUALITY_PRESET 13\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/9.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/18.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 4:
			sflags += "#define FXAA_QUALITY_PRESET 14\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/10.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/20.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 5:
			sflags += "#define FXAA_QUALITY_PRESET 25\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/11.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/22.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 6:
			sflags += "#define FXAA_QUALITY_PRESET 26\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/12.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/24.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 7:
			sflags += "#define FXAA_PC 1\n";
			sflags += "#define FXAA_QUALITY_PRESET 27\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/13.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/26.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 8:
			sflags += "#define FXAA_QUALITY_PRESET 28\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/14.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/28.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 9:
			sflags += "#define FXAA_QUALITY_PRESET 39\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/15.0)\n";
			sflags += "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/32.0)\n";
			sflags += "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		}
	}
}

bool opengl_post_init_shaders()
{
	int idx;
	int flags = 0;

	// figure out which flags we need for the main post process shader
	for (idx = 0; idx < (int)Post_effects.size(); idx++) {
		if (Post_effects[idx].always_on) {
			flags |= (1 << idx);
		}
	}

	if ( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_MAIN, flags) < 0 ) {
		// only the main shader is actually required for post-processing
		return false;
	}
	
	if ( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BRIGHTPASS, 0) < 0 || 
		gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_HORIZONTAL) < 0 || 
		gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_VERTICAL) < 0 ) {
		// disable bloom if we don't have those shaders available
		Cmdline_bloom_intensity = 0;
	}

	if ( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA, 0) < 0 ||
		gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA_PREPASS, 0) < 0 ) {
		Cmdline_fxaa = false;
		fxaa_unavailable = true;
		mprintf(("Error while compiling FXAA shaders. FXAA will be unavailable.\n"));
	}

	return true;
}

// generate and test the framebuffer and textures that we are going to use
static bool opengl_post_init_framebuffer()
{
	bool rval = false;

	// clamp size, if needed
	Post_texture_width = gr_screen.max_w;
	Post_texture_height = gr_screen.max_h;

	if (Post_texture_width > GL_max_renderbuffer_size) {
		Post_texture_width = GL_max_renderbuffer_size;
	}

	if (Post_texture_height > GL_max_renderbuffer_size) {
		Post_texture_height = GL_max_renderbuffer_size;
	}

	if (Cmdline_bloom_intensity > 0) {
		// two more framebuffers, one each for the two different sized bloom textures
		vglGenFramebuffersEXT(1, &Post_framebuffer_id[0]);
		vglGenFramebuffersEXT(1, &Post_framebuffer_id[1]);

		// need to generate textures for bloom too
		glGenTextures(3, Post_bloom_texture_id);

		// half size
		int width = Post_texture_width >> 1;
		int height = Post_texture_height >> 1;

		for (int tex = 0; tex < 3; tex++) {
			GL_state.Texture.SetActiveUnit(0);
			GL_state.Texture.SetTarget(GL_TEXTURE_2D);
			GL_state.Texture.Enable(Post_bloom_texture_id[tex]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

			if (tex == 0) {
				// attach to our bright pass framebuffer and make sure it's ok
				vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Post_framebuffer_id[0]);
				vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Post_bloom_texture_id[tex], 0);

				// if not then clean up and disable bloom
				if ( opengl_check_framebuffer() ) {
					vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
					vglDeleteFramebuffersEXT(1, &Post_framebuffer_id[0]);
					vglDeleteFramebuffersEXT(1, &Post_framebuffer_id[1]);
					Post_framebuffer_id[0] = 0;
					Post_framebuffer_id[1] = 0;

					glDeleteTextures(3, Post_bloom_texture_id);
					memset(Post_bloom_texture_id, 0, sizeof(Post_bloom_texture_id));

					Cmdline_bloom_intensity = 0;

					break;
				}

				// width and height are 1/2 for the bright pass, 1/4 for the blur, so drop down
				width >>= 1;
				height >>= 1;
			} else {
				// attach to our blur pass framebuffer and make sure it's ok
				vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Post_framebuffer_id[1]);
				vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Post_bloom_texture_id[tex], 0);

				// if not then clean up and disable bloom
				if ( opengl_check_framebuffer() ) {
					vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
					vglDeleteFramebuffersEXT(1, &Post_framebuffer_id[0]);
					vglDeleteFramebuffersEXT(1, &Post_framebuffer_id[1]);
					Post_framebuffer_id[0] = 0;
					Post_framebuffer_id[1] = 0;

					glDeleteTextures(3, Post_bloom_texture_id);
					memset(Post_bloom_texture_id, 0, sizeof(Post_bloom_texture_id));

					Cmdline_bloom_intensity = 0;

					break;
				}
			}
		}
	}

	if ( Cmdline_shadow_quality ) {
		int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);

		vglGenFramebuffersEXT(1, &Post_shadow_framebuffer_id);
		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Post_shadow_framebuffer_id);

		glGenTextures(1, &Post_shadow_texture_id);
		
		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY_EXT);
//		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Post_shadow_texture_id);

		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		vglTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGBA32F_ARB, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size, size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

//		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Post_shadow_texture_id, 0);
		vglFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, Post_shadow_texture_id, 0);

		glGenTextures(1, &Post_shadow_depth_texture_id);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY_EXT);
//		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Post_shadow_depth_texture_id);

		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		vglTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

//		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, Post_shadow_depth_texture_id, 0);
		vglFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, Post_shadow_depth_texture_id, 0);
	}

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	GL_state.Texture.Disable();

	rval = true;
	
	if ( opengl_check_for_errors("post_init_framebuffer()") ) {
		rval = false;
	}

	return rval;
}

void opengl_post_process_init()
{
	Post_initialized = 0;

	//We need to read the tbl first. This is mostly for FRED's benefit, as otherwise the list of post effects for the sexp doesn't get updated.
	if ( !opengl_post_init_table() ) {
		mprintf(("  Unable to read post-processing table! Disabling post-processing...\n\n"));
		Cmdline_postprocess = 0;
		return;
	}

	if ( !Cmdline_postprocess ) {
		return;
	}

	if ( !Scene_texture_initialized ) {
		return;
	}

	if ( !Use_GLSL || Cmdline_no_fbo || !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) ) {
		Cmdline_postprocess = 0;
		return;
	}

	// for ease of use we require support for non-power-of-2 textures in one
	// form or another:
	//    - the NPOT extension
	//    - GL version 2.0+ (which should work for non-reporting ATI cards since we don't use mipmaps)
	if ( !(Is_Extension_Enabled(OGL_ARB_TEXTURE_NON_POWER_OF_TWO) || (GL_version >= 20)) ) {
		Cmdline_postprocess = 0;
		return;
	}

	if ( !opengl_post_init_shaders() ) {
		mprintf(("  Unable to initialize post-processing shaders! Disabling post-processing...\n\n"));
		Cmdline_postprocess = 0;
		return;
	}

	if ( !opengl_post_init_framebuffer() ) {
		mprintf(("  Unable to initialize post-processing framebuffer! Disabling post-processing...\n\n"));
		Cmdline_postprocess = 0;
		return;
	}

	Post_initialized = 1;
}

void opengl_post_process_shutdown()
{
	if ( !Post_initialized ) {
		return;
	}

	if (Post_bloom_texture_id[0]) {
		glDeleteTextures(3, Post_bloom_texture_id);
		memset(Post_bloom_texture_id, 0, sizeof(Post_bloom_texture_id));
	}

	if (Post_framebuffer_id[0]) {
		vglDeleteFramebuffersEXT(1, &Post_framebuffer_id[0]);
		Post_framebuffer_id[0] = 0;

		if (Post_framebuffer_id[1]) {
			vglDeleteFramebuffersEXT(1, &Post_framebuffer_id[1]);
			Post_framebuffer_id[1] = 0;
		}
	}

	Post_effects.clear();

	Post_in_frame = false;
	Post_active_shader_index = 0;

	Post_initialized = 0;
}
