#pragma once

#include "stats/medals.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct medal_h {
	int medal;
	medal_h();
	explicit medal_h(int l_medal);
	medal_stuff* getMedal() const;
	bool isValid() const;
	bool isRank() const;
};

DECLARE_ADE_OBJ(l_Medal, medal_h);

} // namespace api
} // namespace scripting