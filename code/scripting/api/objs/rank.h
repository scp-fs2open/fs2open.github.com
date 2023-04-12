#pragma once

#include "stats/scoring.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct rank_h {
	int rank;
	rank_h();
	explicit rank_h(int l_rank);
	rank_stuff* getRank() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Rank, rank_h);

} // namespace api
} // namespace scripting