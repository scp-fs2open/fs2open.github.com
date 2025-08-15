#include "options/Ingame_Options_internal.h"
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

// OptionsManager->persistChanges will try to update all changed options. If any
// are unable to change, they will be returned in the `unchanged` Vector. This usually
// means those options need the game to be restarted. So here we check if we have any
// of those and notify the player of them and that a restart will be necessary.
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

	sprintf(persist_options, XSTR("The following options will require a restart. \n\n%s", 1820), opts.c_str());
	show_persist_popup = true;
}

void OptConfigurator::discard_changes()
{
	options::OptionsManager::instance()->discardChanges();
}

void OptConfigurator::offer_restart_popup()
{
	ImGui::OpenPopup(XSTR("Restart Required", 1821));

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(XSTR("Restart Required", 1821), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("%s", persist_options.c_str());
		ImGui::Separator();
		ImGui::NewLine();
		if (ImGui::Button(XSTR("OK", 925), ImVec2(120, 0))) {
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
	SCP_string dialog_text = XSTR("Option selections have not been saved. Do you wish to apply your changes?", 1822);

	game_flush();

	ImGui::OpenPopup(XSTR("Save Changes?", 1823));

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(XSTR("Save Changes?", 1823), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("%s", dialog_text.c_str());
		ImGui::NewLine();
		if (ImGui::Button(XSTR("Yes", 1394), ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			show_save_options_popup = false;
			options_changed = false;
			maybe_persist_changes();
		}
		ImGui::SameLine();
		if (ImGui::Button(XSTR("No", 1395), ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			show_save_options_popup = false;
			options_changed = false;
			discard_changes();
			notify_close();
		}
		ImGui::SameLine();
		if (ImGui::Button(XSTR("Cancel", 1516), ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			show_save_options_popup = false;
		}
		ImGui::EndPopup();
	}
}

// The main Imgui rendering happens here as well as any i/o checking
void OptConfigurator::onFrame() {
	if (gr_screen.mode == GR_OPENGL)
		ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(gr_screen.max_w, gr_screen.max_h);
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