#include "ingame_options_ui.h"

#include "options/OptionsManager.h"
#include "options/Option.h"

#include "options/Ingame_Options_internal.h"

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

			// Add a tooltip with the option description on mouseover
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
				ImGui::SetTooltip("%s", thisOpt->getDescription().c_str());
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
}

