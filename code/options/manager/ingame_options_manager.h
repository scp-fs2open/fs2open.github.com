#pragma once

#include "options/Ingame_Options.h"
#include "options/dialogs/ingame_options_ui.h"

#include "gamesequence/gamesequence.h"

class OptConfigurator {
  public:
	OptConfigurator();

	// Do rendering and handle keyboard/mouse events
	void onFrame();

	void close() {
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	// Call this function from outside OptConfigurator::onFrame to signal that the SCP Options should close.
	void notify_close() {
		CloseThis = true;
	}

	void offer_restart_popup();
	void offer_save_options_popup();

	void maybe_persist_changes();
	void discard_changes();

	int get_binary_option_index(const SCP_string& title);
	int get_float_range_option_index(const SCP_string& title);
	int get_int_range_option_index(const SCP_string& title);

	bool show_save_options_popup = false;
	bool show_persist_popup = false;
	SCP_string persist_options = "";
	bool options_changed = false;

	// These are used to store options values for imgui since we can't get to the
	// actual globals through the Options Manager. Imgui does not need a value to
	// store multi select options, so that can be done directly instead of through
	// a vector here.
	SCP_vector<std::pair<SCP_string, bool>> binary_options;
	SCP_vector<std::pair<SCP_string, float>> range_float_options;
	SCP_vector<std::pair<SCP_string, int>> range_int_options;
  private:
	bool CloseThis = false;
	OptUi optUi;
};
