#ifndef _WXFREDRENDER_H
#define _WXFREDRENDER_H
/*
 * Created for the FreeSpace2 Source Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "glcViewport.h"

#include "globalincs/pstypes.h"

/**
 * @brief This file defines the graphics interface used by wxFred
 */
namespace wxfred {

/**
 * @brief Inits the graphics engine and viewport for rendering
 */
void render_init(glcViewport *win);

/**
 * @brief Renders a frame on the given viewport
 */
void render_frame(glcViewport *win);


/**
 * @brief Resizes the draw area/canvas
 */
void render_resize(glcViewport *win, int w, int h);

};	// namespace wxfred

#endif // _WXFREDRENDER_H