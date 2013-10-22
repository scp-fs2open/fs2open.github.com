/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <glcviewport.h>
#include <glrcmissionframe.h>

#include <wx/wx.h>

BEGIN_EVENT_TABLE( glcViewport, wxGLCanvas )
	EVT_SIZE( glcViewport::OnSize )
	EVT_PAINT( glcViewport::OnPaint )
	EVT_ERASE_BACKGROUND( glcViewport::OnEraseBackground )
	EVT_MOUSE_EVENTS( glcViewport::OnMouse )
END_EVENT_TABLE()

glcViewport::glcViewport( wxWindow *parent, wxWindowID id )
	: wxGLCanvas( parent, id, NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
	// Initialize camera position and angles
	// Initialize grid
	// Initialize grFrame, if it isn't already
		// Start off with a new context per frame. Later on we'll look into linking existing content
	grFrame = new glrcMissionFrame(this);
}

glcViewport::~glcViewport( void )
{
	delete grFrame;
}


// Handlers for glcViewport
void glcViewport::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);	// Required. Even if we don't directly use it.

	SetCurrent(*grFrame);

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
