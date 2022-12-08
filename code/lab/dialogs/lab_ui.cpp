#include "lab_ui.h"

#include "lab_ui_helpers.h"

#include "graphics/debug_sphere.h"
#include "lab/labv2_internal.h"
#include "ship/shiphit.h"
#include "weapon/weapon.h"

std::map<animation::ModelAnimationTriggerType, std::map<SCP_string, bool>> manual_animation_triggers = {};
std::map<animation::ModelAnimationTriggerType, bool> manual_animations = {};

std::array<bool, MAX_SHIP_PRIMARY_BANKS> triggered_primary_banks = {false, false, false};
std::array<bool, MAX_SHIP_SECONDARY_BANKS> triggered_secondary_banks = {false, false, false, false};

void LabUi::objectChanged()
{
	rebuild_after_object_change = true;

	manual_animation_triggers.clear();

	for (auto& trigger : triggered_primary_banks)
		trigger = false;
	for (auto& trigger : triggered_secondary_banks)
		trigger = false;
}

void LabUi::buildSpeciesEntry(species_info species_def, int species_idx) const
{
	with_TreeNode(species_def.species_name)
	{
		int ship_info_idx = 0;

		for (auto const& class_def : Ship_info) {
			if (class_def.species == species_idx) {
				ImGui::TreeNodeEx((void*)(intptr_t)ship_info_idx,
					ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
					class_def.name);
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					getLabManager()->changeDisplayedObject(LabMode::Ship, ship_info_idx);
				}
			}
			ship_info_idx++;
		}
	}
}

void LabUi::buildShipList() const
{
	with_TreeNode("Ship Classes")
	{
		int species_idx = 0;
		for (auto const& species_def : Species_info) {
			buildSpeciesEntry(species_def, species_idx);
			species_idx++;
		}
	}
}

void LabUi::buildWeaponSubtypeList() const
{
	for (auto weapon_subtype_idx = 0; weapon_subtype_idx < Num_weapon_subtypes; ++weapon_subtype_idx) {
		with_TreeNode(Weapon_subtype_names[weapon_subtype_idx])
		{
			int weapon_idx = 0;

			for (auto const& class_def : Weapon_info) {
				if ((weapon_subtype_idx == 2 && class_def.wi_flags[Weapon::Info_Flags::Beam]) ||
					(class_def.subtype == weapon_subtype_idx && !class_def.wi_flags[Weapon::Info_Flags::Beam])) {
					ImGui::TreeNodeEx((void*)(intptr_t)weapon_idx,
						ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
						class_def.name);

					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						getLabManager()->changeDisplayedObject(LabMode::Weapon, weapon_idx);
					}
				}
				weapon_idx++;
			}
		}
	}
}

void LabUi::buildWeaponList() const
{
	//weapon display needs to be rethought

	//with_TreeNode("Weapon Classes")
	//{
	//	buildWeaponSubtypeList();
	//}
}

void LabUi::buildBackgroundList() const
{
	SCP_vector<SCP_string> missions;

	cf_get_file_list(missions, CF_TYPE_MISSIONS, NOX("*.fs2"));

	static SCP_map<SCP_string, SCP_vector<SCP_string>> directories;

	if (directories.empty()) {
		for (const auto& filename : missions) {
			auto res = cf_find_file_location((filename + ".fs2").c_str(), CF_TYPE_MISSIONS);

			if (res.found) {
				auto location = get_directory_or_vp(res.full_name.c_str());

				directories[location].push_back(filename);
			}
		}
	}

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	ImGui::TreeNodeEx(LAB_MISSION_NONE_STRING, node_flags, LAB_MISSION_NONE_STRING);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		getLabManager()->Renderer->useBackground(LAB_MISSION_NONE_STRING);
	}

	for (auto const& directory : directories) {
		auto directory_name = directory.first.c_str();
		// if the directory name is empty, this indicates loose files in the root data directory
		// since imgui requires tree nodes to have non-empty names, we substitute a static string here
		if (!strlen(directory_name))
			directory_name = "Root";

		with_TreeNode(directory_name)
		{
			for (const auto& mission : directory.second) {
				ImGui::TreeNodeEx(mission.c_str(), node_flags, "%s", mission.c_str());

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					getLabManager()->Renderer->useBackground(mission);
				}
			}
		}
	}
}

void LabUi::buildOptionsMenu()
{
	with_Menu("Options")
	{
		ImGui::MenuItem("Render options", NULL, &show_render_options);
		ImGui::MenuItem("Object selector", NULL, &show_object_selector);
		ImGui::MenuItem("Object options", NULL, &show_object_options);
		ImGui::MenuItem("Close lab", "ESC", &close_lab);
	}
}

void LabUi::buildToolbarEntries()
{
	with_MainMenuBar
	{
		buildOptionsMenu();
	}

	if (close_lab) {
		getLabManager()->notify_close();
		close_lab = false;
	}
}

void LabUi::createUi()
{
	if (show_render_options)
		showRenderOptions();

	if (show_object_options)
		showObjectOptions();

	buildToolbarEntries();

	if (show_object_selector) {
		with_Window("Select object and background")
		{

			with_CollapsingHeader("Displayed Object")
			{
				buildShipList();

				buildWeaponList();
			}

			with_CollapsingHeader("Mission Background")
			{
				buildBackgroundList();
			}
		}
	}

	rebuild_after_object_change = false;
}

const char* antialiasing_settings[] = {
	"None",
	"FXAA Low",
	"FXAA Medium",
	"FXAA High",
	"SMAA Low",
	"SMAA Medium",
	"SMAA High",
	"SMAA Ultra",
};

SCP_string tonemappers[] = {
	"Linear",
	"Uncharted",
	"ACES",
	"ACES Approximate",
	"Cineon",
	"Piecewise Power Curve",
	"Piecewise Power Curve (RGB)",
	"Reinhard Extended",
	"Reinhard Jodie",
};

const char* texture_quality_settings[] = {
	"Minimum",
	"Low",
	"Medium",
	"High",
	"Maximum",
};

void LabUi::buildTextureQualityCombobox()
{
	with_Combo("Texture quality",
		texture_quality_settings[static_cast<int>(getLabManager()->Renderer->getTextureQuality())])
	{
		for (int n = 0; n < IM_ARRAYSIZE(texture_quality_settings); n++) {
			bool is_selected = n == static_cast<int>(getLabManager()->Renderer->getTextureQuality());
			if (ImGui::Selectable(texture_quality_settings[n], is_selected))
				getLabManager()->Renderer->setTextureQuality(static_cast<TextureQuality>(n));

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
	}
}

void buildTeamColorCombobox()
{
	if (!Team_Colors.empty()) {
		with_Combo("Team Color setting", getLabManager()->Renderer->getCurrentTeamColor().c_str())
		{
			for (auto team_color : Team_Colors) {
				bool is_selected = team_color.first == getLabManager()->Renderer->getCurrentTeamColor();

				if (ImGui::Selectable(team_color.first.c_str(), is_selected)) {
					getLabManager()->Renderer->setTeamColor(team_color.first);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
	}
}

void LabUi::buildAntialiasingCombobox()
{
	with_Combo("Antialiasing method", antialiasing_settings[static_cast<int>(Gr_aa_mode)])
	{
		for (int n = 0; n < IM_ARRAYSIZE(antialiasing_settings); n++) {
			bool is_selected = static_cast<int>(Gr_aa_mode) == n;

			if (ImGui::Selectable(antialiasing_settings[n], is_selected))
				getLabManager()->Renderer->setAAMode(static_cast<AntiAliasMode>(n));

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
	}
}

void LabUi::buildToneMapperCombobox()
{
	with_Combo("Tonemapper", lighting_profile::tonemapper_to_name(lighting_profile::current_tonemapper()).c_str())
	{
		for (int n = 0; n < IM_ARRAYSIZE(tonemappers); n++) {
			const bool is_selected =
				lighting_profile::tonemapper_to_name(lighting_profile::current_tonemapper()) == tonemappers[n];
			if (ImGui::Selectable(tonemappers[n].c_str(), is_selected))
				lighting_profile::lab_set_tonemapper(lighting_profile::name_to_tonemapper(tonemappers[n]));

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
	}
}

void LabUi::showRenderOptions()
{
	int bloom_level = gr_bloom_intensity();
	float ambient_factor = lighting_profile::lab_get_ambient();
	float light_factor = lighting_profile::lab_get_light();
	float emissive_factor = lighting_profile::lab_get_emissive();
	float exposure_level = lighting_profile::current_exposure();
	auto ppcv = lighting_profile::lab_get_ppc();

	bool skip_setting_light_options_this_frame = false;

	with_Window("Render options")
	{
		ImGui::Checkbox("Enable Model Rotation", &enable_model_rotation);

		with_CollapsingHeader("Model features")
		{
			ImGui::Checkbox("Rotate/Translate Subsystems", &animate_subsystems);
			ImGui::Checkbox("Show full detail", &show_full_detail);
			ImGui::Checkbox("Show thrusters", &show_thrusters);
			ImGui::Checkbox("Show afterburners", &show_afterburners);
			ImGui::Checkbox("Show weapons", &show_weapons);
			ImGui::Checkbox("Show Insignia", &show_insignia);
			ImGui::Checkbox("Show damage lightning", &show_damage_lightning);
			ImGui::Checkbox("No glowpoints", &no_glowpoints);
		}

		with_CollapsingHeader("Texture options")
		{
			ImGui::Checkbox("Diffuse map", &diffuse_map);
			ImGui::Checkbox("Glow map", &glow_map);
			ImGui::Checkbox("Specular map", &spec_map);
			ImGui::Checkbox("Reflection map", &reflect_map);
			ImGui::Checkbox("Environment map", &env_map);
			ImGui::Checkbox("Normal map", &normal_map);
			ImGui::Checkbox("Height map", &height_map);
			ImGui::Checkbox("Misc map", &misc_map);
			ImGui::Checkbox("AO map", &ao_map);

			buildTextureQualityCombobox();

			buildTeamColorCombobox();
		}

		with_CollapsingHeader("Scene rendering options")
		{
			ImGui::Checkbox("Hide Post Processing", &hide_post_processing);
			ImGui::Checkbox("Render as wireframe", &use_wireframe_rendering);
			ImGui::Checkbox("Render without light", &no_lighting);
			ImGui::Checkbox("Render with emissive lighting", &show_emissive_lighting);
			ImGui::SliderFloat("Light brightness", &light_factor, 0.0f, 10.0f);
			ImGui::SliderFloat("Ambient factor", &ambient_factor, 0.0f, 10.0f);
			ImGui::SliderFloat("Emissive amount", &emissive_factor, 0.0f, 10.0f);
			ImGui::SliderFloat("Exposure", &exposure_level, 0.0f, 8.0f);
			ImGui::SliderInt("Bloom level", &bloom_level, 0, 200);

			buildAntialiasingCombobox();

			buildToneMapperCombobox();

			if (lighting_profile::current_tonemapper() == tnm_PPC ||
				lighting_profile::current_tonemapper() == tnm_PPC_RGB) {
				ImGui::SliderFloat("PPC Toe Strength", &ppcv.toe_strength, 0.0f, 1.0f);
				ImGui::SliderFloat("PPC Toe Length", &ppcv.toe_length, 0.0f, 1.0f);
				ImGui::SliderFloat("PPC Shoulder Angle", &ppcv.shoulder_angle, 0.0f, 1.0f);
				ImGui::SliderFloat("PPC Shoulder Length", &ppcv.shoulder_length, 0.0f, 10.0f);
				ImGui::SliderFloat("PPC Shoulder Strength", &ppcv.shoulder_strength, 0.0f, 1.0f);
			}
		}

		if (getLabManager()->Renderer->currentMissionBackground != LAB_MISSION_NONE_STRING) {
			if (ImGui::Button("Export environment cubemap", ImVec2(-FLT_MIN, ImGui::GetTextLineHeight()*2))) {
				gr_dump_envmap(getLabManager()->Renderer->currentMissionBackground.c_str());
			}
		}

		if (graphics_options_changed()) {
			if (ImGui::Button("Reset graphics settings", ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2))) {
				getLabManager()->resetGraphicsSettings();

				// In order to make the reset button work, we can't set anything here this frame; we'll wait until
				// the next run through this method to update the state then.
				skip_setting_light_options_this_frame = true;
			}
		}
	}

	if (!skip_setting_light_options_this_frame) {
		getLabManager()->Flags.set(ManagerFlags::ModelRotationEnabled, enable_model_rotation);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowInsignia, show_insignia);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowDamageLightning, show_damage_lightning);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::HidePostProcessing, hide_post_processing);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoDiffuseMap, !diffuse_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoGlowMap, !glow_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoSpecularMap, !spec_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoReflectMap, !reflect_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoEnvMap, !env_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoNormalMap, !normal_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoHeightMap, !height_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoMiscMap, !misc_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoAOMap, !ao_map);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoGlowpoints, no_glowpoints);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowWireframe, use_wireframe_rendering);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoLighting, no_lighting);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowFullDetail, show_full_detail);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowThrusters, show_thrusters);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowAfterburners, show_afterburners);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowWeapons, show_weapons);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowEmissiveLighting, show_emissive_lighting);
		getLabManager()->Renderer->setEmissiveFactor(emissive_factor);
		getLabManager()->Renderer->setAmbientFactor(ambient_factor);
		getLabManager()->Renderer->setLightFactor(light_factor);
		getLabManager()->Renderer->setBloomLevel(bloom_level);
		getLabManager()->Renderer->setExposureLevel(exposure_level);
		getLabManager()->Renderer->setPPCValues(ppcv);
	}
}

void do_triggered_anim(animation::ModelAnimationTriggerType type,
	const SCP_string& name,
	bool direction,
	int subtype = animation::ModelAnimationSet::SUBTYPE_DEFAULT)
{
	if (getLabManager()->isSafeForShips()) {
		auto shipp = &Ships[Objects[getLabManager()->CurrentObject].instance];

		if (subtype != animation::ModelAnimationSet::SUBTYPE_DEFAULT)
			Ship_info[shipp->ship_info_index]
				.animations.getAll(model_get_instance(shipp->model_instance_num), type, subtype, true)
				.start(direction ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD);
		else
			Ship_info[shipp->ship_info_index]
				.animations.get(model_get_instance(shipp->model_instance_num), type, name)
				.start(direction ? animation::ModelAnimationDirection::RWD : animation::ModelAnimationDirection::FWD);
	}
}

#define IMGUI_TABLE_ENTRY(colA, colB)     \
	ImGui::TableNextRow();                \
	ImGui::TableSetColumnIndex(0);        \
	ImGui::TextUnformatted(colA);		  \
	ImGui::TableSetColumnIndex(1);		  \
	ImGui::TextUnformatted(colB);         \


void LabUi::buildTableInfoTxtbox(ship_info* sip) const
{
	with_TreeNode("Table information")
	{
		static SCP_string table_text;

		if (table_text.length() == 0 || rebuild_after_object_change)
			table_text = get_ship_table_text(sip);

		ImGui::InputTextMultiline("##table_text",
			const_cast<char*>(table_text.c_str()),
			table_text.length(),
			ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
			ImGuiInputTextFlags_ReadOnly);
	}
}

void buildModelInfoBox_actual(ship_info* sip, polymodel* pm)
{
	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

	with_Table("model info", 2, flags)
	{
		IMGUI_TABLE_ENTRY("Model File", sip->pof_file)
		IMGUI_TABLE_ENTRY("Target model", sip->pof_file_hud)
		IMGUI_TABLE_ENTRY("Techroom model", sip->pof_file_tech)
		IMGUI_TABLE_ENTRY("Cockpit model", sip->cockpit_pof_file)
		IMGUI_TABLE_ENTRY("POF version", std::to_string(pm->version).c_str());
		IMGUI_TABLE_ENTRY("Materials used", std::to_string(pm->n_textures).c_str());
		IMGUI_TABLE_ENTRY("Radius", std::to_string(pm->core_radius).c_str());
	}
}

void LabUi::buildModelInfoBox(ship_info* sip, polymodel* pm) const {
	with_TreeNode("Model information")
	{
		buildModelInfoBox_actual(sip, pm);
	}
}

void LabUi::buildSubsystemList(object* objp, ship* shipp) const {
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	with_TreeNode("Subsystems")
	{
		static ship_subsys* selected_subsys = nullptr;
		int subsys_index = 0;

		for (auto cur_subsys = GET_FIRST(&shipp->subsys_list); cur_subsys != END_OF_LIST(&shipp->subsys_list);
			 cur_subsys = GET_NEXT(cur_subsys)) {

			auto leaf_flags = node_flags;
			if (selected_subsys == cur_subsys) {
				vec3d pos;
				vm_vec_unrotate(&pos, &cur_subsys->system_info->pnt, &objp->orient);
				debug_sphere::add(pos, cur_subsys->system_info->radius);
				leaf_flags |= ImGuiTreeNodeFlags_Selected;
			}

			auto subsys_name_tmp = cur_subsys->sub_name;
			if (strlen(subsys_name_tmp) == 0)
				subsys_name_tmp = cur_subsys->system_info->name;

			SCP_string subsys_name;
			sprintf(subsys_name, "%s (%i)", subsys_name_tmp, subsys_index);

			with_TreeNode(subsys_name.c_str())
			{
				ImGui::TreeNodeEx("Highlight system", leaf_flags);
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					selected_subsys = cur_subsys;
				}

				ImGui::TreeNodeEx("Destroy subsystem", leaf_flags);
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					cur_subsys->current_hits = 0;
					do_subobj_destroyed_stuff(shipp, cur_subsys, nullptr);
				}
			}

			subsys_index++;
		}
	}
}

void LabUi::buildWeaponOptions(ship* shipp) const {
	with_TreeNode("Primaries")
	{
		auto bank = 0;
		for (auto& primary_slot : shipp->weapons.primary_bank_weapons) {
			if (primary_slot != -1) {
				auto wip = &Weapon_info[primary_slot];

				SCP_string text;
				sprintf(text, "##Primary bank %i", bank);
				with_Combo(text.c_str(), wip->name)
				{
					for (auto i = 0; i < Weapon_info.size(); i++) {
						if (Weapon_info[i].subtype == WP_MISSILE)
							continue;
						bool is_selected = i == primary_slot;
						if (ImGui::Selectable(Weapon_info[i].name, is_selected))
							primary_slot = i;
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::SameLine();
				static bool should_fire[MAX_SHIP_PRIMARY_BANKS] = {false, false, false};
				SCP_string cb_text;
				sprintf(cb_text, "Fire bank %i", bank);
				ImGui::Checkbox(cb_text.c_str(), &should_fire[bank]);
				if (should_fire[bank]) {
					getLabManager()->FirePrimaries |= 1 << bank;
				} else {
					getLabManager()->FirePrimaries &= ~(1 << bank);
				}

				bank++;
			}
		}
	}

	with_TreeNode("Secondaries")
	{
		auto bank = 0;
		for (auto& secondary_slot : shipp->weapons.secondary_bank_weapons) {
			if (secondary_slot != -1) {
				auto wip = &Weapon_info[secondary_slot];

				SCP_string text;
				sprintf(text, "##Secondary bank %i", bank);
				with_Combo(text.c_str(), wip->name)
				{
					for (auto i = 0; i < Weapon_info.size(); i++) {
						if (Weapon_info[i].subtype != WP_MISSILE)
							continue;
						bool is_selected = i == secondary_slot;
						if (ImGui::Selectable(Weapon_info[i].name, is_selected))
							secondary_slot = i;
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::SameLine();
				static bool should_fire[MAX_SHIP_SECONDARY_BANKS] = {false, false, false, false};
				SCP_string cb_text;
				sprintf(cb_text, "Fire bank %i##secondary", bank);
				ImGui::Checkbox(cb_text.c_str(), &should_fire[bank]);
				if (should_fire[bank]) {
					getLabManager()->FireSecondaries |= 1 << bank;
				} else {
					getLabManager()->FireSecondaries &= ~(1 << bank);
				}

				bank++;
			}
		}
	}
}

void reset_animations(ship* shipp, ship_info* sip)
{
	polymodel_instance* shipp_pmi = model_get_instance(shipp->model_instance_num);

	for (auto i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
		if (triggered_primary_banks[i]) {
			sip->animations.getAll(shipp_pmi, animation::ModelAnimationTriggerType::PrimaryBank, i)
				.start(animation::ModelAnimationDirection::RWD);
			triggered_primary_banks[i] = false;
		}
	}

	for (auto i = 0; i < MAX_SHIP_SECONDARY_BANKS; ++i) {
		if (triggered_secondary_banks[i]) {
			sip->animations.getAll(shipp_pmi, animation::ModelAnimationTriggerType::SecondaryBank, i)
				.start(animation::ModelAnimationDirection::RWD);
			triggered_secondary_banks[i] = false;
		}
	}

	for (auto entry : manual_animations) {
		if (manual_animations[entry.first]) {
			sip->animations.getAll(shipp_pmi, entry.first).start(animation::ModelAnimationDirection::RWD);
			manual_animations[entry.first] = false;
		}
	}

	for (const auto& entry : manual_animation_triggers) {
		auto animation_type = entry.first;
		sip->animations.getAll(shipp_pmi, animation_type).start(animation::ModelAnimationDirection::RWD);
	}
}

void maybeShowAnimationCategory(const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers,
	animation::ModelAnimationTriggerType trigger_type, SCP_string label)
{
	if (std::any_of(anim_triggers.begin(), anim_triggers.end(), [trigger_type](animation::ModelAnimationSet::RegisteredTrigger t) {
			return t.type == trigger_type;
		})) {
		with_TreeNode(label.c_str())
		{
			for (const auto& anim_trigger : anim_triggers) {
				if (anim_trigger.type == trigger_type) {

					if (ImGui::Button(anim_trigger.name.c_str())) {
						auto& scripted_triggers = manual_animation_triggers[trigger_type];
						auto direction = scripted_triggers[anim_trigger.name];
						do_triggered_anim(trigger_type,
							anim_trigger.name,
							direction,
							anim_trigger.subtype);
						scripted_triggers[anim_trigger.name] = !scripted_triggers[anim_trigger.name];
					}
				}
			}
		}
	}
}

void buildAnimationOptions(ship* shipp, ship_info* sip)
{
	with_TreeNode("Animations")
	{
		const auto& anim_triggers = sip->animations.getRegisteredTriggers();

		if (ImGui::Button("Reset animations")) {
			reset_animations(shipp, sip);
		}

		if (shipp->weapons.num_primary_banks > 0) {
			with_TreeNode("Primary Weapons##Anims")
			{
				for (auto i = 0; i < shipp->weapons.num_primary_banks; ++i) {
					SCP_string button_label;
					sprintf(button_label, "Trigger animation for primary bank %i", i);
					if (ImGui::Button(button_label.c_str())) {
						sip->animations
							.getAll(model_get_instance(shipp->model_instance_num),
								animation::ModelAnimationTriggerType::PrimaryBank,
								i)
							.start(triggered_primary_banks[i] ? animation::ModelAnimationDirection::RWD
															  : animation::ModelAnimationDirection::FWD);
						triggered_primary_banks[i] = !triggered_primary_banks[i];
					}
				}
			}
		}

		if (shipp->weapons.num_secondary_banks > 0) {
			with_TreeNode("Secondary Weapons##Anims")
			{
				for (auto i = 0; i < shipp->weapons.num_secondary_banks; ++i) {
					SCP_string button_label;
					sprintf(button_label, "Trigger animation for secondary bank %i", i);
					if (ImGui::Button(button_label.c_str())) {
						sip->animations
							.getAll(model_get_instance(shipp->model_instance_num),
								animation::ModelAnimationTriggerType::SecondaryBank,
								i)
							.start(triggered_secondary_banks[i] ? animation::ModelAnimationDirection::RWD
																: animation::ModelAnimationDirection::FWD);
						triggered_secondary_banks[i] = !triggered_secondary_banks[i];
					}
				}
			}
		}

		if (std::any_of(anim_triggers.begin(),
				anim_triggers.end(),
				[](animation::ModelAnimationSet::RegisteredTrigger t) {
					return t.type == animation::ModelAnimationTriggerType::Afterburner;
				})) {
			with_TreeNode("Afterburner")
			{
				if (ImGui::Button("Trigger afterburner animations")) {
					for (const auto& anim_trigger : anim_triggers) {
						if (anim_trigger.type == animation::ModelAnimationTriggerType::Afterburner) {
							auto& ab_triggers =
								manual_animation_triggers[animation::ModelAnimationTriggerType::Afterburner];
							auto direction = ab_triggers[anim_trigger.name];
							do_triggered_anim(animation::ModelAnimationTriggerType::Afterburner,
								anim_trigger.name,
								direction,
								anim_trigger.subtype);
							ab_triggers[anim_trigger.name] = !ab_triggers[anim_trigger.name];
						}
					}
				}
			}
		}

		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::TurretFiring,
			"Turret firing##anims");
		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::TurretFired,
			"Turret fired##anims");
		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::Scripted,
			"Scripted animations##anims");
		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::DockBayDoor,
			"Dock bay door##anims");
		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::Docking_Stage1,
			"Docking stage 1##anims");
		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::Docking_Stage2,
			"Docking stage 2##anims");
		maybeShowAnimationCategory(anim_triggers,
			animation::ModelAnimationTriggerType::Docking_Stage3,
			"Docking stage 3##anims");
	}
}

void LabUi::showObjectOptions() const
{
	
	with_Window("Object Information")
	{
		if (getLabManager()->CurrentMode == LabMode::Ship && getLabManager()->isSafeForShips()) {
			auto objp = &Objects[getLabManager()->CurrentObject];
			auto shipp = &Ships[objp->instance];
			auto sip = &Ship_info[shipp->ship_info_index];
			auto pm = model_get(sip->model_num);

			with_CollapsingHeader(sip->name)
			{
				buildTableInfoTxtbox(sip);

				buildModelInfoBox(sip, pm);

				buildSubsystemList(objp, shipp);
			}

			with_CollapsingHeader("Object actions")
			{
				if (getLabManager()->isSafeForShips()) {
					if (ImGui::Button("Destroy ship")) {
						if (Objects[getLabManager()->CurrentObject].type == OBJ_SHIP) {
							auto obj = &Objects[getLabManager()->CurrentObject];

							obj->flags.remove(Object::Object_Flags::Player_ship);
							ship_self_destruct(obj);
						}
					}

					buildAnimationOptions(shipp, sip);
				}
			}

			with_CollapsingHeader("Weapons")
			{
				buildWeaponOptions(shipp);
			}
		}
	}
}

