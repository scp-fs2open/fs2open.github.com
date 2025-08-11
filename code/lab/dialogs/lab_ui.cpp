#include "lab_ui.h"
#include "globalincs/vmallocator.h"

#include "lab_ui_helpers.h"

#include "asteroid/asteroid.h"
#include "graphics/debug_sphere.h"
#include "graphics/matrix.h"
#include "lab/labv2_internal.h"
#include "lighting/lighting_profiles.h"
#include "ship/shiphit.h"
#include "weapon/weapon.h"
#include "mission/missionload.h"

using namespace ImGui;

std::map<animation::ModelAnimationTriggerType, std::map<SCP_string, bool>> manual_animation_triggers = {};
std::map<animation::ModelAnimationTriggerType, bool> manual_animations = {};

std::array<bool, MAX_SHIP_PRIMARY_BANKS> triggered_primary_banks = {false, false, false};
std::array<bool, MAX_SHIP_SECONDARY_BANKS> triggered_secondary_banks = {false, false, false, false};

void LabUi::object_changed()
{
	rebuild_after_object_change = true;

	manual_animation_triggers.clear();

	for (auto& trigger : triggered_primary_banks)
		trigger = false;
	for (auto& trigger : triggered_secondary_banks)
		trigger = false;
}

void LabUi::build_species_entry(const species_info &species_def, int species_idx) const
{
	with_TreeNode(species_def.species_name)
	{
		int ship_info_idx = 0;

		for (auto const& class_def : Ship_info) {
			if (class_def.species == species_idx) {
				SCP_string node_label;
				sprintf(node_label, "##ShipClassIndex%i", ship_info_idx);
				TreeNodeEx(node_label.c_str(),
					ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
					"%s", class_def.name);
				if (IsItemClicked() && !IsItemToggledOpen()) {
					getLabManager()->changeDisplayedObject(LabMode::Ship, ship_info_idx);
				}
			}
			ship_info_idx++;
		}
	}
}

void LabUi::build_ship_list() const
{
	with_TreeNode("Ship Classes")
	{
		int species_idx = 0;
		for (auto const& species_def : Species_info) {
			build_species_entry(species_def, species_idx);
			species_idx++;
		}
	}
}

void LabUi::build_weapon_subtype_list() const
{
	for (auto weapon_subtype_idx = 0; weapon_subtype_idx < Num_weapon_subtypes; ++weapon_subtype_idx) {
		with_TreeNode(Weapon_subtype_names[weapon_subtype_idx])
		{
			int weapon_idx = 0;

			for (auto const& class_def : Weapon_info) {
				if ((weapon_subtype_idx == 2 && class_def.wi_flags[Weapon::Info_Flags::Beam]) ||
					(class_def.subtype == weapon_subtype_idx && !class_def.wi_flags[Weapon::Info_Flags::Beam])) {
					SCP_string node_label;
					sprintf(node_label, "##WeaponClassIndex%i", weapon_idx);
					TreeNodeEx(node_label.c_str(),
						ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
						"%s", class_def.name);

					if (IsItemClicked() && !IsItemToggledOpen()) {
						getLabManager()->changeDisplayedObject(LabMode::Weapon, weapon_idx);
					}
				}
				weapon_idx++;
			}
		}
	}
}

void LabUi::build_asteroid_list()
{
	with_TreeNode("Asteroids")
	{
		int asteroid_idx = 0;

		for (const auto& info : Asteroid_info) {
			if (info.type == ASTEROID_TYPE_DEBRIS) {
				asteroid_idx++;
				continue;
			}

			int subtype_idx = 0;
			for (const auto& subtype : info.subtypes) {
				SCP_string node_label;
				sprintf(node_label, "##AsteroidClassIndex%i_%i", asteroid_idx, subtype_idx);
				TreeNodeEx(node_label.c_str(),
					ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
					"%s (%s)",
					info.name,
					subtype.type_name.c_str());

				if (IsItemClicked() && !IsItemToggledOpen()) {
					getLabManager()->changeDisplayedObject(LabMode::Asteroid, asteroid_idx, subtype_idx);
				}

				subtype_idx++;
			}

			asteroid_idx++;
		}
	}
}

void LabUi::build_debris_list()
{
	with_TreeNode("Debris")
	{
		int debris_idx = 0;

		for (const auto& info : Asteroid_info) {
			if (info.type != ASTEROID_TYPE_DEBRIS) {
				debris_idx++;
				continue;
			}

			SCP_string node_label;
			sprintf(node_label, "##DebrisClassIndex%i", debris_idx);
			TreeNodeEx(node_label.c_str(),
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
				"%s",
				info.name);

			if (IsItemClicked() && !IsItemToggledOpen()) {
				getLabManager()->changeDisplayedObject(LabMode::Asteroid, debris_idx, 0); // Debris subtype is always 0
			}

			debris_idx++;
		}
	}
}

void LabUi::build_weapon_list() const
{
	with_TreeNode("Weapon Classes")
	{
		build_weapon_subtype_list();
	}
}

void LabUi::build_object_list()
{
	with_TreeNode("Asteroid/Debris Types")
	{
		build_asteroid_list();
		build_debris_list();
	}
}

void LabUi::build_background_list() const
{
	SCP_vector<SCP_string> t_missions;

	cf_get_file_list(t_missions, CF_TYPE_MISSIONS, NOX("*.fs2"));

	// Remove any ignored missions from the list
	SCP_vector<SCP_string> missions;
	for (int i = 0; i < (int)t_missions.size(); i++) {
		if (!mission_is_ignored(t_missions[i].c_str())) {
			missions.push_back(t_missions[i]);
		}
	}

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

	TreeNodeEx(LAB_MISSION_NONE_STRING, node_flags, LAB_MISSION_NONE_STRING);
	if (IsItemClicked() && !IsItemToggledOpen()) {
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
			for (const auto& mission_name : directory.second) {
				TreeNodeEx(mission_name.c_str(), node_flags, "%s", mission_name.c_str());

				if (IsItemClicked() && !IsItemToggledOpen()) {
					getLabManager()->Renderer->useBackground(mission_name);
				}
			}
		}
	}
}

void LabUi::build_options_menu()
{
	with_Menu("Options")
	{
		MenuItem("Render options", nullptr, &show_render_options_dialog);
		MenuItem("Object selector", nullptr, &show_object_selection_dialog);
		MenuItem("Background selector", nullptr, &show_background_selection_dialog);
		MenuItem("Object options", nullptr, &show_object_options_dialog);
		MenuItem("Close lab", "ESC", &close_lab);
	}
}

void LabUi::build_toolbar_entries()
{
	with_MainMenuBar
	{
		build_options_menu();
	}

	if (close_lab) {
		getLabManager()->notify_close();
		close_lab = false;
	}
}

void LabUi::show_background_selector() const
{
	with_Window("Select background")
	{
		with_CollapsingHeader("Mission Background")
		{
			build_background_list();
		}
	}
}

void LabUi::show_object_selector() const
{
	with_Window("Select object")
	{
		with_CollapsingHeader("Displayed Object")
		{
			build_ship_list();

			build_weapon_list();

			build_object_list();
		}
	}
}

void LabUi::create_ui()
{
	build_toolbar_entries();

	if (show_render_options_dialog)
		show_render_options();

	if (show_object_options_dialog)
		show_object_options();

	if (show_background_selection_dialog)
		show_background_selector();

	if (show_object_selection_dialog)
		show_object_selector();

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

void LabUi::build_texture_quality_combobox()
{
	with_Combo("Texture quality",
		texture_quality_settings[static_cast<int>(getLabManager()->Renderer->getTextureQuality())])
	{
		for (int n = 0; n < IM_ARRAYSIZE(texture_quality_settings); n++) {
			bool is_selected = n == static_cast<int>(getLabManager()->Renderer->getTextureQuality());
			if (Selectable(texture_quality_settings[n], is_selected))
				getLabManager()->Renderer->setTextureQuality(static_cast<TextureQuality>(n));

			if (is_selected)
				SetItemDefaultFocus();
		}
	}
}

void LabUi::build_team_color_combobox() const
{
	if (!Team_Colors.empty()) {
		with_Combo("Team Color setting", getLabManager()->Renderer->getCurrentTeamColor().c_str())
		{
			for (const auto& team_color_name : Team_Colors) {
				bool is_selected = team_color_name.first == getLabManager()->Renderer->getCurrentTeamColor();

				if (Selectable(team_color_name.first.c_str(), is_selected)) {
					getLabManager()->Renderer->setTeamColor(team_color_name.first);
				}

				if (is_selected)
					SetItemDefaultFocus();
			}
		}
	}
}

void LabUi::build_antialiasing_combobox()
{
	with_Combo("Antialiasing method", antialiasing_settings[static_cast<int>(Gr_aa_mode)])
	{
		for (int n = 0; n < IM_ARRAYSIZE(antialiasing_settings); n++) {
			bool is_selected = static_cast<int>(Gr_aa_mode) == n;

			if (Selectable(antialiasing_settings[n], is_selected))
				getLabManager()->Renderer->setAAMode(static_cast<AntiAliasMode>(n));

			if (is_selected)
				SetItemDefaultFocus();
		}
	}
}
namespace ltp = lighting_profiles;
using namespace ltp;

void LabUi::build_tone_mapper_combobox()
{
	with_Combo("Tonemapper", ltp::tonemapper_to_name(ltp::current_tonemapper()).c_str())
	{
		for (int n = 0; n < IM_ARRAYSIZE(tonemappers); n++) {
			const bool is_selected =
				ltp::tonemapper_to_name(ltp::current_tonemapper()) == tonemappers[n];
			if (Selectable(tonemappers[n].c_str(), is_selected))
				ltp::lab_set_tonemapper(ltp::name_to_tonemapper(tonemappers[n]));

			if (is_selected)
				SetItemDefaultFocus();
		}
	}
}

void LabUi::show_render_options()
{
	auto profile_list = ltp::list_profiles();
	int bloom_level = gr_bloom_intensity();
	float ambient_factor = ltp::lab_get_ambient();
	float light_factor = ltp::lab_get_light();
	float emissive_factor = ltp::lab_get_emissive();
	float exposure_level = ltp::current_exposure();
	auto ppcv = ltp::lab_get_ppc();

	bool skip_setting_light_options_this_frame = false;

	with_Window("Render options")
	{
		Checkbox("Enable Model Rotation", &enable_model_rotation);

		with_CollapsingHeader("Model features")
		{
			if (getLabManager()->CurrentMode == LabMode::Ship) {
				Checkbox("Rotate/Translate Subsystems", &animate_subsystems);
			}
			Checkbox("Show full detail", &show_full_detail);
			if (getLabManager()->CurrentMode != LabMode::Asteroid) {
				Checkbox("Show thrusters", &show_thrusters);
				if (getLabManager()->CurrentMode == LabMode::Ship) {
					Checkbox("Show afterburners", &show_afterburners);
					Checkbox("Show weapons", &show_weapons);
					Checkbox("Show Insignia", &show_insignia);
					Checkbox("Show damage lightning", &show_damage_lightning);
				}
			}
			Checkbox("No glowpoints", &no_glowpoints);
		}

		with_CollapsingHeader("Texture options")
		{
			Checkbox("Diffuse map", &diffuse_map);
			Checkbox("Glow map", &glow_map);
			Checkbox("Specular map", &spec_map);
			Checkbox("Reflection map", &reflect_map);
			Checkbox("Environment map", &env_map);
			Checkbox("Normal map", &normal_map);
			Checkbox("Height map", &height_map);
			Checkbox("Misc map", &misc_map);
			Checkbox("AO map", &ao_map);

			build_texture_quality_combobox();

			build_team_color_combobox();
		}

		with_CollapsingHeader("Scene rendering options")
		{
			Checkbox("Hide Post Processing", &hide_post_processing);
			Checkbox("Hide particles", &no_particles);
			Checkbox("Render as wireframe", &use_wireframe_rendering);
			Checkbox("Render without light", &no_lighting);
			Checkbox("Render with emissive lighting", &show_emissive_lighting);
			SliderFloat("Light brightness", &light_factor, 0.0f, 10.0f);
			SliderFloat("Ambient factor", &ambient_factor, 0.0f, 10.0f);
			SliderFloat("Emissive amount", &emissive_factor, 0.0f, 10.0f);
			SliderFloat("Exposure", &exposure_level, 0.0f, 8.0f);
			SliderInt("Bloom level", &bloom_level, 0, 200);

			build_antialiasing_combobox();

			build_tone_mapper_combobox();

			if (ltp::current_tonemapper() == TonemapperAlgorithm::PPC ||
				ltp::current_tonemapper() == TonemapperAlgorithm::PPC_RGB) {
				SliderFloat("PPC Toe Strength", &ppcv.toe_strength, 0.0f, 1.0f);
				SliderFloat("PPC Toe Length", &ppcv.toe_length, 0.0f, 1.0f);
				SliderFloat("PPC Shoulder Angle", &ppcv.shoulder_angle, 0.0f, 1.0f);
				SliderFloat("PPC Shoulder Length", &ppcv.shoulder_length, 0.0f, 10.0f);
				SliderFloat("PPC Shoulder Strength", &ppcv.shoulder_strength, 0.0f, 1.0f);
			}

			if (profile_list.size()>1) {
				with_CollapsingHeader("Load lighting profile...") {
					for (const auto &s : profile_list) {
						if (Button(s.c_str(), ImVec2(-FLT_MIN, GetTextLineHeight() * 2))) {
							ltp::switch_to(s);
						}
					}
				}
			}
		}

		if (The_mission.volumetrics) {
			with_CollapsingHeader("Volumetric nebula options")
			{
				bool set_to_origin = static_cast<bool>(volumetrics_pos_backup);
				Checkbox("Move Volumetrics to Origin", &set_to_origin);
				if (set_to_origin != static_cast<bool>(volumetrics_pos_backup)) {
					if (set_to_origin) {
						volumetrics_pos_backup = The_mission.volumetrics->pos;
						The_mission.volumetrics->pos = vec3d ZERO_VECTOR;
					}
					else {
						The_mission.volumetrics->pos = *volumetrics_pos_backup;
						volumetrics_pos_backup.reset();
					}
				}
				Separator();
				Text("Quality Settings:");
				Checkbox("Enable Edge Smoothing", &The_mission.volumetrics->doEdgeSmoothing);
				SliderInt("Steps Until Opaque", &The_mission.volumetrics->steps, 2, 30);
				SliderInt("Steps Towards Sun", &The_mission.volumetrics->globalLightSteps, 2, 10);
				Separator();
				Text("Visibility Settings:");
				SliderFloat("Maximum Opacity", &The_mission.volumetrics->alphaLim, 0.0001f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);
				SliderFloat("Opacity Distance", &The_mission.volumetrics->opacityDistance, 0.1f, 100.0f);
				Separator();
				Text("Emissive Settings:");
				SliderFloat("Emissive Light Spreading", &The_mission.volumetrics->emissiveSpread, 0.1f, 2.0f);
				SliderFloat("Emissive Light Intensity", &The_mission.volumetrics->emissiveIntensity, 0.0f, 2.0f);
				SliderFloat("Emissive Light Falloff", &The_mission.volumetrics->emissiveFalloff, 0.1f, 2.0f);
				Separator();
				Text("Global Light Settings:");
				SliderFloat("Henyey-Greenstein", &The_mission.volumetrics->henyeyGreensteinCoeff, -1.0f, 1.0f);
				SliderFloat("Sun Falloff Factor", &The_mission.volumetrics->globalLightDistanceFactor, 0.1f, 3.0f);
				Separator();
				Text("Noise Settings:");
				Checkbox("Noise Active", &The_mission.volumetrics->noiseActive);
				SliderFloat("Noise Intensity", &The_mission.volumetrics->noiseColorIntensity, 0.0f, 3.0f);
				SliderFloat("Noise Scale Base", &std::get<0>(The_mission.volumetrics->noiseScale), 1.0f, 50.0f);
				SliderFloat("Noise Scale Sub", &std::get<1>(The_mission.volumetrics->noiseScale), 1.0f, 50.0f);
			}
		}

		if (getLabManager()->Renderer->currentMissionBackground != LAB_MISSION_NONE_STRING) {
			if (Button("Export environment cubemap", ImVec2(-FLT_MIN, GetTextLineHeight()*2))) {
				gr_dump_envmap(getLabManager()->Renderer->currentMissionBackground.c_str());
			}
		}

		if (graphics_options_changed()) {
			if (Button("Reset graphics settings", ImVec2(-FLT_MIN, GetTextLineHeight() * 2))) {
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
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::MoveSubsystems, animate_subsystems);
		getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoParticles, no_particles);
		getLabManager()->Renderer->setEmissiveFactor(emissive_factor);
		getLabManager()->Renderer->setAmbientFactor(ambient_factor);
		getLabManager()->Renderer->setLightFactor(light_factor);
		getLabManager()->Renderer->setBloomLevel(bloom_level);
		getLabManager()->Renderer->setExposureLevel(exposure_level);
		getLabManager()->Renderer->setPPCValues(ppcv);
	}
}

void LabUi::do_triggered_anim(animation::ModelAnimationTriggerType type,
	const SCP_string& name,
	bool direction,
	int subtype) const
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
	TableNextRow();                \
	TableSetColumnIndex(0);        \
	TextUnformatted(colA);		  \
	TableSetColumnIndex(1);		  \
	TextUnformatted(colB);         \


static void build_ship_table_info_txtbox(ship_info* sip)
{
	with_TreeNode("Table information")
	{
		// since this dialog is closed when the displayed object changes, we need to track whether or not
		// the text needs to be updated separately
		static SCP_string table_text;
		static int old_class = getLabManager()->CurrentClass;

		if (table_text.length() == 0 || old_class != getLabManager()->CurrentClass)
			table_text = get_ship_table_text(sip);

		InputTextMultiline("##table_text",
			const_cast<char*>(table_text.c_str()),
			table_text.length(),
			ImVec2(-FLT_MIN, GetTextLineHeight() * 16),
			ImGuiInputTextFlags_ReadOnly);
	}
}

static void build_weapon_table_info_txtbox(weapon_info* wip)
{
	with_TreeNode("Table information")
	{
		// Cache result across frames for performance
		static SCP_string table_text;
		static int old_class = getLabManager()->CurrentClass;

		if (table_text.length() == 0 || old_class != getLabManager()->CurrentClass) {
			table_text = get_weapon_table_text(wip);
		}

		InputTextMultiline("##weapon_table_text",
			const_cast<char*>(table_text.c_str()),
			table_text.length(),
			ImVec2(-FLT_MIN, GetTextLineHeight() * 16),
			ImGuiInputTextFlags_ReadOnly);
	}
}

void LabUi::build_model_info_box_actual(ship_info* sip, polymodel* pm) const
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

void LabUi::build_model_info_box(ship_info* sip, polymodel* pm) const {
	with_TreeNode("Model information")
	{
		build_model_info_box_actual(sip, pm);
	}
}

void render_subsystem(ship_subsys* ss, object* objp)
{
	SCP_string buf;

	auto pmi = model_get_instance(Ships[objp->instance].model_instance_num);
	auto pm = model_get(pmi->model_num);
	int subobj_num = ss->system_info->subobj_num;

	g3_start_frame(1);
	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	if (subobj_num != -1) {
		auto bsp = &pm->submodel[subobj_num];

		vec3d front_top_left = bsp->bounding_box[7];
		vec3d front_top_right = bsp->bounding_box[6];
		vec3d front_bot_left = bsp->bounding_box[4];
		vec3d front_bot_right = bsp->bounding_box[5];
		vec3d back_top_left = bsp->bounding_box[3];
		vec3d back_top_right = bsp->bounding_box[2];
		vec3d back_bot_left = bsp->bounding_box[0];
		vec3d back_bot_right = bsp->bounding_box[1];

		gr_set_color(255, 32, 32);

		// get into the frame of reference of the submodel
		int g3_count = 1;
		g3_start_instance_matrix(&objp->pos, &objp->orient, true);
		int mn = subobj_num;
		while ((mn >= 0) && (pm->submodel[mn].parent >= 0)) {
			g3_start_instance_matrix(&pm->submodel[mn].offset, &pmi->submodel[mn].canonical_orient, true);
			g3_count++;
			mn = pm->submodel[mn].parent;
		}

		// draw a cube around the subsystem
		g3_draw_htl_line(&front_top_left, &front_top_right);
		g3_draw_htl_line(&front_top_right, &front_bot_right);
		g3_draw_htl_line(&front_bot_right, &front_bot_left);
		g3_draw_htl_line(&front_bot_left, &front_top_left);

		g3_draw_htl_line(&back_top_left, &back_top_right);
		g3_draw_htl_line(&back_top_right, &back_bot_right);
		g3_draw_htl_line(&back_bot_right, &back_bot_left);
		g3_draw_htl_line(&back_bot_left, &back_top_left);

		g3_draw_htl_line(&front_top_left, &back_top_left);
		g3_draw_htl_line(&front_top_right, &back_top_right);
		g3_draw_htl_line(&front_bot_left, &back_bot_left);
		g3_draw_htl_line(&front_bot_right, &back_bot_right);

		// draw another cube around a gun for a two-part turret
		if ((ss->system_info->turret_gun_sobj >= 0) &&
			(ss->system_info->turret_gun_sobj != ss->system_info->subobj_num)) {
			bsp_info* bsp_turret = &pm->submodel[ss->system_info->turret_gun_sobj];

			front_top_left = bsp_turret->bounding_box[7];
			front_top_right = bsp_turret->bounding_box[6];
			front_bot_left = bsp_turret->bounding_box[4];
			front_bot_right = bsp_turret->bounding_box[5];
			back_top_left = bsp_turret->bounding_box[3];
			back_top_right = bsp_turret->bounding_box[2];
			back_bot_left = bsp_turret->bounding_box[0];
			back_bot_right = bsp_turret->bounding_box[1];

			g3_start_instance_matrix(&bsp_turret->offset,
				&pmi->submodel[ss->system_info->turret_gun_sobj].canonical_orient,
				true);

			g3_draw_htl_line(&front_top_left, &front_top_right);
			g3_draw_htl_line(&front_top_right, &front_bot_right);
			g3_draw_htl_line(&front_bot_right, &front_bot_left);
			g3_draw_htl_line(&front_bot_left, &front_top_left);

			g3_draw_htl_line(&back_top_left, &back_top_right);
			g3_draw_htl_line(&back_top_right, &back_bot_right);
			g3_draw_htl_line(&back_bot_right, &back_bot_left);
			g3_draw_htl_line(&back_bot_left, &back_top_left);

			g3_draw_htl_line(&front_top_left, &back_top_left);
			g3_draw_htl_line(&front_top_right, &back_top_right);
			g3_draw_htl_line(&front_bot_left, &back_bot_left);
			g3_draw_htl_line(&front_bot_right, &back_bot_right);

			g3_done_instance(true);
		}

		for (int i = 0; i < g3_count; i++)
			g3_done_instance(true);
	} else {
		vec3d subsys_position;
		vm_vec_add(&subsys_position, &objp->pos, &ss->system_info->pnt);
		vec3d rotated_position;
		vm_vec_unrotate(&rotated_position, &subsys_position, &objp->orient);

		color c;
		gr_init_alphacolor(&c, 255, 32, 32, 128);

		g3_draw_htl_sphere(&c,
			&rotated_position,
			ss->system_info->radius,
			ALPHA_BLEND_ALPHA_BLEND_ALPHA,
			ZBUFFER_TYPE_FULL);
	}

	gr_end_proj_matrix();
	gr_end_view_matrix();
	g3_end_frame();
}

void LabUi::build_subsystem_list(object* objp, ship* shipp) const {
	with_TreeNode("Subsystems")
	{
		size_t subsys_index = 0;
		static SCP_vector<bool> show_subsys;
		static int ship_class_idx = shipp->ship_info_index;
		if (ship_class_idx != shipp->ship_info_index) {
			ship_class_idx = shipp->ship_info_index;
			show_subsys.clear();
		}

		for (auto cur_subsys = GET_FIRST(&shipp->subsys_list); cur_subsys != END_OF_LIST(&shipp->subsys_list);
			 cur_subsys = GET_NEXT(cur_subsys)) {
			if (show_subsys.size() <= subsys_index)
				show_subsys.push_back(false);

			auto subsys_name_tmp = cur_subsys->sub_name;
			if (strlen(subsys_name_tmp) == 0)
				subsys_name_tmp = cur_subsys->system_info->name;

			SCP_string subsys_name;
			sprintf(subsys_name, "%s (%i)", subsys_name_tmp, (int)subsys_index);

			build_subsystem_list_entry(subsys_name, show_subsys, subsys_index, cur_subsys, objp, shipp);

			subsys_index++;
		}
	}
}

void LabUi::build_subsystem_list_entry(SCP_string& subsys_name,
	SCP_vector<bool>& show_subsys,
	const size_t& subsys_index,
	ship_subsys* cur_subsys,
	object* objp,
	ship* shipp) const
{
	with_TreeNode(subsys_name.c_str())
	{
		SCP_string node_name;
		sprintf(node_name, "Highlight system##%s", subsys_name.c_str());

		auto display_this = show_subsys[subsys_index] == true;

		Checkbox(node_name.c_str(), &display_this);

		if (display_this) {
			render_subsystem(cur_subsys, objp);
		}

		show_subsys[subsys_index] = display_this;

		sprintf(node_name, "Destroy system##%s", subsys_name.c_str());

		if (Button(node_name.c_str())) {
			cur_subsys->current_hits = 0;
			do_subobj_destroyed_stuff(shipp, cur_subsys, nullptr);
		}
	}
}

void LabUi::build_weapon_options(ship* shipp) const {
	with_TreeNode("Primaries")
	{
		auto bank = 0;
		for (auto& primary_slot : shipp->weapons.primary_bank_weapons) {
			if (primary_slot != -1) {
				auto wip = &Weapon_info[primary_slot];

				SCP_string text;
				sprintf(text, "##Primary bank %i", bank);

				build_primary_weapon_combobox(text, wip, primary_slot);
				SameLine();
				SCP_string cb_text;
				sprintf(cb_text, "Fire bank %i", bank);
				Checkbox(cb_text.c_str(), &getLabManager()->FirePrimaries[bank]);

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
				build_secondary_weapon_combobox(text, wip, secondary_slot);
				SameLine();
				SCP_string cb_text;
				sprintf(cb_text, "Fire bank %i##secondary", bank);
				Checkbox(cb_text.c_str(), &getLabManager()->FireSecondaries[bank]);

				bank++;
			}
		}
	}

	with_TreeNode("Turrets")
	{
		auto subsys_idx = 0; // unique IDs

		for (auto* subsys = GET_FIRST(&shipp->subsys_list); subsys != END_OF_LIST(&shipp->subsys_list);
			 subsys = GET_NEXT(subsys)) {
			if (subsys->system_info->type != SUBSYSTEM_TURRET)
				continue;

			SCP_string label;
			sprintf(label, "Turret %i - %s", subsys_idx, subsys->system_info->subobj_name);

			if (TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

				Spacing();

				// Find the turret in FireTurrets
				auto it = std::find_if(getLabManager()->FireTurrets.begin(),
					getLabManager()->FireTurrets.end(),
					[subsys](const auto& tuple) { return std::get<0>(tuple) == subsys; });

				int mode = 0; // Default to UVEC aligned
				bool fire_now = false;

				bool multipart = false;
				if (subsys->system_info->turret_gun_sobj >= 0 && subsys->system_info->subobj_num != subsys->system_info->turret_gun_sobj) {
					multipart = true;
				}

				const char* aiming_options[] = {"Random", "UVEC Aligned", "Initial"};

				// In the lab we need to force fire on target because we're faking having an actual target
				subsys->system_info->flags.set(Model::Subsystem_Flags::Fire_on_target);

				// If already added, load current mode + fire state
				if (it != getLabManager()->FireTurrets.end()) {
					mode = static_cast<int>(std::get<1>(*it));
					fire_now = std::get<2>(*it);
				}

				// Fire checkbox
				SCP_string cb_label;
				sprintf(cb_label, "Fire Turret##turret%i", subsys_idx);
				Checkbox(cb_label.c_str(), &fire_now);

				// Mode dropdown
				ImGui::TextUnformatted("Turret Aiming:");
				ImGui::SameLine(150.0f); // Start combo box at 150px
				ImGui::SetNextItemWidth(200.0f);
				ImGui::Combo("##AimingMode", &mode, aiming_options, multipart ? 3 : 2);

				auto modeToAimType = [multipart](int this_mode) -> LabTurretAimType {
					switch (this_mode) {
					case 0:
						return LabTurretAimType::RANDOM;
					case 1:
						return LabTurretAimType::UVEC;
					case 2:
						if (multipart) {
							return LabTurretAimType::INITIAL;
						} else {
							return LabTurretAimType::UVEC;
						}
					default:
						return LabTurretAimType::RANDOM; // fallback
					}
				};

				// Update or add to FireTurrets
				if (it == getLabManager()->FireTurrets.end()) {
					getLabManager()->FireTurrets.emplace_back(subsys, modeToAimType(mode), fire_now);

					// Set timestamps to allow turret to fire immediately
					subsys->turret_next_fire_stamp = timestamp(0);
					auto& weps = subsys->weapons;
					for (auto& stamp : weps.next_primary_fire_stamp) {
						stamp = timestamp(0);
					}

					for (auto& stamp : weps.next_secondary_fire_stamp) {
						stamp = timestamp(0);
					}
				} else {
					std::get<1>(*it) = modeToAimType(mode);
					std::get<2>(*it) = fire_now;
				}

				auto& weps = subsys->weapons;

				// PRIMARY BANKS
				for (int bank = 0; bank < weps.num_primary_banks; ++bank) {
					SCP_string text;
					sprintf(text, "##TurretPrimaryBank%i_%i", subsys_idx, bank);

					auto* wip = &Weapon_info[weps.primary_bank_weapons[bank]];
					build_primary_weapon_combobox(text, wip, weps.primary_bank_weapons[bank]);
				}

				// SECONDARY BANKS
				for (int bank = 0; bank < weps.num_secondary_banks; ++bank) {
					SCP_string text;
					sprintf(text, "##TurretSecondaryBank%i_%i", subsys_idx, bank);

					auto* wip = &Weapon_info[weps.secondary_bank_weapons[bank]];
					build_secondary_weapon_combobox(text, wip, weps.secondary_bank_weapons[bank]);
				}

				TreePop(); // Close this turret node
			}

			subsys_idx++;
		}
	}
}

void LabUi::build_primary_weapon_combobox(SCP_string& text,
	weapon_info* wip,
	int& primary_slot) const
{
	with_Combo(text.c_str(), wip->name)
	{
		for (size_t i = 0; i < Weapon_info.size(); i++) {
			if (Weapon_info[i].subtype == WP_MISSILE)
				continue;
			bool is_selected = i == (size_t)primary_slot;
			if (Selectable(Weapon_info[i].name, is_selected))
				primary_slot = (int)i;
			if (is_selected)
				SetItemDefaultFocus();
		}
	}
}

void LabUi::build_secondary_weapon_combobox(SCP_string& text, weapon_info* wip, int& secondary_slot) const
{
	with_Combo(text.c_str(), wip->name)
	{
		for (size_t i = 0; i < Weapon_info.size(); i++) {
			if (Weapon_info[i].subtype != WP_MISSILE)
				continue;
			bool is_selected = i == (size_t)secondary_slot;
			if (Selectable(Weapon_info[i].name, is_selected))
				secondary_slot = (int)i;
			if (is_selected)
				SetItemDefaultFocus();
		}
	}
}

void LabUi::reset_animations()
{
	// With full animation support for docking stages and fighter bays it's honestly just easier to reload the current object
	getLabManager()->changeDisplayedObject(getLabManager()->CurrentMode, getLabManager()->CurrentClass, getLabManager()->CurrentSubtype);
}

void LabUi::maybe_show_animation_category(const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers,
	animation::ModelAnimationTriggerType trigger_type, SCP_string label) const
{
	if (std::any_of(anim_triggers.begin(), anim_triggers.end(), [trigger_type](animation::ModelAnimationSet::RegisteredTrigger t) {
			return t.type == trigger_type;
		})) {
		with_TreeNode(label.c_str())
		{
			int count = 1;
			for (const auto& anim_trigger : anim_triggers) {
				if (anim_trigger.type == trigger_type) {

					SCP_string button_label = anim_trigger.name;
					switch (trigger_type) {
					case animation::ModelAnimationTriggerType::DockBayDoor:
						button_label += "Trigger Bay Door Animation " + std::to_string(count++);
						break;
					case animation::ModelAnimationTriggerType::Docking_Stage1:
						button_label += "Trigger Docking Stage 1 Animation " + std::to_string(count++);
						break;
					case animation::ModelAnimationTriggerType::Docking_Stage2:
						button_label += "Trigger Docking Stage 2 Animation " + std::to_string(count++);
						break;
					case animation::ModelAnimationTriggerType::Docking_Stage3:
						button_label += "Trigger Docking Stage 3 Animation " + std::to_string(count++);
						break;
					case animation::ModelAnimationTriggerType::Docked:
						button_label += "Trigger Docked Animation " + std::to_string(count++);
						break;
					default:
						// We really shouldn't be here, but just in case
						Assertion(false, "Unexpected animation trigger type %d", static_cast<int>(trigger_type));
						button_label += "Trigger Animation " + std::to_string(count++);
						break;
					}

					if (Button(button_label.c_str())) {
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

void LabUi::build_dock_test_options(ship* shipp)
{
	with_TreeNode("Docking Tests")
	{
		auto dockee_dock_map = get_docking_point_map(Ship_info[shipp->ship_info_index].model_num);

		if (!dockee_dock_map.empty()) {

			if (ImGui::BeginCombo("Docker Ship Class", Ship_info[getLabManager()->DockerClass].name)) {
				for (size_t i = 0; i < Ship_info.size(); ++i) {
					bool is_selected = (static_cast<int>(i) == getLabManager()->DockerClass);
					if (ImGui::Selectable(Ship_info[i].name, is_selected)) {
						getLabManager()->DockerClass = static_cast<int>(i);
						// Load model if needed
						auto& dsip = Ship_info[getLabManager()->DockerClass];
						if (dsip.model_num < 0) {
							dsip.model_num = model_load(dsip.pof_file, &dsip);
						}
						auto new_dock_map = get_docking_point_map(dsip.model_num);

						// Auto-select first available dockpoint (or clear if none)
						if (!new_dock_map.empty()) {
							getLabManager()->DockerDockPoint = new_dock_map.begin()->second;
						} else {
							getLabManager()->DockerDockPoint.clear();
						}
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			auto& dsip = Ship_info[getLabManager()->DockerClass];
			if (dsip.model_num < 0) {
				dsip.model_num = model_load(dsip.pof_file, &dsip);
			}
			auto dock_map = get_docking_point_map(dsip.model_num);

			// Ensure DockerDockPoint is initialized once based on the current DockerClass
			if (getLabManager()->DockerDockPoint.empty()) {
				if (!dock_map.empty()) {
					getLabManager()->DockerDockPoint = dock_map.begin()->second;
				}
			}

			const char* docker_label = getLabManager()->DockerDockPoint.c_str();

			if (ImGui::BeginCombo("Docker Dockpoint", docker_label)) {
				if (!dock_map.empty()) {
					for (const auto& [index, name] : dock_map) {
						bool is_selected = (name == getLabManager()->DockerDockPoint);
						if (ImGui::Selectable(name.c_str(), is_selected)) {
							getLabManager()->DockerDockPoint = name;
						}
						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::EndCombo();
			}

			// Auto-select first dockpoint if none currently selected
			if (getLabManager()->DockeeDockPoint.empty()) {
				getLabManager()->DockeeDockPoint = dockee_dock_map.begin()->second;
			}

			const char* dockee_label = getLabManager()->DockeeDockPoint.c_str();

			if (ImGui::BeginCombo("Dockee Dockpoint", dockee_label)) {
				for (const auto& [index, name] : dockee_dock_map) {
					bool is_selected = (name == getLabManager()->DockeeDockPoint);
					if (ImGui::Selectable(name.c_str(), is_selected)) {
						getLabManager()->DockeeDockPoint = name;
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (Button("Begin Docking Test")) {
				getLabManager()->beginDockingTest();
			}

			if (Button("Begin Undocking Test")) {
				getLabManager()->beginUndockingTest();
			}
		}
	}
}

void LabUi::build_bay_test_options(ship_info* sip)
{
	with_TreeNode("Fighterbay Tests")
	{
		auto bay_paths_map = get_bay_paths_map(sip->model_num);

		if (!bay_paths_map.empty()) {

			if (ImGui::BeginCombo("Bay Arrival/Departure Ship Class", Ship_info[getLabManager()->BayClass].name)) {
				for (size_t i = 0; i < Ship_info.size(); ++i) {
					bool is_selected = (static_cast<int>(i) == getLabManager()->BayClass);
					if (ImGui::Selectable(Ship_info[i].name, is_selected)) {
						getLabManager()->BayClass = static_cast<int>(i);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			auto& bsip = Ship_info[getLabManager()->BayClass];

			// Load the model
			if (bsip.model_num < 0) {
				bsip.model_num = model_load(bsip.pof_file, &bsip);
			}

			// Pick the first entry in bay_paths_map if it's set to 0
			if (getLabManager()->BayPathMask == 0) {
				int first_idx = bay_paths_map.begin()->first;
				getLabManager()->BayPathMask = (1u << first_idx);
			}

			// Get the preview label by finding the one bit that's set
			const char* path_label = "??";
			for (const auto& [idx, name] : bay_paths_map) {
				if (getLabManager()->BayPathMask & (1u << idx)) {
					path_label = name.c_str();
					break;
				}
			}

			if (ImGui::BeginCombo("Bay Path", path_label)) {
				for (const auto& [idx, name] : bay_paths_map) {
					bool is_selected = (getLabManager()->BayPathMask & (1u << idx)) != 0;
					if (ImGui::Selectable(name.c_str(), is_selected)) {
						getLabManager()->BayPathMask = (1u << idx);
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			static const char* mode_names[] = {"Arrival", "Departure"};
			int mode_idx = static_cast<int>(getLabManager()->BayTestMode);

			Assertion(mode_idx == 0 || mode_idx == 1, "Bay test mode is not valid!"); // only two valid modes

			if (ImGui::BeginCombo("Bay Test Mode", mode_names[mode_idx])) {
				// Arrival
				if (ImGui::Selectable("Arrival", getLabManager()->BayTestMode == BayMode::Arrival)) {
					getLabManager()->BayTestMode = BayMode::Arrival;
				}

				// Departure
				if (ImGui::Selectable("Departure", getLabManager()->BayTestMode == BayMode::Departure)) {
					getLabManager()->BayTestMode = BayMode::Departure;
				}

				ImGui::SetItemDefaultFocus();
				ImGui::EndCombo();
			}

			if (Button("Begin Bay Path Test")) {
				getLabManager()->beginBayTest();
			}
		}
	}
}

void LabUi::build_animation_options(ship* shipp, ship_info* sip) const
{
	with_TreeNode("Animations")
	{
		const auto& anim_triggers = sip->animations.getRegisteredTriggers();

		if (Button("Reset animations")) {
			reset_animations();
		}

		if (shipp->weapons.num_primary_banks > 0) {
			create_primary_weapon_anim_node(shipp, sip);
		}

		if (shipp->weapons.num_secondary_banks > 0) {
			create_secondary_weapon_anim_node(shipp, sip);
		}

		if (std::any_of(anim_triggers.begin(),
				anim_triggers.end(),
				[](animation::ModelAnimationSet::RegisteredTrigger t) {
					return t.type == animation::ModelAnimationTriggerType::Afterburner;
				})) {
			create_afterburner_animation_node(anim_triggers);
		}

		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::TurretFiring,
			"Turret firing##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::TurretFired,
			"Turret fired##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::Scripted,
			"Scripted animations##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::DockBayDoor,
			"Dock bay door##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::Docking_Stage1,
			"Docking stage 1##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::Docking_Stage2,
			"Docking stage 2##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::Docking_Stage3,
			"Docking stage 3##anims");
		maybe_show_animation_category(anim_triggers,
			animation::ModelAnimationTriggerType::Docked,
			"Docked animations##anims");
	}
}

void LabUi::create_afterburner_animation_node(
	const SCP_vector<animation::ModelAnimationSet::RegisteredTrigger>& anim_triggers) const
{
	with_TreeNode("Afterburner")
	{
		if (Button("Trigger afterburner animations")) {
			for (const auto& anim_trigger : anim_triggers) {
				if (anim_trigger.type == animation::ModelAnimationTriggerType::Afterburner) {
					auto& ab_triggers = manual_animation_triggers[animation::ModelAnimationTriggerType::Afterburner];
					do_triggered_anim(animation::ModelAnimationTriggerType::Afterburner,
						anim_trigger.name,
						ab_triggers[anim_trigger.name],
						anim_trigger.subtype);
					ab_triggers[anim_trigger.name] = !ab_triggers[anim_trigger.name];
				}
			}
		}
	}
}

void LabUi::create_secondary_weapon_anim_node(
	ship* shipp,
	ship_info* sip) const
{
	with_TreeNode("Secondary Weapons##Anims")
	{
		for (auto i = 0; i < shipp->weapons.num_secondary_banks; ++i) {
			SCP_string button_label;
			sprintf(button_label, "Trigger animation for secondary bank %i", i);
			if (Button(button_label.c_str())) {
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

void LabUi::create_primary_weapon_anim_node(
	ship* shipp,
	ship_info* sip) const
{
	with_TreeNode("Primary Weapons##Anims")
	{
		for (auto i = 0; i < shipp->weapons.num_primary_banks; ++i) {
			SCP_string button_label;
			sprintf(button_label, "Trigger animation for primary bank %i", i);
			if (Button(button_label.c_str())) {
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

void LabUi::show_object_options() const
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
				build_ship_table_info_txtbox(sip);

				build_model_info_box(sip, pm);

				build_subsystem_list(objp, shipp);
			}

			with_CollapsingHeader("Object actions")
			{
				if (getLabManager()->isSafeForShips()) {
					if (Button("Destroy ship")) {
						if (Objects[getLabManager()->CurrentObject].type == OBJ_SHIP) {
							// If we have testing objects, delete them
							getLabManager()->deleteTestObjects();

							auto obj = &Objects[getLabManager()->CurrentObject];

							obj->flags.remove(Object::Object_Flags::Player_ship);
							ship_self_destruct(obj);
						}
					}

					build_dock_test_options(shipp);

					build_bay_test_options(sip);

					build_animation_options(shipp, sip);
				}
			}

			with_CollapsingHeader("Weapons")
			{
				build_weapon_options(shipp);
			}
		} else if (getLabManager()->CurrentMode == LabMode::Weapon && getLabManager()->isSafeForWeapons()) {
			auto wip = &Weapon_info[getLabManager()->CurrentClass];

			with_CollapsingHeader("Weapon Info")
			{
				build_weapon_table_info_txtbox(wip);
			}
			
			with_CollapsingHeader("Object Actions")
			{
				Checkbox("Progress weapon lifetime", &getLabManager()->AllowWeaponDestruction);

				if (VALID_FNAME(wip->tech_model)) {
					if (Checkbox("Show Tech Model", &getLabManager()->ShowingTechModel)) {
						getLabManager()->changeDisplayedObject(LabMode::Weapon, getLabManager()->CurrentClass);
					}
				}
			}
		} else if (getLabManager()->CurrentMode == LabMode::Asteroid && getLabManager()->CurrentClass >= 0) {
			const auto& info = Asteroid_info[getLabManager()->CurrentClass];

			with_CollapsingHeader("Object Info")
			{
				static SCP_string table_text;
				static int old_class = -1;

				if (table_text.empty() || old_class != getLabManager()->CurrentClass) {
					table_text = get_asteroid_table_text(&info);
					old_class = getLabManager()->CurrentClass;
				}

				InputTextMultiline("##asteroid_table_text",
					const_cast<char*>(table_text.c_str()),
					table_text.length(),
					ImVec2(-FLT_MIN, GetTextLineHeight() * 16),
					ImGuiInputTextFlags_ReadOnly);
			}

			with_CollapsingHeader("Object actions")
			{
				if (getLabManager()->isSafeForAsteroids()) {
					if (Button("Destroy object")) {
						if (Objects[getLabManager()->CurrentObject].type == OBJ_ASTEROID) {
							auto obj = &Objects[getLabManager()->CurrentObject];

							if (obj->type == OBJ_ASTEROID) {
								vec3d dummy_pos = obj->pos;
								vec3d dummy_force = ZERO_VECTOR;

								// Apply full asteroid health to guarantee destruction
								asteroid_hit(obj, nullptr, &dummy_pos, obj->hull_strength + 1.0f, &dummy_force);
							}
						}
					}
				}
			}
		}
	}
}

