#include "lab/labv2.h"
#include "lab/labv2_internal.h"
#include "lab/manager/lab_manager.h"

LabManager* LMGR;

void lab_init() {
	LMGR = new LabManager();
}

void lab_close() {
	delete LMGR;
	LMGR = nullptr;
}

void lab_do_frame(float frametime) {
	if (LMGR != nullptr)
		LMGR->onFrame(frametime);
}

