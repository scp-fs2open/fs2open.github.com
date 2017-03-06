#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

/**
 * @brief Flashes the screen with the specified color
 * @param r The red color value
 * @param g The green color value
 * @param b The blue color value
 */
void gr_flash( int r, int g, int b );
/**
 * @brief Flashes the screen with the specified color with an alpha value
 * @param r The red color value
 * @param g The green color value
 * @param b The blue color value
 * @param a The alpha value of the flash
 */
void gr_flash_alpha(int r, int g, int b, int a);

/**
 * @brief Draws a grey-scale bitmap multiplied with the current color
 * @param x The x-coordinate of the draw call
 * @param y The y-coordinate of the draw call
 * @param resize_mode The resize mode for translating the coordinated
 * @param mirror @c true to mirror the image
 */
void gr_aabitmap(int x, int y, int resize_mode = GR_RESIZE_FULL, bool mirror = false);
/**
 * @brief Draws a grey-scale bitmap multiplied with the current color
 * @param x The x-coordinate of the draw call
 * @param y The y-coordinate of the draw call
 * @param w
 * @param h
 * @param sx
 * @param sy
 * @param resize_mode The resize mode for translating the coordinated
 * @param mirror @c true to mirror the image
 */
void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL, bool mirror = false);
/**
 * @brief Draws a normal-colored bitmap to the screen
 * @param x The x-coordinate of the draw call
 * @param y The y-coordinate of the draw call
 * @param w
 * @param h
 * @param sx
 * @param sy
 * @param resize_mode The resize mode for translating the coordinated
 */
void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL);

/**
 * @brief Renders the specified string to the screen using the current font and color
 * @param x The x-coordinate
 * @param y The y-coordinate
 * @param string The string to draw to the screen
 * @param resize_mode The mode for translating the screen positions
 * @param length The number of bytes in the string to render. -1 will render the whole string.
 */
void gr_string(float x, float y, const char* string, int resize_mode = GR_RESIZE_FULL, int length = -1);
/**
 * @brief Renders the specified string to the screen using the current font and color
 * @param x The x-coordinate
 * @param y The y-coordinate
 * @param string The string to draw to the screen
 * @param resize_mode The mode for translating the screen positions
 * @param length The number of bytes in the string to render. -1 will render the whole string.
 */
inline void gr_string(int x, int y, const char* string, int resize_mode = GR_RESIZE_FULL, int length = -1)
{
	gr_string(i2fl(x), i2fl(y), string, resize_mode, length);
}

/**
 * @brief Draws a single line segment to the screen.
 * @param x1 The starting x-coordinate
 * @param y1 The starting y-coordinate
 * @param x2 The end x-coordinate
 * @param y2 The end y-coordinate
 * @param resize_mode The resize mode for translating screen positions
 */
void gr_line(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);
/**
 * @brief Draws an antialiased line is the current color is an alphacolor, otherwise just draws a fast line.
 * This gets called internally by g3_draw_line. This assumes he vertex's are already clipped, so call g3_draw_line
 * not this if you have two 3d points.
 * @param v1 The starting position
 * @param v2 The end position
 */
void gr_aaline(vertex *v1, vertex *v2);
/**
 * @brief Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
 * @param x1 The starting x-coordinate
 * @param y1 The starting y-coordinate
 * @param x2 The end x-coordinate
 * @param y2 The end y-coordinate
 * @param resize_mode The resize mode for translating screen positions
 */
void gr_gradient(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);
/**
 * @brief Sets the specified pixel to the current color
 * @param x The x-coordinate
 * @param y The y-coordinate
 * @param resize_mode The mode for translating the screen positions
 */
void gr_pixel(int x, int y, int resize_mode = GR_RESIZE_FULL);

/**
 * @brief Draws a filled rectangle with the current color
 * @param x The x-coordinate
 * @param y The y-coordinate
 * @param w The width of the rectangle
 * @param h The height of the rectangle
 * @param resize_mode The mode for translating the screen positions
 */
void gr_rect(int x, int y, int w, int h, int resize_mode = GR_RESIZE_FULL);
/**
 * @brief Draws a filled rectangle with the current shading color
 * @param x The x-coordinate
 * @param y The y-coordinate
 * @param w The width of the rectangle
 * @param h The height of the rectangle
 * @param resize_mode The mode for translating the screen positions
 */
void gr_shade(int x, int y, int w, int h, int resize_mode = GR_RESIZE_FULL);

/**
 * @brief Draws a filled circle with the current color
 * @param xc The center x-position of the circle
 * @param yc The center y-position of the circle
 * @param d The diameter of the circle
 * @param resize_mode The mode for translating the screen positions
 */
void gr_circle(int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL);
/**
 * @brief Draws an unfilled circle with the current color
 * @param xc The center x-position of the circle
 * @param yc The center y-position of the circle
 * @param d The diameter of the circle
 * @param resize_mode The mode for translating the screen positions
 */
void gr_unfilled_circle(int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL);
/**
 * @brief Draws a limited circle arc from the specified start to the end angle
 * @param xc The center x-position
 * @param yc The center y-position
 * @param r The radius of the arc
 * @param angle_start The starting angle of the arc
 * @param angle_end The end angle of the arc
 * @param fill @c true to fill the arc segment
 * @param resize_mode The mode for translating the screen positions
 */
void gr_arc(int xc, int yc, float r, float angle_start, float angle_end, bool fill, int resize_mode = GR_RESIZE_FULL);
/**
 * @brief Draws a 90° curve with the center at the specified coordinates
 *
 * The direction specified in which direction the curve should be drawn
 *
 * @param x The center x-position of the curve
 * @param y The center y-position of the curve
 * @param r The radius of the curve
 * @param direction The direction to draw the curve
 * @param resize_mode The mode for translating the screen positions
 */
void gr_curve(int x, int y, int r, int direction, int resize_mode);
