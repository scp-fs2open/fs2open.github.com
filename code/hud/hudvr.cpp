#include "hud/hudvr.h"

#include "graphics/2d.h"
#include "graphics/material.h"
#include "graphics/matrix.h"
#include "graphics/openxr.h"
#include "graphics/render.h"
#include "hud/hud.h"
#include "render/3d.h"

namespace {

// Far enough that the panel is effectively at infinity: brackets then stay on their targets whatever the head does
constexpr float PANEL_DISTANCE = 1000.0f;

// The flat-screen default, which the HUD's layout is made for. Spreading it over the HMD's much wider field of view
// would push the gauges out to where the panel is seen at a steep angle, squashing them.
constexpr float PANEL_FOV = DEFAULT_FOV;

vec3d Eye_pos;
matrix Eye_orient;

int Panel_bitmap = -1;
bool Panel_drawing = false;
vec3d Panel_pos;
matrix Panel_orient;

}

void hud_vr_init()
{
	const auto eye_fov = openxr_get_eye_fov();
	if (!eye_fov) {
		return;
	}

	// As many pixels per angle as the eye's own image at its center: as sharp as the HMD can show, and no more
	const float eye_tan_height = tanf(eye_fov->up) - tanf(eye_fov->down);
	const float panel_tan_height = 2.0f * tanf(PANEL_FOV * PROJ_FOV_FACTOR * 0.5f);
	gr_screen_layer_init(panel_tan_height / eye_tan_height);
}

VrHudPass hud_vr_begin_maybe(const vec3d* camera_pos, const matrix* camera_orient)
{
	if (!openxr_in_stereo_frame()) {
		return VrHudPass::Direct;
	}

	// The eye's view, which drawing the panel into it needs and the HUD pass below replaces
	Eye_pos = Eye_position;
	Eye_orient = Eye_matrix;

	if (openxr_get_current_eye() != 0) {
		return VrHudPass::Reuse;
	}

	Panel_bitmap = gr_screen_layer_begin();
	Assertion(Panel_bitmap >= 0, "The renderer could not create the VR HUD panel!");

	Panel_drawing = true;
	Panel_pos = *camera_pos;
	Panel_orient = *camera_orient;

	// Everything that projects onto the HUD, scripts included, has to do so from the panel's camera
	g3_start_frame(0);
	hud_vr_set_view();
	HUD_set_offsets();
	g3_end_frame();

	return VrHudPass::Panel;
}

void hud_vr_end()
{
	// Shown on the panel, so it's drawn once for both eyes and stays put like the HUD (see gr_flip())
	gr_imgui_draw_overlay();

	gr_screen_layer_end();
	Panel_drawing = false;
}

bool hud_vr_drawing_panel()
{
	return Panel_drawing;
}

void hud_vr_set_view()
{
	g3_set_view_matrix(&Panel_pos, &Panel_orient, PANEL_FOV);
}

void hud_vr_draw_panel(const fov_t& eye_fov)
{
	if (Panel_bitmap < 0) {
		return;
	}

	struct panel_vertex {
		vec3d pos;
		float u, v;
	};

	// The full-screen frustum g3 sets up for a flat-screen fov, which is vertical (see scale_matrix())
	const float tan_y = tanf(PANEL_FOV * PROJ_FOV_FACTOR * 0.5f);
	const float tan_x = tan_y * i2fl(gr_screen.max_w) / (i2fl(gr_screen.max_h) * gr_screen.aspect);

	// Relative to the eye, which is where the view matrix below puts the origin
	vec3d center;
	vm_vec_sub(&center, &Panel_pos, &Eye_pos);

	auto corner = [&](float x, float y, float u, float v) {
		panel_vertex vert;
		vert.pos = center;
		vm_vec_scale_add2(&vert.pos, &Panel_orient.vec.fvec, PANEL_DISTANCE);
		vm_vec_scale_add2(&vert.pos, &Panel_orient.vec.rvec, x * tan_x * PANEL_DISTANCE);
		vm_vec_scale_add2(&vert.pos, &Panel_orient.vec.uvec, y * tan_y * PANEL_DISTANCE);
		vert.u = u;
		vert.v = v;
		return vert;
	};

	// A screen layer keeps the rows in the order of the screen's framebuffer, which OpenGL stores bottom up
	const float v_top = gr_screen.mode == GraphicsAPI::OpenGL ? 1.0f : 0.0f;
	const float v_bottom = 1.0f - v_top;

	panel_vertex verts[4] = {
		corner(-1.0f, 1.0f, 0.0f, v_top),
		corner(1.0f, 1.0f, 1.0f, v_top),
		corner(-1.0f, -1.0f, 0.0f, v_bottom),
		corner(1.0f, -1.0f, 1.0f, v_bottom),
	};

	vertex_layout layout;
	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(panel_vertex), offsetof(panel_vertex, pos));
	layout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(panel_vertex), offsetof(panel_vertex, u));

	material mat;
	material_set_interface(&mat, Panel_bitmap, true, 1.0f);
	mat.set_blend_mode(ALPHA_BLEND_PREMULTIPLIED);

	gr_reset_clip();
	gr_set_proj_matrix(eye_fov * PROJ_FOV_FACTOR, gr_screen.clip_aspect, PANEL_DISTANCE * 0.1f, PANEL_DISTANCE * 10.0f);
	gr_set_view_matrix(&vmd_zero_vector, &Eye_orient);

	gr_render_primitives_immediate(&mat, PRIM_TYPE_TRISTRIP, &layout, 4, verts, sizeof(verts));

	gr_end_view_matrix();
	gr_end_proj_matrix();

	g3_start_frame(0);
	g3_set_view_matrix(&Eye_pos, &Eye_orient, eye_fov);
	g3_end_frame();
}
