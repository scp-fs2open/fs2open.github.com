/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "glrcmissionframe.h"

#include <globalincs/pstypes.h>
#include <physics/physics.h>

#include <wx/glcanvas.h>
#include <wx/wx.h>

glrcMissionFrame::glrcMissionFrame( wxGLCanvas *canvas )
	: wxGLContext(canvas)
{
	// Init graphics engine if it hasn't done so already.
}

// Copy context from an existing one
glrcMissionFrame::glrcMissionFrame( const glrcMissionFrame &other )
	: wxGLContext(other)
{
	// Assume graphics engine has already been init, since the other glrc (or one of its parents) probably did that
}

glrcMissionFrame::~glrcMissionFrame( void )
{
}

void glrcMissionFrame::render( const ViewSettings &settings )
{
	// Render starfield, suns, nebula decals, etc.
	// Render asteroids
	// Render ships and objects
	// Render missiles, projectiles, and beamz
	// Render dakka (special effects, missile trails, thruster trails, misc. particles)

	// Render grid
};
