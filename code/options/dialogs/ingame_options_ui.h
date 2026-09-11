#pragma once

class OptUi {
  public:
	void create_ui();

  private:
	void build_options_menu();
	void build_options_list(const char* category) const;
	void build_toolbar_entries();
	void show_log_filters_window();
	static void show_log_filter_list();
	static void show_log_output();
	void build_hdr_calibration_window();

	// used to track the "Close SCP Options" functions
	bool close_and_save = false;
	bool close_and_discard = false;

	bool show_log_filters_dialog = false;

	// HDR calibration sub-window toggle
	bool show_hdr_calibration = false;
};