#ifndef _WXFREDRENDER_H
#define _WXFREDRENDER_H
/*
 * Created for the FreeSpace2 Source Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"

/**
 * @brief This file defines the graphics interface used by wxFred
 */
namespace wxfred {

struct ViewSettings
{
	ViewSettings(void)
		: show_playerStarts(false), show_ships(true), show_waypoints(false),
		show_background(true), show_compass(true), show_coordinates(true), show_distances(false), show_grid(true),
		show_grid_aa(true), show_grid_doubleFine(false), show_grid_positions(false), show_horizon(false),
		show_lightingFromSuns(true), show_models(true), show_model_paths(false), show_model_dockpoints(false),
		show_outlines(false), show_shipInfo(false) {
		c_pos.xyz.x = 0.0;
		c_pos.xyz.y = 0.0;
		c_pos.xyz.z = 0.0;

		c_rot.p = 0.0;
		c_rot.b = 0.0;
		c_rot.h = 0.0;
	};

	// Camera Data
	vec3d	c_pos;	// Camera Position
	angles	c_rot;	// Camera Rotation/Orientation

	// View|Display Filter...Commands
	bool show_playerStarts;
	bool show_ships;
	bool show_waypoints;

	// View...Commands
	bool show_background;
	bool show_compass;
	bool show_coordinates;
	bool show_distances;
	bool show_grid;
	bool show_grid_aa;
	bool show_grid_doubleFine;
	bool show_grid_positions;
	bool show_horizon;
	bool show_lightingFromSuns;
	bool show_models;
	bool show_model_paths;
	bool show_model_dockpoints;
	bool show_outlines;
	bool show_shipInfo;
};

/**
 * @brief Inits the graphics engine
 */
void render_init();

/**
 * @brief Renders a frame with the given view settings
 */
void render_frame(ViewSettings& vset);

};	// namespace wxfred

#endif // _WXFREDRENDER_H