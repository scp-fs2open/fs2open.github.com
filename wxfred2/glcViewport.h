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
#include <physics/physics.h>

#include <wx/glcanvas.h>


#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

enum cmode
{
	cmode_standard,	// Standard camera movement controls
	cmode_orbit,	// Camera orbits select object(s)
	cmode_ship		// Controls selected object
};

class glcViewport : public wxGLCanvas
{
public:
	glcViewport( wxWindow *parent, wxWindowID id = wxID_ANY );
	~glcViewport( void );

protected:
	// Handlers for glcViewport
	void OnPaint( wxPaintEvent& event );
	void OnSize( wxSizeEvent& event);
	void OnEraseBackground( wxEraseEvent& event);
	void OnMouse( wxMouseEvent& event );

	// Overlays
	void render_compass( void );

private:
	// Member data
	cmode	Control_mode;		// Control behavior setting

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

	// Event handling
	DECLARE_EVENT_TABLE()
};

#endif // _GLCVIEWPORT_H
