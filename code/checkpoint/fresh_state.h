#pragma once

// Mission-start state that a checkpoint load puts back instead of the saved state: the names excluded when the
// checkpoint was saved (ships, wings, variables, containers) and the events flagged as not checkpointed.
//
// capture_fresh_state() runs once the mission has been reloaded and before anything is restored, and copies what the
// mission looks like at its start.  apply_fresh_state() runs once the saved state has been restored, and puts the copy
// back: excluded variables and containers get their start values, flagged events start over, excluded ships and wings
// are re-created as at mission start or go back to waiting for their arrival.

#include "globalincs/pstypes.h"

namespace checkpoint {

void capture_fresh_state(const SCP_vector<SCP_string>& excluded);
void apply_fresh_state();

} // namespace checkpoint
