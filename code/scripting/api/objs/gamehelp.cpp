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
//This won't work. Needs to be ADE_FUNC to getKeys() and getTexts() then index from there. ew.
ADE_LIB_DERIV(l_Help_Section_Keys, "keys", nullptr, nullptr, l_Help_Section);
ADE_INDEXER(l_Help_Section_Keys,
	"number idx",
	"Array of help section keys",
	"string",
	"the key as a string, or empty string if invalid")
{
	help_section_h current;
	int idx;
	if (!ade_get_args(L, "oi", l_Help_Section.Get(&current), &idx))
		return ade_set_error(L, "s", "");
	idx--; // Convert to Lua's 1 based index system
	return ade_set_args(L, "s", current.getSection()->key[idx]);
}

ADE_FUNC(__len, l_Help_Section_Keys, nullptr, "The number of keys in the section", "number", "The number of keys.")
{
	help_section_h current;
	if (!ade_get_args(L, "o", l_Help_Section.Get(&current)))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "i", (int)current.getSection()->key.size());
}

ADE_LIB_DERIV(l_Help_Section_Texts, "texts", nullptr, nullptr, l_Help_Section);
ADE_INDEXER(l_Help_Section_Texts,
	"number idx",
	"Array of help section texts",
	"string",
	"the text as a string, or empty string if invalid")
{
	help_section_h current;
	int idx;
	if (!ade_get_args(L, "oi", l_Help_Section.Get(&current), &idx))
		return ade_set_error(L, "s", "");
	idx--; // Convert to Lua's 1 based index system
	return ade_set_args(L, "s", current.getSection()->text[idx]);
}

ADE_FUNC(__len, l_Help_Section_Texts, nullptr, "The number of texts in the section", "number", "The number of texts.")
{
	help_section_h current;
	if (!ade_get_args(L, "o", l_Help_Section.Get(&current)))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "i", (int)current.getSection()->text.size());
}

} // namespace api
} // namespace scripting