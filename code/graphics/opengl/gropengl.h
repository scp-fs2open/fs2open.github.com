/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _GROPENGL_H
#define _GROPENGL_H

#include <glad/glad.h>
#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"

const ubyte GL_zero_3ub[3] = { 0, 0, 0 };

bool gr_opengl_init(std::unique_ptr<os::GraphicsOperations>&& graphicsOps);
void gr_opengl_cleanup(bool closing, int minimize=1);
int opengl_check_for_errors(const char *err_at = NULL);
bool gr_opengl_is_capable(gr_capability capability);
void gr_opengl_push_debug_group(const char* name);
void gr_opengl_pop_debug_group();
uint opengl_data_type_size(GLenum data_type);

#ifndef NDEBUG
#define GL_CHECK_FOR_ERRORS(s)	opengl_check_for_errors((s))
#else
#define GL_CHECK_FOR_ERRORS(s)
#endif

extern int GL_version;
extern int GLSL_version;

extern int Use_PBOs;

extern GLuint GL_vao;

extern float GL_alpha_threshold;
extern float GL_line_width;

#endif
