#pragma once

#include <ai/aigoals.h>
#include <globalincs/globals.h>
#include <globalincs/pstypes.h>
#include <ship/anchor_t.h>

#include <array>

namespace fso::fred {

class EditorViewport;

enum class ErrorSeverity { InternalError, Error, Warning, Potential };

// Display metadata for one severity level.
// Colors are stored as plain RGB components so this header stays Qt-free;
// callers that need a QColor construct one from (r, g, b) at the use site.
struct SeverityInfo {
	uint8_t     r, g, b;
	const char* label;
	const char* tooltip;
};

// One entry per ErrorSeverity value, index-matched to the enum.
inline constexpr std::array<SeverityInfo, 4> severity_info = {{
	{ 0xCC, 0x33, 0x33, "Critical Error",
	  "A serious structural problem FRED cannot automatically correct. "
	  "The mission may fail to load or behave unpredictably in-game." },
	{ 0xE0, 0x78, 0x30, "Error",
	  "A mission design problem that must be manually fixed. The mission may not play correctly otherwise." },
	{ 0xD4, 0xA0, 0x00, "Warning",
	  "A problem that can be (or has been) auto-corrected. Review the change before saving." },
	{ 0x40, 0x80, 0xCC, "Potential Issue",
	  "A situation that may be intentional but is worth reviewing." },
}};

inline const SeverityInfo& infoFor(ErrorSeverity sev) {
	switch (sev) {
	case ErrorSeverity::InternalError: return severity_info[0];
	case ErrorSeverity::Error:         return severity_info[1];
	case ErrorSeverity::Warning:       return severity_info[2];
	case ErrorSeverity::Potential:     return severity_info[3];
	}
	UNREACHABLE("Unhandled ErrorSeverity value %d", static_cast<int>(sev));
	return severity_info[1];
}

struct ErrorEntry {
	SCP_string message;
	ErrorSeverity severity;
};

enum class ErrorCheckType {
	InitialOrders,  // standalone check (used by ShipGoalsDialogModel)
	ObjectList,     // object integrity + name collection
	Ships,          // ship SEXPs, anchors, AI goals, docking, loadout weapons
	Wings,          // wing structure, SEXPs, anchors, AI goals, thresholds
	WaypointPaths,  // waypoint path name conflicts with objects
	PlayerStarts,   // player start count validity
	Reinforcements, // reinforcement name references
	PlayerWings,    // player wing membership and wave constraints
	MissionEvents,  // mission event SEXP validation
	MissionGoals,   // mission goal SEXP validation
	Briefings,      // briefing icon ID duplicates
	Debriefings,    // debriefing SEXP validation
	WingOrders,          // wing reinforcement flags and accepted orders consistency
	AsteroidTargets,     // asteroid field target ship name validity
	DockingGroupCues,    // initially-docked groups must have exactly one non-false arrival cue
	TeamLoadout,         // weapons used in starting wings but absent from the team loadout pool
};

struct ErrorCheckContext {
	ai_goal* goals = nullptr;
	int ship = -1;
	int wing = -1;
};

class ErrorChecker {
public:
	explicit ErrorChecker(EditorViewport* viewport);

	// Run all checks in collect mode; returns true if errors were found
	bool runFullCheck();

	// Run a specific check in collect mode; returns true if errors were found.
	// Errors are available via getErrors() after the call.
	bool runCheck(ErrorCheckType type, const ErrorCheckContext& ctx = {});

	const SCP_vector<ErrorEntry>& getErrors() const;

private:
	EditorViewport* _viewport;

	// Per-object identity entries built up during ObjectList / Wings / Waypoints checks.
	// Empty name means an object type with no identifier (briefing icons, jump nodes, props).
	struct ObjectName {
		SCP_string name;
	};
	SCP_vector<ObjectName> _object_names;
	// Accumulates whether any error() or warning() has fired during a check run.
	// Those helpers are void (no return value to propagate), so g_err is the only
	// way runFullCheck/runCheck can report non-critical issues that don't cause an
	// early abort. internal_error() sets it too, but also returns -1 for callers
	// that need to halt immediately.
	int g_err = 0;
	SCP_vector<ErrorEntry> _collected_errors;
	SCP_set<anchor_t> _anchors_checked;

	// error() records a user-fixable problem and continues; return type is void so
	// callers cannot short-circuit on it (use internal_error for abort-worthy issues).
	void error(SCP_FORMAT_STRING const char* msg, ...) SCP_FORMAT_STRING_ARGS(2, 3);
	// internal_error() records a data-integrity problem and returns -1 so callers
	// can propagate an early abort when continuing would be unsafe.
	int internal_error(SCP_FORMAT_STRING const char* msg, ...) SCP_FORMAT_STRING_ARGS(2, 3);
	void warning(SCP_FORMAT_STRING const char* msg, ...) SCP_FORMAT_STRING_ARGS(2, 3);
	void potential(SCP_FORMAT_STRING const char* msg, ...) SCP_FORMAT_STRING_ARGS(2, 3);
	int fred_check_sexp(int sexp, int type, const char* location, ...);

	// Populates _object_names from the object list. Safe to call multiple times.
	// Called internally by checkWings() and checkWaypointPaths() — no external call needed.
	void populateNames();

	// Individual check methods (called by runFullCheck in order).
	// Return 0 on success or when only user-fixable errors were found.
	// Return -1 on internal errors severe enough to warrant aborting further checks.
	int checkObjectList();
	int checkShips();
	int checkWings();
	int checkWaypointPaths();
	int checkPlayerStarts();
	int checkReinforcements();
	int checkPlayerWings();
	int checkMissionEvents();
	int checkMissionGoals();
	int checkBriefings();
	int checkDebriefings();
	int checkWingOrders();
	int checkAsteroidTargets();
	int checkDockingGroupCues();
	int checkTeamLoadout();

	// Helper methods
	int checkInitialOrders(ai_goal* goals, int ship, int wing);
};

} // namespace fso::fred
