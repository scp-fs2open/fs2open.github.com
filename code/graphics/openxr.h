#pragma once

#include <array>

struct OpenXRTrackingInfo {
	struct OpenXREyeInfo {
		vec3d offset;
		matrix orientation;
	};
	std::array<OpenXREyeInfo, 2> eyes;
};

/**
 * @brief Performs required Setup actions for OpenXR before actually trying to initialize the graphics
 * @param hudscale Sets the scale at which the HUD is rendered. May need to be tweakable to make the hud visible to players with small FoV
 */
void openxr_prepare(float hudscale = 0.5f);

/**
 * @brief Initializes the OpenXR API and opens a session to the point that we can render. As this waits for OpenXR, this may take a while
 * @param scale Controls the scale of the OpenXR coordinates. This mainly affects eye distance and how far real movement translates to ingame movement
 */
void openxr_init(float scale = 1.0f);

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
 * @brief Polls HMD position and sets up the OpenXR state machine to wait for and accept two frames, one for each eye.
 * You must ensure that after calling this, the next two gr_flip() calls correspond to the frames for each eye!
 * @returns The HMD tracking data including position and rotation in FSO coordinates.
 */
OpenXRTrackingInfo openxr_start_stereo_frame();