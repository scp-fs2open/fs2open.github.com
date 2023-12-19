#include "globalincs/pstypes.h"
#include "io/key.h"
#include "options/renderer/ingame_options_camera.h"
#include "options/ingame_options_internal.h"


OptCamera::~OptCamera() {
	cam_delete(FS_camera);
}

void OrbitCameraOpt::handleInput_opt(int dx, int dy, bool, bool rmbDown, int modifierKeys)
{
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

	updateCamera_opt();
}

void OrbitCameraOpt::displayedObjectChanged_opt()
{
	if (getOptConfigurator()->CurrentObject != -1) {
		distance = Objects[getOptConfigurator()->CurrentObject].radius * 1.6f;
	}

	updateCamera_opt();
}

void OrbitCameraOpt::updateCamera_opt() {
	auto cam = FS_camera.getCamera();
	vec3d new_position;
	new_position.xyz.x = sinf(phi) * cosf(theta);
	new_position.xyz.y = cosf(phi);
	new_position.xyz.z = sinf(phi) * sinf(theta);

	vm_vec_scale(&new_position, distance);

	cam->set_position(&new_position);
	cam->set_rotation_facing(&getOptConfigurator()->CurrentPosition);
}
