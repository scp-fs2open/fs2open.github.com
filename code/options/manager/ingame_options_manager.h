#pragma once

#include "options/ingame_options.h"
#include "options/dialogs/ingame_options_ui.h"

#include <gamesequence/gamesequence.h>

class OptConfigurator {
  public:
	OptConfigurator();
	~OptConfigurator();

	// Do rendering and handle keyboard/mouse events
	void onFrame(float frametime);

	void close() {
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	// Call this function from outside LabManager::onFrame to signal that the lab should close.
	void notify_close() {
		CloseThis = true;
	}

	void offer_restart_popup();

	void offer_save_options_popup();

	void maybe_persist_changes();

	void discard_changes();

	bool show_save_options_popup = false;
	bool show_persist_popup = false;
	SCP_string persist_options = "";
  private:
	bool CloseThis = false;
	OptUi optUi;
};
