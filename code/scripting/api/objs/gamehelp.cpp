#include "gamehelp.h"

#include "gamehelp/gameplayhelp.h"

namespace scripting {
namespace api {

help_section_h::help_section_h() : section(-1) {}
help_section_h::help_section_h(int l_section) : section(l_section) {}

gameplay_help_section* help_section_h::getSection() const
{
	return &Help_text[section];
}

//**********HANDLE: help section
ADE_OBJ(l_Help_Section, help_section_h, "help_section", "Help Section handle");

ADE_VIRTVAR(Title, l_Help_Section, nullptr, "The title of the help section", "string", "The title")
{
	help_section_h current;
	if (!ade_get_args(L, "o", l_Help_Section.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getSection()->title);
}

ADE_VIRTVAR(Header, l_Help_Section, nullptr, "The header of the help section", "string", "The header")
{
	help_section_h current;
	if (!ade_get_args(L, "o", l_Help_Section.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getSection()->header);
}

ADE_VIRTVAR(Keys, l_Help_Section, nullptr, "Gets a table of keys (as a string) in the help section", "table", "The keys table") 
{
	help_section_h current;
	if (!ade_get_args(L, "o", l_Help_Section.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	for (size_t i = 0; i < current.getSection()->key.size(); i++) 
	{
		table.addValue(i + 1, current.getSection()->key[i]); //translate to Lua index
	}

	return ade_set_args(L, "t", &table);	
}

ADE_VIRTVAR(Texts, l_Help_Section, nullptr, "Gets a table of texts (as a string) in the help section", "table", "The texts table")
{
	help_section_h current;
	if (!ade_get_args(L, "o", l_Help_Section.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	auto table = luacpp::LuaTable::create(L);

	for (size_t i = 0; i < current.getSection()->text.size(); i++) {
		table.addValue(i + 1, current.getSection()->text[i]); //translate to Lua index
	}

	return ade_set_args(L, "t", &table);
}

} // namespace api
} // namespace scripting