#pragma once

#include <functional>
#include <memory>
#include <typeinfo>

#include <QUndoCommand>
#include <mission/dialogs/AbstractDialogModel.h>
#include <globalincs/pstypes.h>
#include <math/vecmat.h>
#include <ship/ship.h>
#include <ai/aigoals.h>

#include "ObjectCapture.h"

namespace fso::fred {

class Editor;
class EditorViewport;

// ---------------------------------------------------------------------------
// MoveObjectsCommand — drag / rotate objects in the viewport
// ---------------------------------------------------------------------------

struct ObjectTransform {
	int    signature;
	vec3d  posBefore;
	matrix orientBefore;
	vec3d  posAfter;
	matrix orientAfter;
};

class MoveObjectsCommand : public QUndoCommand {
	SCP_vector<ObjectTransform> _transforms;
	Editor*                     _editor;
	EditorViewport*             _viewport;

public:
	MoveObjectsCommand(SCP_vector<ObjectTransform> transforms,
	                   Editor*                     editor,
	                   EditorViewport*             viewport,
	                   QUndoCommand*               parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// CreateObjectCommand — Ctrl+click to place a new object
// ---------------------------------------------------------------------------

class CreateObjectCommand : public QUndoCommand {
	vec3d           _pos;
	int             _savedModelIndex;
	int             _savedPropIndex;
	int             _waypointInstance;
	SCP_string      _waypointListName; // name of list to append to (empty = create new list)
	SCP_string      _jumpNodeName;     // original JN name; restored on redo for stable identity
	CreateKind      _createKind;
	OtherKind       _otherKind;        // only meaningful when _createKind == Other
	int             _createdObjNum;
	int             _originalSignature; // object signature at creation; restored on redo
	bool            _firstRedo = true;
	Editor*         _editor;
	EditorViewport* _viewport;

public:
	// Pass the already-created object number as initialObjNum.
	// redo() is a no-op on its first call (object already created by caller).
	CreateObjectCommand(const vec3d& pos,
	                    int          modelIndex,
	                    int          propIndex,
	                    int          waypointInstance,
	                    CreateKind   createKind,
	                    OtherKind    otherKind,
	                    int          initialObjNum,
	                    Editor*      editor,
	                    EditorViewport* viewport,
	                    QUndoCommand* parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// DeleteObjectsCommand — delete all currently-marked objects
// ---------------------------------------------------------------------------

// One member slot in a wing at the time of capture (used by DeleteObjectsCommand
// and the three wing commands to track who was where in the wing).
struct SavedWingMemberSlot {
	int  shipIndex;            // Ships[] instance at capture time (can go stale — prefer signature)
	int  signature = -1;       // object signature at capture time (stable across undo cycles)
	char name[NAME_LENGTH];    // ship name at capture time
	bool wasDeleted;           // true if this member is being deleted (DeleteObjectsCommand only)
};

// Sentinel values for SavedWingData::arrival/departure_cue_dup.
// Non-negative values are a live dup'd SEXP node index owned by the command.
static constexpr int WING_CUE_LOCKED_TRUE  = -1; // restore Locked_sexp_true
static constexpr int WING_CUE_LOCKED_FALSE = -2; // restore Locked_sexp_false
static constexpr int WING_CUE_NONE         = -3; // command holds no dup right now

// Full wing state snapshot used by all wing commands.
struct SavedWingData {
	int   wingNum;
	int   waveCount;
	int   special_ship;
	bool  entirelyDeleted;        // true = all members are being deleted (DeleteObjectsCommand only)
	char  name[NAME_LENGTH];
	int   hotkey;
	int   reinforcement_index;
	int   num_waves;
	int   threshold;
	int   wave_delay_min;
	int   wave_delay_max;
	ArrivalLocation  arrival_location;
	int              arrival_distance;
	anchor_t         arrival_anchor;
	int              arrival_path_mask;
	int              arrival_cue_dup;   // ownership-transfer dup (see WING_CUE_* sentinels)
	int              arrival_delay;
	DepartureLocation departure_location;
	anchor_t          departure_anchor;
	int               departure_path_mask;
	int               departure_cue_dup; // ownership-transfer dup (see WING_CUE_* sentinels)
	int               departure_delay;
	flagset<Ship::Wing_Flags> flags;
	ai_goal           ai_goals[MAX_AI_GOALS];
	char              wing_squad_filename[MAX_FILENAME_LEN];
	int               wing_insignia_texture;
	int               formation;
	float             formation_scale;
	SCP_vector<SavedWingMemberSlot> members; // in slot order 0..waveCount-1

	// The wing's Reinforcements[] entry, if any — delete_wing()/disband_wing()
	// erase it, so undo must re-add it.
	bool           hadReinforcement = false;
	reinforcements reinforcementEntry;
};

class DeleteObjectsCommand : public QUndoCommand {
	SCP_vector<CapturedShip>        _ships;
	SCP_vector<CapturedWaypointList> _waypointGroups;
	SCP_vector<CapturedJumpNode>    _jumpNodes;
	SCP_vector<CapturedProp>        _props;
	SCP_vector<SavedWingData>       _affectedWings; // wings with ≥1 deleted member
	// current live obj nums — repopulated by undo(), consumed by redo()
	SCP_vector<int>               _currentObjNums;
	bool                          _firstRedo = true;
	Editor*                       _editor;
	EditorViewport*               _viewport;

public:
	// Construct BEFORE calling delete_marked(). Captures all marked objects.
	DeleteObjectsCommand(Editor* editor, EditorViewport* viewport, QUndoCommand* parent = nullptr);
	~DeleteObjectsCommand() override;
	void undo() override;
	void redo() override;
	bool isEmpty() const;
};

// ---------------------------------------------------------------------------
// ChangeIFFCommand — IFF / team change on marked ships
// ---------------------------------------------------------------------------

struct ShipIFFChange {
	int signature;
	int iffBefore;
	int iffAfter;
};

class ChangeIFFCommand : public QUndoCommand {
	SCP_vector<ShipIFFChange> _changes;
	Editor*                   _editor;

public:
	ChangeIFFCommand(SCP_vector<ShipIFFChange> changes,
	                 Editor*                   editor,
	                 QUndoCommand*             parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// LevelObjectsCommand / AlignObjectsCommand — orientation-only changes
// ---------------------------------------------------------------------------

struct ObjectOrientChange {
	int    signature;
	matrix orientBefore;
	matrix orientAfter;
};

class LevelObjectsCommand : public QUndoCommand {
	SCP_vector<ObjectOrientChange> _changes;
	Editor*                        _editor;
	EditorViewport*                _viewport;

public:
	LevelObjectsCommand(SCP_vector<ObjectOrientChange> changes,
	                    Editor*                        editor,
	                    EditorViewport*                viewport,
	                    QUndoCommand*                  parent = nullptr);
	void undo() override;
	void redo() override;
};

class AlignObjectsCommand : public QUndoCommand {
	SCP_vector<ObjectOrientChange> _changes;
	Editor*                        _editor;
	EditorViewport*                _viewport;

public:
	AlignObjectsCommand(SCP_vector<ObjectOrientChange> changes,
	                    Editor*                        editor,
	                    EditorViewport*                viewport,
	                    QUndoCommand*                  parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// RenameObjectCommand — rename a ship/start, jump node, prop, or waypoint path
// ---------------------------------------------------------------------------

class RenameObjectCommand : public QUndoCommand {
	// For real objects (ships, jump nodes, props): _signature identifies them stably
	// across undo/redo cycles regardless of objNum pool changes.
	// For waypoint paths: _signature is -1 and the path is located by name instead.
	int        _signature;
	SCP_string _oldName;
	SCP_string _newName;
	Editor*    _editor;
	bool       _skipFirstRedo = false; // set when rename already applied by caller

	// name    = the name to apply
	// current = the name the object currently has (lookup key for waypoint paths;
	//           also passed to SEXP update functions as the "old" value)
	void applyName(const SCP_string& name, const SCP_string& current);

public:
	// skipFirstRedo: pass true when the rename has already been applied by the
	// caller (e.g. a dialog slot); the first redo() fired by QUndoStack::push()
	// will be a no-op so the rename is not applied twice.
	RenameObjectCommand(int        objNum,
	                    SCP_string oldName,
	                    SCP_string newName,
	                    Editor*    editor,
	                    bool       skipFirstRedo = false,
	                    QUndoCommand* parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// MoveLayerCommand — move objects between layers
// ---------------------------------------------------------------------------

struct ObjectLayerChange {
	int        signature;
	SCP_string layerBefore;
	SCP_string layerAfter;
};

class MoveLayerCommand : public QUndoCommand {
	SCP_vector<ObjectLayerChange> _changes;
	EditorViewport*               _viewport;
	Editor*                       _editor;

public:
	MoveLayerCommand(SCP_vector<ObjectLayerChange> changes,
	                 EditorViewport*               viewport,
	                 Editor*                       editor,
	                 QUndoCommand*                 parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// CloneMarkedObjectsCommand — duplicate all marked objects (menu action)
// ---------------------------------------------------------------------------

class CloneMarkedObjectsCommand : public QUndoCommand {
	SCP_vector<int> _originalSignatures; // marked objects at construction (stable identity)
	SCP_vector<int> _cloneSignatures;    // refreshed on each redo() — clones are new objects
	int             _previousCurrentObj;
	Editor*         _editor;
	EditorViewport* _viewport;

public:
	// Construct while the objects to clone are still marked.
	CloneMarkedObjectsCommand(Editor*         editor,
	                          EditorViewport* viewport,
	                          QUndoCommand*   parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// CloneDragCommand — Ctrl+drag clone gesture (clone already happened)
// ---------------------------------------------------------------------------
// Handles both regular Ctrl+drag (new copies) and Ctrl+Shift+drag (insert
// waypoints into existing path). The clone and move are baked into a single
// undo step; skipFirstRedo prevents re-cloning on push.
// ---------------------------------------------------------------------------

struct CloneDragEntry {
	int    sourceSignature; // source object (stable, never deleted on undo)
	int    cloneSignature;  // updated each redo() call
	vec3d  initialPos;      // position right after cloning (= source pos at drag start)
	matrix initialOrient;
	vec3d  finalPos;        // position after user finished dragging
	matrix finalOrient;
};

class CloneDragCommand : public QUndoCommand {
	SCP_vector<CloneDragEntry> _entries;
	int             _previousCurrentObj;
	bool            _insert;        // true = DUP_DRAG_INSERT (waypoint insertion)
	bool            _skipFirstRedo;
	int             _wingNum = -1;  // -1 = no wing add; ≥0 = redo re-adds clones to this wing
	Editor*         _editor;
	EditorViewport* _viewport;

public:
	CloneDragCommand(SCP_vector<CloneDragEntry> entries,
	                 int             previousCurrentObj,
	                 bool            insert,
	                 Editor*         editor,
	                 EditorViewport* viewport,
	                 QUndoCommand*   parent = nullptr);
	void undo() override;
	void redo() override;

	// Call after wing mutations complete (first redo already done by renderwidget).
	void recordWingAdd(int wingNum);
};

// ---------------------------------------------------------------------------
// FormWingCommand — form a wing from marked ships
//
// KNOWN LIMITATION: create_wing() frees each member's arrival/departure cues,
// and only the pre-formation names are captured here — so undoing a wing
// formation leaves ex-members with default (locked-false) cues rather than
// their originals. This matches FRED2's destructive formation behavior.
// ---------------------------------------------------------------------------

struct WingMemberPreState {
	int  shipIndex;
	char preName[NAME_LENGTH];  // ship name before wing formation
};

class FormWingCommand : public QUndoCommand {
	int  _wingNum;
	char _wingName[NAME_LENGTH];
	int  _special_ship;
	SCP_vector<WingMemberPreState> _members; // in wing slot order
	Editor*         _editor;
	EditorViewport* _viewport;
	bool            _firstRedo = true;

public:
	// Construct AFTER create_wing() succeeds. First redo() is a no-op.
	FormWingCommand(int                            wingNum,
	                SCP_vector<WingMemberPreState> members,
	                Editor*                        editor,
	                EditorViewport*                viewport,
	                QUndoCommand*                  parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// DisbandWingCommand — disband a wing (keep ships, remove wing structure)
// ---------------------------------------------------------------------------

class DisbandWingCommand : public QUndoCommand {
	SavedWingData   _savedWing;
	Editor*         _editor;
	EditorViewport* _viewport;
	bool            _firstRedo = true;

public:
	// Construct BEFORE disband_wing() is called. First redo() is a no-op.
	DisbandWingCommand(int             wingNum,
	                   Editor*         editor,
	                   EditorViewport* viewport,
	                   QUndoCommand*   parent = nullptr);
	~DisbandWingCommand() override;
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// DeleteWingCommand — delete a wing and all its member ships
// ---------------------------------------------------------------------------

class DeleteWingCommand : public QUndoCommand {
	SavedWingData             _savedWing;
	SCP_vector<CapturedShip>  _savedShips; // member ships in slot order
	SCP_vector<int>          _currentObjNums; // live obj nums (updated by undo)
	int                      _currentWingNum; // live wing slot (updated by undo)
	Editor*                  _editor;
	EditorViewport*          _viewport;
	bool                     _firstRedo = true;

public:
	// Construct BEFORE delete_wing(wingNum, 0) is called. First redo() is a no-op.
	DeleteWingCommand(int             wingNum,
	                  Editor*         editor,
	                  EditorViewport* viewport,
	                  QUndoCommand*   parent = nullptr);
	~DeleteWingCommand() override;
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// GenerateWaypointPathCommand — undo/redo for the waypoint path generator
// ---------------------------------------------------------------------------

class GenerateWaypointPathCommand : public QUndoCommand {
	SCP_string        _pathName;
	SCP_vector<vec3d> _positions; // exact positions from initial generation
	bool              _firstRedo = true;
	Editor*           _editor;

public:
	// Construct AFTER apply() succeeds. First redo() is a no-op.
	GenerateWaypointPathCommand(SCP_string        pathName,
	                            SCP_vector<vec3d> positions,
	                            Editor*           editor,
	                            const QString&    text   = {},
	                            QUndoCommand*     parent = nullptr);
	void undo() override;
	void redo() override;
};

// ---------------------------------------------------------------------------
// ApplyDialogCommand — atomic undo entry pushed when a modal dialog accepts
// ---------------------------------------------------------------------------

class ApplyDialogCommand : public QUndoCommand {
    std::unique_ptr<fso::fred::dialogs::AbstractDialogModel> _model;
    QByteArray                                               _stateBefore;
    QByteArray                                               _stateAfter;
    Editor*                                                  _editor;

public:
    ApplyDialogCommand(std::unique_ptr<fso::fred::dialogs::AbstractDialogModel> model,
                       QByteArray                                               stateBefore,
                       QByteArray                                               stateAfter,
                       Editor*                                                  editor,
                       const QString&                                           text,
                       QUndoCommand*                                            parent = nullptr);
    void undo() override;
    void redo() override;
};

// ---------------------------------------------------------------------------
// FieldId — global field identifiers used by FieldEditCommand for merging.
// Each (dialog, field) pair must have a unique value so that consecutive
// edits to the same field merge while edits to different fields do not.
// ---------------------------------------------------------------------------

namespace FieldId {
    // Jump Node editor
    constexpr int JN_DisplayName = 3001;
    constexpr int JN_ModelFile   = 3002;
    constexpr int JN_ColorR      = 3003;
    constexpr int JN_ColorG      = 3004;
    constexpr int JN_ColorB      = 3005;
    constexpr int JN_ColorA      = 3006;
    constexpr int JN_Hidden      = 3007;
    // Waypoint Path editor    4001–4099
    constexpr int WP_NoDrawLines    = 4001;
    constexpr int WP_HasCustomColor = 4002;
    constexpr int WP_ColorR         = 4003;
    constexpr int WP_ColorG         = 4004;
    constexpr int WP_ColorB         = 4005;
    // Prop editor             4101–4199
    // Background editor       4201–4299
    // Wing editor             4301–4399
    // Ship editor             4401–4499
}

// ---------------------------------------------------------------------------
// FieldEditCommand<T> — undo/redo a single-field change on one or more objects
//
// Add one Entry per selected object via addEntry().  Single-select dialogs
// add one entry; multi-select add N.  Consecutive pushes with the same
// fieldId and entry count/order are merged — only the latest 'after' is kept,
// collapsing rapid spinbox edits into one undo step.
//
// skipFirstRedo: pass true when the caller already applied the change before
// pushing (e.g. via a model setter that handles validation).  The first
// redo() fired by QUndoStack::push() is then a no-op.
// ---------------------------------------------------------------------------

template<typename T>
class FieldEditCommand : public QUndoCommand {
    struct Entry {
        T before;
        T after;
        std::function<void(const T&)> setter;
    };

    SCP_vector<Entry> _entries;
    int               _fieldId;
    SCP_string        _targetKey; // identity of the edited object(s); see setTargetKey()
    Editor*           _editor;
    bool              _skipFirstRedo;

public:
    FieldEditCommand(int            fieldId,
                     Editor*        editor,
                     const QString& text          = {},
                     bool           skipFirstRedo = false,
                     QUndoCommand*  parent        = nullptr)
        : QUndoCommand(text, parent)
        , _fieldId(fieldId)
        , _editor(editor)
        , _skipFirstRedo(skipFirstRedo)
    {}

    // Identity of the edited target(s) — e.g. the ship signature(s) or wing
    // name the setters are bound to. Commands with different keys never merge,
    // so editing the same field on a different selection starts a new undo
    // step instead of cross-wiring the previous command's setters.
    void setTargetKey(SCP_string key) { _targetKey = std::move(key); }

    void addEntry(T before, T after, std::function<void(const T&)> setter) {
        _entries.push_back({ std::move(before), std::move(after), std::move(setter) });
    }

    bool isEmpty() const { return _entries.empty(); }

    void undo() override {
        for (const auto& e : _entries) e.setter(e.before);
        _editor->missionChanged();
    }

    void redo() override {
        if (_skipFirstRedo) { _skipFirstRedo = false; return; }
        for (const auto& e : _entries) e.setter(e.after);
        _editor->missionChanged();
    }

    bool mergeWith(const QUndoCommand* other) override {
        if (typeid(*other) != typeid(*this)) return false;
        const auto* o = static_cast<const FieldEditCommand<T>*>(other);
        if (o->_fieldId != _fieldId) return false;
        if (o->_targetKey != _targetKey) return false;
        if (o->_entries.size() != _entries.size()) return false;
        for (size_t i = 0; i < _entries.size(); ++i)
            _entries[i].after = o->_entries[i].after;
        return true;
    }

    int id() const override { return _fieldId; }
};

} // namespace fso::fred
