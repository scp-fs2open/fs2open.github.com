/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "glcViewport.h"

#include "wxFredRender.h"

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

glcViewport::glcViewport(wxWindow *parent, wxWindowID id)
: wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
	vec3d f, u, r;

	// Init position
	vm_vec_make(&vset.c_pos, 0.0f, 150.0f, -200.0f);

	// Init orientation
	vm_vec_make(&f, 0.0f, -0.5f, 0.866025404f);         // 30 degree angle
	vm_vec_make(&u, 0.0f, 0.866025404f, 0.5f);
	vm_vec_make(&r, 1.0f, 0.0f, 0.0f);
	vm_vector_2_matrix(&vset.c_orient, &f, &u, &r);

	// Init physics
	set_physics();

	// Init grid
	grid = create_default_grid();
	maybe_create_new_grid(grid, &vset.c_pos, &vset.c_orient, 1);

	// Init renderer
	wxfred::render_init(this);
}

glcViewport::~glcViewport(void) {
	// Delete context if we're the last instance of a viewport.
}


// Handlers for glcViewport
void glcViewport::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);	// Required. Even if we don't directly use it.

	wxfred::render_frame(this);
}

void glcViewport::OnSize(wxSizeEvent& event) {
	auto canvas = FindWindowByName("GLCanvas", this);
	// Should never happen, but just in case...
	Assertion(canvas != nullptr, "glcViewport could not find its child canvas!");

	auto oldSize = canvas->GetSize();
	auto newSize = event.GetSize();

	if (oldSize != newSize) {
		wxfred::render_resize(this, newSize.GetWidth(), newSize.GetHeight());
		
		// z64: Blah. Force a wxPaintEvent becuase Refresh and Update arn't working as expected
		wxPaintEvent event_paint(GetId());
		GetEventHandler()->ProcessEvent(event_paint);
	}
}

void glcViewport::OnEraseBackground(wxEraseEvent& event) {}

void glcViewport::OnMouse(wxMouseEvent& event) {}

void glcViewport::set_physics() {
	physics_init(&vset.c_physics);
	vset.c_physics.max_vel.xyz.x *= vset.physics_speed / 3.0f;
	vset.c_physics.max_vel.xyz.y *= vset.physics_speed / 3.0f;
	vset.c_physics.max_vel.xyz.z *= vset.physics_speed / 3.0f;
	vset.c_physics.max_rear_vel *= vset.physics_speed / 3.0f;

	vset.c_physics.max_rotvel.xyz.x *= vset.physics_rot / 30.0f;
	vset.c_physics.max_rotvel.xyz.y *= vset.physics_rot / 30.0f;
	vset.c_physics.max_rotvel.xyz.z *= vset.physics_rot / 30.0f;
	vset.c_physics.flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;
	//theApp.write_ini_file(1);
}
