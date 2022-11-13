
#include "FredRenderer.h"
#include "Editor.h"
#include "EditorViewport.h"

#include <globalincs/alphacolors.h>
#include <mission/missiongrid.h>
#include <globalincs/systemvars.h>
#include <io/timer.h>
#include <osapi/osapi.h>
#include <io/key.h>
#include <object/object.h>
#include <globalincs/linklist.h>
#include <render/3d.h>
#include <graphics/font.h>
#include <graphics/matrix.h>
#include <graphics/light.h>
#include <lighting/lighting.h>
#include <starfield/starfield.h>
#include <ship/ship.h>
#include <jumpnode/jumpnode.h>
#include <asteroid/asteroid.h>
#include <iff_defs/iff_defs.h>
#include <math/fvi.h>
#include <graphics/light.h>
#include <mod_table/mod_table.h>

#include "mission/object.h"
#include "weapon/weapon.h"


namespace {
const float CONVERT_DEGREES = 57.29578f; // conversion factor from radians to degrees

const float FRED_DEFAULT_HTL_FOV = 0.485f;
const float FRED_DEAFULT_HTL_DRAW_DIST = 300000.0f;

const int FRED_COLOUR_WHITE = 0xffffff;
const int FRED_COLOUR_YELLOW_GREEN = 0xc8ff00;

const int BRIEFING_LOOKAT_POINT_ID = 99999;

void enable_htl() {
	gr_set_proj_matrix((4.0f / 9.0f) * PI * FRED_DEFAULT_HTL_FOV,
					   gr_screen.aspect * static_cast<float>(gr_screen.clip_width)
						   / static_cast<float>(gr_screen.clip_height),
					   1.0f,
					   FRED_DEAFULT_HTL_DRAW_DIST);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);
}

void disable_htl() {
	gr_end_proj_matrix();
	gr_end_view_matrix();
}

bool fred_colors_inited = false;
color colour_white;
color colour_green;
color colour_black;
color colour_yellow_green;

void init_fred_colors() {
	if (fred_colors_inited) {
		return;
	}

	fred_colors_inited = true;

	gr_init_alphacolor(&colour_white, 255, 255, 255, 255);
	gr_init_alphacolor(&colour_green, 0, 200, 0, 255);
	gr_init_alphacolor(&colour_yellow_green, 200, 255, 0, 255);
	gr_init_alphacolor(&colour_black, 0, 0, 0, 255);
}

int grid_colors_inited = 0;
color Fred_grid_bright;
color Fred_grid_dark;
color Fred_grid_bright_aa;
color Fred_grid_dark_aa;

void draw_asteroid_field() {
	int i, j;
	vec3d p[8], ip[8];
	vertex v[8], iv[8];

	for (i = 0; i < 1 /*MAX_ASTEROID_FIELDS*/; i++) {
		if (Asteroid_field.num_initial_asteroids) {
			p[0].xyz.x = p[2].xyz.x = p[4].xyz.x = p[6].xyz.x = Asteroid_field.min_bound.xyz.x;
			p[1].xyz.x = p[3].xyz.x = p[5].xyz.x = p[7].xyz.x = Asteroid_field.max_bound.xyz.x;
			p[0].xyz.y = p[1].xyz.y = p[4].xyz.y = p[5].xyz.y = Asteroid_field.min_bound.xyz.y;
			p[2].xyz.y = p[3].xyz.y = p[6].xyz.y = p[7].xyz.y = Asteroid_field.max_bound.xyz.y;
			p[0].xyz.z = p[1].xyz.z = p[2].xyz.z = p[3].xyz.z = Asteroid_field.min_bound.xyz.z;
			p[4].xyz.z = p[5].xyz.z = p[6].xyz.z = p[7].xyz.z = Asteroid_field.max_bound.xyz.z;

			for (j = 0; j < 8; j++) {
				g3_rotate_vertex(&v[j], &p[j]);
			}

			g3_draw_line(&v[0], &v[1]);
			g3_draw_line(&v[2], &v[3]);
			g3_draw_line(&v[4], &v[5]);
			g3_draw_line(&v[6], &v[7]);
			g3_draw_line(&v[0], &v[2]);
			g3_draw_line(&v[1], &v[3]);
			g3_draw_line(&v[4], &v[6]);
			g3_draw_line(&v[5], &v[7]);
			g3_draw_line(&v[0], &v[4]);
			g3_draw_line(&v[1], &v[5]);
			g3_draw_line(&v[2], &v[6]);
			g3_draw_line(&v[3], &v[7]);


			// maybe draw inner box
			if (Asteroid_field.has_inner_bound) {
				gr_set_color(16, 192, 92);

				ip[0].xyz.x = ip[2].xyz.x = ip[4].xyz.x = ip[6].xyz.x = Asteroid_field.inner_min_bound.xyz.x;
				ip[1].xyz.x = ip[3].xyz.x = ip[5].xyz.x = ip[7].xyz.x = Asteroid_field.inner_max_bound.xyz.x;
				ip[0].xyz.y = ip[1].xyz.y = ip[4].xyz.y = ip[5].xyz.y = Asteroid_field.inner_min_bound.xyz.y;
				ip[2].xyz.y = ip[3].xyz.y = ip[6].xyz.y = ip[7].xyz.y = Asteroid_field.inner_max_bound.xyz.y;
				ip[0].xyz.z = ip[1].xyz.z = ip[2].xyz.z = ip[3].xyz.z = Asteroid_field.inner_min_bound.xyz.z;
				ip[4].xyz.z = ip[5].xyz.z = ip[6].xyz.z = ip[7].xyz.z = Asteroid_field.inner_max_bound.xyz.z;

				for (j = 0; j < 8; j++) {
					g3_rotate_vertex(&iv[j], &ip[j]);
				}

				g3_draw_line(&iv[0], &iv[1]);
				g3_draw_line(&iv[2], &iv[3]);
				g3_draw_line(&iv[4], &iv[5]);
				g3_draw_line(&iv[6], &iv[7]);
				g3_draw_line(&iv[0], &iv[2]);
				g3_draw_line(&iv[1], &iv[3]);
				g3_draw_line(&iv[4], &iv[6]);
				g3_draw_line(&iv[5], &iv[7]);
				g3_draw_line(&iv[0], &iv[4]);
				g3_draw_line(&iv[1], &iv[5]);
				g3_draw_line(&iv[2], &iv[6]);
				g3_draw_line(&iv[3], &iv[7]);
			}
		}
	}
}

void fredhtl_render_subsystem_bounding_box(subsys_to_render *s2r)
{
	vertex text_center;
	SCP_string buf;

	auto objp = s2r->ship_obj;
	auto ss = s2r->cur_subsys;

	auto pmi = model_get_instance(Ships[objp->instance].model_instance_num);
	auto pm = model_get(pmi->model_num);
	int subobj_num = ss->system_info->subobj_num;

	auto bsp = &pm->submodel[subobj_num];

	vec3d front_top_left = bsp->bounding_box[7];
	vec3d front_top_right = bsp->bounding_box[6];
	vec3d front_bot_left = bsp->bounding_box[4];
	vec3d front_bot_right = bsp->bounding_box[5];
	vec3d back_top_left = bsp->bounding_box[3];
	vec3d back_top_right = bsp->bounding_box[2];
	vec3d back_bot_left = bsp->bounding_box[0];
	vec3d back_bot_right = bsp->bounding_box[1];

	gr_set_color(255, 32, 32);

	enable_htl();

	// get into the frame of reference of the submodel
	int g3_count = 1;
	g3_start_instance_matrix(&objp->pos, &objp->orient, true);
	int mn = subobj_num;
	while ((mn >= 0) && (pm->submodel[mn].parent >= 0))
	{
		g3_start_instance_matrix(&pm->submodel[mn].offset, &pmi->submodel[mn].canonical_orient, true);
		g3_count++;
		mn = pm->submodel[mn].parent;
	}


	//draw a cube around the subsystem
	g3_draw_htl_line(&front_top_left, &front_top_right);
	g3_draw_htl_line(&front_top_right, &front_bot_right);
	g3_draw_htl_line(&front_bot_right, &front_bot_left);
	g3_draw_htl_line(&front_bot_left, &front_top_left);

	g3_draw_htl_line(&back_top_left, &back_top_right);
	g3_draw_htl_line(&back_top_right, &back_bot_right);
	g3_draw_htl_line(&back_bot_right, &back_bot_left);
	g3_draw_htl_line(&back_bot_left, &back_top_left);

	g3_draw_htl_line(&front_top_left, &back_top_left);
	g3_draw_htl_line(&front_top_right, &back_top_right);
	g3_draw_htl_line(&front_bot_left, &back_bot_left);
	g3_draw_htl_line(&front_bot_right, &back_bot_right);


	//draw another cube around a gun for a two-part turret
	if ((ss->system_info->turret_gun_sobj >= 0) && (ss->system_info->turret_gun_sobj != ss->system_info->subobj_num))
	{
		bsp_info *bsp_turret = &pm->submodel[ss->system_info->turret_gun_sobj];

		front_top_left = bsp_turret->bounding_box[7];
		front_top_right = bsp_turret->bounding_box[6];
		front_bot_left = bsp_turret->bounding_box[4];
		front_bot_right = bsp_turret->bounding_box[5];
		back_top_left = bsp_turret->bounding_box[3];
		back_top_right = bsp_turret->bounding_box[2];
		back_bot_left = bsp_turret->bounding_box[0];
		back_bot_right = bsp_turret->bounding_box[1];

		g3_start_instance_matrix(&bsp_turret->offset, &pmi->submodel[ss->system_info->turret_gun_sobj].canonical_orient, true);

		g3_draw_htl_line(&front_top_left, &front_top_right);
		g3_draw_htl_line(&front_top_right, &front_bot_right);
		g3_draw_htl_line(&front_bot_right, &front_bot_left);
		g3_draw_htl_line(&front_bot_left, &front_top_left);

		g3_draw_htl_line(&back_top_left, &back_top_right);
		g3_draw_htl_line(&back_top_right, &back_bot_right);
		g3_draw_htl_line(&back_bot_right, &back_bot_left);
		g3_draw_htl_line(&back_bot_left, &back_top_left);

		g3_draw_htl_line(&front_top_left, &back_top_left);
		g3_draw_htl_line(&front_top_right, &back_top_right);
		g3_draw_htl_line(&front_bot_left, &back_bot_left);
		g3_draw_htl_line(&front_bot_right, &back_bot_right);

		g3_done_instance(true);
	}

	for (int i = 0; i < g3_count; i++)
		g3_done_instance(true);

	disable_htl();

	// get text
	buf = ss->system_info->subobj_name;

	// add weapons if present
	for (int i = 0; i < ss->weapons.num_primary_banks; ++i)
	{
		int wi = ss->weapons.primary_bank_weapons[i];
		if (wi >= 0)
		{
			buf += "\n";
			buf += Weapon_info[wi].name;
		}
	}
	for (int i = 0; i < ss->weapons.num_secondary_banks; ++i)
	{
		int wi = ss->weapons.secondary_bank_weapons[i];
		if (wi >= 0)
		{
			buf += "\n";
			buf += Weapon_info[wi].name;
		}
	}

	//draw the text.  rotate the center of the subsystem into place before finding out where to put the text
	vec3d center_pt;
	vm_vec_unrotate(&center_pt, &bsp->offset, &objp->orient);
	vm_vec_add2(&center_pt, &objp->pos);
	g3_rotate_vertex(&text_center, &center_pt);
	g3_project_vertex(&text_center);
	gr_set_color_fast(&colour_white);
	gr_string( (int)text_center.screen.xyw.x,  (int)text_center.screen.xyw.y, buf.c_str() );
}

void render_active_rect(bool box_marking, const Marking_box& marking_box) {
	if (box_marking) {
		gr_set_color(255, 255, 255);
		gr_line(marking_box.x1, marking_box.y1, marking_box.x1, marking_box.y2);
		gr_line(marking_box.x1, marking_box.y2, marking_box.x2, marking_box.y2);
		gr_line(marking_box.x2, marking_box.y2, marking_box.x2, marking_box.y1);
		gr_line(marking_box.x2, marking_box.y1, marking_box.x1, marking_box.y1);
	}
}

void draw_compass_arrow(vec3d* v0) {
	vec3d v1 = vmd_zero_vector;
	vertex tv0, tv1;

	g3_rotate_vertex(&tv0, v0);
	g3_rotate_vertex(&tv1, &v1);
	g3_project_vertex(&tv0);
	g3_project_vertex(&tv1);
	//	tv0.sx = (tv0.sx - tv1.sx) * 1 + tv1.sx;
	//	tv0.sy = (tv0.sy - tv1.sy) * 1 + tv1.sy;
	g3_draw_line(&tv0, &tv1);
}

}

namespace fso {
namespace fred {
ViewSettings::ViewSettings() {
}

FredRenderer::FredRenderer(os::Viewport* targetView) : _targetView(targetView) {
	init_fred_colors();
}
FredRenderer::~FredRenderer() {
}
void FredRenderer::setViewport(EditorViewport* viewport) {
	Assertion(_viewport == nullptr, "Resetting viewport is not supported");
	Assertion(viewport != nullptr, "Invalid viewport specified!");

	_viewport = viewport;
}

void FredRenderer::render_grid(grid* gridp) {
	int i, ncols, nrows;

	enable_htl();
	gr_zbuffer_set(0);

	if (!grid_colors_inited) {
		grid_colors_inited = 1;

		gr_init_alphacolor(&Fred_grid_dark_aa, 64, 64, 64, 255);
		gr_init_alphacolor(&Fred_grid_bright_aa, 128, 128, 128, 255);
		gr_init_color(&Fred_grid_dark, 64, 64, 64);
		gr_init_color(&Fred_grid_bright, 128, 128, 128);
	}

	ncols = gridp->ncols;
	nrows = gridp->nrows;
	if (double_fine_gridlines) {
		ncols *= 2;
		nrows *= 2;
	}

	if (view().Aa_gridlines) {
		gr_set_color_fast(&Fred_grid_dark_aa);
	} else {
		gr_set_color_fast(&Fred_grid_dark);
	}

	//	Draw the column lines.
	for (i = 0; i <= ncols; i++) {
		g3_draw_htl_line(&gridp->gpoints1[i], &gridp->gpoints2[i]);
	}
	//	Draw the row lines.
	for (i = 0; i <= nrows; i++) {
		g3_draw_htl_line(&gridp->gpoints3[i], &gridp->gpoints4[i]);
	}

	ncols = gridp->ncols / 2;
	nrows = gridp->nrows / 2;

	// now draw the larger, brighter gridlines that is x10 the scale of smaller one.
	if (view().Aa_gridlines) {
		gr_set_color_fast(&Fred_grid_bright_aa);
	} else {
		gr_set_color_fast(&Fred_grid_bright);
	}

	for (i = 0; i <= ncols; i++) {
		g3_draw_htl_line(&gridp->gpoints5[i], &gridp->gpoints6[i]);
	}

	for (i = 0; i <= nrows; i++) {
		g3_draw_htl_line(&gridp->gpoints7[i], &gridp->gpoints8[i]);
	}

	disable_htl();
	gr_zbuffer_set(1);
}

void FredRenderer::hilight_bitmap() {
	/*
int i;
vertex p[4];

if (Starfield_bitmaps[Cur_bitmap].bitmap_index == -1)  // can't draw if no bitmap
return;

for (i=0; i<4; i++)
{
g3_rotate_faraway_vertex(&p[i], &Starfield_bitmaps[Cur_bitmap].points[i]);
if (p[i].codes & CC_BEHIND)
return;

g3_project_vertex(&p[i]);
if (p[i].flags & PF_OVERFLOW)
return;
}

gr_set_color(255, 255, 255);
g3_draw_line(&p[0], &p[1]);
g3_draw_line(&p[1], &p[2]);
g3_draw_line(&p[2], &p[3]);
g3_draw_line(&p[3], &p[0]);
*/
}

void FredRenderer::display_distances() {
	char buf[20];
	object *objp, *o2;
	vec3d pos;
	vertex v;


	gr_set_color(255, 0, 0);
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))
	{
		if (objp->flags[Object::Object_Flags::Marked])
		{
			o2 = GET_NEXT(objp);
			while (o2 != END_OF_LIST(&obj_used_list))
			{
				if (o2->flags[Object::Object_Flags::Marked])
				{
					rpd_line(&objp->pos, &o2->pos);
					vm_vec_avg(&pos, &objp->pos, &o2->pos);
					g3_rotate_vertex(&v, &pos);
					if (!(v.codes & CC_BEHIND))
						if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
							sprintf(buf, "%.1f", vm_vec_dist(&objp->pos, &o2->pos));
							gr_set_color_fast(&colour_white);
							gr_string((int)v.screen.xyw.x, (int)v.screen.xyw.y, buf);
						}
				}



				o2 = GET_NEXT(o2);
			}
		}

		objp = GET_NEXT(objp);
	}
}

void FredRenderer::display_ship_info(int cur_object_index) {
	char buf[512], pos[80];
	int render = 1;
	object* objp;
	vertex v;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		Assert(objp->type != OBJ_NONE);
		Fred_outline = 0;
		render = 1;
		if (OBJ_INDEX(objp) == cur_object_index) {
			Fred_outline = FRED_COLOUR_WHITE;
		} else if (objp->flags[Object::Object_Flags::Marked]) { // is it a marked object?
			Fred_outline = FRED_COLOUR_YELLOW_GREEN;
		} else {
			Fred_outline = 0;
		}

		if ((objp->type == OBJ_WAYPOINT) && !view().Show_waypoints) {
			render = 0;
		}

		if ((objp->type == OBJ_START) && !view().Show_starts) {
			render = 0;
		}

		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
			if (!view().Show_ships) {
				render = 0;
			}

			if (!view().Show_iff[Ships[objp->instance].team]) {
				render = 0;
			}
		}

		if (objp->flags[Object::Object_Flags::Hidden]) {
			render = 0;
		}

		g3_rotate_vertex(&v, &objp->pos);
		if (!(v.codes & CC_BEHIND) && render) {
			if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
				*buf = 0;
				if (view().Show_ship_info) {
					if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
						ship* shipp;
						int ship_type;

						shipp = &Ships[objp->instance];
						ship_type = shipp->ship_info_index;
						Assert(ship_type >= 0);
						sprintf(buf, "%s\n%s", shipp->ship_name, Ship_info[ship_type].short_name);
					} else if (objp->type == OBJ_WAYPOINT) {
						int idx;
						waypoint_list* wp_list = find_waypoint_list_with_instance(objp->instance, &idx);
						Assert(wp_list != NULL);
						sprintf(buf, "%s\nWaypoint %d", wp_list->get_name(), idx + 1);
					} else if (objp->type == OBJ_POINT) {
						if (objp->instance == BRIEFING_LOOKAT_POINT_ID)
							strcpy_s(buf, "Camera lookat point");
						else
							strcpy_s(buf, "Briefing icon");
					} else if (objp->type == OBJ_JUMP_NODE) {
						strcpy_s(buf, "Jump Node");
					} else
						Assert(0);
				}

				if (view().Show_coordinates) {
					sprintf(pos, "(%.0f,%.0f,%.0f)", objp->pos.xyz.x, objp->pos.xyz.y, objp->pos.xyz.z);
					if (*buf)
						strcat_s(buf, "\n");

					strcat_s(buf, pos);
				}

				if (*buf) {
					if (Fred_outline == FRED_COLOUR_WHITE) {
						gr_set_color_fast(&colour_green);
					} else if (Fred_outline == FRED_COLOUR_YELLOW_GREEN) {
						gr_set_color_fast(&colour_yellow_green);
					} else {
						gr_set_color_fast(&colour_white);
					}

					gr_string((int) v.screen.xyw.x, (int) v.screen.xyw.y, buf);
				}
			}
		}

		objp = GET_NEXT(objp);
	}
}

void FredRenderer::cancel_display_active_ship_subsystem(subsys_to_render& Render_subsys) {
	Render_subsys.do_render = false;
	Render_subsys.ship_obj = NULL;
	Render_subsys.cur_subsys = NULL;
}

void FredRenderer::display_active_ship_subsystem(subsys_to_render& Render_subsys, int cur_object_index) {
	if (cur_object_index != -1) {
		if (Objects[cur_object_index].type == OBJ_SHIP) {
			object* objp = &Objects[cur_object_index];

			// if this option is checked, we want to render info for all subsystems, not just the ones we select with K and Shift-K
			if (view().Highlight_selectable_subsys) {
				auto shipp = &Ships[objp->instance];

				for (auto ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list); ss = GET_NEXT(ss)) {
					if (ss->system_info->subobj_num != -1) {
						subsys_to_render s2r = { true, objp, ss };
						fredhtl_render_subsystem_bounding_box(&s2r);
					}
				}
			}
			// otherwise select individual subsystems, or not, as normal
			else {
				// switching to a new ship, so reset
				if (objp != Render_subsys.ship_obj) {
					cancel_display_active_ship_subsystem(Render_subsys);
					return;
				}

				if (Render_subsys.do_render) {
					fredhtl_render_subsystem_bounding_box(&Render_subsys);
				} else {
					cancel_display_active_ship_subsystem(Render_subsys);
				}
			}
		}
	}
}

void FredRenderer::render_compass() {
	if (!view().Show_compass) {
		return;
	}

	vec3d v, eye = vmd_zero_vector;

	gr_set_clip(gr_screen.max_w - 100, 0, 100, 100);
	g3_start_frame(0); // ** Accounted for
	// required !!!
	vm_vec_scale_add2(&eye, &_viewport->eye_orient.vec.fvec, -1.5f);
	g3_set_view_matrix(&eye, &_viewport->eye_orient, 1.0f);

	v.xyz.x = 1.0f;
	v.xyz.y = v.xyz.z = 0.0f;
	if (vm_vec_dot(&eye, &v) < 0.0f) {
		gr_set_color(159, 20, 20);
	} else {
		gr_set_color(255, 32, 32);
	}
	draw_compass_arrow(&v);

	v.xyz.y = 1.0f;
	v.xyz.x = v.xyz.z = 0.0f;
	if (vm_vec_dot(&eye, &v) < 0.0f) {
		gr_set_color(20, 159, 20);
	} else {
		gr_set_color(32, 255, 32);
	}
	draw_compass_arrow(&v);

	v.xyz.z = 1.0f;
	v.xyz.x = v.xyz.y = 0.0f;
	if (vm_vec_dot(&eye, &v) < 0.0f) {
		gr_set_color(20, 20, 159);
	} else {
		gr_set_color(32, 32, 255);
	}
	draw_compass_arrow(&v);

	g3_end_frame(); // ** Accounted for
}

void FredRenderer::draw_orient_sphere2(int col, object* obj, int r, int g, int b) {
	int flag = 0;
	vertex v;
	vec3d v1, v2;
	float size;

	size = fl_sqrt(vm_vec_dist(&_viewport->eye_pos, &obj->pos) / 20.0f);
	if (size < LOLLIPOP_SIZE) {
		size = LOLLIPOP_SIZE;
	}

	if ((obj->type != OBJ_WAYPOINT) && (obj->type != OBJ_POINT)) {
		flag = (vm_vec_dot(&_viewport->eye_orient.vec.fvec, &obj->orient.vec.fvec) < 0.0f);

		v1 = v2 = obj->pos;
		vm_vec_scale_add2(&v1, &obj->orient.vec.fvec, size);
		vm_vec_scale_add2(&v2, &obj->orient.vec.fvec, size * 1.5f);

		if (!flag) {
			gr_set_color(192, 192, 192);
			rpd_line(&v1, &v2);
		}
	}

	g3_rotate_vertex(&v, &obj->pos);
	if (!(v.codes & CC_BEHIND)) {
		if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
			gr_set_color((col >> 16) & 0xff, (col >> 8) & 0xff, col & 0xff);
			g3_draw_sphere(&v, size);
			gr_set_color(r, g, b);
			g3_draw_sphere(&v, size * 0.75f);
		}
	}

	if (flag) {
		gr_set_color(192, 192, 192);
		rpd_line(&v1, &v2);
	}
}

void FredRenderer::draw_orient_sphere(object* obj, int r, int g, int b) {
	int flag = 0;
	vertex v;
	vec3d v1, v2;
	float size;

	size = fl_sqrt(vm_vec_dist(&_viewport->eye_pos, &obj->pos) / 20.0f);
	if (size < LOLLIPOP_SIZE) {
		size = LOLLIPOP_SIZE;
	}

	if ((obj->type != OBJ_WAYPOINT) && (obj->type != OBJ_POINT)) {
		flag = (vm_vec_dot(&_viewport->eye_orient.vec.fvec, &obj->orient.vec.fvec) < 0.0f);
		v1 = v2 = obj->pos;
		vm_vec_scale_add2(&v1, &obj->orient.vec.fvec, size);
		vm_vec_scale_add2(&v2, &obj->orient.vec.fvec, size * 1.5f);

		if (!flag) {
			gr_set_color(192, 192, 192);
			rpd_line(&v1, &v2);
		}
	}

	gr_set_color(r, g, b);
	g3_rotate_vertex(&v, &obj->pos);
	if (!(v.codes & CC_BEHIND)) {
		if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
			g3_draw_sphere(&v, size);
		}
	}

	if (flag) {
		gr_set_color(192, 192, 192);
		rpd_line(&v1, &v2);
	}
}

void FredRenderer::render_model_x(vec3d* pos, grid* gridp, int  /*col_scheme*/) {
	vec3d gpos; //	Location of point on grid.
	vec3d tpos;
	float dxz;
	plane tplane;
	vec3d* gv;

	if (!view().Show_grid_positions) {
		return;
	}

	tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&gpos, &tplane, pos);
	dxz = vm_vec_dist(pos, &gpos) / 8.0f;
	gv = &gridp->gmatrix.vec.uvec;
	if (gv->xyz.x * pos->xyz.x + gv->xyz.y * pos->xyz.y + gv->xyz.z * pos->xyz.z < -gridp->planeD) {
		gr_set_color(0, 127, 0);
	} else {
		gr_set_color(192, 192, 192);
	}


	rpd_line(&gpos, pos); //	Line from grid to object center.

	tpos = gpos;

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, -dxz / 2);
	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.fvec, -dxz / 2);

	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, dxz / 2);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.fvec, dxz / 2);

	rpd_line(&gpos, &tpos);

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, dxz);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, -dxz);

	rpd_line(&gpos, &tpos);
}

void FredRenderer::render_model_x_htl(vec3d* pos, grid* gridp, int  /*col_scheme*/) {
	vec3d gpos; //	Location of point on grid.
	vec3d tpos;
	float dxz;
	plane tplane;
	vec3d* gv;

	if (!view().Show_grid_positions) {
		return;
	}

	tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&gpos, &tplane, pos);
	dxz = vm_vec_dist(pos, &gpos) / 8.0f;
	gv = &gridp->gmatrix.vec.uvec;
	if (gv->xyz.x * pos->xyz.x + gv->xyz.y * pos->xyz.y + gv->xyz.z * pos->xyz.z < -gridp->planeD) {
		gr_set_color(0, 127, 0);
	} else {
		gr_set_color(192, 192, 192);
	}


	g3_draw_htl_line(&gpos, pos); //	Line from grid to object center.

	tpos = gpos;

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, -dxz / 2);
	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.fvec, -dxz / 2);

	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, dxz / 2);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.fvec, dxz / 2);

	g3_draw_htl_line(&gpos, &tpos);

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, dxz);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, -dxz);

	g3_draw_htl_line(&gpos, &tpos);
}

void FredRenderer::render_one_model_htl(object* objp,
										int cur_object_index,
										bool Bg_bitmap_dialog) {
	int j, z;
	object* o2;

	Assert(objp->type != OBJ_NONE);

	if (objp->type == OBJ_JUMP_NODE) {
		return;
	}

	if ((objp->type == OBJ_WAYPOINT) && !view().Show_waypoints) {
		return;
	}

	if ((objp->type == OBJ_START) && !view().Show_starts) {
		return;
	}

	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
		if (!view().Show_ships) {
			return;
		}

		if (!view().Show_iff[Ships[objp->instance].team]) {
			return;
		}
	}

	if (objp->flags[Object::Object_Flags::Hidden]) {
		return;
	}

	Fred_outline = 0;

	if (!view().Draw_outlines_on_selected_ships && ((OBJ_INDEX(objp) == cur_object_index) || (objp->flags[Object::Object_Flags::Marked]))) {
		/* don't draw the outlines we would normally draw */;
	} else if ((OBJ_INDEX(objp) == cur_object_index) && !Bg_bitmap_dialog) {
		Fred_outline = FRED_COLOUR_WHITE;
	} else if ((objp->flags[Object::Object_Flags::Marked]) && !Bg_bitmap_dialog) { // is it a marked object?
		Fred_outline = FRED_COLOUR_YELLOW_GREEN;
	} else if ((objp->type == OBJ_SHIP) && view().Show_outlines) {
		color* iff_color = iff_get_color_by_team_and_object(Ships[objp->instance].team, -1, 1, objp);

		Fred_outline = (iff_color->red << 16) | (iff_color->green << 8) | (iff_color->blue);
	} else if ((objp->type == OBJ_START) && view().Show_outlines) {
		Fred_outline = 0x007f00;
	} else {
		Fred_outline = 0;
	}

	// build flags
	if ((view().Show_ship_models || view().Show_outlines) && ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))) {
		g3_start_instance_matrix(&Eye_position, &Eye_matrix, 0);
		if (view().Show_ship_models) {
			j = MR_NORMAL;
		} else {
			j = MR_NO_POLYS;
		}

		uint debug_flags = 0;
		if (view().Show_dock_points) {
			debug_flags |= MR_DEBUG_BAY_PATHS;
		}

		if (view().Show_paths_fred) {
			debug_flags |= MR_DEBUG_PATHS;
		}

		z = objp->instance;

		model_clear_instance(Ship_info[Ships[z].ship_info_index].model_num);

		if (!view().Lighting_on) {
			j |= MR_NO_LIGHTING;
		}

		if (view().FullDetail) {
			j |= MR_FULL_DETAIL;
		}

		if (Fred_outline) {
			j |= MR_SHOW_OUTLINE_HTL;
		}

		model_render_params render_info;
		render_info.set_debug_flags(debug_flags);
		render_info.set_color(Fred_outline >> 16, (Fred_outline >> 8) & 0xff, Fred_outline & 0xff);
		render_info.set_replacement_textures(Ships[z].ship_replacement_textures);
		render_info.set_flags(j);

		g3_done_instance(0);

		model_render_immediate(&render_info, Ship_info[Ships[z].ship_info_index].model_num, &objp->orient, &objp->pos);
	} else {
		int r = 0, g = 0, b = 0;

		if (objp->type == OBJ_SHIP) {
			if (!view().Show_ships) {
				return;
			}

			color* iff_color = iff_get_color_by_team_and_object(Ships[objp->instance].team, -1, 1, objp);

			r = iff_color->red;
			g = iff_color->green;
			b = iff_color->blue;
		} else if (objp->type == OBJ_START) {
			r = 0;
			g = 127;
			b = 0;
		} else if (objp->type == OBJ_WAYPOINT) {
			r = 96;
			g = 0;
			b = 112;
		} else if (objp->type == OBJ_POINT) {
			if (objp->instance != BRIEFING_LOOKAT_POINT_ID) {
				///! \fixme Briefing stuff!
				//Assert(Briefing_dialog);
				return;
			}

			r = 196;
			g = 32;
			b = 196;
		} else
			Assert(0);

		float size = fl_sqrt(vm_vec_dist(&_viewport->eye_pos, &objp->pos) / 20.0f);

		if (size < LOLLIPOP_SIZE) {
			size = LOLLIPOP_SIZE;
		}

		if (Fred_outline) {
			gr_set_color(std::min(r * 2, 255), std::min(g * 2, 255), std::min(b * 2, 255));
			g3_draw_htl_sphere(&objp->pos, size * 1.5f);
		} else {
			gr_set_color(r, g, b);
			g3_draw_htl_sphere(&objp->pos, size);
		}
	}

	if (objp->type == OBJ_WAYPOINT) {
		for (auto objIdx : rendering_order) {
			o2 = &Objects[objIdx];
			if (o2->type == OBJ_WAYPOINT) {
				if ((o2->instance == objp->instance - 1) || (o2->instance == objp->instance + 1)) {
					g3_draw_htl_line(&o2->pos, &objp->pos);
				}
			}
		}
	}

	render_model_x_htl(&objp->pos, _viewport->The_grid);
	rendering_order.push_back(OBJ_INDEX(objp));
}

void FredRenderer::render_models(int cur_object_index,
								 bool Bg_bitmap_dialog) {
	gr_set_color_fast(&colour_white);

	rendering_order.clear();

	if ((ENVMAP == -1) && strlen(The_mission.envmap_name)) {
		ENVMAP = bm_load(The_mission.envmap_name);
	}

	bool f = false;
	enable_htl();

	auto render_function = [&](object* objp) {
		this->render_one_model_htl(objp,
								   cur_object_index,
								   Bg_bitmap_dialog);
	};

	obj_render_all(render_function, &f);

	disable_htl();

	///! \fixme Handle briefing stuff properly.
#if 0
    if (Briefing_dialog)
    {
        obj_render_all(render_one_model_briefing_screen, &f);
        Briefing_dialog->batch_render();
    }
#endif
}

void FredRenderer::render_frame(int cur_object_index,
								subsys_to_render& Render_subsys,
								bool box_marking,
								const Marking_box& marking_box,
								bool Bg_bitmap_dialog) {

	// Make sure our OpenGL context is used for rendering
	gr_use_viewport(_targetView);

	// Resize the rendering window in case the previous size was different
	gr_screen_resize(_targetView->getSize().first, _targetView->getSize().second);

	char buf[256];
	int x, y, w, h, inst;
	vec3d pos;
	vertex v;
	angles a, a_deg; //a is in rads, a_deg is in degrees

	if (g3_in_frame())
		g3_end_frame(); // ** Accounted for

	gr_reset_clip();
	gr_clear();
	///! \fixme Briefing related!
#if 0
    if (Briefing_dialog) {
        CRect rect;

        Fred_main_wnd->GetClientRect(rect);
        True_rw = rect.Width();
        True_rh = rect.Height();
        if (Fixed_briefing_size) {
            True_rw = Briefing_window_resolution[0];
            True_rh = Briefing_window_resolution[1];

        }
        else {
            if ((float)True_rh / (float)True_rw > (float)Briefing_window_resolution[1] / (float)Briefing_window_resolution[0]) {
                True_rh = (int)((float)Briefing_window_resolution[1] * (float)True_rw / (float)Briefing_window_resolution[0]);

            }
            else {  // Fred is wider than briefing window
                True_rw = (int)((float)Briefing_window_resolution[0] * (float)True_rh / (float)Briefing_window_resolution[1]);
            }
        }

        g3_start_frame(0); // ** Accounted for
        gr_set_color(255, 255, 255);
        gr_line(0, True_rh, True_rw, True_rh);
        gr_line(True_rw, 0, True_rw, True_rh);
        g3_end_frame();	 // ** Accounted for
        gr_set_clip(0, 0, True_rw, True_rh);
    }
#endif

	g3_start_frame(1); // ** Accounted for
	// 1 means use zbuffering

	font::set_font(font::FONT1);
	light_reset();

	g3_set_view_matrix(&_viewport->eye_pos, &_viewport->eye_orient, 0.5f);

	enable_htl();
	if (Bg_bitmap_dialog) {
		stars_draw(view().Show_stars, 1, view().Show_stars, 0, 0);
	} else {
		stars_draw(view().Show_stars, view().Show_stars, view().Show_stars, 0, 0);
	}
	disable_htl();

	if (view().Show_horizon) {
		gr_set_color(128, 128, 64);
		g3_draw_horizon_line();
	}

	if (view().Show_asteroid_field) {
		gr_set_color(192, 96, 16);
		draw_asteroid_field();
	}

	if (view().Show_grid) {
		render_grid(_viewport->The_grid);
	}
	if (Bg_bitmap_dialog) {
		hilight_bitmap();
	}

	gr_set_color(0, 0, 64);
	render_models(cur_object_index,
				  Bg_bitmap_dialog);

	if (view().Show_distances) {
		display_distances();
	}

	display_ship_info(cur_object_index);
	display_active_ship_subsystem(Render_subsys, cur_object_index);
	render_active_rect(box_marking, marking_box);

	if (query_valid_object(_viewport->Cursor_over)) { // display a tool-tip like infobox
		pos = Objects[_viewport->Cursor_over].pos;
		inst = Objects[_viewport->Cursor_over].instance;
		if ((Objects[_viewport->Cursor_over].type == OBJ_SHIP) || (Objects[_viewport->Cursor_over].type == OBJ_START)) {
			vm_extract_angles_matrix(&a, &Objects[_viewport->Cursor_over].orient);

			a_deg.h = a.h * CONVERT_DEGREES; // convert angles to more readable degrees
			a_deg.p = a.p * CONVERT_DEGREES;
			a_deg.b = a.b * CONVERT_DEGREES;

			sprintf(buf,
					"%s\n%s\n( %.1f , %.1f , %.1f ) \nPitch: %.2f\nBank: %.2f\nHeading: %.2f",
					Ships[inst].ship_name,
					Ship_info[Ships[inst].ship_info_index].short_name,
					pos.xyz.x,
					pos.xyz.y,
					pos.xyz.z,
					a_deg.p,
					a_deg.b,
					a_deg.h);
		} else if (Objects[_viewport->Cursor_over].type == OBJ_WAYPOINT) {
			int idx;
			waypoint_list* wp_list = find_waypoint_list_with_instance(inst, &idx);
			Assert(wp_list != NULL);
			sprintf(buf,
					"%s\nWaypoint %d\n( %.1f , %.1f , %.1f ) ",
					wp_list->get_name(),
					idx + 1,
					pos.xyz.x,
					pos.xyz.y,
					pos.xyz.z);
		} else if (Objects[_viewport->Cursor_over].type == OBJ_POINT) {
			sprintf(buf, "Briefing icon\n( %.1f , %.1f , %.1f ) ", pos.xyz.x, pos.xyz.y, pos.xyz.z);
		} else {
			sprintf(buf, "( %.1f , %.1f , %.1f ) ", pos.xyz.x, pos.xyz.y, pos.xyz.z);
		}

		g3_rotate_vertex(&v, &pos);
		if (!(v.codes & CC_BEHIND)) {
			if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
				gr_get_string_size(&w, &h, buf);

				x = (int) v.screen.xyw.x;
				y = (int) v.screen.xyw.y + 20;

				gr_set_color_fast(&colour_white);
				gr_rect(x - 7, y - 6, w + 8, h + 7);

				gr_set_color_fast(&colour_black);
				gr_rect(x - 5, y - 5, w + 5, h + 5);

				gr_set_color_fast(&colour_white);
				gr_string(x, y, buf);
			}
		}
	}

	gr_set_color(0, 160, 0);

	enable_htl();
	jumpnode_render_all();
	disable_htl();

	sprintf(buf, "(%.1f,%.1f,%.1f)", _viewport->eye_pos.xyz.x, _viewport->eye_pos.xyz.y, _viewport->eye_pos.xyz.z);
	gr_get_string_size(&w, &h, buf);
	gr_set_color_fast(&colour_white);
	gr_string(gr_screen.max_w - w - 2, 2, buf);

	g3_end_frame(); // ** Accounted for
	render_compass();

	gr_flip();

	gr_reset_clip();
	///! \fixme Briefing related!
#if 0
    if (Briefing_dialog)
        gr_set_clip(0, 0, True_rw, True_rh);
#endif

	g3_start_frame(0); // ** Accounted for
	g3_set_view_matrix(&_viewport->eye_pos, &_viewport->eye_orient, 0.5f);
}
void FredRenderer::resize(int width, int height) {
	// Make sure the following call targets the right view port
	gr_use_viewport(_targetView);

	gr_screen_resize(width, height);

	// We need to rerender the scene now
	scheduleUpdate();
}
ViewSettings& FredRenderer::view() {
	return _viewport->view;
}

}
}
