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

namespace fso {
namespace fred {

RenderWindow::RenderWindow(QWidget* parent) : QWindow(parent->windowHandle()) {
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

	setSurfaceType(QWindow::OpenGLSurface);
}

void RenderWindow::initializeGL(const QSurfaceFormat& surfaceFmt) {
	setFormat(surfaceFmt);

	// Force creation of this window so that we can use it for OpenGL
	create();

	fredApp->runAfterInit([this]() { startRendering(); });
}

void RenderWindow::startRendering() {
	_isRendering = true;

	_renderer->resize(size().width(), size().height());
}

void RenderWindow::paintGL() {
	subsys_to_render Render_subsys;

	_renderer->render_frame(-1,
							Render_subsys,
							false,
							Marking_box(),
							-1,
							false);
}

bool RenderWindow::event(QEvent* evt) {
	switch (evt->type()) {
	case QEvent::UpdateRequest:
		updateGL();
		return QWindow::event(evt);
	default:
		return QWindow::event(evt);
	}
}

void RenderWindow::keyPressEvent(QKeyEvent* key) {
	if (key->isAutoRepeat()) {
		QWindow::keyPressEvent(key);
		return;
	}

	auto code = key->key() + (int)key->modifiers();
	if (!qt2fsKeys.count(code)) {
		QWindow::keyPressEvent(key);
		return;
	}

	key->accept();
	key_mark(qt2fsKeys.at(code), 1, 0);
}

void RenderWindow::keyReleaseEvent(QKeyEvent* key) {
	if (key->isAutoRepeat()) {
		QWindow::keyReleaseEvent(key);
		return;
	}

	auto code = key->key() + (int)key->modifiers();
	if (!qt2fsKeys.count(code)) {
		QWindow::keyReleaseEvent(key);
		return;
	}

	key->accept();
	key_mark(qt2fsKeys.at(code), 0, 0);
}

void RenderWindow::mouseReleaseEvent(QMouseEvent* mouse) {
	auto obj_num = _renderer->select_object(mouse->x(), mouse->y(), false);

	fred->selectObject(obj_num);
}
void RenderWindow::resizeEvent(QResizeEvent* event) {
	if (_isRendering) {
		// Only send resize event if we are actually rendering
		_renderer->resize(event->size().width(), event->size().height());
	}
}

void RenderWindow::updateGL() {
	if (_isRendering) {
		paintGL();
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
	connect(_renderer, &FredRenderer::scheduleUpdate, [this]() { requestUpdate(); });
}

RenderWidget::RenderWidget(QWidget* parent) : QWidget(parent) {
	setFocusPolicy(Qt::StrongFocus);

	_window = new RenderWindow(this);

	auto layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->addWidget(QWidget::createWindowContainer(_window, this));
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
}
RenderWindow* RenderWidget::getWindow() const {
	return _window;
}
} // namespace fred
} // namespace fso
