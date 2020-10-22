#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "gamesequence/gamesequence.h"

LabManager* LMGR = nullptr;

void lab_init() {
	gr_set_clear_color(0, 0, 0);

	LMGR = new LabManager();
}

void lab_close() {
	delete LMGR;
	LMGR = nullptr; 
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

void lab_do_frame(float frametime) {
	if (LMGR != nullptr)
		LMGR->onFrame(frametime);
}

