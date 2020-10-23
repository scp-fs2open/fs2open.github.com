#pragma once

#include "camera/camera.h"

class LabCamera {
public:
	LabCamera(camid cam) {
		FS_camera = cam;
	}

	virtual ~LabCamera() = 0;

	camid FS_camera;

	/// <summary>
	/// Returns a string explaining the camera controls
	/// </summary>
	/// <returns></returns>
	virtual SCP_string getUsageInfo() = 0;

	/// <summary>
	/// Returns a string containing miscellaneous info
	/// </summary>
	/// <returns></returns>
	virtual SCP_string getOnFrameInfo() = 0;

	/// <summary>
	/// Called by the Lab Manager each frame to handle input events.
	/// </summary>
	/// <param name="dx">Mouse delta on the x axis</param>
	/// <param name="dy">Mouse delta on the y axis</param>
	/// <param name="lmbDown">State of the left mouse button</param>
	/// <param name="rmbDown">State of the right mouse button</param>
	/// <param name="modifierKeys">State of the various modifier keys. See keys.h</param>
	virtual void handleInput(int dx, int dy, bool lmbDown, bool rmbDown, int modifierKeys) = 0;

	/// <summary>
	/// Called by the lab manager when the displayed object changes
	/// </summary>
	virtual void displayedObjectChanged() = 0;

	/// <summary>
	/// Returns true if this camera also handles object placement
	/// </summary>
	/// <returns></returns>
	virtual bool handlesObjectPlacement() = 0;
};

class OrbitCamera : public LabCamera {
public:
	OrbitCamera() : LabCamera(cam_create("Lab orbit camera")) {}

	SCP_string getUsageInfo() override {
		return "Hold RMB to rotate the Camera. Hold Shift + LMB to zoom in or out.";
	}

	SCP_string getOnFrameInfo() override {
		SCP_stringstream ss;
		ss.setf(std::ios::fixed);

		ss << "Phi: " << phi << " Theta: " << theta << " Distance: " << distance;

		return ss.str();
	}

	void handleInput(int dx, int dy, bool /*lmbDown*/, bool rmbDown, int modifierKeys) override;

	void displayedObjectChanged() override;

	bool handlesObjectPlacement() override { return false; }
private:
	float distance = 100.0f;
	float phi = 1.24f;
	float theta = 2.25f;

	void updateCamera();
};