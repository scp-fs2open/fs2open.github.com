//
//

#include "options.h"
#include "options/OptionsManager.h"
#include "scripting/api/objs/option.h"
#include "scripting/lua/LuaTable.h"

namespace scripting {
namespace api {

//**********LIBRARY: Mission
ADE_LIB(l_Options, "Options", "opt", "Options library");

ADE_VIRTVAR(Options, l_Options, nullptr, "The available options.", "option[]", "A table of all the options.")
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

} // namespace api
} // namespace scripting
