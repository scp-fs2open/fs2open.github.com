//
//

#include "enums.h"
#include "parse_object.h"
#include "ship_registry_entry.h"
#include "ship.h"

#include "ship/ship.h"

namespace scripting {
namespace api {

//**********HANDLE: ShipRegistryEntry
ADE_OBJ(l_ShipRegistryEntry, int, "ship_registry_entry", "Ship entry handle");

ADE_VIRTVAR(Name, l_ShipRegistryEntry, nullptr, "Name of ship", "string", "Ship name, or empty string if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_ShipRegistryEntry.Get(&idx)))
		return ade_set_args(L, "s", "");

	if (idx < 0 || (size_t)idx >= Ship_registry.size())
		return ade_set_args(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "s", Ship_registry[idx].name);
}

ADE_FUNC(isValid, l_ShipRegistryEntry, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if invalid, nil if a syntax/type error occurs")
{
	int idx;
	if (!ade_get_args(L, "o", l_ShipRegistryEntry.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || (size_t)idx >= Ship_registry.size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(Status, l_ShipRegistryEntry, nullptr, "Status of ship", "enumeration", "NOT_YET_PRESENT, PRESENT, EXITED, or nil if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_ShipRegistryEntry.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || (size_t)idx >= Ship_registry.size())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read only.");

	return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_NOT_YET_PRESENT + (int)Ship_registry[idx].status)));
}

ADE_FUNC(getParsedShip, l_ShipRegistryEntry, nullptr, "Return the parsed ship associated with this ship registry entry", "parse_object", "The parsed ship, or nil if handle is invalid.  If this ship entry is for a ship-create'd ship, the returned handle may be invalid.")
{
	int idx;
	if (!ade_get_args(L, "o", l_ShipRegistryEntry.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || (size_t)idx >= Ship_registry.size())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(Ship_registry[idx].p_objp)));
}

ADE_FUNC(getShip, l_ShipRegistryEntry, nullptr, "Return the ship associated with this ship registry entry", "ship", "The ship, or nil if handle is invalid.  The returned handle will be invalid if the ship has not yet arrived in-mission.")
{
	int idx;
	if (!ade_get_args(L, "o", l_ShipRegistryEntry.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || (size_t)idx >= Ship_registry.size())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Ship.Set(object_h(Ship_registry[idx].objp)));
}


}
}
