#pragma once

struct OpenXRTrackingInfo {
	struct OpenXREyeInfo {
		vec3d offset;
		matrix orientation;
	} eyes[2];
};

void openxr_init(float scale = 1.0f);
void openxr_close();
void openxr_poll();
void openxr_reset_offset();
bool openxr_enabled();
OpenXRTrackingInfo openxr_start_stereo_frame();