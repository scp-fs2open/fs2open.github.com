#pragma once

class OptUi {
  public:
	void create_ui();
	void object_changed();

  private:
	void build_options_menu();
	void show_options_menus() const;
	void build_options_list(const char* category) const;
	void build_toolbar_entries();

	// if this is true, the displayed object has changed and so every piece of cached data related to
	// the object must be invalidated
	bool rebuild_after_object_change = false;

	// used to track the "Close SCP Options" function
	bool close_and_save = false;
	bool close_and_discard = false;
};