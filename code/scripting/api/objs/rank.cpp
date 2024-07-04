#include "rank.h"

#include "stats/scoring.h"

namespace scripting {
namespace api {

rank_h::rank_h() : rank(-1) {}
rank_h::rank_h(int l_rank) : rank(l_rank) {}

rank_stuff* rank_h::getRank() const
{
	if (!isValid())
		return nullptr;

	return &Ranks[rank];
}

bool rank_h::isValid() const
{
	return rank >= 0 && rank < (int)Ranks.size();
}

//**********HANDLE: rank
ADE_OBJ(l_Rank, rank_h, "rank", "Rank handle");

ADE_FUNC(isValid, l_Rank, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_Rank, nullptr, "The name of the rank", "string", "The name")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getRank()->name);
}

ADE_VIRTVAR(AltName, l_Rank, nullptr, "The alt name of the rank", "string", "The alt name")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getRank()->alt_name.c_str());
}

ADE_VIRTVAR(Title, l_Rank, nullptr, "The title of the rank", "string", "The title")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getRank()->title.c_str());
}

ADE_VIRTVAR(Bitmap, l_Rank, nullptr, "The bitmap of the rank", "string", "The bitmap")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getRank()->bitmap);
}

ADE_VIRTVAR(Index, l_Rank, nullptr, "The index of the rank within the Ranks list", "number", "The rank index")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", 0);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.rank + 1); // Convert to Lua's 1 based index system
}

} // namespace api
} // namespace scripting
