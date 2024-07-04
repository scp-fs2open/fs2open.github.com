#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"
#include "gamesequence/gamesequence.h"

static std::unique_ptr<LabManager> LMGR;

const std::unique_ptr<LabManager> &getLabManager() {
	if (LMGR == nullptr) {
		LMGR.reset(new LabManager());
	}

	return LMGR;
}

void lab_init() {
	gr_set_clear_color(0, 0, 0);
}

void lab_close() {
	LMGR.reset();
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

void lab_do_frame(float frametime) {
	getLabManager()->onFrame(frametime);
}

