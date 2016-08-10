/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "glcViewport.h"

#include <wx/event.h>
#include <wx/window.h>
#include <wx/windowid.h>
#include <wx/wx.h>

BEGIN_EVENT_TABLE(glcViewport, wxWindow)
EVT_SIZE(glcViewport::OnSize)
EVT_PAINT(glcViewport::OnPaint)
EVT_ERASE_BACKGROUND(glcViewport::OnEraseBackground)
EVT_MOUSE_EVENTS(glcViewport::OnMouse)
END_EVENT_TABLE()

glcViewport::glcViewport( wxWindow *parent, wxWindowID id )
	: wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
	// Create context if we're the first instance of a viewport. Maybe have the wxFredRender code keep a counter.
	// Initialize camera position and angles
	// Initialize grid
}

glcViewport::~glcViewport( void )
{
	// Delete context if we're the last instance of a viewport.
}


// Handlers for glcViewport
void glcViewport::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);	// Required. Even if we don't directly use it.

	wxfred::render_frame(vset);
}

void glcViewport::OnSize( wxSizeEvent& event )
{
}

void glcViewport::OnEraseBackground( wxEraseEvent& event )
{
}

void glcViewport::OnMouse( wxMouseEvent& event )
{
}
