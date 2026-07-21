#if defined(_MSC_VER) && _MSC_VER <= 1920
	// work around MSVC 2015 and 2017 compiler bug
	// https://bugreports.qt.io/browse/QTBUG-72073
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "renderwidget.h"

#include <array>

#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>
#include <QtGui/QtGui>
#include <QtWidgets/QHBoxLayout>
#include <mission/object.h>
#include <object/object.h>
#include <ship/ship.h>
#include <globalincs/linklist.h>
#include <QtWidgets/QMessageBox>

#include "osapi/osapi.h"
#include "ui/ControlBindings.h"
#include "io/timer.h"
#include "starfield/starfield.h"

#include "mission/Editor.h"
#include "mission/commands/FredCommands.h"
#include "FredApplication.h"
#include "ui/FredView.h"

namespace fso::fred {

RenderWindow::RenderWindow(RenderWidget* renderWidget) : QWindow((QWindow*) nullptr), _renderWidget(renderWidget) {
	setSurfaceType(QWindow::OpenGLSurface);
}

void RenderWindow::initializeGL(const QSurfaceFormat& surfaceFmt) {
	setFormat(surfaceFmt);

	// Force creation of this window so that we can use it for OpenGL
	create();
}

void RenderWindow::startRendering() {
	_renderer->resize(size().width(), size().height());
}

void RenderWindow::paintGL() {
	if (!_renderer || !fredApp->isInitializeComplete()) {
		return;
	}

	_renderWidget->renderFrame();
}

bool RenderWindow::event(QEvent* evt) {
	switch (evt->type()) {
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonPress:
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::ShortcutOverride:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
	case QEvent::Wheel: {
		// Redirect all the events to the render widget since we want to handle them in in the QtWidget related code
		qGuiApp->sendEvent(_renderWidget, evt);
		return true;
	}
	case QEvent::UpdateRequest:
		paintGL();
		evt->accept();
		return true;
	default:
		return QWindow::event(evt);
	}
}
void RenderWindow::resizeEvent(QResizeEvent* event) {
	if (_renderer) {
		// Only send resize event if we are actually rendering
		_renderer->resize(event->size().width(), event->size().height());
	}
}
RenderWindow::~RenderWindow() = default;
void RenderWindow::exposeEvent(QExposeEvent* event) {
	if (isExposed()) {
		requestUpdate();

		event->accept();
	} else {
		QWindow::exposeEvent(event);
	}
}
void RenderWindow::setEditor(Editor* editor, FredRenderer* renderer) {
	Assertion(fred == nullptr, "Render widget currently does not support resetting the editor!");
	Assertion(_renderer == nullptr, "Render widget currently does not support resetting the renderer!");

	Assertion(editor != nullptr, "Invalid editor pointer passed!");
	Assertion(renderer != nullptr, "Invalid renderer pointer passed!");

	fred = editor;
	_renderer = renderer;

	// When the editor want to update the main window we have to do that.
	connect(_renderer, &FredRenderer::scheduleUpdate, this, &QWindow::requestUpdate);
}
RenderWidget::RenderWidget(QWidget* parent) : QWidget(parent) {
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	_window = new RenderWindow(this);

	auto layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->addWidget(QWidget::createWindowContainer(_window, this));
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);


	_standardCursor.reset(new QCursor(Qt::ArrowCursor));
	_moveCursor.reset(new QCursor(Qt::SizeAllCursor));

	QPixmap rotatePixmap(":/images/cursor_rotate.png");
	_rotateCursor.reset(new QCursor(rotatePixmap, 15, 16)); // These values are from the original cursor file

	_window->setCursor(*_standardCursor);

	setContextMenuPolicy(Qt::DefaultContextMenu);

	fredApp->runAfterInit([this]() { _window->startRendering(); });
}
QSurface* RenderWidget::getRenderSurface() const {
	return _window;
}
void RenderWidget::setSurfaceFormat(const QSurfaceFormat& fmt) {
	_window->initializeGL(fmt);
}
void RenderWidget::contextMenuEvent(QContextMenuEvent* event) {
	// Suppress context menu if right-button was used for orbit dragging
	if (_rbuttonMoved) {
		event->accept();
		return;
	}

	event->accept();

	auto parentView = static_cast<FredView*>(parentWidget());

	Q_ASSERT(parentView);

	parentView->showContextMenu(event->globalPos());
}

void RenderWidget::focusInEvent(QFocusEvent* e) {
	if (_fredView) {
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
	}
	QWidget::focusInEvent(e);
}

void RenderWidget::keyPressEvent(QKeyEvent* key) {
	if (_viewport != nullptr && key->key() == Qt::Key_Escape && _viewport->button_down) {
		_viewport->cancel_drag();
		_usingMarkingBox = false;
		key->accept();
		return;
	}

	if (!ControlBindings::instance().handleKeyPress(key)) {
		QWidget::keyPressEvent(key);
		return;
	}
	key->accept();
}

void RenderWidget::keyReleaseEvent(QKeyEvent* key) {
	if (!ControlBindings::instance().handleKeyRelease(key)) {
		QWidget::keyReleaseEvent(key);
		return;
	}
	key->accept();
}
bool RenderWidget::event(QEvent* evt) {
	if (evt->type() == QEvent::ShortcutOverride) {
		auto* keyEvent = static_cast<QKeyEvent*>(evt);
		if (ControlBindings::instance().matches(keyEvent)) {
			evt->accept();
			return true;
		}
	}

	return QWidget::event(evt);
}
void RenderWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	event->ignore();
}
void RenderWidget::mousePressEvent(QMouseEvent* event) {
	// Orbit camera: middle button
	if (event->button() == Qt::MiddleButton) {
		if (_viewport != nullptr && _viewport->camera.getViewpoint() == 0 && _viewport->camera.getControlMode() == 0) {
			vec3d pivot = _viewport->orbitCameraGetPivot();
			auto grid_orient = _viewport->The_grid ? &_viewport->The_grid->gmatrix : nullptr;
			_viewport->camera.orbitCameraInitFromCurrentView(&pivot, grid_orient);
			_orbitDragging = true;
			_orbitLastMouse = event->pos();
		}
		return;
	}

	// Orbit camera: right button
	if (event->button() == Qt::RightButton) {
		if (_viewport != nullptr && _viewport->button_down) {
			_viewport->cancel_drag();
			_usingMarkingBox = false;
			event->accept();
			return;
		}

		_rbuttonDown = true;
		_rbuttonMoved = false;
		_rbuttonDownPoint = event->pos();
		_orbitLastMouse = event->pos();

		if (_viewport != nullptr && _viewport->camera.getViewpoint() == 0 && _viewport->camera.getControlMode() == 0) {
			vec3d pivot = _viewport->orbitCameraGetPivot();
			auto grid_orient = _viewport->The_grid ? &_viewport->The_grid->gmatrix : nullptr;
			_viewport->camera.orbitCameraInitFromCurrentView(&pivot, grid_orient);
		}
		return;
	}

	if (event->button() != Qt::LeftButton) {
		// Ignore everything that has nothing to do with the left button
		return QWidget::mousePressEvent(event);
	}

	int waypoint_instance = -1;
	if (fred->cur_waypoint != nullptr) {
		Assertion(fred->cur_waypoint_list != nullptr, "cur_waypoint is set but cur_waypoint_list is null!");
		waypoint_instance = Objects[fred->cur_waypoint->get_objnum()].instance;
	}

	_markingBox.x1 = event->position().x() * _window->devicePixelRatio();
	_markingBox.y1 = event->position().y() * _window->devicePixelRatio();
	_viewport->Dup_drag = 0;
	_wasDupDrag = false;
	_wasInsertDrag = false;
	_preCloneSourceSignatures.clear();

	_viewport->on_object = _viewport->select_object(event->position().x() * _window->devicePixelRatio(),
		event->position().y() * _window->devicePixelRatio());
	_viewport->button_down = 1;

	if (event->modifiers().testFlag(Qt::ControlModifier)) {  // add a new object
		if (_viewport->on_object == -1) {
			_viewport->Selection_lock = false;  // force off selection lock
			const bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
			const bool alt   = event->modifiers().testFlag(Qt::AltModifier);
			auto kind = CreateKind::Ship;
			if (alt && !shift) {
				kind = CreateKind::Other;
			} else if (shift && !alt) {
				kind = CreateKind::Prop;
			}
			_viewport->on_object = _viewport->create_object_on_grid(event->position().x() * _window->devicePixelRatio(), event->position().y() * _window->devicePixelRatio(), waypoint_instance, kind);
			if (_viewport->on_object >= 0) {
				auto* cmd = new fso::fred::CreateObjectCommand(
				    Objects[_viewport->on_object].pos,
				    _viewport->cur_model_index,
				    _viewport->cur_prop_index,
				    waypoint_instance,
				    kind,
				    _viewport->cur_other_kind,
				    _viewport->on_object,
				    fred,
				    _viewport);
				_fredView->mainUndoStack()->push(cmd); // first redo() is a no-op
			}
		} else {
			// Ctrl+drag: duplicate marked objects (waypoints start a new path).
			// Ctrl+Shift+drag: same, except marked waypoints insert into their
			// source path instead of starting a new one.
			_wasInsertDrag = event->modifiers().testFlag(Qt::ShiftModifier);
			_viewport->Dup_drag = _wasInsertDrag ? EditorViewport::DUP_DRAG_INSERT : 1;

			// Capture pre-clone state so mouseReleaseEvent can build CloneDragCommand.
			_wasDupDrag         = true;
			_preCloneCurrentObj = fred->currentObject;
			_preCloneSourceSignatures.clear();
			for (const object* p = GET_FIRST(&obj_used_list);
			     p != END_OF_LIST(&obj_used_list);
			     p = GET_NEXT(p))
			{
				if (p->flags[Object::Object_Flags::Marked]) {
					_preCloneSourceSignatures.push_back(p->signature);
				}
			}
		}

	} else if (!_viewport->Selection_lock) {
		if ((event->modifiers().testFlag(Qt::ShiftModifier)) || (_viewport->on_object == -1)
			|| !(Objects[_viewport->on_object].flags[Object::Object_Flags::Marked])) {
			if (!event->modifiers().testFlag(Qt::ShiftModifier)) {
				fred->unmark_all();
			}

			if (_viewport->on_object != -1) {
				if (Objects[_viewport->on_object].flags[Object::Object_Flags::Marked]) {
					fred->unmarkObject(_viewport->on_object);
				} else {
					fred->markObject(_viewport->on_object);
				}
			}
		}
	}

	// Save drag/rotate backup after selection state is finalized for this click.
	_viewport->drag_rotate_save_backup();

	if (query_valid_object(fred->currentObject)) {
		_viewport->original_pos = Objects[fred->currentObject].pos;
	}

	_viewport->moved = 0;
	if (_viewport->Selection_lock) {
		if (_viewport->Editing_mode == CursorMode::Moving) {
			_viewport->drag_objects(event->position().x() * _window->devicePixelRatio(),
				event->position().y() * _window->devicePixelRatio());
		} else if (_viewport->Editing_mode == CursorMode::Rotating) {
			_viewport->drag_rotate_objects(0, 0);
		}

		_viewport->needsUpdate();
	}
}
void RenderWidget::handleOrbitDrag(QPoint point, Qt::KeyboardModifiers modifiers)
{
	if (_viewport == nullptr || _viewport->camera.getViewpoint() != 0 || _viewport->camera.getControlMode() != 0)
		return;

	int dx = point.x() - _orbitLastMouse.x();
	int dy = point.y() - _orbitLastMouse.y();
	_orbitLastMouse = point;

	if (modifiers & Qt::ShiftModifier)
		_viewport->camera.orbitCameraPan(dx, dy);
	else
		_viewport->camera.orbitCameraRotate(dx, dy);
	_viewport->needsUpdate();
}

void RenderWidget::wheelEvent(QWheelEvent* event)
{
	if (_viewport == nullptr || _viewport->camera.getViewpoint() != 0 || _viewport->camera.getControlMode() != 0) {
		QWidget::wheelEvent(event);
		return;
	}

	if (!_viewport->camera.isOrbitActive()) {
		vec3d pivot = _viewport->orbitCameraGetPivot();
		auto grid_orient = _viewport->The_grid ? &_viewport->The_grid->gmatrix : nullptr;
		_viewport->camera.orbitCameraInitFromCurrentView(&pivot, grid_orient);
	}

	_viewport->camera.orbitCameraZoom(event->angleDelta().y() / -200.0f);
	_viewport->needsUpdate();
	event->accept();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
	auto mouseDX = event->pos() - _lastMouse;
	_lastMouse = event->pos();

	// Orbit camera: middle button drag
	if (_orbitDragging && event->buttons().testFlag(Qt::MiddleButton)) {
		handleOrbitDrag(event->pos(), event->modifiers());
		return;
	}

	// Orbit camera: right button drag
	if (_rbuttonDown && event->buttons().testFlag(Qt::RightButton) && _viewport != nullptr
		&& _viewport->camera.getViewpoint() == 0 && _viewport->camera.getControlMode() == 0) {
		if (!_rbuttonMoved) {
			if (abs(event->pos().x() - _rbuttonDownPoint.x()) > 2
				|| abs(event->pos().y() - _rbuttonDownPoint.y()) > 2)
				_rbuttonMoved = true;
		}
		if (_rbuttonMoved) {
			handleOrbitDrag(event->pos(), event->modifiers());
			return;
		}
	}

// Update marking box
	_markingBox.x2 = event->position().x() * _window->devicePixelRatio();
	_markingBox.y2 = event->position().y() * _window->devicePixelRatio();

	// RT point

	_viewport->Cursor_over = _viewport->select_object(event->position().x() * _window->devicePixelRatio(),
		event->position().y() * _window->devicePixelRatio());
	updateCursor();

	if (!event->buttons().testFlag(Qt::LeftButton)) {
		_viewport->button_down = false;
	}

	if (_viewport->button_down) {
		if (abs(_markingBox.x1 - _markingBox.x2) > 1 || abs(_markingBox.y1 - _markingBox.y2) > 1) {
			_viewport->moved = true;
		}

		if (_viewport->moved) {
			if (_viewport->on_object != -1 || _viewport->Selection_lock) {
				if (_viewport->Editing_mode == CursorMode::Moving) {
					_viewport->drag_objects(event->position().x() * _window->devicePixelRatio(),
						event->position().y() * _window->devicePixelRatio());
				} else if (_viewport->Editing_mode == CursorMode::Rotating) {
					_viewport->drag_rotate_objects(mouseDX.x(), mouseDX.y());
				}

			} else {
				_usingMarkingBox = true;
			}

			if (mouseDX.x() || mouseDX.y()) {
				_viewport->needsUpdate();
			}
		}
	}
}
void RenderWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::MiddleButton) {
		_orbitDragging = false;
		return;
	}

	if (event->button() == Qt::RightButton) {
		bool wasDragging = _rbuttonMoved;
		_rbuttonDown = false;
		_rbuttonMoved = false;

		if (!wasDragging) {
			// No drag occurred — show context menu
			auto parentView = static_cast<FredView*>(parentWidget());
			Q_ASSERT(parentView);
			parentView->showContextMenu(event->globalPosition().toPoint());
		}
		return;
	}

	if (event->button() != Qt::LeftButton) {
		// Ignore everything that has nothing to do with the left button
		return QWidget::mouseReleaseEvent(event);
	}

	_markingBox.x2 = event->position().x() * _window->devicePixelRatio();
	_markingBox.y2 = event->position().y() * _window->devicePixelRatio();

	if (_viewport->button_down) {
		if (abs(_markingBox.x1 - _markingBox.x2) > 1 || abs(_markingBox.y1 - _markingBox.y2) > 1) {
			_viewport->moved = true;
		}

		// True if this gesture was an object drag (not a selection-box sweep).
		const bool wasDragMode = (_viewport->on_object != -1) || _viewport->Selection_lock;
		const bool isMoveOrRotate = _viewport->Editing_mode == CursorMode::Moving
		                         || _viewport->Editing_mode == CursorMode::Rotating;

		if (_viewport->moved) {
			if (wasDragMode) {
				if (_viewport->Editing_mode == CursorMode::Moving) {
					_viewport->drag_objects(event->position().x() * _window->devicePixelRatio(),
						event->position().y() * _window->devicePixelRatio());
				} else if (_viewport->Editing_mode == CursorMode::Rotating) {
					_viewport->drag_rotate_objects(0, 0);
				}
			} else {
				_usingMarkingBox = true;
			}
		}

		// Dup_drag is reset to 0 (or DUP_DRAG_OF_WING) inside drag_objects() once the
		// clone succeeds. If it's still 1 / DUP_DRAG_INSERT here, no clone happened.
		const bool cloneSucceeded = _wasDupDrag
		    && _viewport->moved
		    && _viewport->Dup_drag != 1
		    && _viewport->Dup_drag != EditorViewport::DUP_DRAG_INSERT;

		fso::fred::CloneDragCommand* cloneCmd = nullptr;
		if (cloneSucceeded) {
			// Build CloneDragCommand: clone + final position in one undo step.
			// rotation_backup holds the clones' initial positions (written by
			// drag_rotate_save_backup() immediately after duplicate_marked_objects()).
			SCP_vector<fso::fred::CloneDragEntry> entries;
			int si = 0;
			for (const object* p = GET_FIRST(&obj_used_list);
			     p != END_OF_LIST(&obj_used_list);
			     p = GET_NEXT(p))
			{
				if (!p->flags[Object::Object_Flags::Marked]) continue;
				if (si >= static_cast<int>(_preCloneSourceSignatures.size())) break;
				fso::fred::CloneDragEntry e;
				e.sourceSignature = _preCloneSourceSignatures[si++];
				e.cloneSignature  = p->signature;
				const int n       = OBJ_INDEX(p);
				e.initialPos      = _viewport->rotation_backup[n].pos;
				e.initialOrient   = _viewport->rotation_backup[n].orient;
				e.finalPos        = Objects[n].pos;
				e.finalOrient     = Objects[n].orient;
				entries.push_back(e);
			}
			if (!entries.empty()) {
				cloneCmd = new fso::fred::CloneDragCommand(
				    std::move(entries),
				    _preCloneCurrentObj,
				    _wasInsertDrag,
				    fred,
				    _viewport);
				_fredView->mainUndoStack()->push(cloneCmd);
			}
		} else if (wasDragMode && isMoveOrRotate) {
			// Normal move (no clone): build MoveObjectsCommand outside the moved-guard
			// so that the Selection_lock press-teleport is also captured for undo.
			SCP_vector<fso::fred::ObjectTransform> transforms;
			for (const object* p = GET_FIRST(&obj_used_list);
			     p != END_OF_LIST(&obj_used_list);
			     p = GET_NEXT(p))
			{
				if (!p->flags[Object::Object_Flags::Marked]) continue;
				const int i = OBJ_INDEX(p);
				const auto& before = _viewport->rotation_backup[i];
				if (vm_vec_cmp(&before.pos, &p->pos) || vm_matrix_cmp(&before.orient, &p->orient)) {
					fso::fred::ObjectTransform t;
					t.signature     = p->signature;
					t.posBefore     = before.pos;
					t.orientBefore  = before.orient;
					t.posAfter      = p->pos;
					t.orientAfter   = p->orient;
					transforms.push_back(t);
				}
			}
			if (!transforms.empty()) {
				_fredView->mainUndoStack()->push(
				    new fso::fred::MoveObjectsCommand(std::move(transforms), fred, _viewport));
			}
		}

		_wasDupDrag = false;
		_wasInsertDrag = false;
		_preCloneSourceSignatures.clear();

		if (_usingMarkingBox) {
			_viewport->select_objects(_markingBox);
			_usingMarkingBox = false;

		} else if ((!_viewport->moved && _viewport->on_object != -1) && !_viewport->Selection_lock
			&& !event->modifiers().testFlag(Qt::ShiftModifier)) {
			fred->unmark_all();
			fred->markObject(_viewport->on_object);
		}

		_viewport->button_down = false;
		_viewport->needsUpdate();

		if (_viewport->Dup_drag == EditorViewport::DUP_DRAG_OF_WING) {
			int ship;
			object* objp;

			QString msg = tr("Add cloned ships to wing %1?").arg(Wings[_viewport->Duped_wing].name);
			if (QMessageBox::question(this, tr("Query"), msg) == QMessageBox::Yes) {
				bool addedAny = false;
				objp = GET_FIRST(&obj_used_list);
				while (objp != END_OF_LIST(&obj_used_list)) {
					if (objp->flags[Object::Object_Flags::Marked]) {
						if (Wings[_viewport->Duped_wing].wave_count >= MAX_SHIPS_PER_WING) {
							QMessageBox::information(this, "Warning", "Max ships per wing limit reached");
							break;
						}

						// Can't do player starts, since only player 1 is currently allowed to be in a wing
						Assert(objp->type == OBJ_SHIP);
						ship = objp->instance;
						Assert(Ships[ship].wingnum == -1);
						wing_bash_ship_name(&Ships[ship], &Wings[_viewport->Duped_wing], Wings[_viewport->Duped_wing].wave_count + 1, true);

						Wings[_viewport->Duped_wing].ship_index[Wings[_viewport->Duped_wing].wave_count] = ship;
						Ships[ship].wingnum = _viewport->Duped_wing;

						fred->wing_objects[_viewport->Duped_wing][Wings[_viewport->Duped_wing].wave_count] =
							OBJ_INDEX(objp);
						Wings[_viewport->Duped_wing].wave_count++;
						addedAny = true;
					}

					objp = GET_NEXT(objp);
				}
				if (addedAny && cloneCmd)
					cloneCmd->recordWingAdd(_viewport->Duped_wing);
			}
		}
	}
}
void RenderWidget::updateCursor() const {
	if (_viewport->Cursor_over >= 0) {
		switch (_cursorMode) {
		case CursorMode::Selecting:
			_window->setCursor(*_standardCursor);
			break;
		case CursorMode::Moving:
			_window->setCursor(*_moveCursor);
			break;
		case CursorMode::Rotating:
			_window->setCursor(*_rotateCursor);
			break;
		}
	} else {
		_window->setCursor(*_standardCursor);
	}
}
void RenderWidget::setEditor(Editor* editor, EditorViewport* viewport) {
	Assertion(fred == nullptr, "Render widget currently does not support resetting the editor!");
	Assertion(_viewport == nullptr, "Render widget currently does not support resetting the viewport!");

	Assertion(editor != nullptr, "Invalid editor pointer passed!");
	Assertion(viewport != nullptr, "Invalid viewport pointer passed!");

	fred = editor;
	_viewport = viewport;
	_fredView = static_cast<FredView*>(parentWidget());

	_window->setEditor(editor, _viewport->renderer);
}
void RenderWidget::setCursorMode(CursorMode mode) {
	_cursorMode = mode;
}
void RenderWidget::renderFrame() {
	qreal scale = _window->devicePixelRatio();
	_viewport->renderer->render_frame(fred->currentObject, fred->Render_subsys, _usingMarkingBox, _markingBox, scale);
}
} // namespace fso::fred
