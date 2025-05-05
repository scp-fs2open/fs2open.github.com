//
//

#include "options.h"
#include "options/OptionsManager.h"
#include "network/multi.h"
#include "network/multiui.h"
#include "scripting/api/objs/option.h"
#include "scripting/lua/LuaTable.h"

namespace scripting {
namespace api {

//**********LIBRARY: Mission
ADE_LIB(l_Options, "Options", "opt", "Options library");

ADE_VIRTVAR(Options, l_Options, nullptr, "The available options.", "table", "A table of all the options.")
{
	using namespace luacpp;

	auto& options = options::OptionsManager::instance()->getOptions();

	auto table = LuaTable::create(L);
	int i      = 1;

	for (auto& opt : options) {
		table.addValue(i, l_Option.Set(option_h(opt.get())));
		++i;
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(persistChanges,
	l_Options,
	nullptr,
	"Persist any changes made to the options system. Options can be incapable of applying changes immediately in "
	"which case they are returned here.",
	"option[]",
	"The options that did not support changing their value")
{
	using namespace luacpp;

	auto unchanged = options::OptionsManager::instance()->persistChanges();

	auto table = LuaTable::create(L);
	int i      = 1;

	for (auto& opt : unchanged) {
		table.addValue(i, l_Option.Set(option_h(opt)));
		++i;
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(discardChanges, l_Options, nullptr, "Discard any changes made to the options system.", "boolean",
         "True on success, false otherwise")
{
	options::OptionsManager::instance()->discardChanges();
	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(MultiLogin, l_Options, "string", "The multiplayer PXO login name", "string", "The login name")
{
	const char* login = nullptr;
	ade_get_args(L, "*|s", &login);
	
	if (ADE_SETTING_VAR && (login != nullptr)) {
		strcpy_s(Multi_tracker_login, login);
		os_config_write_string("PXO", "Login", login);
	}

	return ade_set_args(L, "s", Multi_tracker_login);
}

ADE_VIRTVAR(MultiPassword, l_Options, "string", "The multiplayer PXO login password", "boolean", "True if a password is set, false otherwise")
{
	const char* pswd = nullptr;
	ade_get_args(L, "*|s", &pswd);

	if (ADE_SETTING_VAR && (pswd != nullptr)) {
		strcpy_s(Multi_tracker_passwd, pswd);
		os_config_write_string("PXO", "Password", pswd);
	}

	return ade_set_args(L, "b", (strlen(Multi_tracker_passwd) != 0));
}

ADE_VIRTVAR(MultiSquad, l_Options, "string", "The multiplayer PXO squad name", "string", "The squad name")
{
	const char* squad = nullptr;
	ade_get_args(L, "*|s", &squad);

	if (ADE_SETTING_VAR && (squad != nullptr)) {
		strcpy_s(Multi_tracker_squad_name, squad);
		os_config_write_string("PXO", "SquadName", squad);
	}

	return ade_set_args(L, "s", Multi_tracker_squad_name);
}

ADE_FUNC(readIPAddressTable, l_Options, nullptr, "Gets the current multiplayer IP Address list as a table", "table", "The IP Address table")
{
	auto table = luacpp::LuaTable::create(L);

	SCP_list<SCP_string> list;
	multi_join_read_ip_address_file(list);

	int idx = 1;
	for (auto const& ip : list) {
		table.addValue(idx++, ip);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(writeIPAddressTable, l_Options, "table", "Saves the table to the multiplayer IP Address list", "boolean", "True if successful, false otherwise")
{
	
	auto ip_list = luacpp::LuaTable::create(L);

	if (!ade_get_args(L, "t", &ip_list)) {
		return ADE_RETURN_FALSE;
	}

	SCP_list<SCP_string> list;

	if (ip_list.isValid()) {
		for (const auto& item : ip_list) {
			if (item.second.is(luacpp::ValueType::STRING)) {
				// This'll lua-error internally if it's not fed only strings. Additionally, catch the lua exception and
				// then carry on
				try {
					SCP_string ip = item.second.getValue<SCP_string>();
					list.push_back(ip);
				} catch (const luacpp::LuaException& /*e*/) {
					// We were likely fed a userdata that was not an string.
					// Since we can't actually tell whether that's the case before we try to get the value, and the
					// attempt to get the value is printing a LuaError itself, just eat the exception here and return
					return ADE_RETURN_FALSE;
				}
			} else {
				// This happens on a non-userdata value, i.e. a number
				LuaError(L, "Table with strings to be written contained non-string values! Aborting...");
				return ADE_RETURN_FALSE;
			}
		}
	}

	return ade_set_args(L, "b", multi_join_write_ip_address_file(list));
}

ADE_FUNC(verifyIPAddress, l_Options, "string", "Verifies if a string is a valid IP address", "boolean", "True if valid, false otherwise")
{
	
	const char* ip;

	if (!ade_get_args(L, "s", &ip)) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", psnet_is_valid_ip_string(ip));
}

} // namespace api
} // namespace scripting
