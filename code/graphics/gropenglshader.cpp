/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"

#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltnl.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglstate.h"

#include "math/vecmat.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"


SCP_vector<opengl_shader_t> GL_shader;

static char *GLshader_info_log = NULL;
static const int GLshader_info_log_size = 8192;
GLuint Framebuffer_fallback_texture_id = 0;

static int Effect_num = 0;
static float Anim_timer = 0.0f;
/*
struct opengl_shader_file_t {
	char *vert;
	char *frag;

	int flags;

	int num_uniforms;
	char *uniforms[MAX_SHADER_UNIFORMS];
};
*/
static opengl_shader_file_t GL_shader_file[] = {
	{ "null-v.sdr", "null-f.sdr", (0), 0, { NULL }, 0, { NULL } },

	// with diffuse Textures
	{ "l-v.sdr", "lb-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP),
		2, { "sBasemap", "n_lights" }, 0, { NULL } },

	{ "b-v.sdr", "b-f.sdr", (SDR_FLAG_DIFFUSE_MAP),
		1, { "sBasemap" }, 0, { NULL } },

	{ "b-v.sdr", "bg-f.sdr", (SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP),
		2, { "sBasemap", "sGlowmap" }, 0, { NULL } },

	{ "l-v.sdr", "lbg-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP),
		3, { "sBasemap", "sGlowmap", "n_lights" }, 0, { NULL } },

	{ "l-v.sdr", "lbgs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP),
		4, { "sBasemap", "sGlowmap", "sSpecmap", "n_lights" }, 0, { NULL } },

	{ "l-v.sdr", "lbs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP),
		3, { "sBasemap", "sSpecmap", "n_lights" }, 0, { NULL } },

	{ "le-v.sdr", "lbgse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "le-v.sdr", "lbse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		6, { "sBasemap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbgn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sGlowmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbgsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		5, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sBasemap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sSpecmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lbgsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lbsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lf-v.sdr", "lfb-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP),
		2, { "sBasemap", "n_lights" }, 0, { NULL } },

	{ "lf-v.sdr", "lfbg-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP),
		3, { "sBasemap", "sGlowmap", "n_lights" }, 0, { NULL } },

	{ "lf-v.sdr", "lfbgs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP),
		4, { "sBasemap", "sGlowmap", "sSpecmap", "n_lights" }, 0, { NULL } },

	{ "lf-v.sdr", "lfbs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP),
		3, { "sBasemap", "sSpecmap", "n_lights" }, 0, { NULL } },

	{ "lfe-v.sdr", "lfbgse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lfe-v.sdr", "lfbse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		6, { "sBasemap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbgn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sGlowmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbgsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		5, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sBasemap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		4, { "sBasemap", "sSpecmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "lfne-v.sdr", "lfbgsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lfne-v.sdr", "lfbsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		7, { "sBasemap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	// no diffuse Textures 
	{ "l-v.sdr", "null-f.sdr", (SDR_FLAG_LIGHT),
		1, { "n_lights" }, 0, { NULL } },

	{ "l-v.sdr", "lg-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP),
		2, { "sGlowmap", "n_lights" }, 0, { NULL } },

	{ "l-v.sdr", "lgs-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP),
		3, { "sGlowmap", "sSpecmap", "n_lights" }, 0, { NULL } },

	{ "l-v.sdr", "ls-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP),
		2, { "sSpecmap", "n_lights" }, 0, { NULL } },

	{ "le-v.sdr", "lgse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		6, { "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "le-v.sdr", "lse-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP),
		5, { "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lgn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sGlowmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lgsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		4, { "sGlowmap", "sSpecmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "ln-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_NORMAL_MAP),
		2, { "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lsn-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP),
		3, { "sSpecmap", "sNormalmap", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lgsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		7, { "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lsne-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP),
		6, { "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	// Animated Shaders
	{ "la-v.sdr", "la-f.sdr", (SDR_FLAG_ANIMATED),
		5, { "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "la-v.sdr", "lba-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_ANIMATED),
		7, { "sBasemap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "ba-v.sdr", "ba-f.sdr", (SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_ANIMATED),
		6, { "sBasemap", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "ba-v.sdr", "bga-f.sdr", (SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_ANIMATED),
		7, { "sBasemap", "sGlowmap", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "la-v.sdr", "lbga-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_ANIMATED),
		8, { "sBasemap", "sGlowmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "la-v.sdr", "lbgsa-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ANIMATED),
		9, { "sBasemap", "sGlowmap", "sSpecmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "la-v.sdr", "lbsa-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ANIMATED),
		8, { "sBasemap", "sSpecmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lea-v.sdr", "lbgsea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		12, { "sBasemap", "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lea-v.sdr", "lbsea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		11, { "sBasemap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lna-v.sdr", "lbgna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		9, { "sBasemap", "sGlowmap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lna-v.sdr", "lbgsna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		10, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lna-v.sdr", "lbna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		8, { "sBasemap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lna-v.sdr", "lbsna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		9, { "sBasemap", "sSpecmap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lnea-v.sdr", "lbgsnea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		13, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lnea-v.sdr", "lbsnea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		12, { "sBasemap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfa-v.sdr", "lfba-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_ANIMATED),
		7, { "sBasemap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfa-v.sdr", "lfbga-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_ANIMATED),
		8, { "sBasemap", "sGlowmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfa-v.sdr", "lfbgsa-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ANIMATED),
		9, { "sBasemap", "sGlowmap", "sSpecmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfa-v.sdr", "lfbsa-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ANIMATED),
		8, { "sBasemap", "sSpecmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfea-v.sdr", "lfbgsea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		12, { "sBasemap", "sGlowmap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfea-v.sdr", "lfbsea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		11, { "sBasemap", "sSpecmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfna-v.sdr", "lfbgna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		9, { "sBasemap", "sGlowmap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfna-v.sdr", "lfbgsna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		10, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfna-v.sdr", "lfbna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		8, { "sBasemap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfna-v.sdr", "lfbsna-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ANIMATED),
		9, { "sBasemap", "sSpecmap", "sNormalmap", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfnea-v.sdr", "lfbgsnea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		13, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	{ "lfnea-v.sdr", "lfbsnea-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_ENV_MAP | SDR_FLAG_ANIMATED),
		12, { "sBasemap", "sSpecmap", "sNormalmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights", "anim_timer", "effect_num", "sFramebuffer", "vpwidth", "vpheight" }, 0, { NULL } },

	/* No Heightmapping for now - Valathil
	{ "lne-v.sdr", "lgsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		8, { "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		7, { "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbgnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sGlowmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbgsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		6, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sBasemap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lbsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lbgsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		9, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lne-v.sdr", "lbsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbgnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP| SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sGlowmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbgsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		6, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sBasemap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "lfn-v.sdr", "lfbsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "lfne-v.sdr", "lfbgsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		9, { "sBasemap", "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "lfne-v.sdr", "lfbsnhe-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_FOG | SDR_FLAG_DIFFUSE_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP | SDR_FLAG_ENV_MAP),
		8, { "sBasemap", "sSpecmap", "sNormalmap", "sHeightmap", "sEnvmap", "envMatrix", "alpha_spec", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lgnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sGlowmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lgsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_GLOW_MAP | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		5, { "sGlowmap", "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		3, { "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } },

	{ "ln-v.sdr", "lsnh-f.sdr", (SDR_FLAG_LIGHT | SDR_FLAG_SPEC_MAP | SDR_FLAG_NORMAL_MAP | SDR_FLAG_HEIGHT_MAP),
		4, { "sSpecmap", "sNormalmap", "sHeightmap", "n_lights" }, 0, { NULL } }*/

	{ "soft-v.sdr", "soft-f.sdr", (SDR_FLAG_SOFT_QUAD), 
		6, {"baseMap", "depthMap", "window_width", "window_height", "nearZ", "farZ"}, 1, { "radius_in" } }
};

static const int Num_shader_files = sizeof(GL_shader_file) / sizeof(opengl_shader_file_t);

opengl_shader_t *Current_shader = NULL;


void opengl_shader_check_info_log(GLhandleARB shader_object);


void opengl_shader_set_current(opengl_shader_t *shader_obj)
{
	Current_shader = shader_obj;

	if (Current_shader != NULL) {
		vglUseProgramObjectARB(Current_shader->program_id);

#ifndef NDEBUG
		if ( opengl_check_for_errors("shader_set_current()") ) {
			vglValidateProgramARB(Current_shader->program_id);

			GLint obj_status = 0;
			vglGetObjectParameterivARB(Current_shader->program_id, GL_OBJECT_VALIDATE_STATUS_ARB, &obj_status);

			if ( !obj_status ) {
				opengl_shader_check_info_log(Current_shader->program_id);
	
				mprintf(("VALIDATE INFO-LOG:\n"));

				if (strlen(GLshader_info_log) > 5) {
					mprintf(("%s\n", GLshader_info_log));
				} else {
					mprintf(("<EMPTY>\n"));
				}
			}
		}
#endif
	} else {
		vglUseProgramObjectARB(0);
	}
}

int opengl_shader_get_index(int flags)
{
	uint idx;

	for (idx = 0; idx < GL_shader.size(); idx++) {
		if (GL_shader[idx].flags == flags) {
			return idx;
		}
	}

	return -1;
}

void opengl_shader_shutdown()
{
	uint i;

	if ( !Use_GLSL ) {
		return;
	}

	for (i = 0; i < GL_shader.size(); i++) {
		if (GL_shader[i].program_id) {
			vglDeleteObjectARB(GL_shader[i].program_id);
			GL_shader[i].program_id = 0;
		}

		GL_shader[i].uniforms.clear();
	}

	GL_shader.clear();

	if (GLshader_info_log != NULL) {
		vm_free(GLshader_info_log);
		GLshader_info_log = NULL;
	}
}

static char *opengl_load_shader(char *filename, int flags, bool unified)
{
	std::string sflags;

	//sflags += "#version 120\n";

	if (Use_GLSL >= 4) {
		sflags += "#define SHADER_MODEL 4\n";
	} else if (Use_GLSL == 3) {
		sflags += "#define SHADER_MODEL 3\n";
	} else {
		sflags += "#define SHADER_MODEL 2\n";
	}

//	if (flags & SDR_FLAG_LIGHT) {
//		
//	}

	if (true) {
		if (flags & SDR_FLAG_DIFFUSE_MAP) {
			sflags += "#define FLAG_DIFFUSE_MAP\n";
		}

		if (flags & SDR_FLAG_ENV_MAP) {
			sflags += "#define FLAG_ENV_MAP\n";
		}

		if (flags & SDR_FLAG_FOG) {
			sflags += "#define FLAG_FOG\n";
		}

		if (flags & SDR_FLAG_GLOW_MAP) {
			sflags += "#define FLAG_GLOW_MAP\n";
		}

		if (flags & SDR_FLAG_HEIGHT_MAP) {
			sflags += "#define FLAG_HEIGHT_MAP\n";
		}

		if (flags & SDR_FLAG_LIGHT) {
			sflags += "#define FLAG_LIGHT\n";
		}

		if (flags & SDR_FLAG_NORMAL_MAP) {
			sflags += "#define FLAG_NORMAL_MAP\n";
		}

		if (flags & SDR_FLAG_SPEC_MAP) {
			sflags += "#define FLAG_SPEC_MAP\n";
		}

		if (flags & SDR_FLAG_ANIMATED) {
			sflags += "#define FLAG_ANIMATED\n";
		}
	}



	const char *shader_flags = sflags.c_str();
	int flags_len = strlen(shader_flags);

	CFILE *cf_shader = cfopen(filename, "rt", CFILE_NORMAL, CF_TYPE_EFFECTS);
	
	if (cf_shader != NULL) {
		int len = cfilelength(cf_shader);
		char *shader = (char*) vm_malloc(len + flags_len + 1);

		strcpy(shader, shader_flags);
		memset(shader + flags_len, 0, len + 1);
		cfread(shader + flags_len, len + 1, 1, cf_shader);
		cfclose(cf_shader);

		return shader;	
	} else {
		mprintf(("Loading built-in default shader for: %s\n", filename));
		char* def_shader = defaults_get_file(filename);
		size_t len = strlen(def_shader);
		char *shader = (char*) vm_malloc(len + flags_len + 1);

		strcpy(shader, shader_flags);
		strcat(shader, def_shader);
		//memset(shader + flags_len, 0, len + 1);

		return shader;
	}

}

void opengl_shader_init()
{
	char *vert = NULL, *frag = NULL;
	int i, idx;

	if ( !Use_GLSL ) {
		return;
	}

	glGenTextures(1,&Framebuffer_fallback_texture_id);
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Framebuffer_fallback_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	GLuint pixels[4] = {0,0,0,0};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &pixels);

	if (Cmdline_no_glsl_model_rendering) {
		Use_GLSL = 1;
	} else {
		// check if main shaders exist
		bool main_vert = cf_exists_full("main-v.sdr", CF_TYPE_EFFECTS) != 0;
		bool main_frag = cf_exists_full("main-f.sdr", CF_TYPE_EFFECTS) != 0;

		GL_shader.reserve(Num_shader_files+1);

		for (idx = 0; idx < Num_shader_files; idx++) {
			bool in_error = false;
			opengl_shader_t new_shader;
			opengl_shader_file_t *shader_file = &GL_shader_file[idx];

			if ( !Cmdline_glow && (shader_file->flags & SDR_FLAG_GLOW_MAP) ) {
				continue;
			}

			if ( !Cmdline_spec && (shader_file->flags & SDR_FLAG_SPEC_MAP) ) {
				continue;
			}

			if ( !Cmdline_env && (shader_file->flags & SDR_FLAG_ENV_MAP) ) {
				continue;
			}

			if ( !Cmdline_normal && (shader_file->flags & SDR_FLAG_NORMAL_MAP) ) {
				continue;
			}

			if ( !Cmdline_height && (shader_file->flags & SDR_FLAG_HEIGHT_MAP) ) {
				continue;
			}

			// choose appropriate files
			char *vert_name = shader_file->vert;
			char *frag_name = shader_file->frag;

			if (main_vert || !cf_exists_full(vert_name, CF_TYPE_EFFECTS)) {
				vert_name = "main-v.sdr";
			}

			if (main_frag || !cf_exists_full(frag_name, CF_TYPE_EFFECTS)) {
				frag_name = "main-f.sdr";
			}

			if ( shader_file->flags & SDR_FLAG_SOFT_QUAD ) {
				// soft particles use their own shader files
				vert_name = shader_file->vert;
				frag_name = shader_file->frag;
			}


			mprintf(("  Compiling shader: %s (%s), %s (%s)\n", vert_name, GL_shader_file[idx].vert, frag_name, GL_shader_file[idx].frag ));

			// read vertex shader
			if ( (vert = opengl_load_shader(vert_name, shader_file->flags, main_vert)) == NULL ) {
				in_error = true;
				goto Done;
			}

			// read fragment shader
			if ( (frag = opengl_load_shader(frag_name, shader_file->flags, main_frag)) == NULL ) {
				in_error = true;
				goto Done;
			}

			Verify( vert != NULL );
			Verify( frag != NULL );

			new_shader.program_id = opengl_shader_create(vert, frag);

			if ( !new_shader.program_id ) {
				in_error = true;
				goto Done;
			}

			new_shader.flags = shader_file->flags;

			opengl_shader_set_current( &new_shader );

			new_shader.uniforms.reserve(shader_file->num_uniforms);
			new_shader.attributes.reserve(shader_file->num_attributes);

			for (i = 0; i < shader_file->num_uniforms; i++) {
				opengl_shader_init_uniform( shader_file->uniforms[i] );
			}

			for ( i = 0; i < shader_file->num_attributes; i++ ) {
				opengl_shader_init_attribute( shader_file->attributes[i] );
			}

			opengl_shader_set_current();

			// add it to our list of embedded shaders
			GL_shader.push_back( new_shader );

		Done:
			if (vert != NULL) {
				vm_free(vert);
				vert = NULL;
			}

			if (frag != NULL) {
				vm_free(frag);
				frag = NULL;
			}

			if (in_error) {
				// shut off relevant usage things ...
				bool dealt_with = false;

				if (shader_file->flags & SDR_FLAG_HEIGHT_MAP) {
					mprintf(("  Shader in_error!  Disabling height maps!\n"));
					Cmdline_height = 0;
					dealt_with = true;
				}

				if (shader_file->flags & SDR_FLAG_NORMAL_MAP) {
					mprintf(("  Shader in_error!  Disabling normal maps and height maps!\n"));
					Cmdline_height = 0;
					Cmdline_normal = 0;
					dealt_with = true;
				}

				if (!dealt_with) {
					if (idx == 0) {
						mprintf(("  Shader in_error!  Disabling GLSL!\n"));

						Use_GLSL = 0;
						Cmdline_height = 0;
						Cmdline_normal = 0;

						GL_shader.clear();
						break;;
					} else {
						// We died on a lighting shader, probably due to instruction count.
						// Drop down to a special var that will use fixed-function rendering
						// but still allow for post-processing to work
						mprintf(("  Shader in_error!  Disabling GLSL model rendering!\n"));
						Use_GLSL = 1;
						Cmdline_height = 0;
						Cmdline_normal = 0;
						break;;
					}
				}
			}
		}
	}

	mprintf(("\n"));
}

void opengl_shader_check_info_log(GLhandleARB shader_object)
{
	if (GLshader_info_log == NULL) {
		GLshader_info_log = (char *) vm_malloc(GLshader_info_log_size);
	}

	memset(GLshader_info_log, 0, GLshader_info_log_size);

	vglGetInfoLogARB(shader_object, GLshader_info_log_size-1, 0, GLshader_info_log);
}

GLhandleARB opengl_shader_compile_object(const GLcharARB *shader_source, GLenum shader_type)
{
	GLhandleARB shader_object = 0;
	GLint status = 0;

	shader_object = vglCreateShaderObjectARB(shader_type);

	vglShaderSourceARB(shader_object, 1, &shader_source, NULL);
	vglCompileShaderARB(shader_object);

	// check if the compile was successful
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_COMPILE_STATUS_ARB, &status);

	opengl_shader_check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		// basic error check
		mprintf(("%s shader failed to compile:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : "Fragment", GLshader_info_log));

		// this really shouldn't exist, but just in case
		if (shader_object) {
			vglDeleteObjectARB(shader_object);
		}

		return 0;
	}

	// we succeeded, maybe output warnings too
	if (strlen(GLshader_info_log) > 5) {
		nprintf(("SHADER-DEBUG", "%s shader compiled with warnings:\n%s\n", (shader_type == GL_VERTEX_SHADER_ARB) ? "Vertex" : "Fragment", GLshader_info_log));
	}

	return shader_object;
}

GLhandleARB opengl_shader_link_object(GLhandleARB vertex_object, GLhandleARB fragment_object)
{
	GLhandleARB shader_object = 0;
	GLint status = 0;

	shader_object = vglCreateProgramObjectARB();

	if (vertex_object) {
		vglAttachObjectARB(shader_object, vertex_object);
	}

	if (fragment_object) {
		vglAttachObjectARB(shader_object, fragment_object);
	}
	
	vglLinkProgramARB(shader_object);

	// check if the link was successful
	vglGetObjectParameterivARB(shader_object, GL_OBJECT_LINK_STATUS_ARB, &status);

	opengl_shader_check_info_log(shader_object);

	// we failed, bail out now...
	if (status == 0) {
		mprintf(("Shader failed to link:\n%s\n", GLshader_info_log));

		if (shader_object) {
			vglDeleteObjectARB(shader_object);
		}

		return 0;
	}

	// we succeeded, maybe output warnings too
	if (strlen(GLshader_info_log) > 5) {
		nprintf(("SHADER-DEBUG", "Shader linked with warnings:\n%s\n", GLshader_info_log));
	}

	return shader_object;
}

/*GLhandleARB opengl_shader_create(const char *vs, const char *fs)
{
	GLhandleARB vs_o = 0;
	GLhandleARB fs_o = 0;
	GLhandleARB program = 0;

	if (vs) {
		vs_o = opengl_shader_compile_object( (const GLcharARB*)vs, GL_VERTEX_SHADER_ARB );

		if ( !vs_o ) {
			mprintf(("ERROR! Unable to create vertex shader!\n"));
			goto Done;
		}
	}

	if (fs) {
		fs_o = opengl_shader_compile_object( (const GLcharARB*)fs, GL_FRAGMENT_SHADER_ARB );

		if ( !fs_o ) {
			mprintf(("ERROR! Unable to create fragment shader!\n"));
			goto Done;
		}
	}

	program = opengl_shader_link_object(vs_o, fs_o);

	if ( !program ) {
		mprintf(("ERROR! Unable to create shader program!\n"));
	}

Done:
	if (vs_o) {
		vglDeleteObjectARB(vs_o);
	}

	if (fs_o) {
		vglDeleteObjectARB(fs_o);
	}

	return program;
}*/

GLhandleARB opengl_shader_create(const char *vs, const char *fs)
{
	GLhandleARB vs_o = 0;
	GLhandleARB fs_o = 0;
	GLhandleARB program = 0;

	if (vs) {
		vs_o = opengl_shader_compile_object( (const GLcharARB*)vs, GL_VERTEX_SHADER_ARB );

		if ( !vs_o ) {
			mprintf(("ERROR! Unable to create vertex shader!\n"));
			goto Done;
		}
	}

	if (fs) {
		fs_o = opengl_shader_compile_object( (const GLcharARB*)fs, GL_FRAGMENT_SHADER_ARB );

		if ( !fs_o ) {
			mprintf(("ERROR! Unable to create fragment shader!\n"));
			goto Done;
		}
	}

	program = opengl_shader_link_object(vs_o, fs_o);

	if ( !program ) {
		mprintf(("ERROR! Unable to create shader program!\n"));
	}

Done:
	if (vs_o) {
		vglDeleteObjectARB(vs_o);
	}

	if (fs_o) {
		vglDeleteObjectARB(fs_o);
	}

	return program;
}

void opengl_shader_init_attribute(const char *attribute_text)
{
	opengl_shader_uniform_t new_attribute;

	if ( ( Current_shader == NULL ) || ( attribute_text == NULL ) ) {
		Int3();
		return;
	}

	new_attribute.text_id = attribute_text;
	new_attribute.location = vglGetAttribLocationARB(Current_shader->program_id, attribute_text);

	if ( new_attribute.location < 0 ) {
		nprintf(("SHADER-DEBUG", "WARNING: Unable to get shader attribute location for \"%s\"!\n", attribute_text));
		return;
	}

	Current_shader->attributes.push_back( new_attribute );
}

GLint opengl_shader_get_attribute(const char *attribute_text)
{
	if ( (Current_shader == NULL) || (attribute_text == NULL) ) {
		Int3();
		return -1;
	}

	SCP_vector<opengl_shader_uniform_t>::iterator attribute;

	for (attribute = Current_shader->attributes.begin(); attribute != Current_shader->attributes.end(); ++attribute) {
		if ( !attribute->text_id.compare(attribute_text) ) {
			return attribute->location;
		}
	}

	return -1;
}

void opengl_shader_init_uniform(const char *uniform_text)
{
	opengl_shader_uniform_t new_uniform;

	if ( (Current_shader == NULL) || (uniform_text == NULL) ) {
		Int3();
		return;
	}

	new_uniform.text_id = uniform_text;
	new_uniform.location = vglGetUniformLocationARB(Current_shader->program_id, uniform_text);

	if (new_uniform.location < 0) {
		nprintf(("SHADER-DEBUG", "WARNING: Unable to get shader uniform location for \"%s\"!\n", uniform_text));
		return;
	}

	Current_shader->uniforms.push_back( new_uniform );
}

GLint opengl_shader_get_uniform(const char *uniform_text)
{
	if ( (Current_shader == NULL) || (uniform_text == NULL) ) {
		Int3();
		return -1;
	}

	SCP_vector<opengl_shader_uniform_t>::iterator uniform;
	
	for (uniform = Current_shader->uniforms.begin(); uniform != Current_shader->uniforms.end(); ++uniform) {
		if ( !uniform->text_id.compare(uniform_text) ) {
			return uniform->location;
		}
	}

	return -1;
}

void opengl_shader_set_animated_effect(int effect)
{
	Assert(effect > -1);
	Effect_num = effect;
}

int opengl_shader_get_animated_effect()
{
	return Effect_num;
}

void opengl_shader_set_animated_timer(float timer)
{
	Anim_timer = timer;
}

float opengl_shader_get_animated_timer()
{
	return Anim_timer;
}
