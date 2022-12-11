#pragma once

#include "model/model.h"
#include "model/modelanimation.h"
#include "species_defs/species_defs.h"

class LabUi {
  public:
	void createUi();
	void objectChanged();
	void closeUi();

  private:
	void buildOptionsMenu();
	void buildShipList() const;
	void buildSpeciesEntry(species_info species_def, int species_idx) const;
	void buildWeaponList() const;
	void buildWeaponSubtypeList() const;
	void buildBackgroundList() const;
	void showRenderOptions();
	void showObjectOptions() const;
	void buildToolbarEntries();
	void buildTextureQualityCombobox();
	void buildAntialiasingCombobox();
	void buildToneMapperCombobox();
	void buildTableInfoTxtbox(ship_info* sip) const;
	void buildModelInfoBox(ship_info* sip, polymodel* pm) const;
	void buildSubsystemList(object* objp, ship* shipp) const;
	void buildSubsystemListEntry(SCP_string& subsys_name,
		SCP_vector<bool>& show_subsys,
		const size_t& subsys_index,
		ship_subsys* cur_subsys,
		object* objp,
		ship* shipp) const;
	void buildWeaponOptions(ship* shipp) const;
	void buildPrimaryWeaponCombobox(SCP_string& text,
		weapon_info* wip,
		int& primary_slot) const;
	void buildSecondaryWeaponCombobox(SCP_string& text, weapon_info* wip, int& secondary_slot) const;
	void buildAnimationOptions(ship* shipp, ship_info* sip) const;
	void createAfterburnerAnimationNode(
		const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers) const;
	void createSecondaryWeaponAnimNode(
		ship* shipp,
		ship_info* sip) const;
	void createPrimaryWeaponAnimNode(
		ship* shipp,
		ship_info* sip) const;
	void buildModelInfoBox_actual(ship_info* sip, polymodel* pm) const;
	void buildTeamColorCombobox() const;
	void reset_animations(ship* shipp, ship_info* sip) const;
	void do_triggered_anim(animation::ModelAnimationTriggerType type,
		const SCP_string& name,
		bool direction,
		int subtype = animation::ModelAnimationSet::SUBTYPE_DEFAULT) const;
	void maybeShowAnimationCategory(const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers,
		animation::ModelAnimationTriggerType trigger_type,
		SCP_string label) const;

	// if this is true, the displayed object has changed and so every piece of cached data related to
	// the object must be invalidated
	bool rebuild_after_object_change = false;

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