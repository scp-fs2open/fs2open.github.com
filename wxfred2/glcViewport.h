#ifndef _GLCVIEWPORT_H
#define _GLCVIEWPORT_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */
#include "base/wxFRED_base.h"

#include <globalincs/pstypes.h>
#include <mission/missiongrid.h>
#include <physics/physics.h>

#include <wx/event.h>
#include <wx/window.h>
#include <wx/windowid.h>

enum cmode
{
	cmode_standard,	// Standard camera movement controls
	cmode_orbit,	// Camera orbits select object(s)
	cmode_ship		// Controls selected object
};

struct ViewSettings
{
	// Camera Data
	vec3d   c_pos;      // Camera Position
	matrix  c_orient;   // Camera Orientation
	physics_info c_physics; // Camera Physics
	int physics_speed = 1;  // Physics speed multiplier
	int physics_rot = 20;   // Physics rotation multiplier

	// View|Display Filter...Commands
	bool show_playerStarts;
	bool show_ships;
	bool show_waypoints;

	// View...Commands
	bool show_background;
	bool show_compass;
	bool show_coordinates;
	bool show_distances;
	bool show_grid = true;
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

	bool show_asteroids;
	bool show_stars = true;
};

/**
 * @class glcViewport
 *
 * @brief The viewport class.
 *
 * @details Where the bulk of the wxFRED GUI is the frontend to the mission file, this is the frontend to the graphics
 *   system. It provides handlers for the GUI events and calls the appropriate rendering functions provided by wxFredRender
 */
class glcViewport : public wxWindow
{
public:
	glcViewport( wxWindow *parent, wxWindowID id = wxID_ANY );
	~glcViewport( void );


	cmode	Control_mode;		// Control behavior setting. Public for now for lazy reasons

	// These are public for now, because lazy
	ViewSettings vset;  // View settings of this viewport.
	grid* grid;         // Grid of this viewport

protected:
	// Handlers for glcViewport
	void OnPaint( wxPaintEvent& event );
	void OnSize( wxSizeEvent& event);
	void OnEraseBackground( wxEraseEvent& event);
	void OnMouse( wxMouseEvent& event );

	// Internal functions

	/**
	 * @brief Sets the viewport's c_physics
	 *
	 * @note z64: This system is a bit derp. It inits the physics_info and then scales it by the physics_speed and physics_rot factors
	 */
	void set_physics();

private:
	// Event handling
	DECLARE_EVENT_TABLE()
};

#endif // _GLCVIEWPORT_H
