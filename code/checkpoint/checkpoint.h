#pragma once

// Mid-mission checkpoints.
//
// A checkpoint stores the state of the running mission.  Loading it reloads the mission cleanly, removes the objects
// the mission created on start, and restores the saved state over it: registered globals (see checkpoint/registry.h),
// then every saved object in its original slot, after which each object type re-acquires its runtime resources.
//
// Requests are carried out at the end of the current simulation frame, never in the middle of one.  No scripting hooks
// run while a checkpoint reloads and restores the mission; On Checkpoint Save and On Checkpoint Load run around it.
//
// Checkpoints are stored per pilot and mission (see checkpoint/store.h) and used through the savestate-* sexps, which
// follow SaveLoadX's lua-savestate-* ones, or through mn.Checkpoints in Lua.  Names excluded when saving, and events
// flagged as not checkpointed, get their mission-start state when a checkpoint is loaded (see checkpoint/fresh_state.h).
//
// A checkpoint restores what matters to gameplay; it isn't an exact recreation.  Known limitations:
// - Visual effects are dropped: fireballs, particles, trails, decals, small debris, lightning bolts, shield impacts.
// - Warps under way finish at once when loading, without their effect; a player warping out goes to the debriefing.
//   Local SSMs finish their warp stage.
// - Queued and playing messages are lost, including training messages.
// - Camera and view settings aren't restored: cameras, subtitles, cutscene bars, fades, perspective and slew locks,
//   the motion debris override.
// - Moveable submodel poses go back to their initial state; overlapping animations resume approximately.
// - Lua state isn't saved; scripts keep what they need with mn.Checkpoints.setData, e.g. in On Checkpoint Save.
// - Things written outside the mission aren't rolled back: tech room, campaign allowances, medals, promotions.  The
//   pilot's all-time statistics aren't part of a checkpoint.
// - Table fields that scripts change aren't saved; only the ones sexps change are.
// - Random numbers from sources other than util::Random continue differently.
// - A checkpoint saved during the supernova sequence makes the player depart when loaded, since the sequence's control
//   mode is taken for a warp out.
// - Excluded ships and wings come back as at mission start, but events about them and the mission log keep their saved
//   state unless those events are flagged as not checkpointed.  Wings and docked groups are reset as a whole, and the
//   player's ship and wing can't be excluded.
// - Multiplayer isn't supported.

#include "globalincs/pstypes.h"

namespace checkpoint {

// Saves the mission into checkpoint index at the end of the frame.  The checkpoint is stored in the pilot's file for
// this mission.  Excluded ships, wings, variables and containers get their mission-start state when it is loaded.
void request_save(int index, SCP_vector<SCP_string> excluded = {});

// Reloads the mission and restores checkpoint index, at the end of the frame.  Requests made in the first frame after
// a checkpoint was loaded are ignored, since they come from events that would otherwise load again and again.
void request_load(int index);

// Debugging aid.  With frames == 0: saves, loads the save and saves again, then reports every field that differs
// (errors in saving or restoring).  With frames > 0: additionally runs that many frames at a fixed frame time after
// saving and after loading, and reports what differs then (state that is not saved at all).
void request_verify(int frames);

// Whether checkpoint index exists, or any checkpoint if index is negative, for this mission or another one.
bool has_slot(int index, const char* mission = nullptr);

// Removes checkpoint index, or all checkpoints of this mission if index is negative.
void clear_slot(int index);

// The lowest index from 1 up without a checkpoint in this mission.
int next_free_slot();

// The checkpoint this mission was resumed from, or 0.
int loaded_index();

// Whether a ship or wing is in the mission or yet to arrive in a checkpoint, rather than destroyed or departed.  An
// index <= 0 asks about the checkpoint the mission was resumed from.  Without such a checkpoint, or if the name isn't
// in it, the answer is true.
bool ship_status(const char* name, int index);

// Gives these ships at least min_hull percent of their hull and min_ammo percent of their ammunition when a checkpoint
// is loaded later in this mission.  The latest percentages apply to all ships named so far.
void assist(float min_hull, float min_ammo, const SCP_vector<SCP_string>& ships);

// Data scripts keep in checkpoints: it is saved with every checkpoint and restored when one is loaded.
void set_script_data(const SCP_string& key, const SCP_string& value);
bool get_script_data(const SCP_string& key, SCP_string& value);

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
