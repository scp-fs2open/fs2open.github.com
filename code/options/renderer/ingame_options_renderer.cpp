#include "options/renderer/ingame_options_renderer.h"
#include "options/ingame_options_internal.h"
#include "graphics/2d.h"

#include "tracing/tracing.h"

void OptRenderer::onFrame(float frametime) {
	GR_DEBUG_SCOPE("SCP Options Frame");

	gr_reset_clip();
	gr_clear();

	if (!renderFlags[OptRenderFlag::TimeStopped])
		Missiontime += Frametime;
	// Normally, we would call gr_flip here, but because wmcgui conflates rendering and input gathering, this is done at the end of 
	// the LabManager::onFrame method
}