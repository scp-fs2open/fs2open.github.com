
#include "debug.h"

#include "render/3dinternal.h"

namespace graphics {
namespace debug {

namespace {

enum DebugElementType {
	Line,
	Text,
};

struct LineParams {
	vertex projected_from;
	vertex projected_to;
	color line_color;
};

struct TextParams {
	vertex projected_pos;
	color text_color;
	SCP_string text;
};

struct DebugElement {
	DebugElementType type;

	// A variant/union would be good here but our C++ version does not support that yet

	LineParams lineParams;

	TextParams textParams;
};

bool vertex_offscreen(const vertex& v) { return v.flags & PF_OVERFLOW; }

SCP_vector<DebugElement> debugElements;

void render_line(LineParams& lineParams)
{
	if (vertex_offscreen(lineParams.projected_from) && vertex_offscreen(lineParams.projected_to)) {
		return;
	}

	gr_set_color_fast(&lineParams.line_color);
	gr_line(lineParams.projected_from.screen.xyw.x,
			lineParams.projected_from.screen.xyw.y,
			lineParams.projected_to.screen.xyw.x,
			lineParams.projected_to.screen.xyw.y,
			GR_RESIZE_NONE);
}

void render_text(TextParams& params)
{
	if (vertex_offscreen(params.projected_pos)) {
		return;
	}

	font::set_font(font::FONT1);
	gr_set_color_fast(&params.text_color);
	gr_string(params.projected_pos.screen.xyw.x,
			  params.projected_pos.screen.xyw.y,
			  params.text.c_str(),
			  GR_RESIZE_NONE);
}

void render_element(DebugElement& el)
{
	switch (el.type) {
	case Line:
		render_line(el.lineParams);
		break;
	case Text:
		render_text(el.textParams);
		break;
	default:
		UNREACHABLE("Unknown debug element type!");
	}
}

} // namespace

void line_3d(const vec3d* from, const vec3d* to, const color* color)
{
	DebugElement el;
	el.type = DebugElementType::Line;

	bool do_g3 = G3_count < 1;
	if (do_g3)
		g3_start_frame(0);

	g3_rotate_vertex(&el.lineParams.projected_from, from);
	g3_project_vertex(&el.lineParams.projected_from);

	g3_rotate_vertex(&el.lineParams.projected_to, to);
	g3_project_vertex(&el.lineParams.projected_to);

	if (do_g3)
		g3_end_frame();

	el.lineParams.line_color = *color;

	debugElements.push_back(el);
}

void text_3d(const vec3d* world_pos, const color* color, SCP_string text)
{
	DebugElement el;
	el.type = DebugElementType::Text;

	bool do_g3 = G3_count < 1;
	if (do_g3)
		g3_start_frame(0);

	g3_rotate_vertex(&el.textParams.projected_pos, world_pos);
	g3_project_vertex(&el.textParams.projected_pos);

	if (do_g3)
		g3_end_frame();

	el.textParams.text_color = *color;
	el.textParams.text       = std::move(text);

	debugElements.push_back(el);
}

void render_elements()
{
	if (debugElements.empty()) {
		return;
	}

	for (auto& dbgEl : debugElements) {
		render_element(dbgEl);
	}

	debugElements.clear();
}

void cleanup() { debugElements.clear(); }

} // namespace debug
} // namespace graphics
