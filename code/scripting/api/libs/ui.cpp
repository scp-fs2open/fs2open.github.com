//
//
#include "ui.h"
#include "cmdline/cmdline.h"
#include "menuui/barracks.h"
#include "menuui/mainhallmenu.h"
#include "menuui/playermenu.h"
#include "playerman/managepilot.h"
#include "scpui/SoundPlugin.h"
#include "scpui/rocket_ui.h"
#include "scripting/api/objs/player.h"
#include "scripting/lua/LuaTable.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/Lua/LuaType.h>

#pragma pop_macro("Assert")

namespace scripting {
namespace api {

//*************************Testing stuff*************************
// This section is for stuff that's considered experimental.
ADE_LIB(l_UserInterface, "UserInterface", "ui", "Functions for managing the \"scpui\" user interface system.");

ADE_FUNC(setOffset, l_UserInterface, "number x, number y",
         "Sets the offset from the top left corner at which <b>all</b> rocket contexts will be rendered", "boolean",
         "true if the operation was successful, false otherwise")
{

	float x;
	float y;

	if (!ade_get_args(L, "ff", &x, &y)) {
		return ADE_RETURN_FALSE;
	}

	scpui::setOffset(x, y);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(enableInput, l_UserInterface, "Rocket::Context context", "Enables input for the specified libRocket context",
         "boolean", "true if successfull")
{
	using namespace Rocket::Core;

	// Check parameter number
	if (!ade_get_args(L, "*")) {
		return ADE_RETURN_FALSE;
	}

	auto ctx = Lua::LuaType<Context>::check(L, 1);

	if (ctx == nullptr) {
		LuaError(L, "Parameter 1 is not a valid context handle!");
		return ADE_RETURN_FALSE;
	}

	scpui::enableInput(ctx);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(disableInput, l_UserInterface, "", "Disables UI input", "boolean", "true if successfull")
{
	scpui::disableInput();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(playElementSound, l_UserInterface,
         "Rocket::Element element, string event, string state = "
         "",
         "Plays an element specific sound with an optional state for differentiating different UI states.", "boolean",
         "true if a sound was played, false otherwise")
{
	using namespace Rocket::Core;
	const char* event;
	const char* state = "";
	if (!ade_get_args(L, "*s|s", &event, &state)) {
		return ADE_RETURN_FALSE;
	}

	auto el = Lua::LuaType<Element>::check(L, 1);

	if (el == nullptr) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", scpui::SoundPlugin::instance()->PlayElementSound(el, event, state));
}

//**********SUBLIBRARY: UserInterface/PilotSelect
ADE_LIB_DERIV(l_UserInterface_PilotSelect, "PilotSelect", nullptr,
              "API for accessing values specific to the pilot select screen.<br><b>Warning:</b> This is an internal "
              "API for the new UI system. This should not be used by other code and may be removed in the future!",
              l_UserInterface);

ADE_VIRTVAR(MAX_PILOTS, l_UserInterface_PilotSelect, nullptr, "Gets the maximum number of possible pilots.", "number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", MAX_PILOTS);
}
ADE_VIRTVAR(WarningCount, l_UserInterface_PilotSelect, nullptr, "The amount of warnings caused by the mod while loading.", "number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", Global_warning_count);
}
ADE_VIRTVAR(ErrorCount, l_UserInterface_PilotSelect, nullptr, "The amount of errors caused by the mod while loading.", "number",
            "The maximum number of pilots")
{
	return ade_set_args(L, "i", Global_error_count);
}

ADE_FUNC(enumeratePilots, l_UserInterface_PilotSelect, nullptr,
         "Lists all pilots available for the pilot selection<br>", "table{ name... }",
         "A table containing the pilots (without a file extension) or nil on error")
{
	using namespace luacpp;

	auto table = LuaTable::create(L);

	auto pilots = player_select_enumerate_pilots();
	for (size_t i = 0; i < pilots.size(); ++i) {
		table.addValue(i + 1, pilots[i]);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(getLastPilot, l_UserInterface_PilotSelect, nullptr,
         "Reads the last active pilot from the config file and returns some information about it. callsign is the name "
         "of the player and is_multi indicates whether the pilot was last active as a multiplayer pilot.",
         "string callsign", "The pilot name or nil if there was no last pilot")
{
	using namespace luacpp;

	auto callsign = player_get_last_player();
	if (callsign.empty()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "s", callsign.c_str());
}

ADE_FUNC(checkPilotLanguage, l_UserInterface_PilotSelect, "string callsign",
         "Checks if the pilot with the specified callsign has the right language.", "boolean",
         "true if pilot is valid, false otherwise")
{
	const char* callsign;
	if (!ade_get_args(L, "s", &callsign)) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", valid_pilot_lang(callsign));
}

ADE_FUNC(selectPilot, l_UserInterface_PilotSelect, "string callsign, boolean is_multi",
         "Selects the pilot with the specified callsign and advances the game to the main menu.", "nil", "nothing")
{
	const char* callsign;
	bool is_multi;
	if (!ade_get_args(L, "sb", &callsign, &is_multi)) {
		return ADE_RETURN_NIL;
	}

	player_finish_select(callsign, is_multi);

	return ADE_RETURN_NIL;
}

ADE_FUNC(deletePilot, l_UserInterface_PilotSelect, "string callsign",
         "Deletes the pilot with the specified callsign. This is not reversible!", "boolean",
         "true on success, false otherwise")
{
	const char* callsign;
	if (!ade_get_args(L, "s", &callsign)) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", delete_pilot_file(callsign));
}

ADE_FUNC(
    createPilot, l_UserInterface_PilotSelect, "string callsign, boolean is_multi[, string copy_from]",
    "Creates a new pilot in either single or multiplayer mode and optionally copies settings from an existing pilot.",
    "boolean", "true on success, false otherwise")
{
	const char* callsign;
	bool is_multi;
	const char* copy_from = nullptr;
	if (!ade_get_args(L, "sb|s", &callsign, &is_multi, &copy_from)) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", player_create_new_pilot(callsign, is_multi, copy_from));
}

ADE_FUNC(isAutoselect, l_UserInterface_PilotSelect, nullptr,
         "Determines if the pilot selection screen should automatically select the default user.", "boolean",
         "true if autoselect is enabled, false otherwise")
{
	return ade_set_args(L, "b", Cmdline_benchmark_mode);
}

//**********SUBLIBRARY: UserInterface/MainHall
ADE_LIB_DERIV(l_UserInterface_MainHall, "MainHall", nullptr,
              "API for accessing values specific to the main hall screen.<br><b>Warning:</b> This is an internal "
              "API for the new UI system. This should not be used by other code and may be removed in the future!",
              l_UserInterface);

ADE_FUNC(startAmbientSound, l_UserInterface_MainHall, nullptr, "Starts the ambient mainhall sound.", "nil", "nothing")
{
	(void)L;
	main_hall_start_ambient();
	return ADE_RETURN_NIL;
}

//**********SUBLIBRARY: UserInterface/Barracks
ADE_LIB_DERIV(l_UserInterface_Barracks, "Barracks", nullptr,
              "API for accessing values specific to the barracks screen.<br><b>Warning:</b> This is an internal "
              "API for the new UI system. This should not be used by other code and may be removed in the future!",
              l_UserInterface);

ADE_FUNC(listPilotImages, l_UserInterface_Barracks, nullptr, "Lists the names of the available pilot images.",
         "table {string...}", "The list of pilot filenames or nil on error")
{
	pilot_load_pic_list();

	using namespace luacpp;

	LuaTable out = LuaTable::create(L);
	for (auto i = 0; i < Num_pilot_images; ++i) {
		out.addValue(i + 1, Pilot_image_names[i]);
	}
	return ade_set_args(L, "t", &out);
}

ADE_FUNC(listSquadImages, l_UserInterface_Barracks, nullptr, "Lists the names of the available squad images.",
         "table {string...}", "The list of squad filenames or nil on error")
{
	pilot_load_squad_pic_list();

	using namespace luacpp;

	LuaTable out = LuaTable::create(L);
	for (auto i = 0; i < Num_pilot_squad_images; ++i) {
		out.addValue(i + 1, Pilot_squad_image_names[i]);
	}
	return ade_set_args(L, "t", &out);
}

ADE_FUNC(acceptPilot, l_UserInterface_Barracks, "player selection", "Accept the given player as the current player",
         "boolean", "true on sucess, false otherwise")
{
	player_h* plh;
	if (!ade_get_args(L, "o", l_Player.GetPtr(&plh))) {
		return ADE_RETURN_FALSE;
	}

	barracks_accept_pilot(plh->get());
	return ADE_RETURN_TRUE;
}

} // namespace api
} // namespace scripting
