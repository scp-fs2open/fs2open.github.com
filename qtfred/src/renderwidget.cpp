#include "renderwidget.h"

#include <array>

#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>
#include <QtGui/QtGui>

#include "osapi/osapi.h"
#include "io/key.h"
#include "io/timer.h"
#include "starfield/starfield.h"

#include "editor.h"

namespace fso {
namespace fred {

RenderWindow::RenderWindow(QWidget* parent) : QWindow(parent->windowHandle()) {
	qt2fsKeys[Qt::Key_Shift] = KEY_LSHIFT;
	qt2fsKeys[Qt::Key_A] = KEY_A;
	qt2fsKeys[Qt::Key_Z] = KEY_Z;
	qt2fsKeys[Qt::Key_0] = KEY_PAD0;
	qt2fsKeys[Qt::Key_1] = KEY_PAD1;
	qt2fsKeys[Qt::Key_2] = KEY_PAD2;
	qt2fsKeys[Qt::Key_3] = KEY_PAD3;
	qt2fsKeys[Qt::Key_4] = KEY_PAD4;
	qt2fsKeys[Qt::Key_5] = KEY_PAD5;
	qt2fsKeys[Qt::Key_6] = KEY_PAD6;
	qt2fsKeys[Qt::Key_7] = KEY_PAD7;
	qt2fsKeys[Qt::Key_8] = KEY_PAD8;
	qt2fsKeys[Qt::Key_9] = KEY_PAD9;
	qt2fsKeys[Qt::Key_Plus] = KEY_PADPLUS;
	qt2fsKeys[Qt::Key_Minus] = KEY_PADMINUS;

	setSurfaceType(QWindow::OpenGLSurface);
}

void RenderWindow::initializeGL(const QSurfaceFormat& surfaceFmt) {
	setFormat(surfaceFmt);

	// Force creation of this window so that we can use it work OpenGL
	create();
}

void RenderWindow::paintGL() {
	fred->update();
}

void RenderWindow::keyPressEvent(QKeyEvent* key) {
	if (key->isAutoRepeat()) {
		QWindow::keyPressEvent(key);
		return;
	}

	if (!qt2fsKeys.count(key->key())) {
		QWindow::keyPressEvent(key);
		return;
	}

	key->accept();
	key_mark(qt2fsKeys.at(key->key()), 1, 0);
}

void RenderWindow::keyReleaseEvent(QKeyEvent* key) {
	if (key->isAutoRepeat()) {
		QWindow::keyReleaseEvent(key);
		return;
	}

	if (!qt2fsKeys.count(key->key())) {
		QWindow::keyReleaseEvent(key);
		return;
	}

	key->accept();
	key_mark(qt2fsKeys.at(key->key()), 0, 0);
}

void RenderWindow::mouseReleaseEvent(QMouseEvent* mouse) {
	fred->findFirstObjectUnder(mouse->x(), mouse->y());
}
void RenderWindow::resizeEvent(QResizeEvent* event) {
	//fred->resize(event->size().width(), event->size().height());
}
void RenderWindow::updateGL() {
	paintGL();
}

RenderWidget::RenderWidget(QWidget* parent) :
	QWidget(parent) {
	_window = new RenderWindow(this);
}
RenderWindow* RenderWidget::getWindow() const {
	return _window;
}
} // namespace fred
} // namespace fso
