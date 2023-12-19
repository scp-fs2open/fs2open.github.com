#include "options/ingame_options.h"
#include "options/ingame_options_internal.h"
#include "options/manager/ingame_options_manager.h"

#include "options/OptionsManager.h"
#include "options/Option.h"

#include "gamesequence/gamesequence.h"
#include "io/key.h"

#include "freespace.h"

static std::unique_ptr<OptConfigurator> OMGR;

SCP_vector<std::pair<SCP_string, bool>> Option_categories;

const std::unique_ptr<OptConfigurator>& getOptConfigurator()
{
	if (OMGR == nullptr) {
		OMGR.reset(new OptConfigurator());
	}

	return OMGR;
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

	}
}

void ingame_options_close()
{
	OMGR.reset();
	game_flush();
}

void ingame_options_do_frame(float frametime)
{
	getOptConfigurator()->onFrame(frametime);
}
