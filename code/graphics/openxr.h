#pragma once

#include <array>
#include <optional>
#include <utility>
#include "camera/camera.h"

struct OpenXRTrackingInfo {
	struct OpenXREyeInfo {
		vec3d offset;
		matrix orientation;
		fov_t zoom; //zoom, not fov! difference of a factor of PROJ_FOV_FACTOR
	};
	std::array<OpenXREyeInfo, 2> eyes;
};

/**
 * @brief Performs required Setup actions for OpenXR before actually trying to initialize the graphics
 */
void openxr_prepare();

/**
 * @brief Initializes the OpenXR API and opens the XR system
 * @param req_ar The aspect ratio as specified by the user resolution
 * @param scale Controls the scale of the OpenXR coordinates. This mainly affects eye distance and how far real movement translates to ingame movement
 * @returns the expected aspect ratio of the swapchain
 */
float openxr_preinit(float req_ar, float scale = 1.0f);

/**
 * @brief Opens an OpenXR session to the point that we can render. As this waits for OpenXR, this may take a while
 */
void openxr_init();

/**
 * @brief Shuts down OpenXR and cleans up memory
 */
void openxr_close();

/**
 * @brief Polls OpenXR state. Does NOT poll HMD position!
 */
void openxr_poll();

/**
 * @brief Grabs the last known HMD position and sets it as the zero-state (whereever the specified eyepoint is in mission / ship), as well as initializing buffers for the mission
 */
void openxr_start_mission();

/**
 * @brief Polls whether OpenXR is active and can render
 */
bool openxr_enabled();

/**
 * @brief Polls whether OpenXR is requested to start. Required if you need to poll for OpenXR during some initialization where it might not yet be running
 */
bool openxr_requested();

/**
 * @brief The image size the HMD wants for each eye. Available once openxr_preinit() succeeded.
 */
std::optional<std::pair<int, int>> openxr_get_eye_resolution();

/**
 * @brief The field of view of the HMD's first eye, in radians. Available once openxr_init() succeeded.
 */
std::optional<asymmetric_fov> openxr_get_eye_fov();

/**
 * @brief Which eye is being rendered: 0 or 1 during a stereo frame, 0 otherwise
 */
int openxr_get_current_eye();

/**
 * @brief Whether the eyes of a stereo frame are being rendered, from openxr_start_stereo_frame() to the second eye's flip
 */
bool openxr_in_stereo_frame();

/**
 * @brief Makes the current head heading and position the new forward and center, from the next frame on
 */
void openxr_request_recenter();

/**
 * @brief Polls HMD position and sets up the OpenXR state machine to wait for and accept two frames, one for each eye.
 * You must ensure that after calling this, the next two gr_flip() calls correspond to the frames for each eye!
 * @returns The HMD tracking data including position and rotation in FSO coordinates.
 */
OpenXRTrackingInfo openxr_start_stereo_frame();