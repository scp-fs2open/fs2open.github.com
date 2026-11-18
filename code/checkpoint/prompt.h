#pragma once

// The built-in offer of the pilot's checkpoints of a mission (mission flag "Offer Checkpoints"): once the player has
// entered the mission, and from the death popup.  The offer is a popup listing "Start from beginning" and up to five
// checkpoints by name; it is skipped if there are none.

namespace checkpoint {

// Whether the death popup should offer loading a checkpoint.
bool can_offer_checkpoints();

// The player has just entered the mission: offers the checkpoints, unless the mission was loaded from one.
void player_entered();

// The player chose to load a checkpoint in the death popup, which has been closed.
void offer_after_death();

// Shows a pending offer.  Called once per frame during gameplay, where a popup may run; the offer after entering the
// mission waits one frame, so the popup has the mission behind it.
void process_offer();

} // namespace checkpoint
