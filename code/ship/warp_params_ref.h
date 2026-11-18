#pragma once

#include "globalincs/session_index.h"

class WarpParams;

// An index into Warp_params, which grows as tables and missions add warp parameters
struct warp_params_ref : session_index {
	using session_index::session_index;

	WarpParams checkpoint_key() const;
	static warp_params_ref from_checkpoint_key(const WarpParams& params);
};
