#ifndef _GLCVIEWPORT_H
#define _GLCVIEWPORT_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include "globalincs/pstypes.h"
#include "physics/physics.h"

#include "wx/glcanvas.h"


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
	glcViewport( wxWindow *parent, wxWindowID id = wxID_ANY, int*gl_attrib = NULL);

protected:
	// Handlers for glcViewport
	void OnPaint( wxPaintEvent& event );
	void OnSize( wxSizeEvent& event);
	void OnEraseBackgroun( wxEraseEvent& event);

	void OnMouse( wxMouseEvent& event );
private:
	// Member data
	vec3d	c_pos;	// Camera Position
	matrix	c_rot;	// Camera Rotation/Orientation

	// Viewport modes and options
	cmode Control_mode;

	bool show_coordinates;
	bool show_distances;
	bool show_grid;
	bool show_grid_aa;
	bool show_grid_positions;
	bool show_horizon;
	bool show_outlines;


};

#endif // _GLCVIEWPORT_H
