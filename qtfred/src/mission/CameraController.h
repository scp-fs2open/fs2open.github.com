#pragma once

#include <globalincs/pstypes.h>
#include <physics/physics.h>

namespace fso::fred {

class CameraController {
	physics_info view_physics;
	control_info view_controls;

	vec3d saved_cam_pos = vmd_zero_vector;
	matrix saved_cam_orient = {};

	vec3d Last_eye_pos = vmd_zero_vector;
	matrix Last_eye_orient = vmd_identity_matrix;

	int _physicsSpeed = 1;
	int _physicsRot = 25;
	int _viewpoint = 0;
	int _viewObj = -1;
	int _controlMode = 0;
	bool _lookatMode = false;

	// Orbit camera state
	vec3d _orbitPivot = vmd_zero_vector;
	matrix _orbitGridOrient = vmd_identity_matrix;
	float _orbitDistance = 200.0f;
	float _orbitPhi = 1.24f;
	float _orbitTheta = 2.25f;
	bool _orbitActive = false;
	// Default to inverted, matching the engine's F3 lab and POF Tools.
	bool _invertOrbitX = true;
	bool _invertOrbitY = true;

	void orbitCameraApply();

public:
	vec3d eye_pos;
	matrix eye_orient;

	vec3d view_pos;
	matrix view_orient = vmd_identity_matrix;

	void resetView();
	void resetViewPhysics();

	void savePosition();
	void restorePosition();
	bool hasSavedPosition() const;

	bool hasEyeMoved();

	const matrix& getLastRotMat() const;

	// Returns true if the caller should schedule a view update
	bool processControls(vec3d* pos, matrix* orient, float frametime, bool use_editor_physics);

	int getPhysicsSpeed() const { return _physicsSpeed; }
	void setPhysicsSpeed(int speed) { _physicsSpeed = speed; resetViewPhysics(); }

	int getPhysicsRot() const { return _physicsRot; }
	void setPhysicsRot(int rot) { _physicsRot = rot; resetViewPhysics(); }

	int getViewpoint() const { return _viewpoint; }
	void setViewpoint(int vp) { _viewpoint = vp; }

	int getViewObj() const { return _viewObj; }
	void setViewObj(int obj) { _viewObj = obj; }

	int getControlMode() const { return _controlMode; }
	void setControlMode(int mode) { _controlMode = mode; }
	void toggleControlMode() { _controlMode = (_controlMode + 1) % 2; }

	bool getLookatMode() const { return _lookatMode; }
	void setLookatMode(bool mode) { _lookatMode = mode; }

	bool getInvertOrbitX() const { return _invertOrbitX; }
	void setInvertOrbitX(bool invert) { _invertOrbitX = invert; }

	bool getInvertOrbitY() const { return _invertOrbitY; }
	void setInvertOrbitY(bool invert) { _invertOrbitY = invert; }

	void orbitCameraInitFromCurrentView(const vec3d* pivot, const matrix* grid_orient);
	void orbitCameraRotate(int dx, int dy);
	void orbitCameraPan(int dx, int dy);
	void orbitCameraZoom(float delta);

	bool isOrbitActive() const { return _orbitActive; }
};

} // namespace fso::fred
