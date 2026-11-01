#include "CameraController.h"

#include "ui/ControlBindings.h"

#include <io/spacemouse.h>
#include <mod_table/mod_table.h>

#include <algorithm>
#include <cmath>

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

	// Invalidate orbit camera state when the user moves the camera another way
	if (wantsUpdate)
		_orbitActive = false;

	return wantsUpdate;
}

// ---------- Orbit camera functions ----------

void CameraController::orbitCameraInitFromCurrentView(const vec3d *pivot, const matrix *grid_orient)
{
	_orbitPivot = pivot ? *pivot : vmd_zero_vector;
	_orbitGridOrient = grid_orient ? *grid_orient : vmd_identity_matrix;

	vec3d offset;
	vm_vec_sub(&offset, &view_pos, &_orbitPivot);

	_orbitDistance = vm_vec_mag(&offset);
	if (_orbitDistance < 1.0f)
		_orbitDistance = 100.0f;

	// Transform offset into grid-local frame for angle decomposition
	// In local frame, Y is always "up" (the grid plane normal)
	vec3d local_offset;
	vm_vec_rotate(&local_offset, &offset, &_orbitGridOrient);

	_orbitPhi = acosf(std::clamp(local_offset.xyz.y / _orbitDistance, -1.0f, 1.0f));
	_orbitTheta = atan2f(local_offset.xyz.z, local_offset.xyz.x);
	_orbitActive = true;
}

void CameraController::orbitCameraApply()
{
	float sp = sinf(_orbitPhi);

	// Build offset in grid-local coordinates (Y = up/normal)
	vec3d local_pos;
	local_pos.xyz.x = sp * cosf(_orbitTheta);
	local_pos.xyz.y = cosf(_orbitPhi);
	local_pos.xyz.z = sp * sinf(_orbitTheta);

	// Transform back to world space
	vec3d world_offset;
	vm_vec_unrotate(&world_offset, &local_pos, &_orbitGridOrient);

	vm_vec_scale(&world_offset, _orbitDistance);
	vm_vec_add(&view_pos, &_orbitPivot, &world_offset);

	// Point camera at pivot, using grid's up vector
	vec3d look_dir;
	vm_vec_sub(&look_dir, &_orbitPivot, &view_pos);
	if (vm_vec_mag(&look_dir) > 0.001f) {
		vec3d grid_up = _orbitGridOrient.vec.uvec;
		vm_vector_2_matrix(&view_orient, &look_dir, &grid_up, nullptr);
	}
}

void CameraController::orbitCameraRotate(int dx, int dy)
{
	float rot_scale = _physicsRot / 300.0f;
	if (_invertOrbitX)
		dx = -dx;
	if (_invertOrbitY)
		dy = -dy;
	_orbitTheta += dx / 100.0f * rot_scale;
	_orbitPhi += dy / 100.0f * rot_scale;

	CLAMP(_orbitPhi, 0.01f, PI - 0.01f);

	orbitCameraApply();
}

void CameraController::orbitCameraPan(int dx, int dy)
{
	float speed_factor = 1.0f + (_physicsSpeed - 1) / 499.0f * 9.0f;
	float pan_scale = _orbitDistance * 0.002f * speed_factor;

	vec3d pan_delta;
	vm_vec_copy_scale(&pan_delta, &view_orient.vec.rvec, -(float)dx * pan_scale);
	vm_vec_scale_add2(&pan_delta, &view_orient.vec.uvec, (float)dy * pan_scale);

	vm_vec_add2(&_orbitPivot, &pan_delta);

	orbitCameraApply();
}

void CameraController::orbitCameraZoom(float delta)
{
	// Scale the per-notch zoom with the camera move speed: slow speeds give fine, precise
	// zoom, fast speeds give coarse jumps for traversing large scenes. sqrt keeps the
	// 1..100 move-speed range from exploding into absurd steps at the top end.
	float zoom_speed = sqrtf(static_cast<float>(_physicsSpeed)) * 0.5f;
	_orbitDistance *= powf(2.0f, delta * zoom_speed);

	CLAMP(_orbitDistance, 1.0f, 10000000.0f);

	orbitCameraApply();
}

// ---------- End orbit camera functions ----------

} // namespace fso::fred
