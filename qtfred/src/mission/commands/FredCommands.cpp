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
#include "mission/object.h"

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
	if (_createdObjNum >= 0 && query_valid_object(_createdObjNum)) {
		AutoConfirmReferences guard(_editor);
		_editor->delete_object(_createdObjNum);
		_createdObjNum = -1;
	}
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

// Restore a wing into the given slot from saved data. Members are resolved by
// object signature (falling back to wd.members[i].shipIndex); callers that
// recreate deleted member ships must restore them before calling this.
void restoreWingIntoSlot(SavedWingData& wd, int slot, Editor* editor)
{
	wing& w = Wings[slot];
	w.clear();
	strcpy_s(w.name, wd.name);
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

	for (int i = 0; i < (int)wd.members.size(); i++) {
		const int shipIdx = resolveMemberInstance(wd.members[i]);
		w.ship_index[i]             = shipIdx;
		editor->wing_objects[slot][i] = Ships[shipIdx].objnum;
		Ships[shipIdx].wingnum      = slot;
		Ships[shipIdx].arrival_cue  = Locked_sexp_false;
		Ships[shipIdx].departure_cue = Locked_sexp_false;
		editor->rename_ship(shipIdx, wd.members[i].name);
	}
	restoreWingReinforcement(wd, w);
	Num_wings++;
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
			// Points are in ascending original-index order. Reinserting in this order
			// progressively restores the instances of higher-index waypoints, so the
			// stored predecessorInstance is valid by the time each point is reinserted.
			for (const auto& pt : group.points) {
				vec3d pos = pt.pos;
				int newObj;
				if (pt.predecessorInstance < 0) {
					// This was the first waypoint in the list — insert at front.
					waypoint_list* wl = find_matching_waypoint_list(group.listName.c_str());
					if (wl == nullptr || wl->get_waypoints().empty()) continue;
					const int anyInst = Objects[wl->get_waypoints().front().get_objnum()].instance;
					newObj = waypoint_add(&pos, anyInst, /*first_waypoint_in_list=*/true);
					_editor->missionChanged();
				} else {
					// Insert after the predecessor (whose instance has been restored
					// by the previous reinsertions in this group).
					newObj = _editor->create_waypoint(&pos, pt.predecessorInstance);
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
			}
			restoreWingReinforcement(wd, w);
			Num_wings++;
			Editor::update_custom_wing_indexes();
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
			}
		}
	}

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
{}

void RenameObjectCommand::applyName(const SCP_string& name, const SCP_string& current)
{
	if (_signature >= 0) {
		const int objNum = obj_get_by_signature(_signature);
		if (objNum < 0) return; // object doesn't exist in this undo state
		const int type = Objects[objNum].type;
		if (type == OBJ_SHIP || type == OBJ_START) {
			_editor->rename_ship(Objects[objNum].instance, name.c_str());
		} else if (type == OBJ_JUMP_NODE) {
			_editor->rename_jump_node(objNum, name.c_str());
		} else if (type == OBJ_PROP) {
			_editor->rename_prop(objNum, name.c_str());
		}
	} else {
		// Waypoint path: current holds the name it has right now; name is what to rename to.
		_editor->rename_waypoint_list(current.c_str(), name.c_str());
	}
}

void RenameObjectCommand::undo()
{
	// Object is currently named _newName; rename it back to _oldName.
	applyName(_oldName, _newName);
}

void RenameObjectCommand::redo()
{
	if (_skipFirstRedo) { _skipFirstRedo = false; return; }
	// Object is currently named _oldName; rename it to _newName.
	applyName(_newName, _oldName);
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
		_cloneSignatures.clear();
		// After duplicate_marked_objects(), the newly created objects are marked.
		// The originals are unmarked.
		for (const object* p = GET_FIRST(&obj_used_list);
		     p != END_OF_LIST(&obj_used_list);
		     p = GET_NEXT(p))
		{
			if (p->flags[Object::Object_Flags::Marked]) {
				_cloneSignatures.push_back(p->signature);
			}
		}
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
	// inside duplicate_marked_objects), then apply final positions.
	int ei = 0;
	for (object* p = GET_FIRST(&obj_used_list);
	     p != END_OF_LIST(&obj_used_list) && ei < static_cast<int>(_entries.size());
	     p = GET_NEXT(p))
	{
		if (!p->flags[Object::Object_Flags::Marked]) continue;
		auto& e = _entries[ei++];
		e.cloneSignature = p->signature;
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

FormWingCommand::FormWingCommand(int                            wingNum,
                                 SCP_vector<WingMemberPreState> members,
                                 Editor*                        editor,
                                 EditorViewport*                viewport,
                                 QUndoCommand*                  parent)
    : QUndoCommand(QObject::tr("Form Wing"), parent)
    , _wingNum(wingNum)
    , _special_ship(Wings[wingNum].special_ship)
    , _members(std::move(members))
    , _editor(editor)
    , _viewport(viewport)
{
	strcpy_s(_wingName, Wings[wingNum].name);
}

void FormWingCommand::undo()
{
	// Locate the wing by name — the slot may differ from _wingNum if other wings
	// have been created/deleted between formation and this undo call.
	int slot = -1;
	for (int i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count > 0 && !stricmp(Wings[i].name, _wingName)) {
			slot = i;
			break;
		}
	}
	if (slot < 0) return;

	// Snapshot member instances from the live wing before disband_wing() clears
	// it — the instances captured at formation time can go stale if a member
	// was deleted and restored by an intervening undo cycle.
	SCP_vector<int> memberInst;
	for (int i = 0; i < Wings[slot].wave_count; i++) {
		memberInst.push_back(Wings[slot].ship_index[i]);
	}

	// Disband the wing (keeps ships, renames them to generic names).
	{
		AutoConfirmReferences guard(_editor);
		_editor->disband_wing(slot);
	}

	// Restore each ship's pre-formation name via rename_ship (updates SEXP refs).
	// memberInst and _members are both in wing slot order.
	for (size_t i = 0; i < memberInst.size() && i < _members.size(); i++) {
		if (Ships[memberInst[i]].wingnum == -1) {
			_editor->rename_ship(memberInst[i], _members[i].preName);
		}
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

	const int slot = findFreeWingSlot(_wingNum);
	if (slot < 0) return;

	// Resolve members by pre-formation name — undo() renamed them back, and the
	// Ships[] instances captured at formation time can move across undo cycles.
	SCP_vector<int> memberInst;
	memberInst.reserve(_members.size());
	for (const auto& m : _members) {
		const int inst = ship_name_lookup(m.preName, 1);
		if (inst < 0) return; // a member no longer exists; cannot re-form the wing
		memberInst.push_back(inst);
	}

	Wings[slot].clear();
	strcpy_s(Wings[slot].name, _wingName);
	Wings[slot].arrival_cue    = Locked_sexp_true;
	Wings[slot].departure_cue  = Locked_sexp_false;
	Wings[slot].wave_count     = (int)_members.size();
	Wings[slot].special_ship   = _special_ship;
	Wings[slot].hotkey         = -1;

	// Assign hotkey if this is a starting wing.
	for (int i = 0; i < MAX_STARTING_WINGS; i++) {
		if (!stricmp(_wingName, Starting_wing_names[i])) {
			Wings[slot].hotkey = i;
			break;
		}
	}

	for (int i = 0; i < (int)_members.size(); i++) {
		const int shipIdx = memberInst[i];
		Wings[slot].ship_index[i]       = shipIdx;
		_editor->wing_objects[slot][i]  = Ships[shipIdx].objnum;
		Ships[shipIdx].wingnum          = slot;
		Ships[shipIdx].arrival_cue      = Locked_sexp_false;
		Ships[shipIdx].departure_cue    = Locked_sexp_false;
		char buf[NAME_LENGTH];
		wing_bash_ship_name(buf, _wingName, i + 1);
		_editor->rename_ship(shipIdx, buf);
	}

	Num_wings++;
	_editor->mark_wing(slot);
	Editor::update_custom_wing_indexes();
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
	Editor::update_custom_wing_indexes();
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
	Editor::update_custom_wing_indexes();

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

	// Restore saved ships' original indices (they now have new instances from undo).
	// The _savedShips vector still has originalShipIndex from the first capture.
	// After undo restored them, those ship instances changed. We need to update
	// _savedShips.originalShipIndex to the current instances for future undo cycles.
	for (size_t i = 0; i < _savedWing.members.size() && i < _savedShips.size(); i++) {
		_savedShips[i].originalShipIndex = _savedWing.members[i].shipIndex;
	}

	// Re-capture ship SEXP cues and texture replacements before delete_wing() frees them.
	// restoreShip() transferred cue ownership into the live ships (setting cue_dup to
	// SHIP_CUE_NONE), so we must re-dup now before the ships are destroyed.
	for (auto& cs : _savedShips) {
		const int si = cs.originalShipIndex;
		if (si >= 0 && si < MAX_SHIPS && Ships[si].objnum >= 0)
			recaptureShipForRedo(cs, Ships[si].objnum);
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
                                                          Editor*           editor,
                                                          const QString&    text,
                                                          QUndoCommand*     parent)
    : QUndoCommand(text, parent)
    , _pathName(std::move(pathName))
    , _positions(std::move(positions))
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
	waypoint_add(_positions.data(), -1);
	Waypoint_lists[static_cast<size_t>(listIndex)].set_name(_pathName.c_str());
	for (int i = 1; i < static_cast<int>(_positions.size()); ++i)
		waypoint_add(&_positions[i], calc_waypoint_instance(listIndex, i - 1));

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
        Objects[objNum].flags = snap.objFlags;
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
	// All owners share one formula, matching the model setters' multi-edit behavior.
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
