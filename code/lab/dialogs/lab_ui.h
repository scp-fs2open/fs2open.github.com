#pragma once

#include "model/model.h"
#include "model/modelanimation.h"
#include "species_defs/species_defs.h"

class LabUi {
  public:
	void create_ui();
	void object_changed();
	void closeUi();

  private:
	void build_options_menu();
	void build_ship_list() const;
	void build_species_entry(species_info species_def, int species_idx) const;
	void build_weapon_list() const;
	void build_weapon_subtype_list() const;
	void build_background_list() const;
	void show_render_options();
	void show_object_options() const;
	void show_object_selector() const;
	void show_background_selector() const;
	void build_toolbar_entries();
	void build_texture_quality_combobox();
	void build_antialiasing_combobox();
	void build_tone_mapper_combobox();
	void build_table_info_txtbox(ship_info* sip) const;
	void build_model_info_box(ship_info* sip, polymodel* pm) const;
	void build_subsystem_list(object* objp, ship* shipp) const;
	void build_subsystem_list_entry(SCP_string& subsys_name,
		SCP_vector<bool>& show_subsys,
		const size_t& subsys_index,
		ship_subsys* cur_subsys,
		object* objp,
		ship* shipp) const;
	void build_weapon_options(ship* shipp) const;
	void build_primary_weapon_combobox(SCP_string& text,
		weapon_info* wip,
		int& primary_slot) const;
	void build_secondary_weapon_combobox(SCP_string& text, weapon_info* wip, int& secondary_slot) const;
	void build_animation_options(ship* shipp, ship_info* sip) const;
	void create_afterburner_animation_node(
		const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers) const;
	void create_secondary_weapon_anim_node(
		ship* shipp,
		ship_info* sip) const;
	void create_primary_weapon_anim_node(
		ship* shipp,
		ship_info* sip) const;
	void build_model_info_box_actual(ship_info* sip, polymodel* pm) const;
	void build_team_color_combobox() const;
	void reset_animations(ship* shipp, ship_info* sip) const;
	void do_triggered_anim(animation::ModelAnimationTriggerType type,
		const SCP_string& name,
		bool direction,
		int subtype = animation::ModelAnimationSet::SUBTYPE_DEFAULT) const;
	void maybe_show_animation_category(const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers,
		animation::ModelAnimationTriggerType trigger_type,
		SCP_string label) const;

	// if this is true, the displayed object has changed and so every piece of cached data related to
	// the object must be invalidated
	bool rebuild_after_object_change = false;

	// these flags track the state of open windows. We start with the object selector open and all other
	// subdialogs closed
	bool show_render_options_dialog = false;
	bool show_object_selection_dialog = true;
	bool show_object_options_dialog = false;
	bool show_background_selection_dialog = true;

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