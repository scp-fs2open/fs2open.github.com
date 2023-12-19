#include "ingame_options_ui.h"

#include "options/OptionsManager.h"
#include "options/Option.h"

#include "options/ingame_options_internal.h"

using namespace ImGui;

void OptUi::object_changed()
{
	rebuild_after_object_change = true;
}

SCP_vector<std::pair<SCP_string, bool>> Binary_options;
SCP_vector<std::pair<SCP_string, float>> Range_float_options;
SCP_vector<std::pair<SCP_string, int>> Range_int_options;

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
					bool found = false;
					size_t idx = 0;
					for (size_t i = 0; i < Binary_options.size(); i++) {
						if (Binary_options[i].first == thisOpt->getTitle()) {
							found = true;
							idx = i;
							break;
						}
					}

					if (!found) {
						std::pair<SCP_string, bool> thisPair = std::make_pair(thisOpt->getTitle(), false);

						if (val.serialized == "true") {
							thisPair.second = true;
						}

						Binary_options.push_back(thisPair);
						idx = Binary_options.size() - 1;
					}

					if (ImGui::Checkbox(thisOpt->getTitle().c_str(), &Binary_options[idx].second)) {
						if (Binary_options[idx].second) {
							thisOpt->setValueDescription({val.display, "true"});
						} else {
							thisOpt->setValueDescription({val.display, "false"});
						}
					}
				} else {

					// Multi Select options
					with_Combo(thisOpt->getTitle().c_str(), val.display.c_str())
					{
						for (int n = 0; n < static_cast<int>(values.size()); n++) {
							bool is_selected = (values[n].serialized == val.serialized);
							if (Selectable(values[n].display.c_str(), is_selected))
								thisOpt->setValueDescription(values[n]);

							if (is_selected)
								SetItemDefaultFocus();
						}
					}
				}
			// Range Sliders
			} else if (thisOpt->getType() == options::OptionType::Range) {
				auto f_val = std::stof(val.serialized.c_str());
				auto min_max = thisOpt->getRangeValues();

				// Integer Ranges
				if (thisOpt->getFlags()[options::OptionFlags::RangeTypeInteger]){
					bool found = false;
					size_t idx = 0;
					for (size_t i = 0; i < Range_int_options.size(); i++) {
						if (Range_int_options[i].first == thisOpt->getTitle()) {
							found = true;
							idx = i;
							break;
						}
					}

					if (!found) {
						std::pair<SCP_string, int> thisPair = std::make_pair(thisOpt->getTitle().c_str(), static_cast<int>(f_val));

						Range_int_options.push_back(thisPair);
						idx = Range_int_options.size() - 1;
					}

					SliderInt(thisOpt->getTitle().c_str(), &Range_int_options[idx].second, static_cast<int>(min_max.first), static_cast<int>(min_max.second), val.display.c_str());
					if (IsItemDeactivatedAfterEdit()) {
						thisOpt->setValueDescription({std::to_string(Range_int_options[idx].second).c_str(), std::to_string(Range_int_options[idx].second).c_str()});
					}
				} else {
					bool found = false;
					size_t idx = 0;
					for (size_t i = 0; i < Range_float_options.size(); i++) {
						if (Range_float_options[i].first == thisOpt->getTitle()) {
							found = true;
							idx = i;
							break;
						}
					}

					if (!found) {
						std::pair<SCP_string, float> thisPair = std::make_pair(thisOpt->getTitle().c_str(), f_val);

						Range_float_options.push_back(thisPair);
						idx = Range_float_options.size() - 1;
					}

					SliderFloat(thisOpt->getTitle().c_str(), &Range_float_options[idx].second, min_max.first, min_max.second, val.display.c_str());
					if (IsItemDeactivatedAfterEdit()) {
						thisOpt->setValueDescription({std::to_string(Range_float_options[idx].second).c_str(), std::to_string(Range_float_options[idx].second).c_str()});
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
	}

	if (close_and_discard) {
		getOptConfigurator()->discard_changes();
		close_and_discard = false;
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

	//rebuild_after_object_change = false;
}

