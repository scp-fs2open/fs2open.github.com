#include "lab_ui.h"

#include "lab_ui_helpers.h"

#include "graphics/debug_sphere.h"
#include "lab/labv2_internal.h"
#include "ship/shiphit.h"
#include "weapon/weapon.h"

void LabUi::objectChanged()
{
	rebuildAfterObjectChange = true;
}


void LabUi::buildShipList() const
{
	int species_idx = 0;
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	for (auto const& species_def : Species_info) {
		with_TreeNode(species_def.species_name) {
			int ship_info_idx = 0;

			for (auto const& class_def : Ship_info) {
				if (class_def.species == species_idx) {
					ImGui::TreeNodeEx((void*)(intptr_t)ship_info_idx, node_flags, class_def.name);
					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						getLabManager()->changeDisplayedObject(LabMode::Ship, ship_info_idx);
					}
				}
				ship_info_idx++;
			}
		}
		species_idx++;
	}
}

void LabUi::buildWeaponList() const
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	for (auto i = 0; i < Num_weapon_subtypes; ++i) {
		with_TreeNode(Weapon_subtype_names[i]) {
			int weapon_idx = 0;

			for (auto const& class_def : Weapon_info) {
				if ((i == 2 && class_def.wi_flags[Weapon::Info_Flags::Beam]) ||
					(class_def.subtype == i && !class_def.wi_flags[Weapon::Info_Flags::Beam])) {
					ImGui::TreeNodeEx((void*)(intptr_t)weapon_idx, node_flags, class_def.name);

					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
						getLabManager()->changeDisplayedObject(LabMode::Weapon, weapon_idx);
					}
				}
				weapon_idx++;
			}
		}
	}
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

	ImGui::TreeNodeEx("None", node_flags, "None");
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		getLabManager()->Renderer->useBackground("None");
	}

	for (auto const& directory : directories) {
		with_TreeNode(directory.first.c_str()) {
			for (const auto& mission : directory.second) {
				ImGui::TreeNodeEx(mission.c_str(), node_flags, "%s", mission.c_str());

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					getLabManager()->Renderer->useBackground(mission);
				}
			}
		}
	}
}

void LabUi::createUi()
{
	if (show_render_options)
		showRenderOptions();

	if (show_object_options)
		showObjectOptions();

	with_MainMenuBar {
		with_Menu("Options") {
			ImGui::MenuItem("Render options", NULL, &show_render_options);
			ImGui::MenuItem("Object selector", NULL, &show_object_selector);
			ImGui::MenuItem("Object options", NULL, &show_object_options);
			ImGui::MenuItem("Close lab", NULL, &close_lab);
		}
	}

	if (close_lab) {
		getLabManager()->notify_close();
		close_lab = false;
	}

	if (show_object_selector) {
		with_Window("Select object and background")
		{

			if (ImGui::CollapsingHeader("Displayed Object")) {
				with_TreeNode("Ship Classes")
				{
					buildShipList();
				}

				with_TreeNode("Weapon Classes")
				{
					buildWeaponList();
				}
			}

			if (ImGui::CollapsingHeader("Mission Background")) {
				buildBackgroundList();
			}
		}
	}

	rebuildAfterObjectChange = false;
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

void LabUi::showRenderOptions()
{
	int bloom_level = gr_bloom_intensity();
	float ambient_factor = lighting_profile::lab_get_ambient();
	float light_factor = lighting_profile::lab_get_light();
	float emissive_factor = lighting_profile::lab_get_emissive();
	float exposure_level = lighting_profile::current_exposure();
	auto ppcv = lighting_profile::lab_get_ppc();

	with_Window("Render options")
	{
		ImGui::Checkbox("Enable Model Rotation", &enable_model_rotation);

		if (ImGui::CollapsingHeader("Model features")) {
			ImGui::Checkbox("Rotate/Translate Subsystems", &animate_subsystems);
			ImGui::Checkbox("Show full detail", &show_full_detail);
			ImGui::Checkbox("Show thrusters", &show_thrusters);
			ImGui::Checkbox("Show afterburners", &show_afterburners);
			ImGui::Checkbox("Show weapons", &show_weapons);
			ImGui::Checkbox("Show Insignia", &show_insignia);
			ImGui::Checkbox("Show damage lightning", &show_damage_lightning);
			ImGui::Checkbox("No glowpoints", &no_glowpoints);
		}

		if (ImGui::CollapsingHeader("Texture options")) {
			ImGui::Checkbox("Diffuse map", &diffuse_map);
			ImGui::Checkbox("Glow map", &glow_map);
			ImGui::Checkbox("Specular map", &spec_map);
			ImGui::Checkbox("Reflection map", &reflect_map);
			ImGui::Checkbox("Environment map", &env_map);
			ImGui::Checkbox("Normal map", &normal_map);
			ImGui::Checkbox("Height map", &height_map);
			ImGui::Checkbox("Misc map", &misc_map);
			ImGui::Checkbox("AO map", &ao_map);

			const char* texture_quality_settings[] = {
				"Minimum",
				"Low",
				"Medium",
				"High",
				"Maximum",
			};

			with_Combo("Texture quality",
					texture_quality_settings[static_cast<int>(getLabManager()->Renderer->getTextureQuality())]) {
				for (int n = 0; n < IM_ARRAYSIZE(texture_quality_settings); n++) {
					bool is_selected = n == static_cast<int>(getLabManager()->Renderer->getTextureQuality());
					if (ImGui::Selectable(texture_quality_settings[n], is_selected))
						getLabManager()->Renderer->setTextureQuality(static_cast<TextureQuality>(n));

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}
		}

		if (ImGui::CollapsingHeader("Scene rendering options")) {
			ImGui::Checkbox("Hide Post Processing", &hide_post_processing);
			ImGui::Checkbox("Render as wireframe", &use_wireframe_rendering);
			ImGui::Checkbox("Render without light", &no_lighting);
			ImGui::Checkbox("Render with emissive lighting", &show_emissive_lighting);
			ImGui::SliderFloat("Light brightness", &light_factor, 0.0f, 10.0f);
			ImGui::SliderFloat("Ambient factor", &ambient_factor, 0.0f, 10.0f);
			ImGui::SliderFloat("Emissive amount", &emissive_factor, 0.0f, 10.0f);
			ImGui::SliderFloat("Exposure", &exposure_level, 0.0f, 8.0f);
			ImGui::SliderInt("Bloom level", &bloom_level, 0, 200);

			with_Combo("Antialiasing method", antialiasing_settings[static_cast<int>(Gr_aa_mode)]) {
				for (int n = 0; n < IM_ARRAYSIZE(antialiasing_settings); n++) {
					bool is_selected = static_cast<int>(Gr_aa_mode) == n;

					if (ImGui::Selectable(antialiasing_settings[n], is_selected))
						getLabManager()->Renderer->setAAMode(static_cast<AntiAliasMode>(n));

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}

			with_Combo("Tonemapper",
					lighting_profile::tonemapper_to_name(lighting_profile::current_tonemapper()).c_str()) {
				for (int n = 0; n < IM_ARRAYSIZE(tonemappers); n++) {
					const bool is_selected =
						lighting_profile::tonemapper_to_name(lighting_profile::current_tonemapper()) == tonemappers[n];
					if (ImGui::Selectable(tonemappers[n].c_str(), is_selected))
						lighting_profile::lab_set_tonemapper(lighting_profile::name_to_tonemapper(tonemappers[n]));

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}

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
	}

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

std::map<animation::ModelAnimationTriggerType, std::map<SCP_string, bool>> manual_animation_triggers = {};
std::map<animation::ModelAnimationTriggerType, bool> manual_animations = {};

std::array<bool, MAX_SHIP_PRIMARY_BANKS> triggered_primary_banks;
std::array<bool, MAX_SHIP_SECONDARY_BANKS> triggered_secondary_banks;

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
	

void LabUi::showObjectOptions() const
{
	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	with_Window("Object Information")
	{
		if (getLabManager()->CurrentMode == LabMode::Ship) {
			auto objp = &Objects[getLabManager()->CurrentObject];
			auto shipp = &Ships[objp->instance];
			auto sip = &Ship_info[shipp->ship_info_index];
			auto pm = model_get(sip->model_num);


			if (ImGui::CollapsingHeader(sip->name)) {
				with_TreeNode("Table information"){
					static SCP_string table_text;

					if (table_text.length() == 0 || rebuildAfterObjectChange)
						table_text = get_ship_table_text(sip);

					ImGui::InputTextMultiline("##table_text",
						const_cast<char*>(table_text.c_str()),
						table_text.length(),
						ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
						ImGuiInputTextFlags_ReadOnly);
				}

				with_TreeNode("Model information") {
					with_Table("model info", 2, flags) {
						IMGUI_TABLE_ENTRY("Model File", sip->pof_file)
						IMGUI_TABLE_ENTRY("Target model", sip->pof_file_hud)
						IMGUI_TABLE_ENTRY("Techroom model", sip->pof_file_tech)
						IMGUI_TABLE_ENTRY("Cockpit model", sip->cockpit_pof_file)
						IMGUI_TABLE_ENTRY("POF version", std::to_string(pm->version).c_str())
					}
				}
				
				with_TreeNode("Subsystems") {
					static ship_subsys* selected_subsys = nullptr;
					int subsys_index = 0;

					for (auto cur_subsys = GET_FIRST(&shipp->subsys_list);
						 cur_subsys != END_OF_LIST(&shipp->subsys_list);
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

						with_TreeNode(subsys_name.c_str()) {

							ImGui::TreeNodeEx("Highlight system", leaf_flags);
							if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
								selected_subsys = cur_subsys;
							}
						}

						subsys_index++;
					}
				}
			}
		}

		if (ImGui::CollapsingHeader("Object actions")) {
			if (getLabManager()->CurrentMode == LabMode::Ship) {
				if (getLabManager()->isSafeForShips()) {
					if (ImGui::Button("Destroy ship")) {
						if (Objects[getLabManager()->CurrentObject].type == OBJ_SHIP) {
							auto obj = &Objects[getLabManager()->CurrentObject];

							obj->flags.remove(Object::Object_Flags::Player_ship);
							ship_self_destruct(obj);
						}
					}
				}
			}
		}
	}
}

