#include "ingame_options_ui.h"

#include "options/OptionsManager.h"
#include "options/Option.h"

#include "options/Ingame_Options_internal.h"
#include "graphics/2d.h"

#include <cmath>

#include "osapi/outwnd.h"
#include "parse/parselo.h"

using namespace ImGui;

// This is the magic right here. It takes a category and searches for relevant options,
// building a control for each one as it goes. It's notable that binary and range options
// in ImGui require a variable reference. But since we're hooking in to the OptionsManager
// system here we can't get to the global references. So isntead we use a local one and when
// that one changes we notify OptionsManager that the option value has been changed and it
// handles updating the global values.
void OptUi::build_options_list(const char* category) const
{
	auto& optionsList = options::OptionsManager::instance()->getOptions();

	for (auto& option : optionsList) {
		const options::OptionBase* thisOpt = option.get();

		if (thisOpt->getFlags()[options::OptionFlags::RetailBuiltinOption]) {
			continue;
		}

		if (!stricmp(thisOpt->getCategory().c_str(), category)) {

			auto val = thisOpt->getCurrentValueDescription();
			
			// Selectors
			if (thisOpt->getType() == options::OptionType::Selection) {
				auto values = thisOpt->getValidValues();
				
				if ((values.size() == 2) && !(thisOpt->getFlags()[options::OptionFlags::ForceMultiValueSelection])) {

					// On/Off options
					int idx = getOptConfigurator()->get_binary_option_index(thisOpt->getTitle());

					Assertion(SCP_vector_inbounds(getOptConfigurator()->binary_options, idx),
						"Could not find binary option in ui list! Get a coder!");

					if (ImGui::Checkbox(thisOpt->getTitle().c_str(), &getOptConfigurator()->binary_options[idx].second)) {
						if (getOptConfigurator()->binary_options[idx].second) {
							thisOpt->setValueDescription({val.display, "true"}); // OptionsManager stores values as serialized strings
							getOptConfigurator()->options_changed = true;
						} else {
							thisOpt->setValueDescription({val.display, "false"}); // OptionsManager stores values as serialized strings
							getOptConfigurator()->options_changed = true;
						}
					}
				} else {

					// Multi Select options
					with_Combo(thisOpt->getTitle().c_str(), val.display.c_str())
					{
						for (int n = 0; n < static_cast<int>(values.size()); n++) {
							bool is_selected = (values[n].serialized == val.serialized);
							if (Selectable(values[n].display.c_str(), is_selected)) {
								thisOpt->setValueDescription(values[n]);
								getOptConfigurator()->options_changed = true;
							}
							if (is_selected) {
								SetItemDefaultFocus();
							}
						}
					}
				}
			// Range Sliders
			} else if (thisOpt->getType() == options::OptionType::Range) {
				std::pair<float, float> min_max = thisOpt->getRangeValues();

				// Integer Ranges
				if (thisOpt->getFlags()[options::OptionFlags::RangeTypeInteger]){

					int idx = getOptConfigurator()->get_int_range_option_index(thisOpt->getTitle());

					Assertion(SCP_vector_inbounds(getOptConfigurator()->range_int_options, idx),
						"Could not find int range option in ui list! Get a coder!");

					SliderInt(thisOpt->getTitle().c_str(),
						&getOptConfigurator()->range_int_options[idx].second,
						static_cast<int>(min_max.first),
						static_cast<int>(min_max.second),
						val.display.c_str());
					if (IsItemEdited()) {
						SCP_string newVal = std::to_string(getOptConfigurator()->range_int_options[idx].second); // OptionsManager stores values as serialized strings
						thisOpt->setValueDescription({newVal.c_str(), newVal.c_str()}); // OptionsManager handles updating the display values if necessary
						getOptConfigurator()->options_changed = true;
					}

				// Float Ranges
				} else {

					int idx = getOptConfigurator()->get_float_range_option_index(thisOpt->getTitle());

					Assertion(SCP_vector_inbounds(getOptConfigurator()->range_float_options, idx),
						"Could not find float range option in ui list! Get a coder!");

					SliderFloat(thisOpt->getTitle().c_str(),
						&getOptConfigurator()->range_float_options[idx].second,
						min_max.first,
						min_max.second,
						val.display.c_str());
					if (IsItemEdited()) {
						SCP_string newVal = std::to_string(getOptConfigurator()->range_float_options[idx].second); // OptionsManager stores values as serialized strings
						thisOpt->setValueDescription({newVal.c_str(), newVal.c_str()}); // OptionsManager handles updating the display values if necessary
						getOptConfigurator()->options_changed = true;
					}
				}
			}

			// Capture hover state now, since the last ImGui item is still the control itself -- adding the
			// override marker below would otherwise become the "last item" and break IsItemHovered() for the tooltip.
			bool isHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay);

			auto overrideReason = options::OptionsManager::instance()->getOverrideReason(thisOpt->getConfigKey());
			if (overrideReason.has_value()) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "(cmdline)");
			}

			// Add a tooltip with the option description on mouseover
			if (isHovered) {
				if (overrideReason.has_value()) {
					ImGui::SetTooltip("%s\n\nSet via command line: %s", thisOpt->getDescription().c_str(), overrideReason->c_str());
				} else {
					ImGui::SetTooltip("%s", thisOpt->getDescription().c_str());
				}
			}
		}
	}
}

// Build the main menu with selectors for each category and a way to save/discard/close
void OptUi::build_options_menu()
{
	with_Menu(XSTR("SCP Options", 1830))
	{
		for (size_t i = 0; i < Option_categories.size(); i++) {
			SCP_string title = Option_categories[i].first + " " + XSTR("Options", 1036);
			MenuItem(title.c_str(), nullptr, &Option_categories[i].second);
		}
		Separator();
		MenuItem(XSTR("HDR Calibration", -1), nullptr, &show_hdr_calibration);
		Separator();
		MenuItem(XSTR("Log Filters", -1), nullptr, &show_log_filters_dialog);
		MenuItem(XSTR("Save & Close", 1818), XSTR("ESC", 160), &close_and_save);
		MenuItem(XSTR("Discard & Close", 1819), nullptr, &close_and_discard);
	}
}

// Build the toolbar and handle notifications when closing the ui via this menu
void OptUi::build_toolbar_entries()
{
	with_MainMenuBar
	{
		build_options_menu();
	}

	if (close_and_save) {
		getOptConfigurator()->maybe_persist_changes();
		close_and_save = false;
		getOptConfigurator()->options_changed = false;
	}

	if (close_and_discard) {
		getOptConfigurator()->discard_changes();
		close_and_discard = false;
		getOptConfigurator()->options_changed = false;
		getOptConfigurator()->notify_close();
	}
}

// For each category create a rollout that contains the actual options
void OptUi::create_ui()
{
	build_toolbar_entries();

	for (const auto &cat : Option_categories) {
		if (cat.second) {
			SCP_string title = cat.first + " " + XSTR("Options", 1036);
			with_Window(title.c_str())
			{
				build_options_list(cat.first.c_str());
				ImGui::SetWindowSize(ImVec2(0, 0)); //Force window to fit content
				// It would be nice if there was a way to auto-position windows to avoid overlap here
				// but I don't see that functionality currently in ImGui, so we'll have to go without
			}
		}
	}

	if (show_log_filters_dialog) {
		show_log_filters_window();
	}

	if (show_hdr_calibration) {
		build_hdr_calibration_window();
	}
}

// Shows which nprintf()/mprintf() log categories are currently enabled and, if any log
// lines have been captured this session, lets the player browse/search them.
void OptUi::show_log_filters_window()
{
	with_Window(XSTR("Log Filters", -1), &show_log_filters_dialog)
	{
		TextUnformatted(XSTR("Categories are added to this list the first time they are logged this session.", -1));
		Separator();

		show_log_filter_list();

		Separator();
		TextUnformatted(XSTR("Log Output", -1));

		show_log_output();

		if (Button(XSTR("Clear Log", -1))) {
			outwnd_clear_log_history();
		}
	}
}

// Split out of show_log_filters_window() so the with_Child() scope guard below doesn't
// shadow the enclosing with_Window()'s guard (imgui_sugar's scope-guard variable name is fixed,
// so nesting them directly in one function trips MSVC's /WX-promoted C4456).
void OptUi::show_log_filter_list()
{
	with_Child("##LogFilterList", ImVec2(0, 150), true)
	{
		for (auto& filter : outwnd_get_filters()) {
			bool enabled = filter.enabled;
			if (Checkbox(filter.name, &enabled)) {
				outwnd_set_filter_enabled(filter.name, enabled);
			}
		}
	}
}

// See show_log_filter_list() for why this is a separate function.
void OptUi::show_log_output()
{
	static char log_search[64] = "";
	SetNextItemWidth(-1.0f);
	InputTextWithHint("##LogSearch", XSTR("Search log...", -1), log_search, sizeof(log_search));

	with_Child("##LogOutput", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar)
	{
		auto& history = outwnd_get_log_history();

		SCP_vector<size_t> visible;
		visible.reserve(history.size());
		for (size_t i = 0; i < history.size(); i++) {
			if (log_search[0] == '\0' || stristr(history[i].text.c_str(), log_search) != nullptr ||
				stristr(history[i].category.c_str(), log_search) != nullptr) {
				visible.push_back(i);
				}
		}

		ImGuiListClipper clipper;
		clipper.Begin(static_cast<int>(visible.size()));
		while (clipper.Step()) {
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
				auto& entry = history[visible[static_cast<size_t>(row)]];

				SCP_string line = entry.text;
				while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
					line.pop_back();
				}

				Text("[%s] %s", entry.category.c_str(), line.c_str());
			}
		}

		if (GetScrollY() >= GetScrollMaxY()) {
			SetScrollHereY(1.0f);
		}
	}
}

	// A dedicated calibration window for tuning HDR paper white / peak luminance and
// sanity-checking the HDR10 output path with simple test patterns. The patches are
// authored in the same sRGB-encoded space the encode pass expects, so a value of
// 1.0 (white) maps to paper white on the display.
void OptUi::build_hdr_calibration_window()
{
	with_Window(XSTR("HDR Calibration", -1), &show_hdr_calibration)
	{
		ImGui::SetWindowSize(ImVec2(560, 0), ImGuiCond_FirstUseEver);

		// Output status
		if (Gr_hdr_output_active) {
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", XSTR("HDR10 output is ACTIVE.", -1));
		} else if (Gr_enable_hdr) {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "%s",
				XSTR("HDR is enabled but not active. Use the Vulkan renderer with an HDR-capable display and restart.", -1));
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "%s",
				XSTR("HDR output is disabled. Enable 'HDR Output' under Graphics options and restart.", -1));
		}
		Separator();

		auto* mgr = options::OptionsManager::instance();
		const options::OptionBase* pwOpt = mgr->getOptionByKey("Graphics.HDRPaperWhite");
		const options::OptionBase* pkOpt = mgr->getOptionByKey("Graphics.HDRPeakLuminance");

		// Live paper white slider, bound to the persistent option
		if (pwOpt != nullptr) {
			std::pair<float, float> rng = pwOpt->getRangeValues();
			float v = Gr_hdr_paperwhite_nits;
			if (SliderFloat(XSTR("Paper White (nits)", -1), &v, rng.first, rng.second, "%.0f")) {
				SCP_string s = std::to_string(v);
				pwOpt->setValueDescription({s, s});
				getOptConfigurator()->options_changed = true;
			}
			Gr_hdr_paperwhite_nits = v;
		}

		// Live peak luminance slider, bound to the persistent option
		if (pkOpt != nullptr) {
			std::pair<float, float> rng = pkOpt->getRangeValues();
			float v = Gr_hdr_peak_nits;
			if (SliderFloat(XSTR("Peak Luminance (nits)", -1), &v, rng.first, rng.second, "%.0f")) {
				SCP_string s = std::to_string(v);
				pkOpt->setValueDescription({s, s});
				getOptConfigurator()->options_changed = true;
			}
			Gr_hdr_peak_nits = v;
		}

		Separator();
		ImGui::TextWrapped("%s",
			XSTR("Adjust Paper White until the 100%% patch looks like a comfortable SDR white. "
			     "The grayscale ramp below is paper-white-relative.", -1));

		auto* dl = ImGui::GetWindowDrawList();
		const float paperwhite = (Gr_hdr_paperwhite_nits > 1.0f) ? Gr_hdr_paperwhite_nits : 1.0f;
		const float pw = 40.0f, ph = 56.0f, gap = 4.0f;

		// Paper-white-relative grayscale ramp. Each patch is encoded sRGB so that the
		// HDR encode pass reproduces the requested nit level (clamped to paper white).
		const float nit_targets[] = {2.0f, 5.0f, 10.0f, 20.0f, 40.0f, 80.0f, 120.0f, 160.0f, 200.0f};
		const int n = static_cast<int>(sizeof(nit_targets) / sizeof(nit_targets[0]));
		ImVec2 p = ImGui::GetCursorScreenPos();
		for (int i = 0; i < n; ++i) {
			float lin = nit_targets[i] / paperwhite;
			if (lin > 1.0f) {
				lin = 1.0f;
			}
			float enc = powf(lin, 1.0f / 2.2f);
			int c = static_cast<int>(enc * 255.0f + 0.5f);
			c = (c < 0) ? 0 : ((c > 255) ? 255 : c);
			ImVec2 a(p.x + i * (pw + gap), p.y);
			ImVec2 b(a.x + pw, a.y + ph);
			dl->AddRectFilled(a, b, IM_COL32(c, c, c, 255));
		}
		ImGui::Dummy(ImVec2(n * (pw + gap), ph + 6));

		// Primary color patches (full intensity == paper white) to sanity-check the
		// BT.709 -> BT.2020 mapping when HDR is active.
		ImVec2 cp = ImGui::GetCursorScreenPos();
		const ImU32 prim[] = {IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 0, 255),
			IM_COL32(0, 0, 255, 255), IM_COL32(255, 255, 255, 255)};
		for (int i = 0; i < 4; ++i) {
			ImVec2 a(cp.x + i * (pw + gap), cp.y);
			ImVec2 b(a.x + pw, a.y + ph);
			dl->AddRectFilled(a, b, prim[i]);
		}
		ImGui::Dummy(ImVec2(4 * (pw + gap), ph + 6));

		ImGui::TextWrapped("%s",
			XSTR("Note: these patches cannot exceed paper white, so tune Peak Luminance in-game by "
			     "raising it until bright highlights and explosions stop clipping.", -1));
	}
}

