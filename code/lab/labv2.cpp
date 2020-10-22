#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "gamesequence/gamesequence.h"

LabManager* LMGR = nullptr;

LabManager* getLabManager() {
	if (LMGR == nullptr) {
		LMGR = new LabManager();
	}

	return LMGR;
}

void lab_init() {
	gr_set_clear_color(0, 0, 0);
}

void lab_close() {
	delete LMGR;
	LMGR = nullptr; 
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

void lab_do_frame(float frametime) {
	getLabManager()->onFrame(frametime);
}

