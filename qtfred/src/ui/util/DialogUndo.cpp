#include "DialogUndo.h"

#include <globalincs/pstypes.h>

#include <QAction>
#include <QEvent>
#include <QKeySequence>
#include <QLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

namespace {

// Makes the dialog's stack the group's active stack whenever the dialog
// window is activated. Window activation is the reliable signal here —
// QDialog::focusInEvent does not fire when a child widget takes the focus,
// which is nearly always.
class ActiveStackWatcher : public QObject {
public:
	ActiveStackWatcher(QWidget* watched, QUndoGroup* group, QUndoStack* stack)
		: QObject(watched), _group(group), _stack(stack)
	{
	}

protected:
	bool eventFilter(QObject* obj, QEvent* e) override
	{
		if (e->type() == QEvent::WindowActivate)
			_group->setActiveStack(_stack);
		return QObject::eventFilter(obj, e);
	}

private:
	QUndoGroup* _group;
	QUndoStack* _stack;
};

} // anonymous namespace

namespace fso::fred::util {

void setupDialogUndo(QWidget* dialog, QUndoGroup* group, QUndoStack* stack, const QString& scopeName, int undoLimit)
{
	stack->setUndoLimit(undoLimit);
	stack->setObjectName(scopeName); // shown by the main window's undo status indicator

	auto* undoAction = stack->createUndoAction(dialog, QObject::tr("&Undo"));
	auto* redoAction = stack->createRedoAction(dialog, QObject::tr("&Redo"));

	// Real window-scoped shortcuts bound to this dialog's own stack: they can
	// never mis-route, and they work regardless of window modality. FredView's
	// undo/redo shortcuts are window-scoped to the main window, so there is no
	// ambiguity. (Ctrl+Shift+Z stays reserved for camera undo in the main
	// window; dialogs keep redo on Ctrl+Y for consistent muscle memory.)
	undoAction->setShortcuts(QKeySequence::Undo);
	undoAction->setShortcutContext(Qt::WindowShortcut);
	redoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
	redoAction->setShortcutContext(Qt::WindowShortcut);

	// QMainWindow-based editors (Campaign) already have a real menu bar; add
	// Edit to it. Plain QDialogs get a slim bar created above their layout.
	QMenuBar* bar = nullptr;
	if (auto* mainWindow = qobject_cast<QMainWindow*>(dialog)) {
		bar = mainWindow->menuBar();
	} else {
		Assertion(dialog->layout() != nullptr, "setupDialogUndo requires the dialog to have a top-level layout");
		bar = new QMenuBar(dialog);
		dialog->layout()->setMenuBar(bar);
	}
	auto* editMenu = bar->addMenu(QObject::tr("&Edit"));
	editMenu->addAction(undoAction);
	editMenu->addAction(redoAction);

	// The actions must be on the dialog widget itself for the window-scoped
	// shortcuts to be considered while any child of the dialog has focus.
	dialog->addAction(undoAction);
	dialog->addAction(redoAction);

	dialog->installEventFilter(new ActiveStackWatcher(dialog, group, stack));
}

void installMainStackUndoShortcuts(QWidget* window, QUndoStack* mainStack)
{
	auto* undoAction = mainStack->createUndoAction(window, QObject::tr("&Undo"));
	undoAction->setShortcuts(QKeySequence::Undo);
	undoAction->setShortcutContext(Qt::WindowShortcut);
	window->addAction(undoAction);

	auto* redoAction = mainStack->createRedoAction(window, QObject::tr("&Redo"));
	redoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
	redoAction->setShortcutContext(Qt::WindowShortcut);
	window->addAction(redoAction);
}

} // namespace fso::fred::util
