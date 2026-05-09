#include "CameraController.h"

#include "ui/ControlBindings.h"

#include <io/spacemouse.h>
#include <mod_table/mod_table.h>

namespace fso::fred {

void CameraController::resetView() {
	vec3d f, u, r;

	physics_init(&view_physics);
	view_physics.max_vel.xyz.z = 5.0f;
	view_physics.max_rotvel.xyz.x = 1.5f;
	memset(&view_controls, 0, sizeof(control_info));

	vm_vec_make(&view_pos, 0.0f, 150.0f, -200.0f);
	vm_vec_make(&f, 0.0f, -0.5f, 0.866025404f);
	vm_vec_make(&u, 0.0f, 0.866025404f, 0.5f);
	vm_vec_make(&r, 1.0f, 0.0f, 0.0f);
	vm_vector_2_matrix(&view_orient, &f, &u, &r);
}

void CameraController::resetViewPhysics() {
	physics_init(&view_physics);
	view_physics.max_vel.xyz.x *= _physicsSpeed / 3.0f;
	view_physics.max_vel.xyz.y *= _physicsSpeed / 3.0f;
	view_physics.max_vel.xyz.z *= _physicsSpeed / 3.0f;
	view_physics.max_rear_vel *= _physicsSpeed / 3.0f;
	view_physics.max_rotvel.xyz.x *= _physicsRot / 30.0f;
	view_physics.max_rotvel.xyz.y *= _physicsRot / 30.0f;
	view_physics.max_rotvel.xyz.z *= _physicsRot / 30.0f;
	view_physics.flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;
}

void CameraController::savePosition() {
	saved_cam_pos = view_pos;
	saved_cam_orient = view_orient;
}

void CameraController::restorePosition() {
	view_pos = saved_cam_pos;
	view_orient = saved_cam_orient;
}

bool CameraController::hasSavedPosition() const {
	return !IS_VEC_NULL(&saved_cam_orient.vec.fvec);
}

bool CameraController::hasEyeMoved() {
	if (vm_vec_cmp(&eye_pos, &Last_eye_pos) || vm_matrix_cmp(&eye_orient, &Last_eye_orient)) {
		Last_eye_pos = eye_pos;
		Last_eye_orient = eye_orient;
		return true;
	}
	return false;
}

const matrix& CameraController::getLastRotMat() const {
	return view_physics.last_rotmat;
}

bool CameraController::processControls(vec3d* pos, matrix* orient, float frametime, bool use_editor_physics) {
	io::spacemouse::SpaceMouse* const spacemouse = io::spacemouse::SpaceMouse::getSharedSpaceMouse(0);

	memset(&view_controls, 0, sizeof(control_info));

	if (spacemouse != nullptr) {
		auto spacemouse_movement = spacemouse->getMovement();
		spacemouse_movement.handleNonlinearities(Fred_spacemouse_nonlinearity);
		view_controls.pitch += spacemouse_movement.rotation.p;
		view_controls.vertical += spacemouse_movement.translation.xyz.z;
		view_controls.heading += spacemouse_movement.rotation.h;
		view_controls.sideways += spacemouse_movement.translation.xyz.x;
		view_controls.bank += spacemouse_movement.rotation.b;
		view_controls.forward += spacemouse_movement.translation.xyz.y;
	}

	auto& bindings = ControlBindings::instance();
	view_controls.pitch += bindings.isPressed(ControlAction::PitchUp) ? -1.0f : 0.0f;
	view_controls.pitch += bindings.isPressed(ControlAction::PitchDown) ? 1.0f : 0.0f;
	view_controls.heading += bindings.isPressed(ControlAction::YawLeft) ? -1.0f : 0.0f;
	view_controls.heading += bindings.isPressed(ControlAction::YawRight) ? 1.0f : 0.0f;
	view_controls.sideways += bindings.isPressed(ControlAction::MoveLeft) ? -1.0f : 0.0f;
	view_controls.sideways += bindings.isPressed(ControlAction::MoveRight) ? 1.0f : 0.0f;
	view_controls.forward += bindings.isPressed(ControlAction::MoveForward) ? 1.0f : 0.0f;
	view_controls.forward += bindings.isPressed(ControlAction::MoveBackward) ? -1.0f : 0.0f;
	view_controls.vertical += bindings.isPressed(ControlAction::MoveUp) ? 1.0f : 0.0f;
	view_controls.vertical += bindings.isPressed(ControlAction::MoveDown) ? -1.0f : 0.0f;

	bool wantsUpdate = (fabs(view_controls.pitch) > (frametime / 100))
	                || (fabs(view_controls.vertical) > (frametime / 100))
	                || (fabs(view_controls.heading) > (frametime / 100))
	                || (fabs(view_controls.sideways) > (frametime / 100))
	                || (fabs(view_controls.bank) > (frametime / 100))
	                || (fabs(view_controls.forward) > (frametime / 100));

	physics_read_flying_controls(orient, &view_physics, &view_controls, frametime);
	if (use_editor_physics) {
		physics_sim_editor(pos, orient, &view_physics, frametime);
	} else {
		physics_sim(pos, orient, &view_physics, &vmd_zero_vector, frametime);
	}
	return wantsUpdate;
}

} // namespace fso::fred
