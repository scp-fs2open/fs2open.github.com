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

RenderWindow::RenderWindow(QWidget* parent) : QWindow(parent->windowHandle()) {
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
	subsys_to_render Render_subsys;

	_renderer->render_frame(fred->getCurrentObject(),
							Render_subsys,
							false,
							Marking_box(),
							false);
}

bool RenderWindow::event(QEvent* evt) {
	switch (evt->type()) {
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
	_window->installEventFilter(this);

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

void RenderWidget::mouseReleaseEvent(QMouseEvent* mouse) {
	if (mouse->button() == Qt::LeftButton) {
		auto obj_num = _renderer->select_object(mouse->x(), mouse->y(), false);

		fred->selectObject(obj_num);
	}
}
void RenderWidget::mouseMoveEvent(QMouseEvent* event) {
	// No matter in which mode we are, we always check which object is under the cursor
	auto obj_num = _renderer->select_object(event->x(), event->y(), false);
	_renderer->Cursor_over = obj_num;

	if (obj_num >= 0) {
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
bool RenderWidget::eventFilter(QObject* watched, QEvent* event) {
	switch(event->type()) {
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
		// Redirect all the events to us since we want to handle them in in the QtWidget related code
		qGuiApp->sendEvent(this, event);
		return true;
	default:
		// Don't filter event, process as usual
		return false;
	}
}
void RenderWidget::mousePressEvent(QMouseEvent* event) {
	QWidget::mousePressEvent(event);
}
void RenderWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	QWidget::mouseDoubleClickEvent(event);
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
} // namespace fred
} // namespace fso
