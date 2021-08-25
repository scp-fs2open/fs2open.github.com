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
			distance *= 1.0f + (dy / 20.0f);
			if (distance < 1.0f)
				distance = 1.0f;
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
	if (getLabManager()->CurrentObject != -1) {
		distance = Objects[getLabManager()->CurrentObject].radius * 1.6f;
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

	cam->set_position(&new_position);
	cam->set_rotation_facing(&getLabManager()->CurrentPosition);
}
