//
//

// work around MSVC 2015 and 2017 compiler bug
// https://bugreports.qt.io/browse/QTBUG-72073
#define QT_NO_FLOAT16_OPERATORS

#include "renderwidget.h"

#include <array>

#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>
#include <QtGui/QtGui>
#include <QtWidgets/QHBoxLayout>
#include <mission/object.h>
#include <ship/ship.h>
#include <globalincs/linklist.h>
#include <QtWidgets/QMessageBox>

#include "osapi/osapi.h"
#include "io/key.h"
#include "io/timer.h"
#include "starfield/starfield.h"

#include "mission/Editor.h"
#include "FredApplication.h"
#include "ui/FredView.h"

namespace fso {
namespace fred {

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
	case QEvent::MouseButtonPress: {
		auto mouseEvent = static_cast<QMouseEvent*>(evt);

		if (mouseEvent->button() == Qt::RightButton) {
			// Right button events may cause a context menu so we send that to our parent which will handle that
			qGuiApp->sendEvent(parent(), evt);
		} else {
			// The rest will be handled by the render widget
			qGuiApp->sendEvent(_renderWidget, evt);
		}
		return true;
	}
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove: {
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
RenderWindow::~RenderWindow() {
}
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
	setFocusPolicy(Qt::NoFocus);
	setMouseTracking(true);

	_window = new RenderWindow(this);

	auto layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->addWidget(QWidget::createWindowContainer(_window, this));
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	qt2fsKeys[Qt::Key_Shift] = KEY_LSHIFT;
	qt2fsKeys[Qt::Key_A] = KEY_A;
	qt2fsKeys[Qt::Key_Z] = KEY_Z;
	qt2fsKeys[Qt::Key_0 + Qt::KeypadModifier] = KEY_PAD0;
	qt2fsKeys[Qt::Key_1 + Qt::KeypadModifier] = KEY_PAD1;
	qt2fsKeys[Qt::Key_2 + Qt::KeypadModifier] = KEY_PAD2;
	qt2fsKeys[Qt::Key_3 + Qt::KeypadModifier] = KEY_PAD3;
	qt2fsKeys[Qt::Key_4 + Qt::KeypadModifier] = KEY_PAD4;
	qt2fsKeys[Qt::Key_5 + Qt::KeypadModifier] = KEY_PAD5;
	qt2fsKeys[Qt::Key_6 + Qt::KeypadModifier] = KEY_PAD6;
	qt2fsKeys[Qt::Key_7 + Qt::KeypadModifier] = KEY_PAD7;
	qt2fsKeys[Qt::Key_8 + Qt::KeypadModifier] = KEY_PAD8;
	qt2fsKeys[Qt::Key_9 + Qt::KeypadModifier] = KEY_PAD9;
	qt2fsKeys[Qt::Key_Plus + Qt::KeypadModifier] = KEY_PADPLUS;
	qt2fsKeys[Qt::Key_Minus + Qt::KeypadModifier] = KEY_PADMINUS;
	qt2fsKeys[Qt::Key_Space] = KEY_SPACEBAR;
	qt2fsKeys[Qt::Key_Escape] = KEY_ESC;

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
	event->accept();

	auto parentView = static_cast<FredView*>(parentWidget());

	Q_ASSERT(parentView);

	parentView->showContextMenu(event->globalPos());
}

void RenderWidget::keyPressEvent(QKeyEvent* key) {
	if (key->isAutoRepeat()) {
		QWidget::keyPressEvent(key);
		return;
	}

	auto code = key->key() + (int) key->modifiers();
	if (!qt2fsKeys.count(code)) {
		QWidget::keyPressEvent(key);
		return;
	}

	key_mark(qt2fsKeys.at(code), 1, 0);
}

void RenderWidget::keyReleaseEvent(QKeyEvent* key) {
	if (key->isAutoRepeat()) {
		QWidget::keyReleaseEvent(key);
		return;
	}

	auto code = key->key() + (int) key->modifiers();
	if (!qt2fsKeys.count(code)) {
		QWidget::keyReleaseEvent(key);
		return;
	}

	key_mark(qt2fsKeys.at(code), 0, 0);
}
void RenderWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	event->ignore();
}
void RenderWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() != Qt::LeftButton) {
		// Ignore everything that has nothing to to with the left button
		return QWidget::mousePressEvent(event);
	}

	int waypoint_instance = -1;
	if (fred->cur_waypoint != NULL)
	{
		Assert(fred->cur_waypoint_list != NULL);
		waypoint_instance = Objects[fred->cur_waypoint->get_objnum()].instance;
	}

	_markingBox.x1 = event->x();
	_markingBox.y1 = event->y();
	_viewport->Dup_drag = 0;

	_viewport->on_object = _viewport->select_object(event->x(), event->y());
	_viewport->button_down = 1;

	_viewport->drag_rotate_save_backup();

	if (event->modifiers().testFlag(Qt::ControlModifier)) {  // add a new object
		if (!_viewport->Bg_bitmap_dialog) {
			if (_viewport->on_object == -1) {
				_viewport->Selection_lock = 0;  // force off selection lock
				_viewport->on_object = _viewport->create_object_on_grid(event->x(), event->y(), waypoint_instance);

			} else {
				_viewport->Dup_drag = 1;
			}
		} else {
			/*
			Selection_lock = 0;  // force off selection lock
			on_object = Cur_bitmap = create_bg_bitmap();
			Bg_bitmap_dialog->update_data();
			Update_window = 1;
			if (Cur_bitmap == -1)
				MessageBox("Background bitmap limit reached.\nCan't add more.");
			*/
		}

	} else if (!_viewport->Selection_lock) {
		if (_viewport->Bg_bitmap_dialog) {
			/*
			 TODO: Briefing dialog is not yet implemented!
			Cur_bitmap = on_object;
			Bg_bitmap_dialog -> update_data();
*/
		} else if ((event->modifiers().testFlag(Qt::ShiftModifier)) || (_viewport->on_object == -1)
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

	if (query_valid_object(fred->currentObject)) {
		_viewport->original_pos = Objects[fred->currentObject].pos;
	}

	_viewport->moved = 0;
	if (_viewport->Selection_lock) {
		if (_viewport->Editing_mode == CursorMode::Moving) {
			_viewport->drag_objects(event->x(), event->y());
		} else if (_viewport->Editing_mode == CursorMode::Rotating) {
			_viewport->drag_rotate_objects(0, 0);
		}

		_viewport->needsUpdate();
	}

	/*
     TODO: Briefing dialog is not yet implemented!
if (query_valid_object(fred->getCurrentObject()) && (fred->getNumMarked() == 1)
    && (Objects[fred->getCurrentObject()].type == OBJ_POINT)) {
Assert(Briefing_dialog);
Briefing_dialog->icon_select(Objects[cur_object_index].instance);
	} else {
		if (Briefing_dialog) {
			Briefing_dialog->icon_select(-1);
		}
	}
*/
}
void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
	auto mouseDX = event->pos() - _lastMouse;
	_lastMouse = event->pos();

// Update marking box
	_markingBox.x2 = event->x();
	_markingBox.y2 = event->y();

	// RT point

	_viewport->Cursor_over = _viewport->select_object(event->x(), event->y());
	updateCursor();

	if (!event->buttons().testFlag(Qt::LeftButton)) {
		_viewport->button_down = false;
	}

	// The following will cancel a drag operation if another program running in memory
	// happens to jump in and take over (such as new email has arrived popup boxes).
	/*
	TODO: Investiage if this is still required
	if (_viewport->button_down && GetCapture() != this)
		_viewport->cancel_drag();
	 */

	if (_viewport->button_down) {
		if (abs(_markingBox.x1 - _markingBox.x2) > 1 || abs(_markingBox.y1 - _markingBox.y2) > 1) {
			_viewport->moved = true;
		}

		if (_viewport->moved) {
			if (_viewport->on_object != -1 || _viewport->Selection_lock) {
				if (_viewport->Editing_mode == CursorMode::Moving) {
					_viewport->drag_objects(event->x(), event->y());
				} else if (_viewport->Editing_mode == CursorMode::Rotating) {
					_viewport->drag_rotate_objects(mouseDX.x(), mouseDX.y());
				}

			} else if (!_viewport->Bg_bitmap_dialog) {
				_usingMarkingBox = true;
			}

			if (mouseDX.x() || mouseDX.y()) {
				_viewport->needsUpdate();
			}
		}
	}
}
void RenderWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() != Qt::LeftButton) {
		// Ignore everything that has nothing to to with the left button
		return QWidget::mousePressEvent(event);
	}

	_markingBox.x2 = event->x();
	_markingBox.y2 = event->y();

	/*
	TODO: Investiage if this is still required
	if (_viewport->button_down && GetCapture() != this)
		_viewport->cancel_drag();
	*/

	if (_viewport->button_down) {
		if (abs(_markingBox.x1 - _markingBox.x2) > 1 || abs(_markingBox.y1 - _markingBox.y2) > 1) {
			_viewport->moved = true;
		}

		if (_viewport->moved) {
			if ((_viewport->on_object != -1) || _viewport->Selection_lock) {
				if (_viewport->Editing_mode == CursorMode::Moving) {
					_viewport->drag_objects(event->x(), event->y());
				} else if (_viewport->Editing_mode == CursorMode::Rotating) {
					_viewport->drag_rotate_objects(0, 0);
				}

				fred->missionChanged();

			} else {
				_usingMarkingBox = true;
			}
		}

		if (_viewport->Bg_bitmap_dialog) {
			_usingMarkingBox = true;

		} else {
			if (_usingMarkingBox) {
				_viewport->select_objects(_markingBox);
				_usingMarkingBox = 0;

			} else if ((!_viewport->moved && _viewport->on_object != -1) && !_viewport->Selection_lock
				&& !event->modifiers().testFlag(Qt::ShiftModifier)) {
				fred->unmark_all();
				fred->markObject(_viewport->on_object);
			}
		}

		_viewport->button_down = false;
		_viewport->needsUpdate();
		if (_viewport->Dup_drag == EditorViewport::DUP_DRAG_OF_WING) {
			char msg[256];
			int ship;
			object* objp;

			sprintf(msg, "Add cloned ships to wing %s?", Wings[_viewport->Duped_wing].name);
			if (QMessageBox::question(this, tr("Query"), msg) == QMessageBox::Yes) {
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
						wing_bash_ship_name(Ships[ship].ship_name,
											Wings[_viewport->Duped_wing].name,
											Wings[_viewport->Duped_wing].wave_count + 1);

						Wings[_viewport->Duped_wing].ship_index[Wings[_viewport->Duped_wing].wave_count] = ship;
						Ships[ship].wingnum = _viewport->Duped_wing;

						fred->wing_objects[_viewport->Duped_wing][Wings[_viewport->Duped_wing].wave_count] =
							OBJ_INDEX(objp);
						Wings[_viewport->Duped_wing].wave_count++;
					}

					objp = GET_NEXT(objp);
				}
			}
		}
	}

	/*
	TODO: Briefing dialog related stuff
	if (query_valid_object() && (Marked == 1) && (Objects[cur_object_index].type == OBJ_POINT)) {
		Assert(Briefing_dialog);
		Briefing_dialog->icon_select(Objects[cur_object_index].instance);

	} else {
		if (Briefing_dialog)
			Briefing_dialog->icon_select(-1);
	}
	 */
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

	_window->setEditor(editor, _viewport->renderer);
}
void RenderWidget::setCursorMode(CursorMode mode) {
	_cursorMode = mode;
}
void RenderWidget::renderFrame() {
	_viewport->renderer->render_frame(fred->currentObject, fred->Render_subsys, _usingMarkingBox, _markingBox, false);
}
} // namespace fred
} // namespace fso
