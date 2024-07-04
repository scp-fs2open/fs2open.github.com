#include "options/Ingame_Options.h"
#include "options/Ingame_Options_internal.h"
#include "options/manager/ingame_options_manager.h"

#include "options/OptionsManager.h"
#include "options/Option.h"

static std::unique_ptr<OptConfigurator> OCGR;

// A list of all options categories: the category name and a boolean value for whether or not the window is active
SCP_vector<std::pair<SCP_string, bool>> Option_categories;

const std::unique_ptr<OptConfigurator>& getOptConfigurator()
{
	if (OCGR == nullptr) {
		OCGR.reset(new OptConfigurator());
	}

	return OCGR;
}

void ingame_options_init()
{
	gr_set_clear_color(0, 0, 0);
	auto& optionsList = options::OptionsManager::instance()->getOptions();

	// Get a list of all option categories
	for (auto& option : optionsList) {
		const options::OptionBase* thisOpt = option.get();

		if (thisOpt->getFlags()[options::OptionFlags::RetailBuiltinOption]) {
			continue;
		}

		// Get the category and save it for the window list
		SCP_string cat = thisOpt->getCategory();

		bool found = false;
		for (size_t i = 0; i < Option_categories.size(); i++) {
			if (Option_categories[i].first == cat) {
				found = true;
				break;
			}
		}

		if (!found) {
			Option_categories.push_back(std::make_pair(cat, true));
		}

		auto val = thisOpt->getCurrentValueDescription();

		// If it's a binary option then setup the necessary boolean for imgui
		if (thisOpt->getType() == options::OptionType::Selection) {
			auto values = thisOpt->getValidValues();

			if ((values.size() == 2) && !(thisOpt->getFlags()[options::OptionFlags::ForceMultiValueSelection])) {

				// On/Off options
				if (getOptConfigurator()->get_binary_option_index(thisOpt->getTitle()) < 0) {
					std::pair<SCP_string, bool> thisPair = std::make_pair(thisOpt->getTitle(), false);

					// Options manager stores the boolean as a serialized string, so we need to set our own bool
					if (val.serialized == "true") {
						thisPair.second = true;
					}

					getOptConfigurator()->binary_options.push_back(thisPair);
				}
			}
		// If it's a range option then setup the necessary float or int variables for imgui
		} else if (thisOpt->getType() == options::OptionType::Range) {

			// OptionsManager stores the value as a serialized string, so we need to return it to a float
			float f_val;
			try {
				f_val = std::stof(val.serialized.c_str());
			} catch (...) {
				Warning(LOCATION, "Error getting value for option '%s'. Setting to 0.0f!", thisOpt->getTitle().c_str());
				f_val = 0.0f;
			}

			// Integer Ranges
			if (thisOpt->getFlags()[options::OptionFlags::RangeTypeInteger]) {
				if (getOptConfigurator()->get_int_range_option_index(thisOpt->getTitle()) < 0) {
					std::pair<SCP_string, int> thisPair = std::make_pair(thisOpt->getTitle().c_str(), static_cast<int>(f_val));
					getOptConfigurator()->range_int_options.push_back(thisPair);
				}

			// Float Ranges
			} else {
				if (getOptConfigurator()->get_float_range_option_index(thisOpt->getTitle()) < 0) {
					std::pair<SCP_string, float> thisPair = std::make_pair(thisOpt->getTitle().c_str(), f_val);
					getOptConfigurator()->range_float_options.push_back(thisPair);
				}
			}
		}

	}
}

void ingame_options_close()
{
	OCGR.reset();
}

void ingame_options_do_frame()
{
	getOptConfigurator()->onFrame();
}
