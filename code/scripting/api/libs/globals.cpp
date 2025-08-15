//
//

#include "globalincs/version.h"

#include "globals.h"

#include "missionui/missionscreencommon.h"
#include "playerman/managepilot.h"

namespace scripting::api {

//**********LIBRARY: Globals
ADE_LIB(l_GlobalVariables, "GlobalVariables", "gl", "Library of global values");

ADE_VIRTVAR(MAX_PILOTS,
	l_GlobalVariables,
	nullptr,
	"Gets the maximum number of possible pilots.",
	"number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", MAX_PILOTS);
}

ADE_VIRTVAR(MAX_SHIPS_PER_WING,
	l_GlobalVariables,
	nullptr,
	"The maximum ships that can be in a wing",
	"number",
	"A maximum number of ships")
{

	return ade_set_args(L, "i", MAX_SHIPS_PER_WING);
}

ADE_VIRTVAR(MAX_WING_SLOTS,
	l_GlobalVariables,
	nullptr,
	"The maximum ships that can be in a loadout wing",
	"number",
	"A maximum number of ships")
{

	return ade_set_args(L, "i", MAX_WING_SLOTS);
}

ADE_VIRTVAR(MAX_WING_BLOCKS,
	l_GlobalVariables,
	nullptr,
	"The maximum wings that can be in a loadout",
	"number",
	"A maximum number of wings")
{

	return ade_set_args(L, "i", MAX_WING_BLOCKS);
}

ADE_VIRTVAR(MAX_SHIP_PRIMARY_BANKS,
	l_GlobalVariables,
	nullptr,
	"The maximum primary banks a ship can have",
	"number",
	"A maximum number of primary banks")
{

	return ade_set_args(L, "i", MAX_SHIP_PRIMARY_BANKS);
}

ADE_VIRTVAR(MAX_SHIP_SECONDARY_BANKS,
	l_GlobalVariables,
	nullptr,
	"The maximum secondary banks a ship can have",
	"number",
	"A maximum number of secondary banks")
{

	return ade_set_args(L, "i", MAX_SHIP_SECONDARY_BANKS);
}

} // namespace scripting::api
