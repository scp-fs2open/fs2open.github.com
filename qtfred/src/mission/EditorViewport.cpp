//
//

#include <globalincs/linklist.h>
#include <object/object.h>
#include <render/3d.h>
#include <ship/ship.h>
#include <io/key.h>

#include "object.h"

#include "EditorViewport.h"
#include <math/fvi.h>

namespace {

const fix MAX_FRAMETIME = (F1_0 / 4); // Frametime gets saturated at this.
const fix MIN_FRAMETIME = (F1_0 / 120);

void process_movement_keys(int key, vec3d* mvec, angles* angs) {
	int raw_key;

	mvec->xyz.x = 0.0f;
	mvec->xyz.y = 0.0f;
	mvec->xyz.z = 0.0f;
	angs->p = 0.0f;
	angs->b = 0.0f;
	angs->h = 0.0f;

	raw_key = key & 0xff;

	switch (raw_key) {
	case KEY_PAD1:
		mvec->xyz.x += -1.0f;
		break;
	case KEY_PAD3:
		mvec->xyz.x += +1.0f;
		break;
	case KEY_PADPLUS:
		mvec->xyz.y += -1.0f;
		break;
	case KEY_PADMINUS:
		mvec->xyz.y += +1.0f;
		break;
	case KEY_A:
		mvec->xyz.z += +1.0f;
		break;
	case KEY_Z:
		mvec->xyz.z += -1.0f;
		break;
	case KEY_PAD4:
		angs->h += -0.1f;
		break;
	case KEY_PAD6:
		angs->h += +0.1f;
		break;
	case KEY_PAD8:
		angs->p += -0.1f;
		break;
	case KEY_PAD2:
		angs->p += +0.1f;
		break;
	case KEY_PAD7:
		angs->b += -0.1f;
		break;
	case KEY_PAD9:
		angs->b += +0.1f;
		break;
	}

	if (key & KEY_SHIFTED) {
		vm_vec_scale(mvec, 5.0f);
		angs->p *= 5.0f;
		angs->b *= 5.0f;
		angs->h *= 5.0f;
	}
}
void align_vector_to_axis(vec3d* v) {
	float x, y, z;

	x = v->xyz.x;
	if (x < 0) {
		x = -x;
	}

	y = v->xyz.y;
	if (y < 0) {
		y = -y;
	}

	z = v->xyz.z;
	if (z < 0) {
		z = -z;
	}

	if ((x > y) && (x > z)) { // x axis
		if (v->xyz.x < 0) // negative x
			vm_vec_make(v, -1.0f, 0.0f, 0.0f);
		else // positive x
			vm_vec_make(v, 1.0f, 0.0f, 0.0f);
	}
	else if (y > z) { // y axis
		if (v->xyz.y < 0) // negative y
			vm_vec_make(v, 0.0f, -1.0f, 0.0f);
		else // positive y
			vm_vec_make(v, 0.0f, 1.0f, 0.0f);
	}
	else { // z axis
		if (v->xyz.z < 0) // negative z
			vm_vec_make(v, 0.0f, 0.0f, -1.0f);
		else // positive z
			vm_vec_make(v, 0.0f, 0.0f, 1.0f);
	}
}
void verticalize_object(matrix* orient) {
	align_vector_to_axis(&orient->vec.fvec);
	align_vector_to_axis(&orient->vec.uvec);
	align_vector_to_axis(&orient->vec.rvec);
	vm_fix_matrix(orient); // just in case something odd occurs.
}

}

namespace fso {
namespace fred {

EditorViewport::EditorViewport(Editor* in_editor, std::unique_ptr<FredRenderer>&& in_renderer) :
	_renderer(std::move(in_renderer)), editor(in_editor) {
	renderer = _renderer.get();

	_renderer->setViewport(this);

	vm_vec_make(&Constraint, 1.0f, 0.0f, 1.0f);
	vm_vec_make(&Anticonstraint, 0.0f, 1.0f, 0.0f);
	resetView();
}
void EditorViewport::needsUpdate() {
	_renderer->scheduleUpdate();
}
void EditorViewport::resetViewPhysics() {
	physics_init(&view_physics);
	view_physics.max_vel.xyz.x *= physics_speed / 3.0f;
	view_physics.max_vel.xyz.y *= physics_speed / 3.0f;
	view_physics.max_vel.xyz.z *= physics_speed / 3.0f;
	view_physics.max_rear_vel *= physics_speed / 3.0f;
	view_physics.max_rotvel.xyz.x *= physics_rot / 30.0f;
	view_physics.max_rotvel.xyz.y *= physics_rot / 30.0f;
	view_physics.max_rotvel.xyz.z *= physics_rot / 30.0f;
	view_physics.flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;
}
void EditorViewport::select_objects(const Marking_box& box) {
	int	x, y, valid, icon_mode = 0;
	vertex	v;
	object	*ptr;

	// Copy this so we can modify it
	auto marking_box = box;

	if (marking_box.x1 > marking_box.x2) {
		x = marking_box.x1;
		marking_box.x1 = marking_box.x2;
		marking_box.x2 = x;
	}

	if (marking_box.y1 > marking_box.y2) {
		y = marking_box.y1;
		marking_box.y1 = marking_box.y2;
		marking_box.y2 = y;
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		valid = 1;
		if (ptr->flags[Object::Object_Flags::Hidden])
			valid = 0;

		Assert(ptr->type != OBJ_NONE);
		switch (ptr->type) {
		case OBJ_WAYPOINT:
			if (!Show_waypoints)
				valid = 0;
			break;

		case OBJ_START:
			if (!view.Show_starts || !view.Show_ships)
				valid = 0;
			break;

		case OBJ_SHIP:
			if (!view.Show_ships)
				valid = 0;

			if (!view.Show_iff[Ships[ptr->instance].team])
				valid = 0;

			break;
		}

		g3_rotate_vertex(&v, &ptr->pos);
		if (!(v.codes & CC_BEHIND) && valid)
			if (!(g3_project_vertex(&v) & PF_OVERFLOW)) {
				x = (int) v.screen.xyw.x;
				y = (int) v.screen.xyw.y;

				if (x >= marking_box.x1 && x <= marking_box.x2 && y >= marking_box.y1 && y <= marking_box.y2) {
					if (ptr->flags[Object::Object_Flags::Marked])
						editor->unmarkObject(OBJ_INDEX(ptr));
					else
						editor->markObject(OBJ_INDEX(ptr));

					if (ptr->type == OBJ_POINT)
						icon_mode = 1;
				}
			}

		ptr = GET_NEXT(ptr);
	}

	if (icon_mode) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if ((ptr->flags[Object::Object_Flags::Marked]) && (ptr->type != OBJ_POINT))
				editor->unmarkObject(OBJ_INDEX(ptr));

			ptr = GET_NEXT(ptr);
		}
	}

	needsUpdate();
}

void EditorViewport::resetView() {
	my_pos = vmd_zero_vector;
	my_pos.xyz.z = -5.0f;
	vec3d f, u, r;

	physics_init(&view_physics);
	view_physics.max_vel.xyz.z = 5.0f; //forward/backward
	view_physics.max_rotvel.xyz.x = 1.5f; //pitch
	memset(&view_controls, 0, sizeof(control_info));

	vm_vec_make(&view_pos, 0.0f, 150.0f, -200.0f);
	vm_vec_make(&f, 0.0f, -0.5f, 0.866025404f); // 30 degree angle
	vm_vec_make(&u, 0.0f, 0.866025404f, 0.5f);
	vm_vec_make(&r, 1.0f, 0.0f, 0.0f);
	vm_vector_2_matrix(&view_orient, &f, &u, &r);

	The_grid = create_default_grid();
	maybe_create_new_grid(The_grid, &view_pos, &view_orient, 1);
	//	vm_set_identity(&view_orient);
}


void EditorViewport::move_mouse(int btn, int mdx, int mdy) {
	int dx, dy;

	dx = mdx - last_x;
	dy = mdy - last_y;
	last_x = mdx;
	last_y = mdy;

	if (btn & 1) {
		matrix tempm, mousem;

		if (dx || dy) {
			vm_trackball(dx, dy, &mousem);
			vm_matrix_x_matrix(&tempm, &trackball_orient, &mousem);
			trackball_orient = tempm;
			view_orient = trackball_orient;
		}
	}

	if (btn & 2) {
		my_pos.xyz.z += (float)dy;
	}
}

///////////////////////////////////////////////////
void EditorViewport::process_system_keys(int key) {
	//	mprintf(("Key = %d\n", key));
	switch (key) {
	case KEY_LAPOSTRO:
		///! \todo cycle through axis-constraints for rotations.
		//CFREDView::GetView()->cycle_constraint();
		break;

	case KEY_R: // for some stupid reason, an accelerator for 'R' doesn't work.
				///! \todo Change editing mode to 'move and rotate'.
				//Editing_mode = 2;
		break;

	case KEY_SPACEBAR:
		///! \todo Toggle selection lock.
		//Selection_lock = !Selection_lock;
		break;

	case KEY_ESC:
		///! \todo Cancel drag.
		//if (button_down)
		//	cancel_drag();

		break;
	}
}

void EditorViewport::process_controls(vec3d* pos, matrix* orient, float frametime, int key, int mode) {
	if (Flying_controls_mode) {
		grid_read_camera_controls(&view_controls, frametime);

		if (key_get_shift_status()) {
			memset(&view_controls, 0, sizeof(control_info));
		}

		if ((fabs(view_controls.pitch) > (frametime / 100)) || (fabs(view_controls.vertical) > (frametime / 100))
			|| (fabs(view_controls.heading) > (frametime / 100)) || (fabs(view_controls.sideways) > (frametime / 100))
			|| (fabs(view_controls.bank) > (frametime / 100)) || (fabs(view_controls.forward) > (frametime / 100))) {
			needsUpdate();
		}

		//view_physics.flags |= (PF_ACCELERATES | PF_SLIDE_ENABLED);
		physics_read_flying_controls(orient, &view_physics, &view_controls, frametime);
		if (mode) {
			physics_sim_editor(pos, orient, &view_physics, frametime);
		}
		else {
			physics_sim(pos, orient, &view_physics, frametime);
		}
	}
	else {
		vec3d movement_vec, rel_movement_vec;
		angles rotangs;
		matrix newmat, rotmat;

		process_movement_keys(key, &movement_vec, &rotangs);
		vm_vec_rotate(&rel_movement_vec, &movement_vec, &The_grid->gmatrix);
		vm_vec_add2(pos, &rel_movement_vec);

		vm_angles_2_matrix(&rotmat, &rotangs);
		if (rotangs.h && view.Universal_heading) {
			vm_transpose(orient);
		}
		vm_matrix_x_matrix(&newmat, orient, &rotmat);
		*orient = newmat;
		if (rotangs.h && view.Universal_heading) {
			vm_transpose(orient);
		}
	}
}

/**
* @brief Increments mission time
*
* @details This only increments the mission time if the time difference is greater than the minimum frametime to avoid
* excessive computation
*
* @return @c true if the mission time was incremented, @c false otherwise.
*/
bool EditorViewport::inc_mission_time() {
	fix thistime = timer_get_fixed_seconds();
	fix time_diff; // This holds the computed time difference since the last time this function was called
	if (!lasttime) {
		time_diff = F1_0 / 30;
	}
	else {
		time_diff = thistime - lasttime;
	}

	if (time_diff > MAX_FRAMETIME) {
		time_diff = MAX_FRAMETIME;
	}
	else if (time_diff < MIN_FRAMETIME) {
		return false;
	}

	Frametime = time_diff;
	Missiontime += Frametime;
	lasttime = thistime;

	return true;
}

void EditorViewport::game_do_frame(const int cur_object_index) {
	int key, cmode;
	vec3d viewer_position, control_pos;
	object* objp;
	matrix control_orient;

	if (!inc_mission_time()) {
		// Don't do anything if the mission time wasn't incremented
		return;
	}

	viewer_position = my_orient.vec.fvec;
	vm_vec_scale(&viewer_position, my_pos.xyz.z);

	if ((viewpoint == 1) && !query_valid_object(view_obj)) {
		viewpoint = 0;
	}

	key = key_inkey();
	process_system_keys(key);
	cmode = Control_mode;
	if ((viewpoint == 1) && !cmode) {
		cmode = 2;
	}

	control_pos = Last_control_pos;
	control_orient = Last_control_orient;

	//	if ((key & KEY_MASK) == key)  // unmodified
	switch (cmode) {
	case 0: //	Control the viewer's location and orientation
		process_controls(&view_pos, &view_orient, f2fl(Frametime), key, 1);
		control_pos = view_pos;
		control_orient = view_orient;
		break;

	case 2: // Control viewpoint object
		process_controls(&Objects[view_obj].pos, &Objects[view_obj].orient, f2fl(Frametime), key);
		object_moved(&Objects[view_obj]);
		control_pos = Objects[view_obj].pos;
		control_orient = Objects[view_obj].orient;
		break;

	case 1: //	Control the current object's location and orientation
		if (query_valid_object(cur_object_index)) {
			vec3d delta_pos, leader_old_pos;
			matrix leader_orient, leader_transpose, tmp;
			object* leader;

			leader = &Objects[cur_object_index];
			leader_old_pos = leader->pos; // save original position
			leader_orient = leader->orient; // save original orientation
			vm_copy_transpose(&leader_transpose, &leader_orient);

			process_controls(&leader->pos, &leader->orient, f2fl(Frametime), key);
			vm_vec_sub(&delta_pos, &leader->pos, &leader_old_pos); // get position change
			control_pos = leader->pos;
			control_orient = leader->orient;

			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				Assert(objp->type != OBJ_NONE);
				if ((objp->flags[Object::Object_Flags::Marked]) && (cur_object_index != OBJ_INDEX(objp))) {
					if (Group_rotate) {
						matrix rot_trans;
						vec3d tmpv1, tmpv2;

						// change rotation matrix to rotate in opposite direction.  This rotation
						// matrix is what the leader ship has rotated by.
						vm_copy_transpose(&rot_trans, &view_physics.last_rotmat);

						// get point relative to our point of rotation (make POR the origin).  Since
						// only the leader has been moved yet, and not the objects, we have to use
						// the old leader's position.
						vm_vec_sub(&tmpv1, &objp->pos, &leader_old_pos);

						// convert point from real-world coordinates to leader's relative coordinate
						// system (z=forward vec, y=up vec, x=right vec
						vm_vec_rotate(&tmpv2, &tmpv1, &leader_orient);

						// now rotate the point by the transpose from above.
						vm_vec_rotate(&tmpv1, &tmpv2, &rot_trans);

						// convert point back into real-world coordinates
						vm_vec_rotate(&tmpv2, &tmpv1, &leader_transpose);

						// and move origin back to real-world origin.  Object is now at its correct
						// position.  Note we used the leader's new position, instead of old position.
						vm_vec_add(&objp->pos, &leader->pos, &tmpv2);

						// Now fix the object's orientation to what it should be.
						vm_matrix_x_matrix(&tmp, &objp->orient, &view_physics.last_rotmat);
						vm_orthogonalize_matrix(&tmp); // safety check
						objp->orient = tmp;
					}
					else {
						vm_vec_add2(&objp->pos, &delta_pos);
						vm_matrix_x_matrix(&tmp, &objp->orient, &view_physics.last_rotmat);
						objp->orient = tmp;
					}
				}

				objp = GET_NEXT(objp);
			}

			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (objp->flags[Object::Object_Flags::Marked]) {
					object_moved(objp);
				}

				objp = GET_NEXT(objp);
			}

			// Notify the editor that the mission has changed
			editor->missionChanged();
		}

		break;

	default:
		Assert(0);
	}

	if (Lookat_mode && query_valid_object(cur_object_index)) {
		float dist;

		dist = vm_vec_dist(&view_pos, &Objects[cur_object_index].pos);
		vm_vec_scale_add(&view_pos, &Objects[cur_object_index].pos, &view_orient.vec.fvec, -dist);
	}

	switch (viewpoint) {
	case 0:
		eye_pos = view_pos;
		eye_orient = view_orient;
		break;

	case 1:
		eye_pos = Objects[view_obj].pos;
		eye_orient = Objects[view_obj].orient;
		break;

	default:
		Assert(0);
	}

	maybe_create_new_grid(The_grid, &eye_pos, &eye_orient);

	if (Cursor_over != Last_cursor_over) {
		Last_cursor_over = Cursor_over;
		needsUpdate();
	}

	// redraw screen if controlled object moved or rotated
	if (vm_vec_cmp(&control_pos, &Last_control_pos) || vm_matrix_cmp(&control_orient, &Last_control_orient)) {
		needsUpdate();
		Last_control_pos = control_pos;
		Last_control_orient = control_orient;
	}

	// redraw screen if current viewpoint moved or rotated
	if (vm_vec_cmp(&eye_pos, &Last_eye_pos) || vm_matrix_cmp(&eye_orient, &Last_eye_orient)) {
		needsUpdate();
		Last_eye_pos = eye_pos;
		Last_eye_orient = eye_orient;
	}
}

void EditorViewport::level_controlled() {
	int cmode, count = 0;
	object* objp;

	cmode = Control_mode;
	if ((viewpoint == 1) && !cmode) {
		cmode = 2;
	}

	switch (cmode) {
	case 0: //	Control the viewer's location and orientation
		level_object(&view_orient);
		break;

	case 2: // Control viewpoint object
		level_object(&Objects[view_obj].orient);
		object_moved(&Objects[view_obj]);
		///! \todo Notify.
		editor->missionChanged();
		//FREDDoc_ptr->autosave("level object");
		break;

	case 1: //	Control the current object's location and orientation
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				level_object(&objp->orient);
			}

			objp = GET_NEXT(objp);
		}

		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				object_moved(objp);
				count++;
			}

			objp = GET_NEXT(objp);
		}

		///! \todo Notify.
		if (count) {
			/*
			if (count > 1)
			FREDDoc_ptr->autosave("level objects");
			else
			FREDDoc_ptr->autosave("level object");
			*/

			editor->missionChanged();
		}

		break;
	}

	return;
}

void EditorViewport::verticalize_controlled() {
	int cmode, count = 0;
	object* objp;

	cmode = Control_mode;
	if ((viewpoint == 1) && !cmode) {
		cmode = 2;
	}

	switch (cmode) {
	case 0: //	Control the viewer's location and orientation
		verticalize_object(&view_orient);
		break;

	case 2: // Control viewpoint object
		verticalize_object(&Objects[view_obj].orient);
		object_moved(&Objects[view_obj]);
		///! \todo notify.
		//FREDDoc_ptr->autosave("align object");
		editor->missionChanged();
		break;

	case 1: //	Control the current object's location and orientation
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				verticalize_object(&objp->orient);
			}

			objp = GET_NEXT(objp);
		}

		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				object_moved(objp);
				count++;
			}

			objp = GET_NEXT(objp);
		}

		///! \todo Notify.
		if (count) {
			/*
			if (count > 1)
			FREDDoc_ptr->autosave("align objects");
			else
			FREDDoc_ptr->autosave("align object");
			*/

			editor->missionChanged();
		}

		break;
	}

	return;
}

void EditorViewport::level_object(matrix* orient) {
	vec3d u;

	u = orient->vec.uvec = The_grid->gmatrix.vec.uvec;
	if (u.xyz.x) // y-z plane
	{
		orient->vec.fvec.xyz.x = orient->vec.rvec.xyz.x = 0.0f;
	}
	else if (u.xyz.y) { // x-z plane
		orient->vec.fvec.xyz.y = orient->vec.rvec.xyz.y = 0.0f;
	}
	else if (u.xyz.z) { // x-y plane
		orient->vec.fvec.xyz.z = orient->vec.rvec.xyz.z = 0.0f;
	}

	vm_fix_matrix(orient);
}

int EditorViewport::object_check_collision(object* objp,
	vec3d* p0,
	vec3d* p1,
	vec3d* hitpos) {
	mc_info mc;
	mc_info_init(&mc);

	if ((objp->type == OBJ_NONE) || (objp->type == OBJ_POINT)) {
		return 0;
	}

	if ((objp->type == OBJ_WAYPOINT) && !view.Show_waypoints) {
		return 0;
	}

	if ((objp->type == OBJ_START) && !view.Show_starts) {
		return 0;
	}

	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
		if (!view.Show_ships) {
			return 0;
		}

		if (!view.Show_iff[Ships[objp->instance].team]) {
			return 0;
		}
	}

	if (objp->flags[Object::Object_Flags::Hidden]) {
		return 0;
	}

	if ((view.Show_ship_models || view.Show_outlines) && (objp->type == OBJ_SHIP)) {
		mc.model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num; // Fill in the model to check
	}
	else if ((view.Show_ship_models || view.Show_outlines) && (objp->type == OBJ_START)) {
		mc.model_num = Ship_info[Ships[objp->instance].ship_info_index].model_num; // Fill in the model to check
	}
	else {
		return fvi_ray_sphere(hitpos, p0, p1, &objp->pos, (objp->radius > 0.1f) ? objp->radius : LOLLIPOP_SIZE);
	}

	mc.model_instance_num = -1;
	mc.orient = &objp->orient; // The object's orient
	mc.pos = &objp->pos; // The object's position
	mc.p0 = p0; // Point 1 of ray to check
	mc.p1 = p1; // Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL | MC_CHECK_RAY; // flags
	model_collide(&mc);
	*hitpos = mc.hit_point_world;
	if (mc.num_hits < 1) {
		// check shield
		mc.orient = &objp->orient; // The object's orient
		mc.pos = &objp->pos; // The object's position
		mc.p0 = p0; // Point 1 of ray to check
		mc.p1 = p1; // Point 2 of ray to check
		mc.flags = MC_CHECK_SHIELD; // flags
		model_collide(&mc);
		*hitpos = mc.hit_point_world;
	}

	return mc.num_hits;
}

int EditorViewport::select_object(int cx,
	int cy,
	bool Selection_lock) {
	int best = -1;
	double dist, best_dist = 9e99;
	vec3d p0, p1, v, hitpos;
	vertex vt;

	///! \fixme Briefing!
#if 0
	if (Briefing_dialog) {
		best = Briefing_dialog->check_mouse_hit(cx, cy);
		if (best >= 0)
		{
			if (Selection_lock && !(Objects[best].flags & OF_MARKED))
			{
				return -1;
			}
			return best;
		}
	}
#endif

	/*	gr_reset_clip();
	g3_start_frame(0); ////////////////
	g3_set_view_matrix(&eye_pos, &eye_orient, 0.5f);*/

	//	Get 3d vector specified by mouse cursor location.
	g3_point_to_vec(&v, cx, cy);

	//	g3_end_frame();
	if (!v.xyz.x && !v.xyz.y && !v.xyz.z) { // zero vector {
		return -1;
	}

	p0 = view_pos;
	vm_vec_scale_add(&p1, &p0, &v, 100.0f);

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (object_check_collision(objp, &p0, &p1, &hitpos)) {
			hitpos.xyz.x = objp->pos.xyz.x - view_pos.xyz.x;
			hitpos.xyz.y = objp->pos.xyz.y - view_pos.xyz.y;
			hitpos.xyz.z = objp->pos.xyz.z - view_pos.xyz.z;
			dist = hitpos.xyz.x * hitpos.xyz.x + hitpos.xyz.y * hitpos.xyz.y + hitpos.xyz.z * hitpos.xyz.z;
			if (dist < best_dist) {
				best = OBJ_INDEX(objp);
				best_dist = dist;
			}
		}
	}

	if (best >= 0) {
		if (Selection_lock && !(Objects[best].flags[Object::Object_Flags::Marked])) {
			return -1;
		}
		return best;
	}

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		g3_rotate_vertex(&vt, &objp->pos);
		if (!(vt.codes & CC_BEHIND)) {
			if (!(g3_project_vertex(&vt) & PF_OVERFLOW)) {
				hitpos.xyz.x = vt.screen.xyw.x - cx;
				hitpos.xyz.y = vt.screen.xyw.y - cy;
				dist = hitpos.xyz.x * hitpos.xyz.x + hitpos.xyz.y * hitpos.xyz.y;
				if ((dist < 8) && (dist < best_dist)) {
					best = OBJ_INDEX(objp);
					best_dist = dist;
				}
			}
		}
	}

	if (Selection_lock && !(Objects[best].flags[Object::Object_Flags::Marked])) {
		return -1;
	}

	return best;
}
}
}
