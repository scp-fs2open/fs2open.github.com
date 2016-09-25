
#include "cmdline/cmdline.h"
#include "freespace.h"
#include "def_files/def_files.h"
#include "gropengl.h"
#include "gropengldraw.h"
#include "gropenglpostprocessing.h"
#include "gropenglshader.h"
#include "gropenglstate.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"
#include "nebula/neb.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "tracing/tracing.h"


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

const int MAX_MIP_BLUR_LEVELS = 4;

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

static GLuint Bloom_framebuffer = 0;
static GLuint Bloom_textures[2] = { 0 };

static GLuint Post_framebuffer_id[2] = { 0 };
static GLuint Post_shadow_framebuffer_id = 0;
static GLuint Post_shadow_texture_id = 0;
static GLuint Post_shadow_depth_texture_id = 0;

static int Post_texture_width = 0;
static int Post_texture_height = 0;

void opengl_post_pass_tonemap()
{
	GR_DEBUG_SCOPE("Tonemapping");
	profile_auto trace_scope("Tonemapping");

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_TONEMAPPING, 0) );

	Current_shader->program->Uniforms.setUniformi("tex", 0);
	Current_shader->program->Uniforms.setUniformf("exposure", 4.0f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_ldr_texture, 0);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);
}

void opengl_post_pass_bloom()
{
	GR_DEBUG_SCOPE("Bloom");
	profile_auto trace_scope("Bloom");

	// we need the scissor test disabled
	GLboolean scissor_test = GL_state.ScissorTest(GL_FALSE);

	// ------  begin bright pass ------
	int width, height;
	{
		GR_DEBUG_SCOPE("Bloom bright pass");
		profile_auto draw_scope("Bloom bright pass");

		glBindFramebuffer(GL_FRAMEBUFFER, Bloom_framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Bloom_textures[0], 0);

		// width and height are 1/2 for the bright pass
		width = Post_texture_width >> 1;
		height = Post_texture_height >> 1;

		glViewport(0, 0, width, height);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BRIGHTPASS, 0));

		Current_shader->program->Uniforms.setUniformi("tex", 0);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Scene_color_texture);

		opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	// ------ end bright pass ------

	// ------ begin blur pass ------

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Bloom_textures[0]);

	glGenerateMipmap(GL_TEXTURE_2D);

	for ( int iteration = 0; iteration < 2; iteration++) {
		for (int pass = 0; pass < 2; pass++) {
			GR_DEBUG_SCOPE("Bloom iteration step");
			profile_auto draw_scope("Bloom iteration step");

			GLuint source_tex = Bloom_textures[pass];
			GLuint dest_tex = Bloom_textures[1 - pass];

			if (pass) {
				opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_HORIZONTAL));
			} else {
				opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_VERTICAL));
			}

			Current_shader->program->Uniforms.setUniformi("tex", 0);

			GL_state.Texture.SetActiveUnit(0);
			GL_state.Texture.SetTarget(GL_TEXTURE_2D);
			GL_state.Texture.Enable(source_tex);

			for (int mipmap = 0; mipmap < MAX_MIP_BLUR_LEVELS; ++mipmap) {
				int bloom_width = width >> mipmap;
				int bloom_height = height >> mipmap;

				Current_shader->program->Uniforms.setUniformf("texSize", (pass) ? 1.0f / i2fl(bloom_width) : 1.0f / i2fl(bloom_height));
				Current_shader->program->Uniforms.setUniformi("level", mipmap);
				Current_shader->program->Uniforms.setUniformf("tapSize", 1.0f);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dest_tex, mipmap);

				glViewport(0, 0, bloom_width, bloom_height);

				opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}

	// composite blur to the color texture
	{
		GR_DEBUG_SCOPE("Bloom composite step");
		profile_auto draw_scope("Bloom composite step");

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_color_texture, 0);

		opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLOOM_COMP, 0));

		Current_shader->program->Uniforms.setUniformi("tex", 0);
		Current_shader->program->Uniforms.setUniformi("levels", MAX_MIP_BLUR_LEVELS);
		Current_shader->program->Uniforms.setUniformf("bloom_intensity", Cmdline_bloom_intensity / 100.0f);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Bloom_textures[0]);

		GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);

		glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

		opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	}

	// ------ end blur pass --------

	// reset viewport, scissor test and exit
	GL_state.ScissorTest(scissor_test);
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

	glBindFramebuffer(GL_FRAMEBUFFER, Post_framebuffer_id[0]);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

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
	GR_DEBUG_SCOPE("FXAA");
	profile_auto trace_scope("FXAA");

	//If the preset changed, recompile the shader
	if (Fxaa_preset_last_frame != Cmdline_fxaa_preset) {
		recompile_fxaa_shader();
	}

	// We only want to draw to ATTACHMENT0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Do a prepass to convert the main shaders' RGBA output into RGBL
	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA_PREPASS, 0) );

	// basic/default uniforms
	Current_shader->program->Uniforms.setUniformi( "tex", 0 );

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_luminance_texture, 0);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_ldr_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	// set and configure post shader ..
	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_FXAA, 0) );

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_ldr_texture, 0);

	// basic/default uniforms
	Current_shader->program->Uniforms.setUniformi( "tex0", 0 );
	Current_shader->program->Uniforms.setUniformf( "rt_w", static_cast<float>(Post_texture_width));
	Current_shader->program->Uniforms.setUniformf( "rt_h", static_cast<float>(Post_texture_height));

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_luminance_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

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
void opengl_post_lightshafts()
{
	GR_DEBUG_SCOPE("Lightshafts");
	profile_auto trace_scope("Lightshafts");

	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, 0));

	float x, y;

	// should we even be here?
	if ( !Game_subspace_effect && ls_on && !ls_force_off ) {
		int n_lights = light_get_global_count();

		for ( int idx = 0; idx<n_lights; idx++ ) {
			vec3d light_dir;
			vec3d local_light_dir;
			light_get_global_dir(&light_dir, idx);
			vm_vec_rotate(&local_light_dir, &light_dir, &Eye_matrix);

			if ( !stars_sun_has_glare(idx) ) {
				continue;
			}

			float dot;
			if ( (dot = vm_vec_dot(&light_dir, &Eye_matrix.vec.fvec)) > 0.7f ) {

				x = asinf(vm_vec_dot(&light_dir, &Eye_matrix.vec.rvec)) / PI*1.5f + 0.5f; //cant get the coordinates right but this works for the limited glare fov
				y = asinf(vm_vec_dot(&light_dir, &Eye_matrix.vec.uvec)) / PI*1.5f*gr_screen.clip_aspect + 0.5f;
				Current_shader->program->Uniforms.setUniform2f("sun_pos", x, y);
				Current_shader->program->Uniforms.setUniformi("scene", 0);
				Current_shader->program->Uniforms.setUniformi("cockpit", 1);
				Current_shader->program->Uniforms.setUniformf("density", ls_density);
				Current_shader->program->Uniforms.setUniformf("falloff", ls_falloff);
				Current_shader->program->Uniforms.setUniformf("weight", ls_weight);
				Current_shader->program->Uniforms.setUniformf("intensity", Sun_spot * ls_intensity);
				Current_shader->program->Uniforms.setUniformf("cp_intensity", Sun_spot * ls_cpintensity);

				GL_state.Texture.SetActiveUnit(0);
				GL_state.Texture.SetTarget(GL_TEXTURE_2D);
				GL_state.Texture.Enable(Scene_depth_texture);
				GL_state.Texture.SetActiveUnit(1);
				GL_state.Texture.SetTarget(GL_TEXTURE_2D);
				GL_state.Texture.Enable(Cockpit_depth_texture);
				GL_state.Blend(GL_TRUE);
				GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);

				opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

				GL_state.Blend(GL_FALSE);
				break;
			}
		}
	}

	if ( zbuffer_saved ) {
		zbuffer_saved = false;
		gr_zbuffer_set(GR_ZBUFF_FULL);
		glClear(GL_DEPTH_BUFFER_BIT);
		gr_zbuffer_set(GR_ZBUFF_NONE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Scene_depth_texture, 0);
	}
}

void gr_opengl_post_process_end()
{
	GR_DEBUG_SCOPE("Draw scene texture");
	profile_auto trace_scope("Draw scene texture");

	// state switch just the once (for bloom pass and final render-to-screen)
	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	GL_state.Texture.SetShaderMode(GL_TRUE);
	
	// do bloom, hopefully ;)
	opengl_post_pass_bloom();

	// do tone mapping
	opengl_post_pass_tonemap();

    // Do FXAA
    if (Cmdline_fxaa && !fxaa_unavailable && !GL_rendering_to_texture) {
        opengl_post_pass_fxaa();
    }

	// render lightshafts
	opengl_post_lightshafts();

	GR_DEBUG_SCOPE("Draw post effects");
	profile_auto draw_scope("Draw post effects");

	// now write to the on-screen buffer
	glBindFramebuffer(GL_FRAMEBUFFER, opengl_get_rtt_framebuffer());

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

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
	Current_shader->program->Uniforms.setUniformi( "tex", 0 );
	Current_shader->program->Uniforms.setUniformi( "depth_tex", 1);
	Current_shader->program->Uniforms.setUniformf( "timer", static_cast<float>(timer_get_milliseconds() % 100 + 1) );

	for (size_t idx = 0; idx < Post_effects.size(); idx++) {
		if ( GL_shader[post_sdr_handle].flags & (1<<idx) ) {
			const char *name = Post_effects[idx].uniform_name.c_str();
			float value = Post_effects[idx].intensity;

			Current_shader->program->Uniforms.setUniformf( name, value);
		}
	}

	// now render it to the screen ...
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	//GL_state.Texture.Enable(Scene_color_texture);
	GL_state.Texture.Enable(Scene_ldr_texture);

	GL_state.Texture.SetActiveUnit(1);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_depth_texture);

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, Scene_texture_u_scale, Scene_texture_u_scale);

	//Shadow Map debug window
//#define SHADOW_DEBUG
#ifdef SHADOW_DEBUG
	opengl_shader_set_current( &GL_post_shader[8] );	
	GL_state.Texture.SetActiveUnit(0);
//	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
//	GL_state.Texture.Enable(Shadow_map_depth_texture);
	extern GLuint Shadow_map_texture;
	extern GLuint Post_shadow_texture_id;
	GL_state.Texture.Enable(Shadow_map_texture);
	glUniform1iARB( opengl_shader_get_uniform("shadow_map"), 0);
	glUniform1iARB( opengl_shader_get_uniform("index"), 0);
	//opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, Scene_texture_u_scale, Scene_texture_u_scale);
	//opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.5f);
	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 1.0f, 1.0f);
	glUniform1iARB( opengl_shader_get_uniform("index"), 1);
	//opengl_draw_textured_quad(-1.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.0f, 0.75f, 0.25f);
	opengl_draw_textured_quad(-1.0f, -0.5f, 0.0f, 0.0f, -0.5f, 0.0f, 1.0f, 1.0f);
	glUniform1iARB( opengl_shader_get_uniform("index"), 2);
	opengl_draw_textured_quad(-0.5f, -1.0f, 0.0f, 0.0f, 0.0f, -0.5f, 1.0f, 1.0f);
	glUniform1iARB( opengl_shader_get_uniform("index"), 3);
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

	GL_state.Texture.SetShaderMode(GL_FALSE);

	// reset state
	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
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
			Current_shader->program->Uniforms.initUniform(Post_effects[idx].uniform_name.c_str());
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Cockpit_depth_texture, 0);
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
			read_file_text_from_default(defaults_get_file("post_processing.tbl"));

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

void opengl_post_shader_header(SCP_stringstream &sflags, shader_type shader_t, int flags)
{
	if ( shader_t == SDR_TYPE_POST_PROCESS_MAIN ) {
		for (size_t idx = 0; idx < Post_effects.size(); idx++) {
			if (flags & (1 << idx)) {
				sflags << "#define ";
				sflags << Post_effects[idx].define_name.c_str();
				sflags << "\n";
			}
		}
	} else if ( shader_t == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS ) {
		char temp[64];
		sprintf(temp, "#define SAMPLE_NUM %d\n", ls_samplenum);
		sflags << temp;
	} else if ( shader_t == SDR_TYPE_POST_PROCESS_FXAA ) {
		/* GLSL version < 120 are guarded against reaching this code
		   path via testing is_minimum_GLSL_version().
		   Accordingly do not test for them again here. */
		if (GLSL_version == 120) {
			sflags << "#define FXAA_GLSL_120 1\n";
			sflags << "#define FXAA_GLSL_130 0\n";
		}
		if (GLSL_version > 120) {
			sflags << "#define FXAA_GLSL_120 0\n";
			sflags << "#define FXAA_GLSL_130 1\n";
		}

		switch (Cmdline_fxaa_preset) {
		case 0:
			sflags << "#define FXAA_QUALITY_PRESET 10\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/6.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/12.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 1:
			sflags << "#define FXAA_QUALITY_PRESET 11\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/7.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/14.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 2:
			sflags << "#define FXAA_QUALITY_PRESET 12\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/8.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/16.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 3:
			sflags << "#define FXAA_QUALITY_PRESET 13\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/9.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/18.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 4:
			sflags << "#define FXAA_QUALITY_PRESET 14\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/10.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/20.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 5:
			sflags << "#define FXAA_QUALITY_PRESET 25\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/11.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/22.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 6:
			sflags << "#define FXAA_QUALITY_PRESET 26\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/12.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/24.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 7:
			sflags << "#define FXAA_PC 1\n";
			sflags << "#define FXAA_QUALITY_PRESET 27\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/13.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/26.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 8:
			sflags << "#define FXAA_QUALITY_PRESET 28\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/14.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/28.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
			break;
		case 9:
			sflags << "#define FXAA_QUALITY_PRESET 39\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD (1.0/15.0)\n";
			sflags << "#define FXAA_QUALITY_EDGE_THRESHOLD_MIN (1.0/32.0)\n";
			sflags << "#define FXAA_QUALITY_SUBPIX 0.33\n";
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
		gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLUR, SDR_FLAG_BLUR_VERTICAL) < 0 ||
		gr_opengl_maybe_create_shader(SDR_TYPE_POST_PROCESS_BLOOM_COMP, 0) < 0) {
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

void opengl_setup_bloom_textures()
{
	// two more framebuffers, one each for the two different sized bloom textures
	glGenFramebuffers(1, &Bloom_framebuffer);

	// need to generate textures for bloom too
	glGenTextures(2, Bloom_textures);

	// half size
	int width = Post_texture_width >> 1;
	int height = Post_texture_height >> 1;

	for (int tex = 0; tex < 2; tex++) {
		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Bloom_textures[tex]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MAX_MIP_BLUR_LEVELS-1);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	opengl_setup_bloom_textures();

	if ( Cmdline_shadow_quality ) {
		int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);

		glGenFramebuffers(1, &Post_shadow_framebuffer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, Post_shadow_framebuffer_id);

		glGenTextures(1, &Post_shadow_texture_id);
		
		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
//		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Post_shadow_texture_id);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size, size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Post_shadow_texture_id, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Post_shadow_texture_id, 0);

		glGenTextures(1, &Post_shadow_depth_texture_id);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
//		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Post_shadow_depth_texture_id);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Post_shadow_depth_texture_id, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Post_shadow_depth_texture_id, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	rval = true;
	
	if ( opengl_check_for_errors("post_init_framebuffer()") ) {
		rval = false;
	}

	return rval;
}



void opengl_post_process_shutdown_bloom()
{
	if ( Bloom_textures[0] ) {
		glDeleteTextures(1, &Bloom_textures[0]);
		Bloom_textures[0] = 0;
	}

	if ( Bloom_textures[1] ) {
		glDeleteTextures(1, &Bloom_textures[1]);
		Bloom_textures[1] = 0;
	}

	if ( Bloom_framebuffer > 0 ) {
		glDeleteFramebuffers(1, &Bloom_framebuffer);
		Bloom_framebuffer = 0;
	}
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

	if ( Cmdline_no_fbo ) {
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

	if (Post_framebuffer_id[0]) {
		glDeleteFramebuffers(1, &Post_framebuffer_id[0]);
		Post_framebuffer_id[0] = 0;

		if (Post_framebuffer_id[1]) {
			glDeleteFramebuffers(1, &Post_framebuffer_id[1]);
			Post_framebuffer_id[1] = 0;
		}
	}

	Post_effects.clear();

	opengl_post_process_shutdown_bloom();

	Post_in_frame = false;
	Post_active_shader_index = 0;

	Post_initialized = 0;
}
