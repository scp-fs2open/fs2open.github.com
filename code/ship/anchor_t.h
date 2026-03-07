#ifndef SHIP_ANCHOR_T_H
#define SHIP_ANCHOR_T_H

#include "utils/id.h"

// The "strong typedef" pattern; see timer.h for more information
struct anchor_tag {};
class anchor_t : public util::ID<anchor_tag, int, -1>
{
public:
	static anchor_t invalid() { return {}; }
	anchor_t() = default;
	explicit anchor_t(int val) : ID(val) { }
};

// use high non-negative bits to mark anchor flags; flags should be high enough to avoid conflicting with ship registry indexes
constexpr int ANCHOR_SPECIAL_ARRIVAL = (1 << 30);
constexpr int ANCHOR_SPECIAL_ARRIVAL_PLAYER = (1 << 29);
constexpr int ANCHOR_IS_PARSE_NAMES_INDEX = (1 << 28);


#endif // SHIP_ANCHOR_T_H
