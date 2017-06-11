#include "renderwidget.h"

#include <array>

#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>
#include <QtGui/QtGui>
#include <QtWidgets/QHBoxLayout>

#include "osapi/osapi.h"
#include "io/key.h"
#include "io/timer.h"
#include "starfield/starfield.h"

#include "mission/editor.h"
#include "FredApplication.h"
#include "ui/FredView.h"

namespace fso {
namespace fred {

RenderWindow::RenderWindow(RenderWidget* renderWidget) : QWindow((QWindow*)nullptr), _renderWidget(renderWidget) {
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
		}
		else {
			// The rest will be handled by the render widget
			qGuiApp->sendEvent(_renderWidget, evt);
		}
		return true;
	}
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
	{
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

	auto code = key->key() + (int)key->modifiers();
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

	auto code = key->key() + (int)key->modifiers();
	if (!qt2fsKeys.count(code)) {
		QWidget::keyReleaseEvent(key);
		return;
	}

	key_mark(qt2fsKeys.at(code), 0, 0);
}
void RenderWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	QWidget::mouseDoubleClickEvent(event);
}
void RenderWidget::mousePressEvent(QMouseEvent* event) {
	if (!event->buttons().testFlag(Qt::LeftButton)) {
		// Ignore everything that has nothing to to with the left button
		return QWidget::mousePressEvent(event);
	}

	_markingBox.x1 = event->x();
	_markingBox.y1 = event->y();

	auto on_object = _renderer->select_object(event->x(), event->y(), false);

	if (event->modifiers().testFlag(Qt::ControlModifier)) {
		// TODO: Add object creation
	} else { // TODO: if (!Selection_lock)
		if ((event->modifiers().testFlag(Qt::ShiftModifier)) || (on_object == -1) || !(Objects[on_object].flags[Object::Object_Flags::Marked])) {
			if (!event->modifiers().testFlag(Qt::ShiftModifier))
				fred->unmark_all();

			if (on_object != -1) {
				if (Objects[on_object].flags[Object::Object_Flags::Marked])
					fred->unmarkObject(on_object);
				else
					fred->markObject(on_object);
			}
		}
	}

	if (on_object < 0) {
		// Start dragging the marking box
		_usingMarkingBox = true;
	}
}
void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
	auto mouseDX = event->pos() - _lastMouse;
	_lastMouse = event->pos();

	// Update marking box
	_markingBox.x2 = event->x();
	_markingBox.y2 = event->y();

	if (!event->buttons().testFlag(Qt::LeftButton)) {
		// In case the button was released without the button release event
		_usingMarkingBox = false;
		_renderer->scheduleUpdate();
	}

	// No matter in which mode we are, we always check which object is under the cursor
	auto obj_num = _renderer->select_object(event->x(), event->y(), false);
	_renderer->Cursor_over = obj_num;
	updateCursor();

	if (event->buttons().testFlag(Qt::LeftButton)) {
		auto moved = false;
		if (abs(_markingBox.x1 - _markingBox.x2) > 1 || abs(_markingBox.y1 - _markingBox.y2) > 1)
			moved = true;

		if (moved) {
			/*
			TODO: Add this once dragging is implemented
			if (on_object != -1 || Selection_lock) {
				if (Editing_mode == 1)
					drag_objects();
				else if (Editing_mode == 2)
					drag_rotate_objects();

			} else if (!Bg_bitmap_dialog)
				box_marking = 1;
			 */

			if (mouseDX.manhattanLength() > 0) {
				// Marking box has changed -> need to rerender
				_renderer->scheduleUpdate();
			}
		}
	}
}
void RenderWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() != Qt::LeftButton) {
		// Ignore everything that has nothing to to with the left button
		return QWidget::mousePressEvent(event);
	}

	Qt::MouseButton button = event->button();
	if (button == Qt::LeftButton) {
		if (_usingMarkingBox) {
			_usingMarkingBox = false;

			_renderer->select_objects(_markingBox);
			_renderer->scheduleUpdate();
		}
	}
}
void RenderWidget::updateCursor() const {
	if (_renderer->Cursor_over >= 0) {
		switch(_cursorMode) {
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
void RenderWidget::setEditor(Editor* editor, FredRenderer* renderer) {
	Assertion(fred == nullptr, "Render widget currently does not support resetting the editor!");
	Assertion(_renderer == nullptr, "Render widget currently does not support resetting the renderer!");

	Assertion(editor != nullptr, "Invalid editor pointer passed!");
	Assertion(renderer != nullptr, "Invalid renderer pointer passed!");

	fred = editor;
	_renderer = renderer;

	_window->setEditor(editor, renderer);
}
void RenderWidget::setCursorMode(CursorMode mode) {
	_cursorMode = mode;
}
void RenderWidget::renderFrame() {
	subsys_to_render Render_subsys;

	_renderer->render_frame(fred->getCurrentObject(),
							Render_subsys,
							_usingMarkingBox,
							_markingBox,
							false);
}
} // namespace fred
} // namespace fso
