#include "lab/renderer/lab_renderer.h"
#include "lab/wmcgui.h"
#include "graphics/2d.h"


void LabRenderer::onFrame(float frametime) {
	GR_DEBUG_SCOPE("Lab Frame");

	gr_reset_clip();
	gr_clear();	
	// TODO: Actually render a thing

}

void LabRenderer::useBackground(SCP_string mission_name) {}