#include "ingame_options_ui.h"

#include "options/OptionsManager.h"
#include "options/Option.h"

#include "options/ingame_options_internal.h"

using namespace ImGui;

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

					Assertion((idx >= 0), "Could not find binary option in ui list! Get a coder!");

					if (ImGui::Checkbox(thisOpt->getTitle().c_str(), &getOptConfigurator()->binary_options[idx].second)) {
						if (getOptConfigurator()->binary_options[idx].second) {
							thisOpt->setValueDescription({val.display, "true"});
							getOptConfigurator()->options_changed = true;
						} else {
							thisOpt->setValueDescription({val.display, "false"});
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

					Assertion((idx >= 0), "Could not find int range option in ui list! Get a coder!");

					SliderInt(thisOpt->getTitle().c_str(),
						&getOptConfigurator()->range_int_options[idx].second,
						static_cast<int>(min_max.first),
						static_cast<int>(min_max.second),
						val.display.c_str());
					if (IsItemEdited()) {
						SCP_string newVal = std::to_string(getOptConfigurator()->range_int_options[idx].second);
						thisOpt->setValueDescription({newVal.c_str(), newVal.c_str()});
						getOptConfigurator()->options_changed = true;
					}

				// Float Ranges
				} else {

					int idx = getOptConfigurator()->get_float_range_option_index(thisOpt->getTitle());

					Assertion((idx >= 0), "Could not find float range option in ui list! Get a coder!");

					SliderFloat(thisOpt->getTitle().c_str(),
						&getOptConfigurator()->range_float_options[idx].second,
						min_max.first,
						min_max.second,
						val.display.c_str());
					if (IsItemEdited()) {
						SCP_string newVal = std::to_string(getOptConfigurator()->range_float_options[idx].second);
						thisOpt->setValueDescription({newVal.c_str(), newVal.c_str()});
						getOptConfigurator()->options_changed = true;
					}
				}
			}
		}
	}
}

void OptUi::build_options_menu()
{
	with_Menu("SCP Options")
	{
		for (size_t i = 0; i < Option_categories.size(); i++) {
			SCP_string title = Option_categories[i].first + " options"; // needs XSTR support
			MenuItem(title.c_str(), nullptr, &Option_categories[i].second);
		}
		MenuItem("Save & Close", "ESC", &close_and_save);
		MenuItem("Discard & Close", nullptr, &close_and_discard);
	}
}

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

void OptUi::show_options_menus() const
{
	for (auto cat : Option_categories) {
		SCP_string title = cat.first + " options";
		with_Window(title.c_str()) 
		{
			build_options_list(cat.first.c_str());
		}
	}
}

void OptUi::create_ui()
{
	build_toolbar_entries();

	for (auto cat : Option_categories) {
		if (cat.second) {
			SCP_string title = cat.first + " options"; // needs XSTR support
			with_Window(title.c_str())
			{
				build_options_list(cat.first.c_str());
			}
		}
	}
}

