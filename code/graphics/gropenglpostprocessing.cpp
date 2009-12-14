#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglshader.h"

#include "io/timer.h"
#include "nebula/neb.h"
#include "parse/parselo.h"

using namespace opengl;

bool post_effect::changed = true;

texture *bloom::blur(texture *image, int size) {
	Assert(image);
	Assert(size != 0);

	// Acquire render target
	opengl::render_target *target;

	target = render_target::acquire(gr_screen.max_w / size, gr_screen.max_h / size);

	Assert(target);

	// Acquire two textures
	texture *texture0 = texture_pool::acquire(target->get_width(), target->get_height());
	texture *texture1 = texture_pool::acquire(target->get_width(), target->get_height());

	Assert(texture0);
	Assert(texture1);

	target->apply();
	target->attach_texture(texture1);
	target->clear(render_target::clr_color);
	// Apply bright pass filter
	special_shader *bright_sdr = shader_manager::get()->apply_special_shader(special_shader::bright_pass);

	if (!bright_sdr)
		return NULL;
	
	bright_sdr->set_texture(b_texture, image);

	render_target::draw_texture();

	// Blur
	special_shader *blur_sdr = shader_manager::get()->apply_special_shader(special_shader::blur);
	if (!blur_sdr)
		return NULL;

	for (int pass = 0; pass < 2; pass++) {
		target->attach_texture(!pass ? texture0 : texture1);
		target->clear(render_target::clr_color);
		blur_sdr->start_pass(pass);
		blur_sdr->set_texture(b_texture, !pass ? texture1 : texture0);
		if (pass % 2 == 0)
			blur_sdr->set_uniform(blur_size, target->get_width() * 1.0f);
		else
			blur_sdr->set_uniform(blur_size, target->get_height() * 1.0f);

		render_target::draw_texture();
	}

	render_target::release(target);
	texture_pool::release(texture0);

	return texture1;
}

bool bloom::begin(texture *pp_img) {
	Assert(pp_img);

	if (config::get_integer(config::bloom_int)) {
		blurred = blur(pp_img, 4);

		return blurred != NULL;
	} else
		return true;
}

void bloom::end() {
	if (config::get_integer(config::bloom_int)) {
		Assert(blurred);
		texture_pool::release(blurred);
	}
}

void bloom::set_uniforms(post_shader *sdr) {
	Assert(sdr);

	if (config::get_integer(config::bloom_int)) {
		Assert(blurred);
		sdr->set_texture(post_shader::bloom, bloomed, blurred);
	}

	// We need different settings in nebula missions. This hack will do this, until
	// configuration manager is fully implemented.
	if (Neb2_render_mode == NEB2_RENDER_NONE)
		sdr->set_uniform(post_shader::bloom, intensity, min(config::get_integer(config::bloom_int), 200) / 100.0f);
	else
		sdr->set_uniform(post_shader::bloom, intensity, min(config::get_integer(config::bloom_int), 200) / 3 / 100.0f);
}

#ifdef DEPTH_OF_FIELD
bool depth_of_field::begin(texture *pp_img) {
	Assert(pp_img);

	render_target *target;

	target = render_target::acquire(gr_screen.max_w / 4, gr_screen.max_h / 4);

	Assert(target);

	target->apply();

	texture *texture0 = texture_pool::acquire(target->get_width(), target->get_height());
	texture *texture1 = texture_pool::acquire(target->get_width(), target->get_height());

	Assert(texture0);
	Assert(texture1);

	special_shader *blur_sdr = shader_manager::get()->apply_special_shader(special_shader::blur);

	if (!blur_sdr)
		return false;

	for (int pass = 0; pass < 2; pass++) {
		target->attach_texture(!pass ? texture0 : texture1);
		target->clear(render_target::clr_color);
		blur_sdr->start_pass(pass);
		blur_sdr->set_texture(b_texture, !pass ? pp_img : texture0);
		if (pass % 2 == 0)
			blur_sdr->set_uniform(blur_size, target->get_width() * 1.0f);
		else
			blur_sdr->set_uniform(blur_size, target->get_height() * 1.0f);

		render_target::draw_texture();
	}

	render_target::release(target);
	texture_pool::release(texture0);

	blurred = texture1;

	return true;
}

void depth_of_field::set_uniforms(post_shader *sdr) {
	Assert(sdr);

	sdr->set_texture(post_shader::dof, blurred_tex, blurred);
	sdr->set_texture(post_shader::dof, depth_tex, post_processing::get()->get_target()->get_depth());
}

void depth_of_field::end() {
	Assert(blurred);
	texture_pool::release(blurred);
}
#endif

SCP_vector<opengl::post_effect> opengl::simple_effects::effects; 

simple_effects::simple_effects() {
	read_list();
}

simple_effects::~simple_effects() {
	effects.clear();
}

void simple_effects::read_list() {
	if (!resources::text_file::file_exist("post_processing.tbl", resources::text_file::table_file))
		return;

	// legacy way of parsing tbl file

	read_file_text("post_processing.tbl", CF_TYPE_TABLES);
	reset_parse();

	required_string("#Effects");
	while (required_string_either("#End", "$Name:")) {
		post_effect eff;
		char buf[NAME_LENGTH];

		required_string("$Name:");
		stuff_string(buf, F_NAME, NAME_LENGTH);
		eff.name = vm_strdup(buf);

		required_string("$Uniform:");
		stuff_string(buf, F_NAME, NAME_LENGTH);
		eff.uniform_name =  vm_strdup(buf);

		required_string("$Define:");
		stuff_string(buf, F_NAME, NAME_LENGTH);
		eff.define_name =  vm_strdup(buf);

		required_string("$AlwaysOn:");
		stuff_boolean(&eff.always_on);

		required_string("$Default:");
		stuff_float(&eff.default_intensity);

		required_string("$Div:");
		stuff_float(&eff.div);

		required_string("$Add:");
		stuff_float(&eff.add);

		effects.push_back(eff);
	}

	required_string("#End");
}

void simple_effects::get_uniforms(SCP_vector<SCP_string> &uniforms) {
	for (unsigned int i = 0; i < effects.size(); i++)
		uniforms.push_back(SCP_string(effects[i].uniform_name.c_str()));
}

SCP_vector<post_effect> &simple_effects::get_effects() {
		if (effects.empty())
			read_list();

		return effects;
}

void simple_effects::set_effect(SCP_string &name, int intensity) {
	post_effect::changed = true;

	for (unsigned int i = 0; i < effects.size(); i++)
		if (!effects[i].name.compare(name))
			effects[i].intensity = intensity / effects[i].div + effects[i].add;
}

void simple_effects::restore_defaults() {
	post_effect::changed = true;

	for (unsigned int i = 0; i < effects.size(); i++)
		effects[i].intensity = effects[i].default_intensity;
}

void simple_effects::set_uniforms(post_shader *sdr) {
	Assert(sdr);

	for (unsigned int i = 0; i < effects.size(); i++)
		if (sdr->get_id() & (1 << i))
			sdr->set_uniform(post_shader::simple, i, effects[i].intensity);
}

post_processing *post_processing::instance = NULL;

void post_processing::create() {
	Assert(instance == NULL);
	if (config::is_enabled(config::post_process))
		instance = new post_processing;
}

void post_processing::remove() {
	if (instance)
		delete instance;
}

inline post_processing *post_processing::get() {
	Assert(instance != NULL);
	Assert(config::is_enabled(config::post_process));
	return instance;
}

post_processing::post_processing() {
	// Check FBO
	target = render_target::acquire(gr_screen.max_w, gr_screen.max_h);

	target->apply();
	image = texture_pool::acquire(target->get_width(), target->get_height());
	target->attach_texture(image);

	if (!target->check_status()) {
		mprintf(("FBO error. Could not initialize post-processing.\n"));
		config::disable(config::post_process);
	} else {
		bloom_eff = new bloom;
		simple_eff = new simple_effects;
#ifdef DEPTH_OF_FIELD
		dof_eff = new depth_of_field;

		depth = texture::create(target->get_width(), target->get_height(), texture::fmt_depth);
#endif
	}

	texture_pool::release(image);
	render_target::release(target);
}

post_processing::~post_processing() {
	if (bloom_eff)
		delete bloom_eff;

	if (simple_eff)
		delete simple_eff;

#ifdef DEPTH_OF_FIELD
	if (dof_eff)
		delete dof_eff;
#endif
}


void post_processing::before() {
	target = render_target::acquire(gr_screen.max_w, gr_screen.max_h);

	Assert(target);

	target->apply();

#ifdef DEPTH_OF_FIELD
	Assert(depth);
	target->set_depth(depth);
#endif

	image = texture_pool::acquire(target->get_width(), target->get_height());
	Assert(image);
	target->attach_texture(image);

	Assert(target->check_status());

	target->clear(render_target::clr_color | render_target::clr_depth, false);
}

void post_processing::disable() {
	mprintf(("Post-processing disabled.\n"));

	config::disable(config::post_process);
	render_target::apply_default();
	render_target::clear_default(render_target::clr_color);

	opengl::shader_manager::get()->apply_fixed_pipeline();
}

void post_processing::after() {
	Assert(target);
	Assert(image);

	// Existing texture management code doesn't like me.
	// This is a very nasty workaround (part 1), that prevents
	// post-processing from messing up bound textures. I'm going
	// to solve that problem when I'm creating new resources
	// and textures managers. Hery
	GLint tex0, tex1, tex2;
	vglActiveTextureARB(GL_TEXTURE0_ARB);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex0);
	vglActiveTextureARB(GL_TEXTURE1_ARB);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex1);
	vglActiveTextureARB(GL_TEXTURE2_ARB);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex2);

	render_target::release(target);

	// Fix problems when player is dead and the screen is clipped.
	glDisable(GL_SCISSOR_TEST);

	// Let effects prepare
#ifdef DEPTH_OF_FIELD
	if (!bloom_eff->begin(image) || !dof_eff->begin(image)) {
#else
	if (!bloom_eff->begin(image)) {
#endif
		texture_pool::release(image);
		disable();
		return;
	}

	// Fix problems when player is dead and the screen is clipped.
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, (gr_screen.max_h-gr_screen.clip_height)/2, gr_screen.max_w, gr_screen.clip_height);

	// Main post-processing render
	render_target::apply_default();
	
	render_target::clear_default(render_target::clr_color,true);

	post_shader *sdr = opengl::shader_manager::get()->apply_post_shader();

	if (!sdr) {
		bloom_eff->end();
#ifdef DEPTH_OF_FIELD
	dof_eff->end();
#endif
		texture_pool::release(image);
		disable();
		return;
	}

	// Set all uniforms
	sdr->set_texture(post_shader::texture, image);
	sdr->set_uniform(post_shader::timer, static_cast<float>(timer_get_milliseconds() % 100 + 1));

	bloom_eff->set_uniforms(sdr);
	simple_eff->set_uniforms(sdr);
#ifdef DEPTH_OF_FIELD
	dof_eff->set_uniforms(sdr);
#endif

	// Render
	render_target::draw_texture();

	texture_pool::release(image);

	// Let effects clean their stuff
	bloom_eff->end();
#ifdef DEPTH_OF_FIELD
	dof_eff->end();
#endif

	// This is a very nasty workaround (part 2)
	// More info in the comment to the first part above.
	vglActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, tex1);

	vglActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D, tex2);

	vglActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, tex0);

	opengl::shader_manager::get()->apply_fixed_pipeline();
}

// C wrappers for post-processing functions
void gr_opengl_set_post_effect(SCP_string &str, int x) {
	if (opengl::config::is_enabled(opengl::config::post_process))
		opengl::simple_effects::set_effect(str, x);
}

void gr_opengl_set_default_post_process() {
	if (opengl::config::is_enabled(opengl::config::post_process))
		opengl::simple_effects::restore_defaults();
}

void gr_opengl_post_process_release() {
	if (opengl::config::is_enabled(opengl::config::post_process))
		opengl::post_processing::remove();
}

void gr_opengl_post_process_init() {
	if (!opengl::config::is_enabled(opengl::config::post_process))
		return;

	// Check if we can use post processing
	if (!opengl::config::is_enabled(opengl::config::glsl) || opengl::config::is_enabled(opengl::config::no_fbo)) {
		Cmdline_postprocess = 0;
		return;
	}

	opengl::post_processing::create();

	opengl::render_target::apply_default();
}

void gr_opengl_save_zbuffer() {
	if (opengl::config::is_enabled(opengl::config::post_process))
		opengl::post_processing::get()->get_target()->use_depth_rb();
}

void gr_opengl_post_process_before() {
	if (opengl::config::is_enabled(opengl::config::post_process))
		opengl::post_processing::get()->before();
}

void gr_opengl_post_process_after() {
	if (opengl::config::is_enabled(opengl::config::post_process))
		opengl::post_processing::get()->after();
}
