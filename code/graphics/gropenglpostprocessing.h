
#ifndef _GROPENGLPOSTPROCESSING_H
#define _GROPENGLPOSTPROCESSING_H

void opengl_post_process_init();
void opengl_post_process_shutdown();

void gr_opengl_post_process_set_effect(const char *name, int x);
void gr_opengl_post_process_set_defaults();
void gr_opengl_post_process_save_zbuffer();
void gr_opengl_post_process_begin();
void gr_opengl_post_process_end();
void get_post_process_effect_names(SCP_vector<SCP_string> &names);

void opengl_post_init_uniforms(int flags);
void opengl_post_load_shader(SCP_string &sflags, shader_type shader, int flags);

#endif	// _GROPENGLPOSTPROCESSING_H
