#include "renderwidget.h"

#include <array>

#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>

#include "osapi/osapi.h"
#include "io/key.h"
#include "io/timer.h"
#include "starfield/starfield.h"

#include "editor.h"

namespace fso {
namespace fred {

RenderWidget::RenderWidget(QWidget *parent) :
    QGLWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);
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

}

void RenderWidget::initializeGL()
{
    qDebug() << "fso::fred::RenderWidget::initializeGL()";
    fred->initializeRenderer();
    QTimer *updater(new QTimer(this));
    connect(updater, SIGNAL(timeout()), this, SLOT(updateGL()));
    updater->setInterval(30);

    ///! \todo Maybe an option for smooth rendering vs only on events?
    updater->start();
}

void RenderWidget::paintGL()
{
    fred->update();
}

void RenderWidget::resizeGL(int w, int h)
{
    fred->resize(w, h);
}

void RenderWidget::keyPressEvent(QKeyEvent *key)
{
    if (key->isAutoRepeat())
    {
        QGLWidget::keyPressEvent(key);
        return;
    }

    if (!qt2fsKeys.count(key->key())) {
        QGLWidget::keyPressEvent(key);
        return;
    }

    key->accept();
    key_mark(qt2fsKeys.at(key->key()), 1, 0);
}

void RenderWidget::keyReleaseEvent(QKeyEvent *key)
{
    if (key->isAutoRepeat())
    {
        QGLWidget::keyReleaseEvent(key);
        return;
    }

    if (!qt2fsKeys.count(key->key())) {
        QGLWidget::keyReleaseEvent(key);
        return;
    }

    key->accept();
    key_mark(qt2fsKeys.at(key->key()), 0, 0);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *mouse)
{
    fred->findFirstObjectUnder(mouse->x(), mouse->y());
}

} // namespace fred
} // namespace fso
