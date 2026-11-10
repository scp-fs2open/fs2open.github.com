#include "FredCommands.h"
#include "ObjectCapture.h"

#include <algorithm>

#include <object/object.h>
#include <object/waypoint.h>
#include <ship/ship.h>
#include <jumpnode/jumpnode.h>
#include <prop/prop.h>
#include <parse/sexp.h>
#include <globalincs/linklist.h>
#include <math/vecmat.h>
#include <mission/missionmessage.h>
#include <mission/missionbriefcommon.h>
#include <missionui/missioncmdbrief.h>

#include "mission/Editor.h"
#include "mission/EditorViewport.h"
#include "mission/dialogs/ReorderDialogModel.h"
#include "mission/object.h"
#include <missioneditor/common.h>
#include "mission/dialogs/state/DialogStateHelpers.h"

#include <QDataStream>
#include <QIODevice>

namespace fso::fred {

namespace {

// Scope guard: auto-confirm "referenced by ... delete anyway?" prompts while a
// delete replays inside undo()/redo(). undo() restores the references that the
// original deletion invalidated, so the prompt would fire again on every redo —
// and canceling it mid-replay would desync the stack from the mission state.
class AutoConfirmReferences {
	Editor* _editor;
	bool    _prev;

public:
	explicit AutoConfirmReferences(Editor* editor)
	    : _editor(editor), _prev(editor->auto_confirm_reference_deletion)
	{
		_editor->auto_confirm_reference_deletion = true;
	}
	~AutoConfirmReferences() { _editor->auto_confirm_reference_deletion = _prev; }
	AutoConfirmReferences(const AutoConfirmReferences&)            = delete;
	AutoConfirmReferences& operator=(const AutoConfirmReferences&) = delete;
};

} // namespace

// ===========================================================================
// MoveObjectsCommand
// ===========================================================================

MoveObjectsCommand::MoveObjectsCommand(SCP_vector<ObjectTransform> transforms,
                                       Editor*                     editor,
                                       EditorViewport*             viewport,
                                       QUndoCommand*               parent)
    : QUndoCommand(QObject::tr("Move Objects"), parent)
    , _transforms(std::move(transforms))
    , _editor(editor)
    , _viewport(viewport)
{}

void MoveObjectsCommand::undo()
{
	for (const auto& t : _transforms) {
		const int objNum = obj_get_by_signature(t.signature);
		if (objNum < 0) continue;
		Objects[objNum].pos    = t.posBefore;
		Objects[objNum].orient = t.orientBefore;
		object_moved(&Objects[objNum]);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

void MoveObjectsCommand::redo()
{
	for (const auto& t : _transforms) {
		const int objNum = obj_get_by_signature(t.signature);
		if (objNum < 0) continue;
		Objects[objNum].pos    = t.posAfter;
		Objects[objNum].orient = t.orientAfter;
		object_moved(&Objects[objNum]);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// CreateObjectCommand
// ===========================================================================

CreateObjectCommand::CreateObjectCommand(const vec3d&    pos,
                                         int             modelIndex,
                                         int             propIndex,
                                         int             waypointInstance,
                                         CreateKind      createKind,
                                         OtherKind       otherKind,
                                         int             initialObjNum,
                                         Editor*         editor,
                                         EditorViewport* viewport,
                                         QUndoCommand*   parent)
    : QUndoCommand(QObject::tr("Create Object"), parent)
    , _pos(pos)
    , _savedModelIndex(modelIndex)
    , _savedPropIndex(propIndex)
    , _waypointInstance(waypointInstance)
    , _createKind(createKind)
    , _otherKind(otherKind)
    , _createdObjNum(initialObjNum)
    , _originalSignature(-1)
    , _editor(editor)
    , _viewport(viewport)
{
	// Capture the list name now so redo() can find the right list by a stable identity
	// rather than relying on the raw cur_waypoint pointer (which can be invalidated by
	// any vector reallocation inside waypoint_add).
	if (_waypointInstance >= 0) {
		const waypoint_list* wl = find_waypoint_list_with_instance(_waypointInstance, nullptr);
		if (wl != nullptr) {
			_waypointListName = wl->get_name();
		}
	}
	// Capture the object's signature so redo() can restore it to the new slot.
	// FSO's object pool is FIFO, so a recreated object gets a different objNum; restoring
	// the original signature lets obj_get_by_signature() find it in all subsequent commands.
	if (query_valid_object(initialObjNum)) {
		_originalSignature = Objects[initialObjNum].signature;

		// Jump nodes are referenced by name; capture it so redo() can restore it
		// instead of letting create_object() auto-name the recreated node.
		if (Objects[initialObjNum].type == OBJ_JUMP_NODE) {
			for (const auto& jn : Jump_nodes) {
				if (jn.GetSCPObjectNumber() == initialObjNum) {
					_jumpNodeName = jn.GetName();
					break;
				}
			}
		}
	}
}

void CreateObjectCommand::undo()
{
	// Find the object by signature: a later delete + undo recreates it in a
	// different slot, and its old slot may now hold an unrelated object.
	const int objNum = (_originalSignature > 0) ? obj_get_by_signature(_originalSignature) : _createdObjNum;
	if (objNum >= 0 && query_valid_object(objNum)) {
		AutoConfirmReferences guard(_editor);
		_editor->delete_object(objNum);
	}
	_createdObjNum = -1;
}

void CreateObjectCommand::redo()
{
	if (_firstRedo) {
		_firstRedo = false;
		return; // object was already created by the caller
	}

	const int savedModel = _viewport->cur_model_index;
	const int savedProp  = _viewport->cur_prop_index;
	_viewport->cur_model_index = _savedModelIndex;
	_viewport->cur_prop_index  = _savedPropIndex;

	// Find which waypoint list to append to by name (stable across redo cycles).
	// Using cur_waypoint is unsafe: waypoint_add's vector insert can reallocate the
	// waypoints vector, and markObject only refreshes cur_waypoint when currentObject == -1.
	int waypointInstance = -1;
	if (!_waypointListName.empty()) {
		const waypoint_list* wl = find_matching_waypoint_list(_waypointListName.c_str());
		if (wl != nullptr && !wl->get_waypoints().empty()) {
			// Use the last waypoint currently in the list as the anchor so waypoint_add
			// appends to the end, preserving correct ordering.
			waypointInstance = Objects[wl->get_waypoints().back().get_objnum()].instance;
		}
	}
	const OtherKind savedOther = _viewport->cur_other_kind;
	_viewport->cur_other_kind  = _otherKind;
	_createdObjNum = _viewport->create_object(&_pos, waypointInstance, _createKind);
	_viewport->cur_other_kind  = savedOther;

	_viewport->cur_model_index = savedModel;
	_viewport->cur_prop_index  = savedProp;

	// Restore the original signature so obj_get_by_signature() in subsequent commands
	// (field edits, moves, etc.) can still find this object by the signature they captured,
	// even though the pool slot may differ from the original objNum.
	if (_createdObjNum >= 0 && _originalSignature > 0)
		Objects[_createdObjNum].signature = _originalSignature;

	// Restore the original jump node name (create_object auto-named the new one).
	if (_createdObjNum >= 0 && !_jumpNodeName.empty()) {
		for (auto& jn : Jump_nodes) {
			if (jn.GetSCPObjectNumber() == _createdObjNum) {
				jn.SetName(_jumpNodeName.c_str());
				break;
			}
		}
	}

	if (_createdObjNum >= 0) {
		_editor->markObject(_createdObjNum);
		_editor->missionChanged();
		_viewport->needsUpdate();
	}
}

// ===========================================================================
// Wing undo helpers (shared between multiple command classes)
// ===========================================================================

namespace {

// Capture a wing's arrival/departure cue for ownership transfer.
// Locked sentinel nodes are never dup'd — they use WING_CUE_LOCKED_* sentinels.
int captureWingCue(int cue)
{
	if (cue == Locked_sexp_true)  return WING_CUE_LOCKED_TRUE;
	if (cue == Locked_sexp_false) return WING_CUE_LOCKED_FALSE;
	if (cue < 0)                  return WING_CUE_NONE;
	return dup_sexp_chain(cue);
}

// Restore a cue from its saved dup. For non-sentinel dups, transfers ownership
// back to the wing (sets cue_dup to WING_CUE_NONE).
int restoreWingCue(int& cue_dup)
{
	if (cue_dup == WING_CUE_LOCKED_TRUE)  return Locked_sexp_true;
	if (cue_dup == WING_CUE_LOCKED_FALSE) return Locked_sexp_false;
	// TODO(sexp_tree_refactor): WING_CUE_NONE means cue was not captured; default to always-arrive
	if (cue_dup == WING_CUE_NONE)         return Locked_sexp_true;
	Assertion(cue_dup >= 0, "restoreWingCue: unknown negative cue_dup %d", cue_dup);
	const int node = cue_dup;
	cue_dup = WING_CUE_NONE;
	return node;
}

// Free a command-owned cue dup (no-op for sentinels).
void freeCueDup(int& cue_dup)
{
	if (cue_dup >= 0) {
		free_sexp2(cue_dup);
	}
	cue_dup = WING_CUE_NONE;
}

// Re-capture cues from a live wing slot before deletion frees them.
void recaptureCues(SavedWingData& wd, int slot)
{
	freeCueDup(wd.arrival_cue_dup);
	freeCueDup(wd.departure_cue_dup);
	wd.arrival_cue_dup   = captureWingCue(Wings[slot].arrival_cue);
	wd.departure_cue_dup = captureWingCue(Wings[slot].departure_cue);
}

// Capture all static wing fields from Wings[wingNum] into wd.
// captureCues=true dups arrival/departure cues for ownership transfer.
void captureWingToData(int wingNum, SavedWingData& wd, bool captureCues)
{
	const wing& w = Wings[wingNum];
	wd.wingNum            = wingNum;
	wd.waveCount          = w.wave_count;
	wd.special_ship       = w.special_ship;
	wd.entirelyDeleted    = false;
	strcpy_s(wd.name, w.name);
	wd.display_name        = w.display_name;
	wd.hotkey             = w.hotkey;
	wd.reinforcement_index = w.reinforcement_index;
	wd.num_waves          = w.num_waves;
	wd.threshold          = w.threshold;
	wd.wave_delay_min     = w.wave_delay_min;
	wd.wave_delay_max     = w.wave_delay_max;
	wd.arrival_location   = w.arrival_location;
	wd.arrival_distance   = w.arrival_distance;
	wd.arrival_anchor     = w.arrival_anchor;
	wd.arrival_path_mask  = w.arrival_path_mask;
	wd.arrival_delay      = w.arrival_delay;
	wd.departure_location = w.departure_location;
	wd.departure_anchor   = w.departure_anchor;
	wd.departure_path_mask = w.departure_path_mask;
	wd.departure_delay    = w.departure_delay;
	wd.flags              = w.flags;
	for (int _i = 0; _i < MAX_AI_GOALS; _i++) wd.ai_goals[_i] = w.ai_goals[_i];
	strcpy_s(wd.wing_squad_filename, w.wing_squad_filename);
	wd.wing_insignia_texture = w.wing_insignia_texture;
	wd.formation          = w.formation;
	wd.formation_scale    = w.formation_scale;

	wd.arrival_cue_dup   = captureCues ? captureWingCue(w.arrival_cue)   : WING_CUE_NONE;
	wd.departure_cue_dup = captureCues ? captureWingCue(w.departure_cue) : WING_CUE_NONE;

	wd.members.clear();
	for (int i = 0; i < w.wave_count; i++) {
		SavedWingMemberSlot slot;
		slot.shipIndex = w.ship_index[i];
		slot.signature = Objects[Ships[slot.shipIndex].objnum].signature;
		strcpy_s(slot.name, Ships[slot.shipIndex].ship_name);
		slot.display_name     = Ships[slot.shipIndex].display_name;
		slot.has_display_name = Ships[slot.shipIndex].flags[Ship::Ship_Flags::Has_display_name];
		slot.wasDeleted = false;
		wd.members.push_back(slot);
	}

	// Reinforcement entry (erased by delete_wing/disband_wing; re-added on undo)
	wd.hadReinforcement = false;
	for (const auto& r : Reinforcements) {
		if (!stricmp(r.name, w.name)) {
			wd.hadReinforcement   = true;
			wd.reinforcementEntry = r;
			break;
		}
	}
}

// rename_ship() resets a ship's display name; put back the one captured with the wing.
void restoreMemberDisplayName(int shipIdx, const SavedWingMemberSlot& m)
{
	Ships[shipIdx].display_name = m.display_name;
	Ships[shipIdx].flags.set(Ship::Ship_Flags::Has_display_name, m.has_display_name);
}

// Re-add the wing's Reinforcements[] entry that delete_wing()/disband_wing()
// erased, and point the restored wing at it.
void restoreWingReinforcement(const SavedWingData& wd, wing& w)
{
	if (!wd.hadReinforcement)
		return;
	int idx = -1;
	for (int i = 0; i < (int)Reinforcements.size(); i++) {
		if (!stricmp(Reinforcements[i].name, wd.name)) {
			idx = i;
			break;
		}
	}
	if (idx < 0) {
		Reinforcements.push_back(wd.reinforcementEntry);
		idx = (int)Reinforcements.size() - 1;
	}
	w.reinforcement_index = idx;
}

// Resolve a wing member to its current Ships[] instance. The instance captured
// at command creation goes stale if the ship was deleted and restored by an
// intervening undo cycle, so prefer the (restored) object signature.
int resolveMemberInstance(const SavedWingMemberSlot& m)
{
	const int n = obj_get_by_signature(m.signature);
	if (n >= 0) return Objects[n].instance;
	return m.shipIndex;
}

// Clear a wing slot and copy the saved wing fields into it: everything except
// its cues, members and wave count, which callers fill in.
void fillWingFromData(wing& w, const SavedWingData& wd)
{
	w.clear();
	strcpy_s(w.name, wd.name);
	w.display_name        = wd.display_name;
	w.hotkey              = wd.hotkey;
	w.reinforcement_index = wd.reinforcement_index;
	w.num_waves           = wd.num_waves;
	w.threshold           = wd.threshold;
	w.wave_delay_min      = wd.wave_delay_min;
	w.wave_delay_max      = wd.wave_delay_max;
	w.arrival_location    = wd.arrival_location;
	w.arrival_distance    = wd.arrival_distance;
	w.arrival_anchor      = wd.arrival_anchor;
	w.arrival_path_mask   = wd.arrival_path_mask;
	w.arrival_delay       = wd.arrival_delay;
	w.departure_location  = wd.departure_location;
	w.departure_anchor    = wd.departure_anchor;
	w.departure_path_mask = wd.departure_path_mask;
	w.departure_delay     = wd.departure_delay;
	w.flags               = wd.flags;
	for (int _i = 0; _i < MAX_AI_GOALS; _i++) w.ai_goals[_i] = wd.ai_goals[_i];
	strcpy_s(w.wing_squad_filename, wd.wing_squad_filename);
	w.wing_insignia_texture = wd.wing_insignia_texture;
	w.formation           = wd.formation;
	w.formation_scale     = wd.formation_scale;
	w.special_ship        = wd.special_ship;
}

// Restore a wing into the given slot from saved data. Members are resolved by
// object signature (falling back to wd.members[i].shipIndex); callers that
// recreate deleted member ships must restore them before calling this.
void restoreWingIntoSlot(SavedWingData& wd, int slot, Editor* editor)
{
	wing& w = Wings[slot];
	fillWingFromData(w, wd);
	w.arrival_cue         = restoreWingCue(wd.arrival_cue_dup);
	w.departure_cue       = restoreWingCue(wd.departure_cue_dup);
	w.wave_count          = (int)wd.members.size();

	for (int i = 0; i < (int)wd.members.size(); i++) {
		const int shipIdx = resolveMemberInstance(wd.members[i]);
		w.ship_index[i]             = shipIdx;
		editor->wing_objects[slot][i] = Ships[shipIdx].objnum;
		Ships[shipIdx].wingnum      = slot;
		Ships[shipIdx].arrival_cue  = Locked_sexp_false;
		Ships[shipIdx].departure_cue = Locked_sexp_false;
		editor->rename_ship(shipIdx, wd.members[i].name);
		restoreMemberDisplayName(shipIdx, wd.members[i]);
	}
	restoreWingReinforcement(wd, w);
	Num_wings++;
	syncAllShipRegistryEntries();
}

// Find a free wing slot, preferring preferredSlot if available.
int findFreeWingSlot(int preferredSlot = -1)
{
	if (preferredSlot >= 0 && preferredSlot < MAX_WINGS && !Wings[preferredSlot].wave_count)
		return preferredSlot;
	for (int i = 0; i < MAX_WINGS; i++) {
		if (!Wings[i].wave_count) return i;
	}
	return -1;
}

// Capture a ship into CapturedShip (includes wing membership fields).
void doCaptureShip(int obj, SCP_vector<CapturedShip>& out)
{
	out.push_back(captureShip(obj));
}

} // namespace

// ===========================================================================
// DeleteObjectsCommand
// ===========================================================================

DeleteObjectsCommand::DeleteObjectsCommand(Editor* editor, EditorViewport* viewport, QUndoCommand* parent)
    : QUndoCommand(QObject::tr("Delete Objects"), parent)
    , _editor(editor)
    , _viewport(viewport)
{
	for (const object* p = GET_FIRST(&obj_used_list);
	     p != END_OF_LIST(&obj_used_list);
	     p = GET_NEXT(p))
	{
		if (!p->flags[Object::Object_Flags::Marked]) continue;
		const int obj = OBJ_INDEX(p);

		if (p->type == OBJ_SHIP || p->type == OBJ_START) {
			doCaptureShip(obj, _ships);
		} else if (p->type == OBJ_JUMP_NODE) {
			_jumpNodes.push_back(captureJumpNode(obj));
		} else if (p->type == OBJ_PROP) {
			_props.push_back(captureProp(obj));
		}
	}

	// Capture waypoints grouped by their parent list, preserving within-list order.
	// Iterating Waypoint_lists (not obj_used_list) gives us list membership and ordering.
	for (int li = 0; li < (int)Waypoint_lists.size(); li++) {
		const auto& wl   = Waypoint_lists[li];
		const auto& wpts = wl.get_waypoints();

		// Collect the obj nums of marked waypoints in this list
		SCP_vector<int> deletedObjNums;
		bool            entireList = true;
		for (const auto& wp : wpts) {
			const int objNum = wp.get_objnum();
			if (objNum >= 0 && Objects[objNum].flags[Object::Object_Flags::Marked])
				deletedObjNums.push_back(objNum);
			else
				entireList = false;
		}

		if (!deletedObjNums.empty()) {
			_waypointGroups.push_back(captureWaypointList(li, deletedObjNums, entireList));
		}
	}

	// Capture wing context for any wing that has at least one marked member.
	// We need this both for the partial-delete naming fix and for full-wing restore.
	// Build a set of deleted ship instances for quick membership check.
	SCP_vector<int> deletedShipInsts;
	for (const auto& sd : _ships) {
		deletedShipInsts.push_back(sd.originalShipIndex);
	}

	// Identify affected wings.
	SCP_vector<int> processedWings;
	for (const auto& sd : _ships) {
		if (sd.wingNum < 0) continue;
		if (std::find(processedWings.begin(), processedWings.end(), sd.wingNum) != processedWings.end())
			continue;
		processedWings.push_back(sd.wingNum);

		SavedWingData wd;
		captureWingToData(sd.wingNum, wd, false); // cue dup handled below based on entirelyDeleted

		// Mark which members are deleted vs surviving.
		bool allDeleted = true;
		for (auto& mem : wd.members) {
			mem.wasDeleted = (std::find(deletedShipInsts.begin(), deletedShipInsts.end(), mem.shipIndex)
			                  != deletedShipInsts.end());
			if (!mem.wasDeleted) allDeleted = false;
		}
		wd.entirelyDeleted = allDeleted;

		// For entirely-deleted wings, dup the cues now (before delete_marked frees them).
		if (wd.entirelyDeleted) {
			wd.arrival_cue_dup   = captureWingCue(Wings[sd.wingNum].arrival_cue);
			wd.departure_cue_dup = captureWingCue(Wings[sd.wingNum].departure_cue);
		}

		_affectedWings.push_back(std::move(wd));
	}
}

DeleteObjectsCommand::~DeleteObjectsCommand()
{
	for (auto& wd : _affectedWings) {
		freeCueDup(wd.arrival_cue_dup);
		freeCueDup(wd.departure_cue_dup);
	}
}

bool DeleteObjectsCommand::isEmpty() const
{
	return _ships.empty() && _waypointGroups.empty() && _jumpNodes.empty() && _props.empty();
}

void DeleteObjectsCommand::dropSurvivors()
{
	auto survived = [](int sig) { return sig > 0 && obj_get_by_signature(sig) >= 0; };

	SCP_vector<int> survivingShipInsts;
	for (auto it = _ships.begin(); it != _ships.end();) {
		if (survived(it->signature)) {
			survivingShipInsts.push_back(it->originalShipIndex);
			it = _ships.erase(it);
		} else {
			++it;
		}
	}

	for (auto it = _affectedWings.begin(); it != _affectedWings.end();) {
		bool anyDeleted = false;
		bool allDeleted = true;
		for (auto& mem : it->members) {
			if (mem.wasDeleted && std::find(survivingShipInsts.begin(), survivingShipInsts.end(), mem.shipIndex)
			                          != survivingShipInsts.end())
				mem.wasDeleted = false;
			if (mem.wasDeleted)
				anyDeleted = true;
			else
				allDeleted = false;
		}
		if (it->entirelyDeleted && !allDeleted) {
			// The wing survived, so the cue copies taken for recreating it are not needed.
			freeCueDup(it->arrival_cue_dup);
			freeCueDup(it->departure_cue_dup);
			it->entirelyDeleted = false;
		}
		if (!anyDeleted) {
			freeCueDup(it->arrival_cue_dup);
			freeCueDup(it->departure_cue_dup);
			it = _affectedWings.erase(it);
		} else {
			++it;
		}
	}

	for (auto it = _waypointGroups.begin(); it != _waypointGroups.end();) {
		auto& pts = it->points;
		const auto before = pts.size();
		pts.erase(std::remove_if(pts.begin(), pts.end(),
		                         [&](const CapturedWaypointPoint& pt) { return survived(pt.signature); }),
		          pts.end());
		if (pts.size() != before)
			it->entirePath = false;
		if (pts.empty())
			it = _waypointGroups.erase(it);
		else
			++it;
	}

	_jumpNodes.erase(std::remove_if(_jumpNodes.begin(), _jumpNodes.end(),
	                                [&](const CapturedJumpNode& jn) { return survived(jn.signature); }),
	                 _jumpNodes.end());
	_props.erase(std::remove_if(_props.begin(), _props.end(),
	                            [&](const CapturedProp& p) { return survived(p.signature); }),
	             _props.end());
}

void DeleteObjectsCommand::redo()
{
	if (_firstRedo) {
		_firstRedo = false;
		return; // deletion already performed by caller
	}

	// For entirely-deleted wings that will be recreated by undo(), we must re-dup their
	// cues BEFORE delete_marked() frees them. Find the live wing slot by name.
	for (auto& wd : _affectedWings) {
		if (!wd.entirelyDeleted) continue;
		for (int i = 0; i < MAX_WINGS; i++) {
			if (Wings[i].wave_count > 0 && !stricmp(Wings[i].name, wd.name)) {
				recaptureCues(wd, i);
				break;
			}
		}
	}

	// Re-dup ship SEXP cues and remove texture replacements BEFORE delete_marked()
	// frees the nodes and the ships cease to exist.
	// Find each live ship by name (ship names are unique within a valid mission).
	for (auto& cs : _ships) {
		for (const object* p = GET_FIRST(&obj_used_list);
		     p != END_OF_LIST(&obj_used_list);
		     p = GET_NEXT(p))
		{
			if ((p->type == OBJ_SHIP || p->type == OBJ_START) &&
			    !stricmp(Ships[p->instance].ship_name, cs.ship_name))
			{
				recaptureShipForRedo(cs, OBJ_INDEX(p));
				break;
			}
		}
	}

	// Re-mark the objects by their current live obj nums and delete them.
	_editor->unmark_all();
	for (const int n : _currentObjNums) {
		if (query_valid_object(n)) {
			_editor->markObject(n);
		}
	}
	AutoConfirmReferences guard(_editor);
	_editor->delete_marked();
}

void DeleteObjectsCommand::undo()
{
	// KNOWN LIMITATION: delete_marked() invalidates sexp references to deleted
	// ships (bashing them to <name>) and renumbers subsequent waypoints in
	// referencing sexps. Undo restores the objects themselves but does not
	// reverse those rewrites — that would require capturing every referencing
	// tree. Fully-deleted wings are the exception (see the fixup below).
	_currentObjNums.clear();

	// Phase 1: restore all ships; collect (oldShipInst → newObjNum) mapping.
	SCP_map<int, int> oldInstToNewObj; // oldShipInst → new obj num
	for (auto& cs : _ships) {
		const int newObj = restoreShip(cs, _editor);
		if (newObj >= 0) {
			_currentObjNums.push_back(newObj);
			oldInstToNewObj[cs.originalShipIndex] = newObj;
		}
	}

	// Restore waypoints group by group.
	for (const auto& group : _waypointGroups) {
		if (group.entirePath) {
			// Recreate the whole path as a new auto-named list, then rename it back
			// so any SEXP references to the original list name remain valid.
			int lastInstance = -1;
			bool renamedList = false;
			for (const auto& pt : group.points) {
				vec3d pos = pt.pos;
				const int newObj = _editor->create_waypoint(&pos, lastInstance);
				if (newObj < 0) continue;
				if (pt.signature > 0)
					Objects[newObj].signature = pt.signature;
				_currentObjNums.push_back(newObj);
				lastInstance = Objects[newObj].instance;
				if (!renamedList) {
					waypoint_list* wl = find_waypoint_list_with_instance(lastInstance, nullptr);
					if (wl != nullptr)
						wl->set_name(group.listName.c_str());
					renamedList = true;
				}
			}
		} else {
			// Partial deletion: reinsert each waypoint back into the surviving path.
			// Points are in ascending original-index order, so reinserting in this
			// order puts each one back at its original index. The path is found by
			// name and the predecessor by index at this point, because waypoint
			// instances shift when an earlier path is deleted or recreated.
			for (const auto& pt : group.points) {
				vec3d pos = pt.pos;
				waypoint_list* wl = find_matching_waypoint_list(group.listName.c_str());
				if (wl == nullptr || wl->get_waypoints().empty()) continue;
				const auto& wpts = wl->get_waypoints();
				int newObj;
				if (pt.indexInPath <= 0) {
					// This was the first waypoint in the list: insert at front.
					const int anyInst = Objects[wpts.front().get_objnum()].instance;
					newObj = waypoint_add(&pos, anyInst, /*first_waypoint_in_list=*/true);
					_editor->missionChanged();
				} else {
					const size_t predIndex = std::min(static_cast<size_t>(pt.indexInPath), wpts.size()) - 1;
					newObj = _editor->create_waypoint(&pos, Objects[wpts[predIndex].get_objnum()].instance);
				}
				if (newObj >= 0) {
					if (pt.signature > 0)
						Objects[newObj].signature = pt.signature;
					_currentObjNums.push_back(newObj);
				}
			}
		}
	}
	for (const auto& jd : _jumpNodes) {
		const int newObj = restoreJumpNode(jd, _editor);
		if (newObj >= 0) _currentObjNums.push_back(newObj);
	}
	for (const auto& pd : _props) {
		const int newObj = restoreProp(pd, _editor);
		if (newObj >= 0) _currentObjNums.push_back(newObj);
	}

	// Restore waypoint-list-level properties (color, no_lines, layer) that
	// survive independent of the individual waypoint points.
	for (const auto& wg : _waypointGroups) {
		restoreWaypointListProperties(wg);
	}

	// Phase 2: move newly created objects into obj_used_list so markObject's
	// query_valid_object check and all list traversals see them.
	obj_merge_created_list();

	// Sync each restored object into the correct layer slot. The fred_layer string
	// was already set by the restore functions, but _objectLayers has no entry for
	// the new object indices yet (it's only rebuilt by reloadLayersFromMission()).
	for (int objNum : _currentObjNums) {
		if (query_valid_object(objNum))
			_viewport->registerObjectInLayer(objNum);
	}
	_editor->notifyLayerStructureChanged();

	// Phase 3: restore wing membership for affected wings.
	// NOTE: we do NOT mutate wd.members[j].shipIndex here because _ships[i].originalShipIndex
	// (used as the key in oldInstToNewObj) is immutable; the mapping is recomputed correctly
	// each time undo() is called, so mutating would corrupt subsequent cycles.
	for (auto& wd : _affectedWings) {
		// Build a per-member resolved ship index for THIS undo cycle.
		// For wasDeleted members: map old → new instance. For survivors: use existing index.
		SCP_vector<int> resolvedInst(wd.members.size(), -1);
		bool canRestore = true;
		for (int j = 0; j < (int)wd.members.size(); j++) {
			if (wd.members[j].wasDeleted) {
				auto it = oldInstToNewObj.find(wd.members[j].shipIndex);
				if (it == oldInstToNewObj.end()) { canRestore = false; break; }
				resolvedInst[j] = Objects[it->second].instance;
			} else {
				resolvedInst[j] = resolveMemberInstance(wd.members[j]);
			}
		}
		if (!canRestore) continue;

		if (wd.entirelyDeleted) {
			// The wing was fully destroyed. Find a free slot and restore it.
			const int slot = findFreeWingSlot(wd.wingNum);
			if (slot < 0) continue;

			// Un-invalidate the SEXP refs that delete_wing turned into <WingName>.
			char invalidatedName[NAME_LENGTH + 2];
			sprintf(invalidatedName, "<%s>", wd.name);
			update_sexp_references(invalidatedName, wd.name);
			_editor->ai_update_goal_references(sexp_ref_type::WING, invalidatedName, wd.name);

			// Restore the wing struct using resolved ship indices.
			wing& w = Wings[slot];
			w.clear();
			strcpy_s(w.name, wd.name);
			w.display_name        = wd.display_name;
			w.hotkey              = wd.hotkey;
			w.reinforcement_index = wd.reinforcement_index;
			w.num_waves           = wd.num_waves;
			w.threshold           = wd.threshold;
			w.wave_delay_min      = wd.wave_delay_min;
			w.wave_delay_max      = wd.wave_delay_max;
			w.arrival_location    = wd.arrival_location;
			w.arrival_distance    = wd.arrival_distance;
			w.arrival_anchor      = wd.arrival_anchor;
			w.arrival_path_mask   = wd.arrival_path_mask;
			w.arrival_delay       = wd.arrival_delay;
			w.departure_location  = wd.departure_location;
			w.departure_anchor    = wd.departure_anchor;
			w.departure_path_mask = wd.departure_path_mask;
			w.departure_delay     = wd.departure_delay;
			w.flags               = wd.flags;
			for (int _i = 0; _i < MAX_AI_GOALS; _i++) w.ai_goals[_i] = wd.ai_goals[_i];
			strcpy_s(w.wing_squad_filename, wd.wing_squad_filename);
			w.wing_insignia_texture = wd.wing_insignia_texture;
			w.formation           = wd.formation;
			w.formation_scale     = wd.formation_scale;
			w.arrival_cue         = restoreWingCue(wd.arrival_cue_dup);
			w.departure_cue       = restoreWingCue(wd.departure_cue_dup);
			w.wave_count          = (int)wd.members.size();
			w.special_ship        = wd.special_ship;
			for (int j = 0; j < (int)wd.members.size(); j++) {
				const int shipIdx = resolvedInst[j];
				w.ship_index[j]                  = shipIdx;
				_editor->wing_objects[slot][j]   = Ships[shipIdx].objnum;
				Ships[shipIdx].wingnum           = slot;
				Ships[shipIdx].arrival_cue       = Locked_sexp_false;
				Ships[shipIdx].departure_cue     = Locked_sexp_false;
				_editor->rename_ship(shipIdx, wd.members[j].name);
				restoreMemberDisplayName(shipIdx, wd.members[j]);
			}
			restoreWingReinforcement(wd, w);
			Num_wings++;
			update_custom_wing_indexes();
		} else {
			// Partial delete: surviving members are still in the wing but may have been
			// renamed by delete_ship_from_wing. Rebuild the wing member arrays and
			// rename all members back to their saved names.
			const int wingNum = wd.wingNum;
			if (wingNum < 0 || Wings[wingNum].wave_count == 0) continue;

			Wings[wingNum].wave_count   = (int)wd.members.size();
			Wings[wingNum].special_ship = wd.special_ship;
			for (int j = 0; j < (int)wd.members.size(); j++) {
				const int shipIdx = resolvedInst[j];
				Wings[wingNum].ship_index[j]         = shipIdx;
				_editor->wing_objects[wingNum][j]    = Ships[shipIdx].objnum;
				Ships[shipIdx].wingnum               = wingNum;
				if (wd.members[j].wasDeleted) {
					Ships[shipIdx].arrival_cue   = Locked_sexp_false;
					Ships[shipIdx].departure_cue = Locked_sexp_false;
				}
				_editor->rename_ship(shipIdx, wd.members[j].name);
				restoreMemberDisplayName(shipIdx, wd.members[j]);
			}
		}
	}

	// Restoring a ship can briefly give it a name a wing survivor still had, and
	// the renames above then move the wrong registry entries; repair the keys.
	syncAllShipRegistryEntries();

	// Phase 4: mark the restored objects.
	for (const int n : _currentObjNums) {
		if (query_valid_object(n)) {
			_editor->markObject(n);
		}
	}

	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// RenameObjectCommand
// ===========================================================================

RenameObjectCommand::RenameObjectCommand(int        objNum,
                                         SCP_string oldName,
                                         SCP_string newName,
                                         Editor*    editor,
                                         bool       skipFirstRedo,
                                         QUndoCommand* parent)
    : QUndoCommand(QObject::tr("Rename"), parent)
    , _signature(objNum >= 0 ? Objects[objNum].signature : -1)
    , _oldName(std::move(oldName))
    , _newName(std::move(newName))
    , _editor(editor)
    , _skipFirstRedo(skipFirstRedo)
{
	// Record the display name of the side the object is on now.
	if (objNum >= 0) {
		auto current = readDisplayName(objNum);
		if (skipFirstRedo)
			_newDisplay = std::move(current);
		else
			_oldDisplay = std::move(current);
	}
}

std::optional<RenameObjectCommand::DisplayNameState> RenameObjectCommand::readDisplayName(int objNum)
{
	const int type = Objects[objNum].type;
	if (type == OBJ_SHIP || type == OBJ_START) {
		const ship& s = Ships[Objects[objNum].instance];
		return DisplayNameState{ s.display_name, s.flags[Ship::Ship_Flags::Has_display_name] };
	}
	if (type == OBJ_JUMP_NODE) {
		for (const auto& jn : Jump_nodes) {
			if (jn.GetSCPObjectNumber() == objNum)
				return DisplayNameState{ jn.HasDisplayName() ? jn.GetDisplayName() : "", jn.HasDisplayName() };
		}
	}
	return std::nullopt;
}

void RenameObjectCommand::writeDisplayName(int objNum, const DisplayNameState& d)
{
	const int type = Objects[objNum].type;
	if (type == OBJ_SHIP || type == OBJ_START) {
		ship& s = Ships[Objects[objNum].instance];
		s.display_name = d.name;
		s.flags.set(Ship::Ship_Flags::Has_display_name, d.has);
	} else if (type == OBJ_JUMP_NODE) {
		for (auto& jn : Jump_nodes) {
			if (jn.GetSCPObjectNumber() == objNum) {
				// SetDisplayName() clears it when given the node's own name
				jn.SetDisplayName(d.has ? d.name.c_str() : jn.GetName());
				break;
			}
		}
	}
}

void RenameObjectCommand::applyName(const SCP_string& name, const SCP_string& current,
                                    std::optional<DisplayNameState>& target,
                                    std::optional<DisplayNameState>& leaving)
{
	if (_signature >= 0) {
		const int objNum = obj_get_by_signature(_signature);
		if (objNum < 0) return; // object doesn't exist in this undo state
		if (!leaving)
			leaving = readDisplayName(objNum);
		const int type = Objects[objNum].type;
		if (type == OBJ_SHIP || type == OBJ_START) {
			// Leave the display name alone when there is a recorded one to put back.
			_editor->rename_ship(Objects[objNum].instance, name.c_str(), !target.has_value());
		} else if (type == OBJ_JUMP_NODE) {
			_editor->rename_jump_node(objNum, name.c_str());
		} else if (type == OBJ_PROP) {
			_editor->rename_prop(objNum, name.c_str());
		}
		if (target)
			writeDisplayName(objNum, *target);
		else
			target = readDisplayName(objNum);
	} else {
		// Waypoint path: current holds the name it has right now; name is what to rename to.
		_editor->rename_waypoint_list(current.c_str(), name.c_str());
	}
}

void RenameObjectCommand::undo()
{
	// Object is currently named _newName; rename it back to _oldName.
	applyName(_oldName, _newName, _oldDisplay, _newDisplay);
}

void RenameObjectCommand::redo()
{
	if (_skipFirstRedo) { _skipFirstRedo = false; return; }
	// Object is currently named _oldName; rename it to _newName.
	applyName(_newName, _oldName, _newDisplay, _oldDisplay);
}

// ===========================================================================
// MoveLayerCommand
// ===========================================================================

MoveLayerCommand::MoveLayerCommand(SCP_vector<ObjectLayerChange> changes,
                                   EditorViewport*               viewport,
                                   Editor*                       editor,
                                   QUndoCommand*                 parent)
    : QUndoCommand(QObject::tr("Move to Layer"), parent)
    , _changes(std::move(changes))
    , _viewport(viewport)
    , _editor(editor)
{}

void MoveLayerCommand::undo()
{
	for (const auto& c : _changes) {
		const int objNum = obj_get_by_signature(c.signature);
		if (objNum >= 0)
			_viewport->moveObjectToLayer(objNum, c.layerBefore);
	}
	_editor->missionChanged();
}

void MoveLayerCommand::redo()
{
	for (const auto& c : _changes) {
		const int objNum = obj_get_by_signature(c.signature);
		if (objNum >= 0)
			_viewport->moveObjectToLayer(objNum, c.layerAfter);
	}
	_editor->missionChanged();
}

// ===========================================================================
// LayerStructureCommand
// ===========================================================================

LayerStructureCommand::LayerStructureCommand(EditorViewport* viewport,
                                             Editor*         editor,
                                             const QString&  text,
                                             QUndoCommand*   parent)
    : QUndoCommand(text, parent)
    , _viewport(viewport)
    , _editor(editor)
{
	_before = capture();
}

void LayerStructureCommand::captureAfter()
{
	_after = capture();
}

LayerStructureCommand::Snapshot LayerStructureCommand::capture() const
{
	Snapshot snap;
	snap.layerNames = _viewport->getLayerNames();
	snap.layerVisibility.reserve(snap.layerNames.size());
	for (const auto& name : snap.layerNames) {
		bool visible = true;
		_viewport->getLayerVisibility(name, &visible);
		snap.layerVisibility.push_back(visible);
	}

	for (auto* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		const int objNum = OBJ_INDEX(objp);
		snap.objectLayers.push_back({objp->signature, _viewport->getObjectLayerName(objNum)});
	}
	return snap;
}

void LayerStructureCommand::restore(const Snapshot& snap)
{
	// Rebuild the layer list from the mission-file representation, then put every
	// object back on its captured layer. reloadLayersFromMission() derives object
	// assignments from each object's own fred_layer string, which delete/rename
	// may have rewritten, so the per-object pass below is what actually restores them.
	The_mission.fred_layers = snap.layerNames;
	_viewport->reloadLayersFromMission();

	// Only touch objects that actually need moving. reloadLayersFromMission() has
	// already put every object whose fred_layer string still resolves back on the
	// right layer, so for an add or rename this loop is usually a no-op and for a
	// delete it only covers the objects that were on the deleted layer.
	// The guard matters: moveObjectToLayer() emits layerStructureChanged per call,
	// and the Scene Browser answers that by cancelling its debounce and rebuilding
	// its whole tree, so moving every object in the mission would rebuild it once
	// per object.
	for (const auto& entry : snap.objectLayers) {
		const int objNum = obj_get_by_signature(entry.signature);
		if (objNum < 0)
			continue;
		if (_viewport->getObjectLayerName(objNum) == entry.layerName)
			continue;
		_viewport->moveObjectToLayer(objNum, entry.layerName);
	}

	// reloadLayersFromMission() resets every layer to visible, so this only has to
	// re-hide. setLayerVisibility() has no unchanged-value guard and notifies on
	// every call, so skip the ones already correct.
	for (size_t i = 0; i < snap.layerNames.size() && i < snap.layerVisibility.size(); ++i) {
		if (snap.layerVisibility[i])
			continue;
		_viewport->setLayerVisibility(snap.layerNames[i], false);
	}

	_editor->missionChanged();
}

void LayerStructureCommand::undo()
{
	restore(_before);
}

void LayerStructureCommand::redo()
{
	if (_skipFirstRedo) { _skipFirstRedo = false; return; }
	restore(_after);
}

// ===========================================================================
// ReorderCommand
// ===========================================================================

ReorderCommand::ReorderCommand(int             type,
                               int             fromPos,
                               int             toPos,
                               EditorViewport* viewport,
                               QUndoCommand*   parent)
    : QUndoCommand(QObject::tr("Reorder Objects"), parent)
    , _type(type)
    , _fromPos(fromPos)
    , _toPos(toPos)
    , _viewport(viewport)
    , _skipFirstRedo(true)
{}

void ReorderCommand::undo()
{
	using Model = dialogs::ReorderDialogModel;
	Model::applyMove(_viewport, static_cast<Model::Type>(_type), _toPos, _fromPos);
	_viewport->editor->missionChanged();
}

void ReorderCommand::redo()
{
	if (_skipFirstRedo) { _skipFirstRedo = false; return; }
	using Model = dialogs::ReorderDialogModel;
	Model::applyMove(_viewport, static_cast<Model::Type>(_type), _fromPos, _toPos);
	_viewport->editor->missionChanged();
}

// ===========================================================================
// ChangeIFFCommand
// ===========================================================================

ChangeIFFCommand::ChangeIFFCommand(SCP_vector<ShipIFFChange> changes,
                                   Editor*                   editor,
                                   QUndoCommand*             parent)
    : QUndoCommand(QObject::tr("Change IFF"), parent)
    , _changes(std::move(changes))
    , _editor(editor)
{}

void ChangeIFFCommand::undo()
{
	for (const auto& c : _changes) {
		const int objNum = obj_get_by_signature(c.signature);
		if (objNum < 0) continue;
		Ships[Objects[objNum].instance].team = c.iffBefore;
	}
	_editor->missionChanged();
}

void ChangeIFFCommand::redo()
{
	for (const auto& c : _changes) {
		const int objNum = obj_get_by_signature(c.signature);
		if (objNum < 0) continue;
		Ships[Objects[objNum].instance].team = c.iffAfter;
	}
	_editor->missionChanged();
}

// ===========================================================================
// LevelObjectsCommand
// ===========================================================================

LevelObjectsCommand::LevelObjectsCommand(SCP_vector<ObjectOrientChange> changes,
                                         Editor*                        editor,
                                         EditorViewport*                viewport,
                                         QUndoCommand*                  parent)
    : QUndoCommand(QObject::tr("Level Objects"), parent)
    , _changes(std::move(changes))
    , _editor(editor)
    , _viewport(viewport)
{}

void LevelObjectsCommand::undo()
{
	for (const auto& c : _changes) {
		const int cur = obj_get_by_signature(c.signature);
		if (cur < 0) continue;
		Objects[cur].orient = c.orientBefore;
		object_moved(&Objects[cur]);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

void LevelObjectsCommand::redo()
{
	for (const auto& c : _changes) {
		const int cur = obj_get_by_signature(c.signature);
		if (cur < 0) continue;
		Objects[cur].orient = c.orientAfter;
		object_moved(&Objects[cur]);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// AlignObjectsCommand
// ===========================================================================

AlignObjectsCommand::AlignObjectsCommand(SCP_vector<ObjectOrientChange> changes,
                                         Editor*                        editor,
                                         EditorViewport*                viewport,
                                         QUndoCommand*                  parent)
    : QUndoCommand(QObject::tr("Align Objects"), parent)
    , _changes(std::move(changes))
    , _editor(editor)
    , _viewport(viewport)
{}

void AlignObjectsCommand::undo()
{
	for (const auto& c : _changes) {
		const int cur = obj_get_by_signature(c.signature);
		if (cur < 0) continue;
		Objects[cur].orient = c.orientBefore;
		object_moved(&Objects[cur]);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

void AlignObjectsCommand::redo()
{
	for (const auto& c : _changes) {
		const int cur = obj_get_by_signature(c.signature);
		if (cur < 0) continue;
		Objects[cur].orient = c.orientAfter;
		object_moved(&Objects[cur]);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// CloneMarkedObjectsCommand
// ===========================================================================

CloneMarkedObjectsCommand::CloneMarkedObjectsCommand(Editor*         editor,
                                                     EditorViewport* viewport,
                                                     QUndoCommand*   parent)
    : QUndoCommand(QObject::tr("Clone Objects"), parent)
    , _previousCurrentObj(editor->currentObject)
    , _editor(editor)
    , _viewport(viewport)
{
	for (const object* p = GET_FIRST(&obj_used_list);
	     p != END_OF_LIST(&obj_used_list);
	     p = GET_NEXT(p))
	{
		if (p->flags[Object::Object_Flags::Marked]) {
			_originalSignatures.push_back(p->signature);
		}
	}
}

void CloneMarkedObjectsCommand::redo()
{
	// Restore the original selection so duplicate_marked_objects knows what to clone.
	_editor->unmark_all();
	for (const int sig : _originalSignatures) {
		const int n = obj_get_by_signature(sig);
		if (n >= 0) {
			_editor->markObject(n);
		}
	}

	if (_editor->getNumMarked() > 0 && _viewport->duplicate_marked_objects() >= 0) {
		// After duplicate_marked_objects(), the newly created objects are marked.
		// The originals are unmarked. The first redo records the clones'
		// signatures; later redos give the new clones those same signatures
		// (in creation order), so commands pushed after this one still find them.
		const bool firstClone = _cloneSignatures.empty();
		size_t i = 0;
		for (object* p = GET_FIRST(&obj_used_list);
		     p != END_OF_LIST(&obj_used_list);
		     p = GET_NEXT(p))
		{
			if (!p->flags[Object::Object_Flags::Marked]) continue;
			if (firstClone)
				_cloneSignatures.push_back(p->signature);
			else if (i < _cloneSignatures.size())
				p->signature = _cloneSignatures[i];
			i++;
		}
	}

	// Nothing was cloned on the push: QUndoStack::push() drops an obsolete
	// command instead of adding an empty undo step.
	if (_cloneSignatures.empty()) {
		setObsolete(true);
		return;
	}

	_editor->missionChanged();
	_viewport->needsUpdate();
}

void CloneMarkedObjectsCommand::undo()
{
	AutoConfirmReferences guard(_editor);
	for (const int sig : _cloneSignatures) {
		const int n = obj_get_by_signature(sig);
		if (n >= 0) {
			_editor->delete_object(n);
		}
	}

	_editor->unmark_all();
	for (const int sig : _originalSignatures) {
		const int n = obj_get_by_signature(sig);
		if (n >= 0) {
			_editor->markObject(n);
		}
	}
	if (query_valid_object(_previousCurrentObj)) {
		_editor->selectObject(_previousCurrentObj);
	}

	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// CloneDragCommand
// ===========================================================================

CloneDragCommand::CloneDragCommand(SCP_vector<CloneDragEntry> entries,
                                   int             previousCurrentObj,
                                   bool            insert,
                                   Editor*         editor,
                                   EditorViewport* viewport,
                                   QUndoCommand*   parent)
    : QUndoCommand(insert ? QObject::tr("Insert Waypoints") : QObject::tr("Clone Objects"), parent)
    , _entries(std::move(entries))
    , _previousCurrentObj(previousCurrentObj)
    , _insert(insert)
    , _skipFirstRedo(true)
    , _editor(editor)
    , _viewport(viewport)
{}

void CloneDragCommand::undo()
{
	AutoConfirmReferences guard(_editor);
	for (auto& e : _entries) {
		const int n = obj_get_by_signature(e.cloneSignature);
		if (n >= 0) _editor->delete_object(n);
	}
	_editor->unmark_all();
	for (const auto& e : _entries) {
		const int n = obj_get_by_signature(e.sourceSignature);
		if (n >= 0) _editor->markObject(n);
	}
	if (query_valid_object(_previousCurrentObj)) {
		_editor->selectObject(_previousCurrentObj);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

void CloneDragCommand::redo()
{
	if (_skipFirstRedo) { _skipFirstRedo = false; return; }

	_editor->unmark_all();
	for (const auto& e : _entries) {
		const int n = obj_get_by_signature(e.sourceSignature);
		if (n >= 0) _editor->markObject(n);
	}

	_viewport->duplicate_marked_objects(_insert);

	// Pair clones with entries by creation order (matches source iteration order
	// inside duplicate_marked_objects), give each clone the signature recorded
	// when it was first made (so later commands still find it), then apply
	// final positions.
	int ei = 0;
	for (object* p = GET_FIRST(&obj_used_list);
	     p != END_OF_LIST(&obj_used_list) && ei < static_cast<int>(_entries.size());
	     p = GET_NEXT(p))
	{
		if (!p->flags[Object::Object_Flags::Marked]) continue;
		auto& e = _entries[ei++];
		p->signature = e.cloneSignature;
		const int n = OBJ_INDEX(p);
		Objects[n].pos    = e.finalPos;
		Objects[n].orient = e.finalOrient;
		object_moved(&Objects[n]);
	}

	if (_wingNum >= 0) {
		for (const auto& e : _entries) {
			if (Wings[_wingNum].wave_count >= MAX_SHIPS_PER_WING) break;
			const int objNum = obj_get_by_signature(e.cloneSignature);
			if (objNum < 0) continue;
			const int ship = Objects[objNum].instance;
			if (Ships[ship].wingnum != -1) continue;
			// As the clone gesture does: rename_ship keeps sexps and the ship registry
			// in step, then bash again for the display name.
			char newName[NAME_LENGTH];
			wing_bash_ship_name(newName, Wings[_wingNum].name, Wings[_wingNum].wave_count + 1);
			_editor->rename_ship(ship, newName);
			wing_bash_ship_name(&Ships[ship], &Wings[_wingNum], Wings[_wingNum].wave_count + 1, true);
			Wings[_wingNum].ship_index[Wings[_wingNum].wave_count] = ship;
			Ships[ship].wingnum = _wingNum;
			_editor->wing_objects[_wingNum][Wings[_wingNum].wave_count] = objNum;
			Wings[_wingNum].wave_count++;
		}
	}

	_editor->missionChanged();
	_viewport->needsUpdate();
}

void CloneDragCommand::recordWingAdd(int wingNum)
{
	_wingNum = wingNum;
	setText(QObject::tr("Clone Ships to Wing"));
}

// ===========================================================================
// FormWingCommand
// ===========================================================================

namespace {

QByteArray serializeCue(int cue)
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);
	state::writeSexp(ds, cue);
	return data;
}

// Replace a live cue with a fresh copy of a serialized one.
void replaceCue(int& cue, const QByteArray& data)
{
	state::freeSexpFormula(cue);
	QDataStream ds(data);
	cue = state::readSexp(ds);
}

int liveWingSlot(const char* name)
{
	for (int i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count > 0 && !stricmp(Wings[i].name, name))
			return i;
	}
	return -1;
}

bool nameInList(const SCP_vector<SCP_string>& list, const char* name)
{
	return std::any_of(list.begin(), list.end(), [&](const SCP_string& n) { return !stricmp(n.c_str(), name); });
}

} // namespace

FormWingCommand::State FormWingCommand::capture(const SCP_vector<int>& shipSignatures,
                                                const SCP_vector<SCP_string>& wingNames)
{
	State st;
	for (const auto& name : wingNames) {
		const int slot = liveWingSlot(name.c_str());
		if (slot < 0) continue;
		WingState ws;
		captureWingToData(slot, ws.data, /*captureCues=*/false);
		ws.arrivalCue   = serializeCue(Wings[slot].arrival_cue);
		ws.departureCue = serializeCue(Wings[slot].departure_cue);
		st.wings.push_back(std::move(ws));
	}
	for (const int sig : shipSignatures) {
		const int objNum = obj_get_by_signature(sig);
		if (objNum < 0) continue;
		const ship& s = Ships[Objects[objNum].instance];
		ShipState ss;
		ss.signature      = sig;
		strcpy_s(ss.name, s.ship_name);
		ss.displayName    = s.display_name;
		ss.hasDisplayName = s.flags[Ship::Ship_Flags::Has_display_name];
		ss.arrivalCue     = serializeCue(s.arrival_cue);
		ss.departureCue   = serializeCue(s.departure_cue);
		for (const auto& r : Reinforcements) {
			if (!stricmp(r.name, s.ship_name)) {
				ss.isReinforcement    = true;
				ss.reinforcementEntry = r;
				break;
			}
		}
		st.ships.push_back(std::move(ss));
	}
	return st;
}

FormWingCommand::FormWingCommand(State before, State after, SCP_string newWingName,
                                 Editor* editor, EditorViewport* viewport, QUndoCommand* parent)
    : QUndoCommand(QObject::tr("Form Wing"), parent)
    , _before(std::move(before))
    , _after(std::move(after))
    , _newWingName(std::move(newWingName))
    , _editor(editor)
    , _viewport(viewport)
{}

void FormWingCommand::apply(const State& target, const State& other)
{
	// Every wing either state knows about is taken apart and rebuilt from target.
	SCP_vector<SCP_string> names;
	for (const State* st : { &target, &other }) {
		for (const auto& ws : st->wings) {
			if (!nameInList(names, ws.data.name))
				names.emplace_back(ws.data.name);
		}
	}

	// 1. Detach the live wings: members become loose ships, the wing slot is freed.
	SCP_vector<SCP_string> wasLive;
	for (const auto& name : names) {
		const int slot = liveWingSlot(name.c_str());
		if (slot < 0) continue;
		wing& w = Wings[slot];
		for (int i = 0; i < w.wave_count; i++) {
			Ships[w.ship_index[i]].wingnum = -1;
			_editor->wing_objects[slot][i] = -1;
		}
		state::freeSexpFormula(w.arrival_cue);
		state::freeSexpFormula(w.departure_cue);
		w.arrival_cue   = Locked_sexp_true;
		w.departure_cue = Locked_sexp_false;
		w.wave_count    = 0;
		Num_wings--;
		wasLive.push_back(name);
	}

	// 2. Ship names, in two passes through unique temporary names so names that
	// trade places (a wing's renumbered survivors) never collide. rename_ship()
	// carries sexp references, goals, texture replacements and the registry along.
	SCP_vector<std::pair<int, const ShipState*>> live;
	for (const auto& ss : target.ships) {
		const int objNum = obj_get_by_signature(ss.signature);
		if (objNum >= 0)
			live.emplace_back(Objects[objNum].instance, &ss);
	}
	for (size_t i = 0; i < live.size(); i++) {
		char tmp[NAME_LENGTH];
		sprintf(tmp, "__undo_form_wing_%d", static_cast<int>(i));
		_editor->rename_ship(live[i].first, tmp, false);
	}
	for (const auto& [inst, ss] : live)
		_editor->rename_ship(inst, ss->name, false);

	// 3. The rest of each ship's state.
	for (const auto& [inst, ss] : live) {
		ship& s = Ships[inst];
		s.display_name = ss->displayName;
		s.flags.set(Ship::Ship_Flags::Has_display_name, ss->hasDisplayName);
		replaceCue(s.arrival_cue, ss->arrivalCue);
		replaceCue(s.departure_cue, ss->departureCue);

		Reinforcements.erase(std::remove_if(Reinforcements.begin(), Reinforcements.end(),
		                                    [&](const reinforcements& r) { return !stricmp(r.name, s.ship_name); }),
		                     Reinforcements.end());
		if (ss->isReinforcement)
			Reinforcements.push_back(ss->reinforcementEntry);
		s.flags.set(Ship::Ship_Flags::Reinforcement, ss->isReinforcement);
	}

	// 4. Rebuild the target's wings.
	SCP_vector<SCP_string> rebuilt;
	for (const auto& ws : target.wings) {
		const SavedWingData& wd = ws.data;
		const int slot = findFreeWingSlot(wd.wingNum);
		if (slot < 0) continue;
		wing& w = Wings[slot];
		fillWingFromData(w, wd);
		replaceCue(w.arrival_cue, ws.arrivalCue);
		replaceCue(w.departure_cue, ws.departureCue);
		int count = 0;
		for (const auto& m : wd.members) {
			const int objNum = obj_get_by_signature(m.signature);
			if (objNum < 0) continue;
			const int inst = Objects[objNum].instance;
			w.ship_index[count]                = inst;
			_editor->wing_objects[slot][count] = objNum;
			Ships[inst].wingnum                = slot;
			count++;
		}
		w.wave_count = count;
		if (w.special_ship >= count)
			w.special_ship = 0;
		restoreWingReinforcement(wd, w);
		Num_wings++;
		rebuilt.emplace_back(wd.name);
	}

	// 5. Wing references: a wing that stops existing is invalidated the way
	// delete_wing() does it (<Name>); one that comes back is un-invalidated.
	for (const auto& name : names) {
		const bool before = nameInList(wasLive, name.c_str());
		const bool after  = nameInList(rebuilt, name.c_str());
		const SCP_string invalidated = "<" + name + ">";
		if (before && !after) {
			update_sexp_references(name.c_str(), invalidated.c_str());
			_editor->ai_update_goal_references(sexp_ref_type::WING, name.c_str(), invalidated.c_str());
			Reinforcements.erase(std::remove_if(Reinforcements.begin(), Reinforcements.end(),
			                                    [&](const reinforcements& r) { return !stricmp(r.name, name.c_str()); }),
			                     Reinforcements.end());
		} else if (!before && after) {
			update_sexp_references(invalidated.c_str(), name.c_str());
			_editor->ai_update_goal_references(sexp_ref_type::WING, invalidated.c_str(), name.c_str());
		}
	}

	update_custom_wing_indexes();
	syncAllShipRegistryEntries();
}

void FormWingCommand::undo()
{
	apply(_before, _after);

	// Select the ships that formed the wing.
	_editor->unmark_all();
	for (const auto& ss : _after.ships) {
		const int objNum = obj_get_by_signature(ss.signature);
		if (objNum >= 0)
			_editor->markObject(objNum);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

void FormWingCommand::redo()
{
	if (_firstRedo) {
		_firstRedo = false;
		return; // wing was already formed by the caller
	}

	apply(_after, _before);

	const int slot = liveWingSlot(_newWingName.c_str());
	if (slot >= 0)
		_editor->mark_wing(slot);
	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// DisbandWingCommand
// ===========================================================================

DisbandWingCommand::DisbandWingCommand(int             wingNum,
                                       Editor*         editor,
                                       EditorViewport* viewport,
                                       QUndoCommand*   parent)
    : QUndoCommand(QObject::tr("Disband Wing"), parent)
    , _editor(editor)
    , _viewport(viewport)
{
	captureWingToData(wingNum, _savedWing, /*captureCues=*/true);
}

DisbandWingCommand::~DisbandWingCommand()
{
	freeCueDup(_savedWing.arrival_cue_dup);
	freeCueDup(_savedWing.departure_cue_dup);
}

void DisbandWingCommand::undo()
{
	const int slot = findFreeWingSlot(_savedWing.wingNum);
	if (slot < 0) return;

	restoreWingIntoSlot(_savedWing, slot, _editor);
	_editor->mark_wing(slot);
	update_custom_wing_indexes();
	_editor->missionChanged();
	_viewport->needsUpdate();
}

void DisbandWingCommand::redo()
{
	if (_firstRedo) {
		_firstRedo = false;
		return; // disband was already performed by the caller
	}

	// Find the current live wing slot (might differ from _savedWing.wingNum after undo).
	int slot = -1;
	for (int i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count > 0 && !stricmp(Wings[i].name, _savedWing.name)) {
			slot = i;
			break;
		}
	}
	if (slot < 0) return;

	// Re-capture cues before disband_wing frees them.
	recaptureCues(_savedWing, slot);

	{
		AutoConfirmReferences guard(_editor);
		_editor->disband_wing(slot);
	}
	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// DeleteWingCommand
// ===========================================================================

DeleteWingCommand::DeleteWingCommand(int             wingNum,
                                     Editor*         editor,
                                     EditorViewport* viewport,
                                     QUndoCommand*   parent)
    : QUndoCommand(QObject::tr("Delete Wing"), parent)
    , _currentWingNum(wingNum)
    , _editor(editor)
    , _viewport(viewport)
{
	captureWingToData(wingNum, _savedWing, /*captureCues=*/true);

	// Capture all member ships' full data (they will be deleted).
	for (const auto& mem : _savedWing.members) {
		const int objNum = Ships[mem.shipIndex].objnum;
		doCaptureShip(objNum, _savedShips);
	}
}

DeleteWingCommand::~DeleteWingCommand()
{
	freeCueDup(_savedWing.arrival_cue_dup);
	freeCueDup(_savedWing.departure_cue_dup);
}

void DeleteWingCommand::dropSurvivors()
{
	_savedShips.erase(std::remove_if(_savedShips.begin(), _savedShips.end(),
	                                 [](const CapturedShip& cs) {
		                                 return cs.signature > 0 && obj_get_by_signature(cs.signature) >= 0;
	                                 }),
	                  _savedShips.end());
}

void DeleteWingCommand::undo()
{
	_currentObjNums.clear();

	// Restore member ships.
	SCP_map<int, int> oldInstToNewObj;
	for (auto& cs : _savedShips) {
		const int newObj = restoreShip(cs, _editor);
		if (newObj >= 0) {
			_currentObjNums.push_back(newObj);
			oldInstToNewObj[cs.originalShipIndex] = newObj;
		}
	}

	obj_merge_created_list();

	// Sync restored ships into their original layers.
	for (int objNum : _currentObjNums) {
		if (query_valid_object(objNum))
			_viewport->registerObjectInLayer(objNum);
	}
	_editor->notifyLayerStructureChanged();

	// Update member ship indices in _savedWing to the new instances.
	for (auto& mem : _savedWing.members) {
		auto it = oldInstToNewObj.find(mem.shipIndex);
		if (it != oldInstToNewObj.end()) {
			mem.shipIndex = Objects[it->second].instance;
		}
	}

	// Restore the wing struct.
	const int slot = findFreeWingSlot(_savedWing.wingNum);
	if (slot < 0) {
		_editor->missionChanged();
		_viewport->needsUpdate();
		return;
	}
	_currentWingNum = slot;

	// Un-invalidate SEXP refs that delete_wing renamed to <WingName>.
	char invalidatedName[NAME_LENGTH + 2];
	sprintf(invalidatedName, "<%s>", _savedWing.name);
	update_sexp_references(invalidatedName, _savedWing.name);
	_editor->ai_update_goal_references(sexp_ref_type::WING, invalidatedName, _savedWing.name);

	restoreWingIntoSlot(_savedWing, slot, _editor);
	update_custom_wing_indexes();

	// Select the restored wing (mark_wing calls unmark_all first, so markObject calls are redundant).
	_editor->mark_wing(slot);

	_editor->missionChanged();
	_viewport->needsUpdate();
}

void DeleteWingCommand::redo()
{
	if (_firstRedo) {
		_firstRedo = false;
		return; // deletion was already performed by the caller
	}

	if (_currentWingNum < 0 || Wings[_currentWingNum].wave_count == 0) return;

	// Re-capture wing cues before delete_wing frees them.
	recaptureCues(_savedWing, _currentWingNum);

	// Re-capture ship SEXP cues and texture replacements before delete_wing() frees them.
	// restoreShip() transferred cue ownership into the live ships (setting cue_dup to
	// SHIP_CUE_NONE), so we must re-dup now before the ships are destroyed. Ships are
	// found by signature: undo recreated them in new slots, and _savedShips no longer
	// lines up with the wing slots once survivors are dropped.
	for (auto& cs : _savedShips) {
		const int n = obj_get_by_signature(cs.signature);
		if (n < 0) continue;
		cs.originalShipIndex = Objects[n].instance;
		recaptureShipForRedo(cs, n);
	}
	// Members that survived the original delete (dropSurvivors) are deleted this
	// time, since redo confirms references; capture them so undo can bring them back.
	for (const auto& mem : _savedWing.members) {
		const int n = obj_get_by_signature(mem.signature);
		if (n < 0) continue;
		const bool captured = std::any_of(_savedShips.begin(), _savedShips.end(),
		                                  [&](const CapturedShip& cs) { return cs.signature == mem.signature; });
		if (captured) continue;
		doCaptureShip(n, _savedShips);
		removeShipTextureReplacements(Ships[Objects[n].instance].ship_name); // as recaptureShipForRedo does
	}

	{
		AutoConfirmReferences guard(_editor);
		if (_editor->delete_wing(_currentWingNum, 0) == 0) {
			_currentWingNum = -1;
		}
	}

	_editor->missionChanged();
	_viewport->needsUpdate();
}

// ===========================================================================
// GenerateWaypointPathCommand
// ===========================================================================

GenerateWaypointPathCommand::GenerateWaypointPathCommand(SCP_string        pathName,
                                                          SCP_vector<vec3d> positions,
                                                          SCP_vector<int>   signatures,
                                                          Editor*           editor,
                                                          const QString&    text,
                                                          QUndoCommand*     parent)
    : QUndoCommand(text, parent)
    , _pathName(std::move(pathName))
    , _positions(std::move(positions))
    , _signatures(std::move(signatures))
    , _editor(editor)
{}

void GenerateWaypointPathCommand::undo()
{
	// Waypoints created by apply() land in obj_create_list, not obj_used_list.
	// If undo fires before the first render frame, query_valid_object (called by
	// delete_object → unmarkObject) won't find them.  Promote them first.
	obj_merge_created_list();

	// Find the generated path by name.
	waypoint_list* wl = nullptr;
	for (auto& candidate : Waypoint_lists) {
		if (!stricmp(candidate.get_name(), _pathName.c_str())) {
			wl = &candidate;
			break;
		}
	}
	if (!wl)
		return;

	// Collect all object numbers before any deletion; waypoint_remove() shifts
	// the instance values of remaining waypoints as each one is removed, but the
	// object numbers themselves remain stable until obj_delete() is called.
	SCP_vector<int> objnums;
	objnums.reserve(wl->get_waypoints().size());
	for (const auto& wp : wl->get_waypoints())
		objnums.push_back(wp.get_objnum());

	for (int objnum : objnums)
		_editor->delete_object(objnum);
}

void GenerateWaypointPathCommand::redo()
{
	if (_firstRedo) {
		_firstRedo = false;
		return;
	}
	if (_positions.empty())
		return;

	int listIndex = static_cast<int>(Waypoint_lists.size());
	// Give each point the signature it had when generated, so later commands
	// (moves, path color or layer) still find it.
	auto setSignature = [this](int objNum, size_t i) {
		if (objNum >= 0 && i < _signatures.size() && _signatures[i] > 0)
			Objects[objNum].signature = _signatures[i];
	};
	setSignature(waypoint_add(_positions.data(), -1), 0);
	Waypoint_lists[static_cast<size_t>(listIndex)].set_name(_pathName.c_str());
	for (int i = 1; i < static_cast<int>(_positions.size()); ++i)
		setSignature(waypoint_add(&_positions[i], calc_waypoint_instance(listIndex, i - 1)), static_cast<size_t>(i));

	obj_merge_created_list();
	_editor->missionChanged();
}

// ===========================================================================
// VoiceActingBatchCommand
// ===========================================================================

VoiceActingBatchCommand::VoiceActingBatchCommand(Snapshot       before,
                                                 Snapshot       after,
                                                 Editor*        editor,
                                                 const QString& text,
                                                 QUndoCommand*  parent)
    : QUndoCommand(text, parent)
    , _before(std::move(before))
    , _after(std::move(after))
    , _editor(editor)
{}

void VoiceActingBatchCommand::restore(const Snapshot& snap)
{
    for (const auto& s : snap.cmdBriefFilenames)
        strcpy(Cmd_briefs[0].stage[s.stageIdx].wave_filename, s.filename.c_str());

    for (const auto& s : snap.briefingVoices)
        strcpy(Briefings[0].stages[s.stageIdx].voice, s.filename.c_str());

    for (const auto& s : snap.debriefingVoices)
        strcpy(Debriefings[0].stages[s.stageIdx].voice, s.filename.c_str());

    for (const auto& m : snap.messages) {
        MMessage* msg = nullptr;
        for (int i = Num_builtin_messages; i < Num_messages; ++i) {
            if (!stricmp(Messages[i].name, m.msgName.c_str())) { msg = &Messages[i]; break; }
        }
        if (!msg) continue;
        if (msg->wave_info.name) { free(msg->wave_info.name); msg->wave_info.name = nullptr; }
        if (!m.waveFilename.empty()) msg->wave_info.name = strdup(m.waveFilename.c_str());
        if (msg->avi_info.name)  { free(msg->avi_info.name);  msg->avi_info.name  = nullptr; }
        if (!m.aviFilename.empty())  msg->avi_info.name  = strdup(m.aviFilename.c_str());
        msg->persona_index = m.personaIndex;
    }

    for (const auto& s : snap.ships) {
        const int objNum = obj_get_by_signature(s.sig);
        if (objNum >= 0) Ships[Objects[objNum].instance].persona_index = s.personaIndex;
    }
}

void VoiceActingBatchCommand::undo()
{
    restore(_before);
    _editor->missionChanged();
}

void VoiceActingBatchCommand::redo()
{
    restore(_after);
    _editor->missionChanged();
}

// ===========================================================================
// BatchFlagCommand
// ===========================================================================

BatchFlagCommand::BatchFlagCommand(SCP_vector<ShipSnapshot> before,
                                   SCP_vector<ShipSnapshot> after,
                                   Editor*                  editor,
                                   const QString&           text,
                                   QUndoCommand*            parent)
    : QUndoCommand(text, parent)
    , _before(std::move(before))
    , _after(std::move(after))
    , _editor(editor)
{}

void BatchFlagCommand::restore(const SCP_vector<ShipSnapshot>& snaps)
{
    for (const auto& snap : snaps) {
        const int objNum = obj_get_by_signature(snap.sig);
        if (objNum < 0) continue;
        Objects[objNum].flags = withLiveEditorObjectFlags(snap.objFlags, Objects[objNum].flags);
        Ships[Objects[objNum].instance].flags = snap.shipFlags;
    }
}

void BatchFlagCommand::undo()
{
    restore(_before);
    _editor->missionChanged();
}

void BatchFlagCommand::redo()
{
    restore(_after);
    _editor->missionChanged();
}

// ===========================================================================
// ApplyDialogCommand
// ===========================================================================

ApplyDialogCommand::ApplyDialogCommand(std::unique_ptr<fso::fred::dialogs::AbstractDialogModel> model,
                                       QByteArray                                               stateBefore,
                                       QByteArray                                               stateAfter,
                                       Editor*                                                  editor,
                                       const QString&                                           text,
                                       QUndoCommand*                                            parent)
    : QUndoCommand(text, parent)
    , _model(std::move(model))
    , _stateBefore(std::move(stateBefore))
    , _stateAfter(std::move(stateAfter))
    , _editor(editor)
{}

void ApplyDialogCommand::undo()
{
    _model->restoreState(_stateBefore);
    _editor->missionChanged();
}

void ApplyDialogCommand::redo()
{
    // The dialog's apply() has already written stateAfter, so the push does
    // nothing. Restoring here would round-trip every OK through the
    // serializer and turn any serialization flaw into data loss.
    if (_firstRedo) {
        _firstRedo = false;
        return;
    }
    _model->restoreState(_stateAfter);
    _editor->missionChanged();
}

// ---------------------------------------------------------------------------
// DialogSnapshotCommand
// ---------------------------------------------------------------------------

DialogSnapshotCommand::DialogSnapshotCommand(QByteArray                             before,
                                             QByteArray                             after,
                                             std::function<void(const QByteArray&)> restore,
                                             const QString&                         text,
                                             bool                                   skipFirstRedo,
                                             int                                    mergeId,
                                             QUndoCommand*                          parent)
	: QUndoCommand(text, parent)
	, _before(std::move(before))
	, _after(std::move(after))
	, _restore(std::move(restore))
	, _skipFirstRedo(skipFirstRedo)
	, _mergeId(mergeId)
{
}

void DialogSnapshotCommand::undo()
{
	_restore(_before);
}

void DialogSnapshotCommand::redo()
{
	if (_skipFirstRedo) {
		_skipFirstRedo = false;
		return;
	}
	_restore(_after);
}

bool DialogSnapshotCommand::mergeWith(const QUndoCommand* other)
{
	if (typeid(*other) != typeid(*this))
		return false;
	const auto* o = static_cast<const DialogSnapshotCommand*>(other);
	if (_mergeId == -1 || o->_mergeId != _mergeId)
		return false;
	_after = o->_after;
	return true;
}

// ---------------------------------------------------------------------------
// SexpCueEditCommand
// ---------------------------------------------------------------------------

SexpCueEditCommand::SexpCueEditCommand(Editor*        editor,
                                       const QString& text,
                                       bool           skipFirstRedo,
                                       QUndoCommand*  parent)
	: QUndoCommand(text, parent)
	, _afterDup(SHIP_CUE_NONE)
	, _editor(editor)
	, _skipFirstRedo(skipFirstRedo)
{
}

SexpCueEditCommand::~SexpCueEditCommand()
{
	for (auto& o : _owners)
		freeSexpCueDup(o.beforeDup);
	freeSexpCueDup(_afterDup);
}

void SexpCueEditCommand::addOwner(int beforeCue, std::function<int()> getCue, std::function<void(int)> setCue)
{
	_owners.push_back({ captureSexpCue(beforeCue), std::move(getCue), std::move(setCue) });
}

void SexpCueEditCommand::captureAfter(int afterCue)
{
	freeSexpCueDup(_afterDup);
	_afterDup = captureSexpCue(afterCue);
}

void SexpCueEditCommand::freeCurrentCues()
{
	SCP_vector<int> freed;
	for (auto& o : _owners) {
		const int cue = o.getCue();
		if (cue < 0 || cue == Locked_sexp_true || cue == Locked_sexp_false)
			continue;
		if (std::find(freed.begin(), freed.end(), cue) != freed.end())
			continue;
		free_sexp2(cue);
		freed.push_back(cue);
	}
}

void SexpCueEditCommand::undo()
{
	freeCurrentCues();
	for (auto& o : _owners)
		o.setCue(materializeSexpCue(o.beforeDup));
	_editor->missionChanged();
}

void SexpCueEditCommand::redo()
{
	if (_skipFirstRedo) {
		_skipFirstRedo = false;
		return;
	}
	freeCurrentCues();
	// Every caller adds a single owner (one wing, one ship), so this is a per-owner
	// restore. Do NOT reintroduce multi-owner use with a shared formula: the model
	// setters serialize a separate expression per ship, and aliasing several owners
	// onto one index means freeing one corrupts the rest.
	const int formula = materializeSexpCue(_afterDup);
	for (auto& o : _owners)
		o.setCue(formula);
	_editor->missionChanged();
}

// ---------------------------------------------------------------------------
// TextureReplacementCommand
// ---------------------------------------------------------------------------

TextureReplacementCommand::TextureReplacementCommand(SCP_string               shipName,
                                                     SCP_vector<texture_replace> before,
                                                     Editor*                  editor,
                                                     QUndoCommand*            parent)
    : QUndoCommand(QObject::tr("Edit Texture Replacements"), parent)
    , _shipName(std::move(shipName))
    , _before(std::move(before))
    , _editor(editor)
{}

void TextureReplacementCommand::setAfter(SCP_vector<texture_replace> after)
{
    _after = std::move(after);
}

void TextureReplacementCommand::apply(const SCP_vector<texture_replace>& entries)
{
    removeShipTextureReplacements(_shipName.c_str());
    for (const auto& tr : entries)
        Fred_texture_replacements.push_back(tr);

    // Rebuild the pmi so the viewport reflects the texture change immediately.
    const int shipIdx = ship_name_lookup(_shipName.c_str());
    if (shipIdx >= 0)
        rebuildShipPmiTextures(shipIdx);

    _editor->missionChanged();
}

void TextureReplacementCommand::undo() { apply(_before); }
void TextureReplacementCommand::redo() { apply(_after);  }

} // namespace fso::fred
