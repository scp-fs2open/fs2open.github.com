#include "rank.h"

#include "stats/scoring.h"

namespace scripting {
namespace api {

rank_h::rank_h() : rank(-1) {}
rank_h::rank_h(int l_rank) : rank(l_rank) {}

rank_stuff* rank_h::getRank() const
{
	return &Ranks[rank];
}

//**********HANDLE: rank
ADE_OBJ(l_Rank, rank_h, "medal", "Medal handle");

ADE_VIRTVAR(Name, l_Rank, nullptr, "The name of the rank", "string", "The name")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getRank()->name);
}

ADE_VIRTVAR(Bitmap, l_Rank, nullptr, "The bitmap of the rank", "string", "The bitmap")
{
	rank_h current;
	if (!ade_get_args(L, "o", l_Rank.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getRank()->bitmap);
}

} // namespace api
} // namespace scripting