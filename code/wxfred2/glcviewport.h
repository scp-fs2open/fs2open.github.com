#ifndef _GLCVIEWPORT_H
#define _GLCVIEWPORT_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */
#include <glrcmissionframe.h>

#include <base/wxfred_base.h>

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
	ViewSettings	settings;	// View/render settings
	cmode	Control_mode;		// Control behavior setting

	glrcMissionFrame	*grFrame;

	// Event handling
	DECLARE_EVENT_TABLE()
};

#endif // _GLCVIEWPORT_H
