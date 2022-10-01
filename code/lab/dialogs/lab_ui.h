#pragma once

class LabUi {
  public:
	void createUi();
	void objectChanged();
	void closeUi();

  private:
	void buildShipList() const;
	void buildWeaponList() const;
	void buildBackgroundList() const;
	void showRenderOptions();
	void showObjectOptions() const;

	// if this is true, the displayed object has changed and so every piece of cached data related to
	// the object must be invalidated
	bool rebuildAfterObjectChange = false;

	// these flags track the state of open windows. We start with the object selector open and all other
	// subdialogs closed
	bool show_render_options = false;
	bool show_object_selector = true;
	bool show_object_options = false;

	// used to track the "Close Lab" function
	bool close_lab = false;

	// various settings from the graphics dialog
	bool enable_model_rotation = false;
	bool show_insignia = false;
	bool show_damage_lightning = false;
	bool animate_subsystems = false;
	bool hide_post_processing = false;
	bool diffuse_map = true;
	bool glow_map = true;
	bool spec_map = true;
	bool reflect_map = true;
	bool env_map = true;
	bool normal_map = true;
	bool height_map = true;
	bool misc_map = true;
	bool ao_map = true;
	bool no_glowpoints = false;
	bool use_wireframe_rendering = false;
	bool no_lighting = false;
	bool show_full_detail = false;
	bool show_thrusters = false;
	bool show_afterburners = false;
	bool show_weapons = false;
	bool show_emissive_lighting = false;
};