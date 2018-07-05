#pragma once

#include "globalincs/pstypes.h"
#include "2d.h"

namespace graphics {

/**
 * @brief A class for batching multiple line draw operations together
 *
 * This should be used when a lot of lines are going to be rendered together. The actual rendering operation will only
 * be done once flush() is called so the lines may overlap other objects that were rendered after the line was added to
 * this class.
 */
class line_draw_list {
	struct line_vertex {
		vec2d position;
		vec4 color;
	};
	SCP_vector<line_vertex> _line_vertices;

	void add_vertex(int x, int y, int resize_mode, const color* color);
 public:
	line_draw_list();

	/**
	 * @brief Adds a line to this draw list. The coordinates are screen pixel positions.
	 * @param x1 Start X-Coordinate of the line
	 * @param y1 Start Y-Coordinate of the line
	 * @param x2 End X-Coordinate of the line
	 * @param y2 End Y-Coordinate of the line
	 * @param resize_mode The resize mode of the screen position
	 */
	void add_line(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);

	/**
	 * @brief Adds a gradient to this draw list. The coordinates are screen pixel positions.
	 *
	 * A gradient is like a normal line but line will fade out towards the end of the line.
	 *
	 * @param x1 Start X-Coordinate of the line
	 * @param y1 Start Y-Coordinate of the line
	 * @param x2 End X-Coordinate of the line
	 * @param y2 End Y-Coordinate of the line
	 * @param resize_mode The resize mode of the screen position
	 */
	void add_gradient(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);

	/**
	 * @brief Flushes the stored line draws
	 *
	 * This will clear al previously drawn line segments so after this is called the instance can be filled with new
	 * line draws.
	 */
	void flush();
};

}

