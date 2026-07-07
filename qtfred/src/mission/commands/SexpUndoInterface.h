#pragma once

#include <memory>

class QUndoCommand;

namespace fso::fred {

// ---------------------------------------------------------------------------
// ISexpUndoSource — extension point for sexp-tree edits to participate in the
// undo stack.
//
// Dialogs hosting sexp trees implement this interface. Shared tree code (or
// the dialog's own tree slots) calls pushSexpEdit() when the user mutates the
// tree, and the dialog routes the command to whichever QUndoStack is
// appropriate:
//   - its own _dialogStack (MissionEventsDialog, BriefingEditorDialog — Phase 9)
//   - the main stack via _fredView->mainUndoStack() (ShipEditorDialog and
//     WingEditorDialog arrival/departure cues, and other direct-edit dialogs)
//
// Ship/wing cue trees push SexpCueEditCommand (FredCommands.h) directly from
// their sexp_tree_view::modified slots and do not need this indirection; the
// interface exists for tree-hosting dialogs where the mutation site (shared
// tree widget code) cannot know the destination stack.
// ---------------------------------------------------------------------------

class ISexpUndoSource {
public:
	virtual ~ISexpUndoSource() = default;
	virtual void pushSexpEdit(std::unique_ptr<QUndoCommand> cmd) = 0;
};

} // namespace fso::fred
