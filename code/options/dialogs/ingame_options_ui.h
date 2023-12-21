#pragma once

class OptUi {
  public:
	void create_ui();

  private:
	void build_options_menu();
	void show_options_menus() const;
	void build_options_list(const char* category) const;
	void build_toolbar_entries();

	// used to track the "Close SCP Options" function
	bool close_and_save = false;
	bool close_and_discard = false;
};