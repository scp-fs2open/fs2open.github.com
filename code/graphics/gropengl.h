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

#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"

const ubyte GL_zero_3ub[3] = { 0, 0, 0 };

bool gr_opengl_init(os::GraphicsOperations* graphicsOps);
void gr_opengl_cleanup(os::GraphicsOperations* graphicsOps, bool closing, int minimize=1);
int opengl_check_for_errors(char *err_at = NULL);
bool is_minimum_GLSL_version();

#ifndef NDEBUG
#define GL_CHECK_FOR_ERRORS(s)	opengl_check_for_errors((s))
#else
#define GL_CHECK_FOR_ERRORS(s)
#endif

extern int GL_version;
extern int GLSL_version;

extern int Use_VBOs;
extern int Use_PBOs;

#endif
