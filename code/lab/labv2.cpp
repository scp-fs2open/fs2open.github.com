#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"

LabManager* LMGR;

void lab_init() {
	gr_set_clear_color(0, 0, 0);

	LMGR = new LabManager();

	Game_mode |= GM_LAB;
}

void lab_close() {
	delete LMGR;
	LMGR = nullptr;
}

void lab_do_frame(float frametime) {
	if (LMGR != nullptr)
		LMGR->onFrame(frametime);
}

