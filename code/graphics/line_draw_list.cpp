//
//

#include "line_draw_list.h"
#include "2d.h"
#include "material.h"
#include "tracing/tracing.h"

namespace graphics {

line_draw_list::line_draw_list() {
}
void line_draw_list::add_line(int x1, int y1, int x2, int y2, int resize_mode) {
	add_vertex(x1, y1, resize_mode, &gr_screen.current_color);
	add_vertex(x2, y2, resize_mode, &gr_screen.current_color);
}
void line_draw_list::add_gradient(int x1, int y1, int x2, int y2, int resize_mode) {
	add_vertex(x1, y1, resize_mode, &gr_screen.current_color);

	color endColor = gr_screen.current_color;
	endColor.alpha = 0;
	add_vertex(x2, y2, resize_mode, &endColor);
}
void line_draw_list::flush() {
	if (_line_vertices.empty()) {
		// Nothing to do here...
		return;
	}

	GR_DEBUG_SCOPE("Line draw list flush");
	TRACE_SCOPE(tracing::LineDrawListFlush);

	material line_mat;
	line_mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	line_mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	line_mat.set_cull_mode(false);
	line_mat.set_color(1.0f, 1.0f, 1.0f, 1.0f); // Color is handled by the vertices


	vertex_layout layout;
	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(line_vertex), offsetof(line_vertex, position));
	layout.add_vertex_component(vertex_format_data::COLOR4F, sizeof(line_vertex), offsetof(line_vertex, color));

	gr_render_primitives_2d_immediate(&line_mat,
									  PRIM_TYPE_LINES,
									  &layout,
									  static_cast<int>(_line_vertices.size()),
									  _line_vertices.data(),
									  _line_vertices.size() * sizeof(line_vertex));

	_line_vertices.clear();
}
void line_draw_list::add_vertex(int x, int y, int resize_mode, const color* color) {
	line_vertex vtx{};
	vtx.position.x = i2fl(x);
	vtx.position.y = i2fl(y);

	gr_resize_screen_posf(&vtx.position.x, &vtx.position.y, nullptr, nullptr, resize_mode);
	vtx.color.xyzw.x = color->red / 255.f;
	vtx.color.xyzw.y = color->green / 255.f;
	vtx.color.xyzw.z = color->blue / 255.f;
	vtx.color.xyzw.w = color->is_alphacolor ? color->alpha / 255.f : 1.f;

	_line_vertices.push_back(vtx);
}

}
