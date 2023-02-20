//
//

#include "fireballclass.h"
#include "model.h"
#include "fireball/fireballs.h"

namespace scripting {
namespace api {


//**********HANDLE: Fireballclass
ADE_OBJ(l_Fireballclass, int, "fireballclass", "Fireball class handle");

ADE_FUNC(__tostring, l_Fireballclass, NULL, "Fireball class name", "string", "Fireball class unique id, or an empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= Num_fireball_types)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Fireball_info[idx].unique_id);
}

ADE_FUNC(__eq, l_Fireballclass, "fireballclass, fireballclass", "Checks if the two classes are equal", "boolean", "true if equal, false otherwise")
{
	int idx1,idx2;
	if(!ade_get_args(L, "oo", l_Fireballclass.Get(&idx1), l_Fireballclass.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if(idx1 < 0 || idx1 >= Num_fireball_types)
		return ade_set_error(L, "b", false);

	if(idx2 < 0 || idx2 >= Num_fireball_types)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_VIRTVAR(UniqueID, l_Fireballclass, "string", "Fireball class name", "string", "Fireball class unique id, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= Num_fireball_types)
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "s", Fireball_info[idx].unique_id);
}

ADE_VIRTVAR(Filename, l_Fireballclass, NULL, "Fireball class animation filename (LOD 0)", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= Num_fireball_types)
		return ade_set_error(L, "s", "");

	//Currently not settable as the bitmaps are only loaded once at level start
	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "s", Fireball_info[idx].lod[0].filename);
}

ADE_VIRTVAR(NumberFrames, l_Fireballclass, NULL, "Amount of frames the animation has (LOD 0)", "number", "Amount of frames, or -1 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ade_set_error(L, "i", -1);

	if(idx < 0 || idx >= Num_fireball_types)
		return ade_set_error(L, "i", -1);

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "i", Fireball_info[idx].lod[0].num_frames);
}

ADE_VIRTVAR(FPS, l_Fireballclass, NULL, "The FPS with which this fireball's animation is played (LOD 0)", "number", "FPS, or -1 if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ade_set_error(L, "i", -1);

	if (idx < 0 || idx >= Num_fireball_types)
		return ade_set_error(L, "i", -1);

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "i", Fireball_info[idx].lod[0].fps);
}

ADE_FUNC(isValid, l_Fireballclass, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= Num_fireball_types)
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getTableIndex, l_Fireballclass, NULL, "Gets the index value of the fireball class", "number", "index value of the fireball class")
{
	int idx;
	if(!ade_get_args(L, "o", l_Fireballclass.Get(&idx)))
		return ade_set_args(L, "i", -1);

	if(idx < 0 || idx >= Num_fireball_types)
		return ade_set_args(L, "i", -1);

	return ade_set_args(L, "i", idx + 1);
}

}
}
