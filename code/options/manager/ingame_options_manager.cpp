#include "options/ingame_options_internal.h"
#include "options/manager/ingame_options_manager.h"
#include "io/key.h"
#include "parse/parselo.h"

#include "options/OptionsManager.h"
#include "options/Option.h"
#include "popup/popup.h"

#include "freespace.h"


OptConfigurator::OptConfigurator() {
	optUi = OptUi();
}

int OptConfigurator::get_binary_option_index(const SCP_string& title)
{
	for (size_t i = 0; i < getOptConfigurator()->binary_options.size(); i++) {
		if (getOptConfigurator()->binary_options[i].first == title) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

int OptConfigurator::get_float_range_option_index(const SCP_string& title)
{
	for (size_t i = 0; i < getOptConfigurator()->range_float_options.size(); i++) {
		if (getOptConfigurator()->range_float_options[i].first == title) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

int OptConfigurator::get_int_range_option_index(const SCP_string& title)
{
	for (size_t i = 0; i < getOptConfigurator()->range_int_options.size(); i++) {
		if (getOptConfigurator()->range_int_options[i].first == title) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

void OptConfigurator::maybe_persist_changes()
{
	auto unchanged = options::OptionsManager::instance()->persistChanges();

	if (unchanged.empty()) {
		notify_close();
		return;
	}

	SCP_string opts = "";
	for (auto& opt : unchanged) {
		opts += opt->getTitle();
		opts += "\n";
	}

	sprintf(persist_options, XSTR("The following options will require a restart. \n\n%s", -1), opts.c_str()); // XSTR ID needed before merge
	show_persist_popup = true;
}

void OptConfigurator::discard_changes()
{
	options::OptionsManager::instance()->discardChanges();
}

void OptConfigurator::offer_restart_popup()
{
	ImGui::OpenPopup("Restart Required");

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Restart Required", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text(persist_options.c_str());
		ImGui::Separator();
		ImGui::NewLine();
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			persist_options = "";
			show_persist_popup = false;
			notify_close();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}

void OptConfigurator::offer_save_options_popup()
{
	SCP_string dialog_text = XSTR("Option selections have not been saved. Do you wish to apply your changes?", -1); // XSTR ID needed before merge

	game_flush();

	ImGui::OpenPopup("Save Changes?");

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Save Changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text(dialog_text.c_str());
		ImGui::NewLine();
		if (ImGui::Button("Yes", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			show_save_options_popup = false;
			options_changed = false;
			maybe_persist_changes();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			show_save_options_popup = false;
			options_changed = false;
			discard_changes();
			notify_close();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			show_save_options_popup = false;
		}
		ImGui::EndPopup();
	}
}

void OptConfigurator::onFrame(float frametime) {
	if (gr_screen.mode == GR_OPENGL)
		ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	gr_reset_clip();
	gr_clear();

	optUi.create_ui();

	int key = game_check_key();

	if (key != 0) {
		// handle any key presses
		switch (key) {

		case KEY_ESC:
			if (options_changed) {
				show_save_options_popup = true;
			} else {
				notify_close();
			}
			break;

		default:
			break;
		}
	}

	if (show_save_options_popup) {
		offer_save_options_popup();
	}

	if (show_persist_popup) {
		offer_restart_popup();
	}
	
	if (Cmdline_show_imgui_debug)
		ImGui::ShowDemoWindow();
	ImGui::Render();
	if (gr_screen.mode == GR_OPENGL)
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (CloseThis) {
		close();
	}

	gr_flip();
}