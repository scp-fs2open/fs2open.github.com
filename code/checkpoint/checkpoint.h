#pragma once

// Mid-mission checkpoints.
//
// A checkpoint stores the state of the running mission.  Loading it reloads the mission cleanly, removes the objects
// the mission created on start, and restores the saved state over it: registered globals (see checkpoint/registry.h),
// then every saved object in its original slot, after which each object type re-acquires its runtime resources.
//
// Requests are carried out at the end of the current simulation frame, never in the middle of one.  No scripting hooks
// run while a checkpoint reloads and restores the mission.
//
// A checkpoint restores what matters to gameplay; it isn't an exact recreation.  Known limitations:
// - Visual effects are dropped: fireballs, particles, trails, decals, small debris, lightning bolts, shield impacts.
// - Warps under way finish at once when loading, without their effect; a player warping out goes to the debriefing.
//   Local SSMs finish their warp stage.
// - Queued and playing messages are lost, including training messages.
// - Camera and view settings aren't restored: cameras, subtitles, cutscene bars, fades, perspective and slew locks,
//   the motion debris override.
// - Moveable submodel poses go back to their initial state; overlapping animations resume approximately.
// - Lua state isn't saved; scripts are expected to keep their state through checkpoint hooks.
// - Things written outside the mission aren't rolled back: tech room, campaign allowances, medals, promotions.  The
//   pilot's all-time statistics aren't part of a checkpoint.
// - Table fields that scripts change aren't saved; only the ones sexps change are.
// - Random numbers from sources other than util::Random continue differently.
// - A checkpoint saved during the supernova sequence makes the player depart when loaded, since the sequence's control
//   mode is taken for a warp out.
// - Multiplayer isn't supported.

#include "globalincs/pstypes.h"

namespace checkpoint {

void request_save(int slot);
void request_load(int slot);

// Debugging aid.  With frames == 0: saves, loads the save and saves again, then reports every field that differs
// (errors in saving or restoring).  With frames > 0: additionally runs that many frames at a fixed frame time after
// saving and after loading, and reports what differs then (state that is not saved at all).
void request_verify(int frames);

bool has_slot(int slot);
void clear_slot(int slot);

// Called at the end of game_simulation_frame().
void process_requests();

// Called once a mission has been parsed, before anything ran; records its structure, so a checkpoint can tell whether
// a changed mission file still fits it.
void record_mission_structure();

// Called once the mission reload of a pending load has entered gameplay.
void apply_pending_restore();

// True while a checkpoint load reloads the mission and restores it.  No scripting hooks run then.
bool is_loading();

// When verifying, replaces the frame time and advances the timestamp clock by it, so frames are reproducible.
// Returns true if the frame time was replaced.
bool override_frametime(fix& frametime);

} // namespace checkpoint
