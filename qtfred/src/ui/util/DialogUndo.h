#pragma once

#include <QString>
#include <QUndoGroup>
#include <QUndoStack>
#include <QWidget>

namespace fso::fred::util {

// Sets up a modal dialog's internal undo UX in one call:
//  - an Edit -> Undo/Redo menu bound to the dialog's own stack. QDialogs get
//    a slim menu bar created above their content via layout()->setMenuBar();
//    QMainWindow-based editors get the menu added to their existing bar. The
//    actions carry real window-scoped Ctrl+Z / Ctrl+Y shortcuts: every
//    editing window owns its shortcuts, so key delivery never depends on
//    reaching another window's actions.
//  - a window-activation watcher that makes this stack the undo group's
//    active stack whenever the dialog window is active (drives the main
//    window's Edit menu and the status-bar scope indicator; FredView points
//    the group back at the main stack when the main window reactivates)
//  - the stack's depth cap, and its scope name, which the main window's
//    status bar shows as the current undo target
//
// The dialog must already have a top-level layout.
void setupDialogUndo(QWidget* dialog, QUndoGroup* group, QUndoStack* stack, const QString& scopeName, int undoLimit = 100);

// For editor windows whose edits push to the MAIN stack (direct-edit and
// batch dialogs: ship, wing, waypoint, ...): installs invisible
// window-scoped Ctrl+Z / Ctrl+Y actions bound to that stack, replacing the
// application-wide shortcut reach these windows previously relied on.
void installMainStackUndoShortcuts(QWidget* window, QUndoStack* mainStack);

} // namespace fso::fred::util
