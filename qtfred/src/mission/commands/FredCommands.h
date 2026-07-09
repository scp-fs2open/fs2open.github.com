#pragma once

#include <functional>
#include <memory>
#include <typeinfo>

#include <QByteArray>
#include <QUndoCommand>
#include <mission/dialogs/AbstractDialogModel.h>
#include <globalincs/pstypes.h>
#include <math/vecmat.h>
#include <mission/missionparse.h>
#include <object/object.h>
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
// BatchFlagCommand — set a global flag on all (or all fighter/bomber) ships
// ---------------------------------------------------------------------------

class BatchFlagCommand : public QUndoCommand {
public:
    struct ShipSnapshot {
        int sig;
        flagset<Object::Object_Flags> objFlags;
        flagset<Ship::Ship_Flags>     shipFlags;
        bool operator==(const ShipSnapshot& o) const {
            return sig == o.sig && objFlags == o.objFlags && shipFlags == o.shipFlags;
        }
        bool operator!=(const ShipSnapshot& o) const { return !(*this == o); }
    };

    BatchFlagCommand(SCP_vector<ShipSnapshot> before,
                     SCP_vector<ShipSnapshot> after,
                     Editor*                  editor,
                     const QString&           text   = {},
                     QUndoCommand*            parent = nullptr);
    void undo() override;
    void redo() override;

private:
    SCP_vector<ShipSnapshot> _before;
    SCP_vector<ShipSnapshot> _after;
    Editor*                  _editor;

    static void restore(const SCP_vector<ShipSnapshot>& snaps);
};

// ---------------------------------------------------------------------------
// VoiceActingBatchCommand — undo/redo for the 5 mutation buttons in the
//                           Voice Acting Manager dialog.
// ---------------------------------------------------------------------------

class VoiceActingBatchCommand : public QUndoCommand {
public:
    struct BriefStage {
        int        stageIdx;
        SCP_string filename;
    };
    struct MessageSnapshot {
        SCP_string msgName;         // Messages[i].name — stable identity across add/remove
        SCP_string waveFilename;    // Messages[i].wave_info.name (empty = nullptr)
        SCP_string aviFilename;     // Messages[i].avi_info.name  (empty = nullptr)
        int        personaIndex;
    };
    struct ShipPersona {
        int sig;
        int personaIndex;
    };
    struct Snapshot {
        SCP_vector<BriefStage>      cmdBriefFilenames;
        SCP_vector<BriefStage>      briefingVoices;
        SCP_vector<BriefStage>      debriefingVoices;
        SCP_vector<MessageSnapshot> messages;
        SCP_vector<ShipPersona>     ships;
    };

    VoiceActingBatchCommand(Snapshot       before,
                            Snapshot       after,
                            Editor*        editor,
                            const QString& text   = {},
                            QUndoCommand*  parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Snapshot _before;
    Snapshot _after;
    Editor*  _editor;

    static void restore(const Snapshot& snap);
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
    constexpr int Prop_Flags = 4101;
    // Background editor       4201–4299
    constexpr int BG_AngleFormat    = 4201;
    constexpr int BG_BitmapName     = 4202;
    constexpr int BG_BitmapPitch    = 4203;
    constexpr int BG_BitmapBank     = 4204;
    constexpr int BG_BitmapHeading  = 4205;
    constexpr int BG_BitmapScaleX   = 4206;
    constexpr int BG_BitmapScaleY   = 4207;
    constexpr int BG_BitmapDivX     = 4208;
    constexpr int BG_BitmapDivY     = 4209;
    constexpr int BG_SunName        = 4210;
    constexpr int BG_SunPitch       = 4211;
    constexpr int BG_SunHeading     = 4212;
    constexpr int BG_SunScale       = 4213;
    constexpr int BG_FullNebula     = 4214;
    constexpr int BG_NebulaRange    = 4215;
    constexpr int BG_NebulaPattern  = 4216;
    constexpr int BG_Lightning      = 4217;
    constexpr int BG_Poofs          = 4218;
    constexpr int BG_ShipTrails     = 4219;
    constexpr int BG_Fog1000m       = 4220;
    constexpr int BG_FogNear        = 4221;
    constexpr int BG_FogSkybox      = 4222;
    constexpr int BG_FogClip        = 4223;
    constexpr int BG_DisplayBgInNeb = 4224;
    constexpr int BG_FogOverride    = 4225;
    constexpr int BG_FogR           = 4226;
    constexpr int BG_FogG           = 4227;
    constexpr int BG_FogB           = 4228;
    constexpr int BG_OldNebPattern  = 4229;
    constexpr int BG_OldNebColor    = 4230;
    constexpr int BG_OldNebPitch    = 4231;
    constexpr int BG_OldNebBank     = 4232;
    constexpr int BG_OldNebHeading  = 4233;
    constexpr int BG_AmbientR       = 4234;
    constexpr int BG_AmbientG       = 4235;
    constexpr int BG_AmbientB       = 4236;
    constexpr int BG_SkyboxModel    = 4237;
    constexpr int BG_SkyboxPitch    = 4238;
    constexpr int BG_SkyboxBank     = 4239;
    constexpr int BG_SkyboxHeading  = 4240;
    constexpr int BG_SkyboxFlags    = 4241;
    constexpr int BG_NumStars       = 4242;
    constexpr int BG_Subspace       = 4243;
    constexpr int BG_EnvMap         = 4244;
    constexpr int BG_LightProfile   = 4245;
    // Wing editor             4301–4399
    constexpr int Wing_Name              = 4301;
    constexpr int Wing_DisplayName       = 4302;
    constexpr int Wing_Leader            = 4303;
    constexpr int Wing_NumWaves          = 4304;
    constexpr int Wing_Threshold         = 4305;
    constexpr int Wing_Hotkey            = 4306;
    constexpr int Wing_Formation         = 4307;
    constexpr int Wing_FormationScale    = 4308;
    constexpr int Wing_SquadLogo         = 4309;
    constexpr int Wing_ArrivalType       = 4310;
    constexpr int Wing_ArrivalDelay      = 4311;
    constexpr int Wing_WaveDelays        = 4312;
    constexpr int Wing_ArrivalTarget     = 4313;
    constexpr int Wing_ArrivalDist       = 4314;
    constexpr int Wing_ArrivalPaths      = 4315;
    constexpr int Wing_NoArrivalWarp     = 4316;
    constexpr int Wing_NoArrivalWarpAdj  = 4317;
    constexpr int Wing_DepartureType     = 4318;
    constexpr int Wing_DepartureDelay    = 4319;
    constexpr int Wing_DepartureTarget   = 4320;
    constexpr int Wing_DeparturePaths    = 4321;
    constexpr int Wing_NoDepartureWarp   = 4322;
    constexpr int Wing_NoDepartureWarpAdj = 4323;
    constexpr int Wing_Flags             = 4324;
    constexpr int Wing_InitialOrders     = 4325;
    constexpr int Wing_WarpinParams      = 4326;
    constexpr int Wing_WarpoutParams     = 4327;
    // Ship editor             4401–4499
    constexpr int Ship_DisplayName       = 4402;
    constexpr int Ship_Class             = 4403;
    constexpr int Ship_AIClass           = 4404;
    constexpr int Ship_Team              = 4405;
    constexpr int Ship_Cargo             = 4406;
    constexpr int Ship_CargoTitle        = 4407;
    constexpr int Ship_AltName           = 4408;
    constexpr int Ship_Callsign          = 4409;
    constexpr int Ship_Hotkey            = 4410;
    constexpr int Ship_Persona           = 4411;
    constexpr int Ship_Score             = 4412;
    constexpr int Ship_Assist            = 4413;
    constexpr int Ship_Player            = 4414;
    constexpr int Ship_Respawn           = 4415;
    constexpr int Ship_ArrivalLocation   = 4416;
    constexpr int Ship_ArrivalTarget     = 4417;
    constexpr int Ship_ArrivalDistance   = 4418;
    constexpr int Ship_ArrivalDelay      = 4419;
    constexpr int Ship_NoArrivalWarp     = 4420;
    constexpr int Ship_ArrivalPaths      = 4421;
    constexpr int Ship_DepartureLocation = 4422;
    constexpr int Ship_DepartureTarget   = 4423;
    constexpr int Ship_DepartureDelay    = 4424;
    constexpr int Ship_NoDepartureWarp   = 4425;
    constexpr int Ship_DeparturePaths    = 4426;
    constexpr int Ship_PlayerOrders      = 4427;
    constexpr int Ship_WarpinParams      = 4428;
    constexpr int Ship_WarpoutParams     = 4429;
    constexpr int Ship_Flags             = 4430;
    constexpr int Ship_InitialStatus     = 4431;
    constexpr int Ship_InitialOrders     = 4432;
    constexpr int Ship_Weapons           = 4433;
    constexpr int Ship_AltClass          = 4434;
    constexpr int Ship_SpecialStats      = 4435;
    constexpr int Ship_Reset             = 4436;
    constexpr int Ship_DockWarpin        = 4437;
    constexpr int Ship_DockWarpout       = 4438;

    // Debriefing editor       4501–4899 (in-dialog stack; per-stage ids are
    // offset by team * MAX_DEBRIEF_STAGES + stage so edits to different
    // stages never merge)
    constexpr int Deb_CurrentTeam        = 4501;
    constexpr int Deb_SuccessMusic       = 4502;
    constexpr int Deb_AverageMusic       = 4503;
    constexpr int Deb_FailureMusic       = 4504;
    constexpr int Deb_StageText          = 4600; // 4600–4679
    constexpr int Deb_RecommendationText = 4680; // 4680–4759
    constexpr int Deb_VoiceFile          = 4760; // 4760–4839
    // Mission Goals editor (in-dialog stack): goal count is unbounded, so
    // per-goal ids are goalIndex * Goal_FieldStride + the field constant,
    // starting well above the fixed ranges.
    constexpr int Goal_DisplayFilter = 4900;
    constexpr int Goal_FieldStride = 16;
    constexpr int Goal_Name        = 100000; // + goalIndex * Goal_FieldStride
    constexpr int Goal_Message     = 100001;
    constexpr int Goal_Score       = 100002;
    constexpr int Goal_Team        = 100003;
    constexpr int Goal_Invalid     = 100004;
    constexpr int Goal_NoMusic     = 100005;
    // Mission Cutscenes editor (in-dialog stack): same unbounded-index scheme
    // in its own range.
    constexpr int Cutscene_DisplayFilter = 4901;
    constexpr int Cutscene_FieldStride   = 16;
    constexpr int Cutscene_Filename      = 200000; // + cutsceneIndex * Cutscene_FieldStride
    // Briefing editor         5001–5999 (in-dialog stack; per-stage ids are
    // offset by team * MAX_BRIEF_STAGES + stage)
    constexpr int Brief_CurrentTeam = 5001;
    constexpr int Brief_Music       = 5002;
    constexpr int Brief_SubMusic    = 5003;
    constexpr int Brief_StageText   = 5100; // 5100–5179
    constexpr int Brief_VoiceFile   = 5200; // 5200–5279
    constexpr int Brief_CameraTime  = 5300; // 5300–5379
    constexpr int Brief_CutToNext   = 5400; // 5400–5479
    constexpr int Brief_CutToPrev   = 5500; // 5500–5579
    constexpr int Brief_DisableGrid = 5600; // 5600–5679
    constexpr int Brief_StageCamera = 5700; // 5700–5779
    // Mission Events editor (in-dialog stack): event and message counts are
    // unbounded, so per-item ids are index * FieldStride + the field constant.
    constexpr int Event_FieldStride     = 32;
    constexpr int Event_Name            = 300000; // + eventIndex * Event_FieldStride
    constexpr int Event_RepeatCount     = 300001;
    constexpr int Event_TriggerCount    = 300002;
    constexpr int Event_Interval        = 300003;
    constexpr int Event_Score           = 300004;
    constexpr int Event_Chained         = 300005;
    constexpr int Event_ChainDelay      = 300006;
    constexpr int Event_UseMsecs        = 300007;
    constexpr int Event_Team            = 300008;
    constexpr int Event_DirectiveText   = 300009;
    constexpr int Event_DirectiveKey    = 300010;
    constexpr int Event_LogTrue         = 300011;
    constexpr int Event_LogFalse        = 300012;
    constexpr int Event_LogPrevious     = 300013;
    constexpr int Event_LogAlwaysFalse  = 300014;
    constexpr int Event_LogFirstRepeat  = 300015;
    constexpr int Event_LogLastRepeat   = 300016;
    constexpr int Event_LogFirstTrigger = 300017;
    constexpr int Event_LogLastTrigger  = 300018;
    constexpr int Msg_FieldStride       = 32;
    constexpr int Msg_Name              = 400000; // + messageIndex * Msg_FieldStride
    constexpr int Msg_Text              = 400001;
    constexpr int Msg_Note              = 400002;
    constexpr int Msg_Ani               = 400003;
    constexpr int Msg_Persona           = 400004;
    constexpr int Msg_Team              = 400005;
    // Wave-file edits are message-scope snapshots (they can auto-select the
    // persona and head ani as a side effect); merge id is base + messageIndex.
    constexpr int Msg_SnapWave          = 500000;
    // Campaign editor (in-dialog stack; file-scoped, no apply step)
    constexpr int Camp_Name         = 6001;
    constexpr int Camp_Description  = 6002;
    constexpr int Camp_Type         = 6003;
    constexpr int Camp_TechReset    = 6004;
    constexpr int Camp_CustomData   = 6005;
    constexpr int Camp_RetailFormat = 6006;
    // Command Briefing editor 6101–6499 (in-dialog stack; per-stage ids are
    // offset by team * CMD_BRIEF_STAGES_MAX + stage; backgrounds are per team)
    constexpr int CmdBrief_CurrentTeam = 6101;
    constexpr int CmdBrief_LowResBg    = 6110; // + team
    constexpr int CmdBrief_HighResBg   = 6120; // + team
    constexpr int CmdBrief_StageText   = 6200; // 6200–6219
    constexpr int CmdBrief_AniFile     = 6300; // 6300–6319
    constexpr int CmdBrief_WaveFile    = 6400; // 6400–6419
    constexpr int Camp_ShipAllowed   = 700000; // + shipClassIndex
    constexpr int Camp_WeaponAllowed = 710000; // + weaponClassIndex
    // Per-mission fields: base + missionIndex * Camp_MissionFieldStride
    constexpr int Camp_MissionFieldStride = 8;
    constexpr int Camp_MissionCutscene    = 720000;
    constexpr int Camp_MissionMainhall    = 720001;
    constexpr int Camp_MissionSubMainhall = 720002;
    constexpr int Camp_MissionPersona     = 720003;
    constexpr int Camp_MissionNodePos     = 720004;
    // Per-branch loop fields: base + (missionIndex * Camp_BranchesPerMission
    // + branchIndex) * Camp_BranchFieldStride
    constexpr int Camp_BranchesPerMission = 100;
    constexpr int Camp_BranchFieldStride  = 4;
    constexpr int Camp_LoopDescription    = 800000;
    constexpr int Camp_LoopAnim           = 800001;
    constexpr int Camp_LoopVoice          = 800002;
    // Variable editor (in-dialog stack): index-strided per-variable and
    // per-container ranges. The persistence/network/eternal commands track
    // the whole flags word since the bits interact (persistence None clears
    // Eternal); distinct field constants keep different controls from
    // merging with each other.
    constexpr int Var_FieldStride  = 8;
    constexpr int Var_Name         = 850000; // + variableIndex * Var_FieldStride
    constexpr int Var_Value        = 850001;
    constexpr int Var_Persistence  = 850002;
    constexpr int Var_Network      = 850003;
    constexpr int Var_Eternal      = 850004;
    constexpr int Cont_FieldStride = 8;
    constexpr int Cont_Name        = 860000; // + containerIndex * Cont_FieldStride
    constexpr int Cont_KeyType     = 860001;
    constexpr int Cont_Persistence = 860002;
    constexpr int Cont_Network     = 860003;
    constexpr int Cont_Eternal     = 860004;
    // Container item cells: base + containerIndex * Item_ContainerStride
    // + itemIndex (list values and map values; map keys are snapshots
    // because editing one re-sorts the map)
    constexpr int Item_ContainerStride = 2048;
    constexpr int Item_ListValue       = 900000;  // containers 0–97
    constexpr int Item_MapValue        = 1100000; // containers 0–97
    // Briefing icon-edit snapshot merge ids (DialogSnapshotCommand): base +
    // (team * MAX_BRIEF_STAGES + stage) + firstSelectedIcon * 80, so edits to
    // a different stage or selection never merge.
    constexpr int Brief_SnapIconLabel   = 600000;
    constexpr int Brief_SnapIconCloseup = 610000;
    constexpr int Brief_SnapIconId      = 620000;
    constexpr int Brief_SnapIconScale   = 630000;
    constexpr int Brief_SnapIconImage   = 640000;
    constexpr int Brief_SnapIconShip    = 650000;
    constexpr int Brief_SnapIconTeam    = 660000;
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
//
// editor may be null for dialog-internal commands (per-dialog undo stacks):
// the setters then write the dialog's working copy instead of mission data,
// no missionChanged() is emitted, and each setter is responsible for
// refreshing the dialog UI.
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
    bool              _allowMerge = true;

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

    // Call for subdialog-triggered commands: each open/close is a discrete
    // action and should never be collapsed with a subsequent invocation.
    void setNoMerge() { _allowMerge = false; }

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
        if (_editor) _editor->missionChanged();
    }

    void redo() override {
        if (_skipFirstRedo) { _skipFirstRedo = false; return; }
        for (const auto& e : _entries) e.setter(e.after);
        if (_editor) _editor->missionChanged();
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

    int id() const override { return _allowMerge ? _fieldId : -1; }
};

// ---------------------------------------------------------------------------
// DialogSnapshotCommand — before/after blob undo for dialog-internal state
//
// Used by the per-dialog undo stacks for edits where per-field commands
// don't fit — chiefly sexp tree mutations, where a single modified() signal
// can mean any structural change, and icon edits that propagate across
// stages. The dialog serializes the relevant slice of its working state
// (its choice of format) and provides a restore callback that applies a
// blob and refreshes the UI. skipFirstRedo follows the usual pattern: the
// edit has already been applied when the command is pushed.
//
// mergeId: -1 (default) never merges. A non-negative id merges consecutive
// pushes with the same id — first 'before' and latest 'after' are kept —
// collapsing continuous edits (typing in an icon label, spinner drags) into
// one undo step, mirroring FieldEditCommand's fieldId merging. Ids share
// the FieldId number space and must not collide with any fieldId.
// ---------------------------------------------------------------------------

class DialogSnapshotCommand : public QUndoCommand {
	QByteArray                             _before;
	QByteArray                             _after;
	std::function<void(const QByteArray&)> _restore;
	bool                                   _skipFirstRedo;
	int                                    _mergeId;

public:
	DialogSnapshotCommand(QByteArray                             before,
	                      QByteArray                             after,
	                      std::function<void(const QByteArray&)> restore,
	                      const QString&                         text,
	                      bool                                   skipFirstRedo = true,
	                      int                                    mergeId       = -1,
	                      QUndoCommand*                          parent        = nullptr);
	void undo() override;
	void redo() override;
	int id() const override { return _mergeId; }
	bool mergeWith(const QUndoCommand* other) override;
};

// ---------------------------------------------------------------------------
// SexpCueEditCommand — undo/redo one arrival/departure cue tree edit
//
// Pushed by the ship/wing editors after a sexp_tree_view mutation has been
// applied to mission data (skipFirstRedo pattern). The command owns dup'd
// copies of the before/after SEXP chains; every undo/redo hands the owners a
// fresh dup so the masters stay valid across repeated cycles.
//
// Owners are located inside the get/set lambdas (by ship signature or wing
// name, matching FieldEditCommand conventions). setCue must only assign the
// formula — the command frees each owner's current cue exactly once first,
// which matters because a multi-edit assigns one shared formula index to
// every marked ship.
// ---------------------------------------------------------------------------

class SexpCueEditCommand : public QUndoCommand {
public:
	SexpCueEditCommand(Editor*        editor,
	                   const QString& text,
	                   bool           skipFirstRedo = false,
	                   QUndoCommand*  parent        = nullptr);
	~SexpCueEditCommand() override;

	// Call BEFORE the edit is applied to mission data: dups beforeCue immediately.
	// getCue reads the owner's current cue slot; setCue assigns a formula to it.
	void addOwner(int beforeCue, std::function<int()> getCue, std::function<void(int)> setCue);

	// Call AFTER the edit is applied, with the new live formula: dups it.
	void captureAfter(int afterCue);

	bool isEmpty() const { return _owners.empty(); }

	void undo() override;
	void redo() override;

private:
	struct Owner {
		int                      beforeDup; // SHIP_CUE_* sentinel or command-owned dup
		std::function<int()>     getCue;
		std::function<void(int)> setCue;
	};

	// Free each owner's current live cue exactly once (owners may share one
	// formula index after a multi-edit; freeing per-owner would corrupt nodes
	// reused by the dups allocated in between).
	void freeCurrentCues();

	SCP_vector<Owner> _owners;
	int               _afterDup;
	Editor*           _editor;
	bool              _skipFirstRedo;
};

// ---------------------------------------------------------------------------
// TextureReplacementCommand — undo/redo for the Ship Texture Replacement dialog
// ---------------------------------------------------------------------------

class TextureReplacementCommand : public QUndoCommand {
	SCP_string                    _shipName;
	SCP_vector<texture_replace>   _before;
	SCP_vector<texture_replace>   _after;
	Editor*                       _editor;

public:
	// Construct with the before-state already captured; call setAfter() once apply() succeeds.
	TextureReplacementCommand(SCP_string               shipName,
	                          SCP_vector<texture_replace> before,
	                          Editor*                  editor,
	                          QUndoCommand*            parent = nullptr);
	void setAfter(SCP_vector<texture_replace> after);
	void undo() override;
	void redo() override;

private:
	void apply(const SCP_vector<texture_replace>& entries);
};

} // namespace fso::fred
