//
//

#include <parse/parselo.h>
#include <ship/ship.h>
#include <globalincs/linklist.h>
#include <iff_defs/iff_defs.h>
#include "FormWingDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {


FormWingDialogModel::FormWingDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
}
bool FormWingDialogModel::apply() {
	drop_white_space(_name);
	if (_name.empty()) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
													"Error",
													"You must give a name before you can continue.",
													{ DialogButton::Ok });
		return false;
	}

	for (auto i = 0; i < MAX_WINGS; i++) {
		if (!stricmp(Wings[i].name, _name.c_str()) && Wings[i].wave_count) {
			SCP_string msg;
			sprintf(msg, "The name \"%s\" is already being used by another wing", _name.c_str());

			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
														"Error",
														msg,
														{ DialogButton::Ok });
			return false;
		}
	}

	auto ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			auto i = ptr->instance;
			if (!strnicmp(_name.c_str(), Ships[i].ship_name, _name.size())) {
				char* namep;

				namep = Ships[i].ship_name + _name.size();
				if (*namep == ' ') {
					namep++;
					while (*namep) {
						if (!isdigit(*namep)) {
							break;
						}

						namep++;
					}
				}

				if (!*namep) {
					_viewport->dialogProvider->showButtonDialog(DialogType::Error,
																"Error",
																"This wing name is already being used by a ship",
																{ DialogButton::Ok });
					return false;
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

	for (auto i = 0; i < (int) Ai_tp_list.size(); i++) {
		if (!stricmp(_name.c_str(), Ai_tp_list[i].name)) {
			SCP_string msg;
			sprintf(msg, "The name \"%s\" is already being used by a target priority group", _name.c_str());

			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
														"Error",
														msg,
														{ DialogButton::Ok });
			return false;
		}
	}

	if (find_matching_waypoint_list(_name.c_str()) != NULL) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
													"Error",
													"This wing name is already being used by a waypoint path",
													{ DialogButton::Ok });
		return false;
	}

	if (_name[0] == '<') {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
													"Error",
													"Wing names not allowed to begin with <",
													{ DialogButton::Ok });
		return false;
	}

	return true;
}
void FormWingDialogModel::reject() {

}
const SCP_string& FormWingDialogModel::getName() const {
	return _name;
}
void FormWingDialogModel::setName(const SCP_string& name) {
	modify(_name, name);
}

}
}
}
