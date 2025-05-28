#include "globalincs/pstypes.h"
#include "io/key.h"
#include "lab/renderer/lab_cameras.h"
#include "lab/labv2_internal.h"


LabCamera::~LabCamera() {
	cam_delete(FS_camera);
}

void OrbitCamera::handleInput(int dx, int dy, bool, bool rmbDown, int modifierKeys) {
	if (dx == 0 && dy == 0)
		return;

	if (rmbDown) {
		if (modifierKeys & KEY_SHIFTED) {
			distance *= 1.0f + (dy / 200.0f);
			CLAMP(distance, 1.0f, 10000000.0f);
		}
		else {
			theta += dx / 100.0f;
			phi += dy / 100.0f;

			CLAMP(phi, 0.01f, PI - 0.01f);
		}
	}

	updateCamera();
}

void OrbitCamera::displayedObjectChanged() {
	float distance_multiplier = 1.6f;

	if (getLabManager()->CurrentObject != -1) {
		object* obj = &Objects[getLabManager()->CurrentObject];
		
		// Ships and Missiles use the object radius to get a camera distance
		distance = obj->radius * distance_multiplier;

		// Beams use the muzzle radius
		if (obj->type == OBJ_BEAM) {
			weapon_info* wip = &Weapon_info[Beams[obj->instance].weapon_info_index];
			if (wip != nullptr) {
				distance = wip->b_info.beam_muzzle_radius * distance_multiplier;
			}
		// Lasers use the laser length
		} else if (obj->type == OBJ_WEAPON) {
			weapon_info* wip = &Weapon_info[Weapons[obj->instance].weapon_info_index];
			if (wip != nullptr && wip->render_type == WRT_LASER) {
				distance = wip->laser_length * distance_multiplier;
			}
		}
	}

	updateCamera();
}

void OrbitCamera::updateCamera() {
	auto cam = FS_camera.getCamera();
	vec3d new_position;
	new_position.xyz.x = sinf(phi) * cosf(theta);
	new_position.xyz.y = cosf(phi);
	new_position.xyz.z = sinf(phi) * sinf(theta);

	vm_vec_scale(&new_position, distance);

	object* obj = &Objects[getLabManager()->CurrentObject];
	vec3d target = obj->pos;

	if (obj->type == OBJ_WEAPON) {
		weapon_info* wip = &Weapon_info[Weapons[obj->instance].weapon_info_index];
		if (wip != nullptr && wip->render_type == WRT_LASER) {
			// Offset target by half the laser length forward along the facing
			vec3d forward;
			vm_vec_copy_normalize(&forward, &obj->orient.vec.fvec);
			vm_vec_scale_add2(&target, &forward, wip->laser_length * 0.5f);
		}
		
		vm_vec_add2(&new_position, &target);
	}

	cam->set_position(&new_position);

	// If these are the same then that's not great so do nothing and use the last facing value
	if (!vm_vec_same(&new_position, &target)) {
		cam->set_rotation_facing(&target);
	}
}
