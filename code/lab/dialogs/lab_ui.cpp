#include "lab_ui.h"
#include "lab/labv2_internal.h"
#include "ship/shiphit.h"
#include "weapon/weapon.h"


void LabUi::buildShipList() const
{
	int species_idx = 0;
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	for (auto const& species_def : Species_info) {
		if (ImGui::TreeNode(species_def.species_name)) {
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
			ImGui::TreePop();
		}
		species_idx++;
	}

	ImGui::TreePop();
}

void LabUi::buildWeaponList() const
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	for (auto i = 0; i < Num_weapon_subtypes; ++i) {
		if (ImGui::TreeNode(Weapon_subtype_names[i])) {
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

			ImGui::TreePop();
		}
	}

	ImGui::TreePop();
}

SCP_string get_directory_or_vp(const char* path)
{
	SCP_string result(path);

	// Is this a mission in a directory?
	size_t found = result.find("data" DIR_SEPARATOR_STR "missions");

	if (found == std::string::npos) // Guess not
	{
		found = result.find(".vp");
	}

	const auto directory_name_pos = result.rfind(DIR_SEPARATOR_CHAR, found - strlen(DIR_SEPARATOR_STR) - 1);

	result = result.substr(directory_name_pos, found - directory_name_pos);

	found = result.find(DIR_SEPARATOR_CHAR);
	// Strip directory separators
	while (found != std::string::npos) {
		result.erase(found, strlen(DIR_SEPARATOR_STR));
		found = result.find(DIR_SEPARATOR_CHAR);
	}

	return result;
}

void LabUi::buildBackgroundList() const
{
	SCP_vector<SCP_string> missions;

	cf_get_file_list(missions, CF_TYPE_MISSIONS, NOX("*.fs2"));

	SCP_map<SCP_string, SCP_vector<SCP_string>> directories;

	for (const auto& filename : missions) {
		auto res = cf_find_file_location((filename + ".fs2").c_str(), CF_TYPE_MISSIONS);

		if (res.found) {
			auto location = get_directory_or_vp(res.full_name.c_str());

			directories[location].push_back(filename);
		}
	}

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	ImGui::TreeNodeEx("None", node_flags, "None");
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		getLabManager()->Renderer->useBackground("None");
	}

	for (auto const& directory : directories) {
		if (ImGui::TreeNode(directory.first.c_str())) {
			for (const auto& mission : directory.second) {
				ImGui::TreeNodeEx(mission.c_str(), node_flags, "%s", mission.c_str());

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
					getLabManager()->Renderer->useBackground(mission);
				}
			}

			ImGui::TreePop();
		}
	}
}

void LabUi::createUi() const
{
	static bool show_render_options = false;
	static bool show_object_selector = true;
	static bool show_object_options = false;

	if (show_render_options)
		showRenderOptions();

	if (show_object_options)
		showObjectOptions();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Options")) {
			ImGui::MenuItem("Render options", NULL, &show_render_options);
			ImGui::MenuItem("Object selector", NULL, &show_object_selector);
			ImGui::MenuItem("Object options", NULL, &show_object_options);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (show_object_selector) {
		ImGui::Begin("Select object and background");

		if (ImGui::CollapsingHeader("Displayed Object")) {
			if (ImGui::TreeNode("Ship Classes")) {
				buildShipList();
			}

			if (ImGui::TreeNode("Weapon Classes")) {
				buildWeaponList();
			}
		}

		if (ImGui::CollapsingHeader("Mission Background")) {
			buildBackgroundList();
		}
	}

	ImGui::End();
}

void LabUi::showRenderOptions() const
{
	static bool enable_model_rotation = false;
	static bool show_insignia = false;
	static bool show_damage_lightning = false;
	static bool animate_subsystems = false;
	static bool hide_post_processing = false;
	static bool diffuse_map = true;
	static bool glow_map = true;
	static bool spec_map = true;
	static bool reflect_map = true;
	static bool env_map = true;
	static bool normal_map = true;
	static bool height_map = true;
	static bool misc_map = true;
	static bool ao_map = true;
	static bool no_glowpoints = false;
	static bool use_wireframe_rendering = false;
	static bool no_lighting = false;
	static bool show_full_detail = false;
	static bool show_thrusters = false;
	static bool show_afterburners = false;
	static bool show_weapons = false;
	static bool show_emissive_lighting = false;
	int bloom_level = gr_bloom_intensity();
	float ambient_factor = lighting_profile::lab_get_ambient();
	float light_factor = lighting_profile::lab_get_light();
	float emissive_factor = lighting_profile::lab_get_emissive();
	float exposure_level = lighting_profile::current_exposure();
	auto ppcv = lighting_profile::lab_get_ppc();

	ImGui::Begin("Render options");

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

		if (ImGui::BeginCombo("Texture quality", texture_quality_settings[static_cast<int>(getLabManager()->Renderer->getTextureQuality())])) {
			for (int n=0; n < IM_ARRAYSIZE(texture_quality_settings); n++) {
				const bool is_selected = n == static_cast<int>(getLabManager()->Renderer->getTextureQuality());
				if (ImGui::Selectable(texture_quality_settings[n], is_selected))
					getLabManager()->Renderer->setTextureQuality(static_cast<TextureQuality>(n));

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
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

		if (ImGui::BeginCombo("Antialiasing method", antialiasing_settings[static_cast<int>(Gr_aa_mode)])) {
			for (int n=0; n < IM_ARRAYSIZE(antialiasing_settings); n++) {
				const bool is_selected = static_cast<int>(Gr_aa_mode) == n;

				if (ImGui::Selectable(antialiasing_settings[n], is_selected))
					getLabManager()->Renderer->setAAMode(static_cast<AntiAliasMode>(n));

				if (is_selected)
					ImGui::SetItemDefaultFocus();

			}

			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo("Tonemapper", lighting_profile::tonemapper_to_name(lighting_profile::current_tonemapper()).c_str())) {
			for (int n = 0; n < IM_ARRAYSIZE(tonemappers); n++) {
				const bool is_selected =
					lighting_profile::tonemapper_to_name(lighting_profile::current_tonemapper()) == tonemappers[n];
				if (ImGui::Selectable(tonemappers[n].c_str(), is_selected)) 
					lighting_profile::lab_set_tonemapper(lighting_profile::name_to_tonemapper(tonemappers[n]));

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (lighting_profile::current_tonemapper() == tnm_PPC || lighting_profile::current_tonemapper() == tnm_PPC_RGB) {
			ImGui::SliderFloat("PPC Toe Strength", &ppcv.toe_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("PPC Toe Length", &ppcv.toe_length, 0.0f, 1.0f);
			ImGui::SliderFloat("PPC Shoulder Angle", &ppcv.shoulder_angle, 0.0f, 1.0f);
			ImGui::SliderFloat("PPC Shoulder Length", &ppcv.shoulder_length, 0.0f, 10.0f);
			ImGui::SliderFloat("PPC Shoulder Strength", &ppcv.shoulder_strength, 0.0f, 1.0f);
		}
	}

	ImGui::End();

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

void LabUi::showObjectOptions() const
{
	ImGui::Begin("Object Information");

	if (ImGui::CollapsingHeader("Actions")) {
		if (getLabManager()->CurrentMode == LabMode::Ship) {
			if (ImGui::Button("Destroy ship")) {
				if (Objects[getLabManager()->CurrentObject].type == OBJ_SHIP) {
					auto obj = &Objects[getLabManager()->CurrentObject];

					obj->flags.remove(Object::Object_Flags::Player_ship);
					ship_self_destruct(obj);
				}
			}
			
		}
	}
	if (ImGui::CollapsingHeader("Description texts")) {}
	if (ImGui::CollapsingHeader("Class values")) {}

	ImGui::End();
}
