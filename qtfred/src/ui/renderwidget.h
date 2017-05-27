#pragma once

#include <memory>
#include <unordered_map>

#include <QWindow>
#include <QOpenGLWidget>
#include <osapi/osapi.h>

namespace fso {
namespace fred {

class Editor;

class RenderWindow: public QWindow {
 Q_OBJECT
 public:
	explicit RenderWindow(QWidget* parent = 0);

	void setEditor(Editor* editor) {
		fred = editor;
	}

	void initializeGL(const QSurfaceFormat& surfaceFmt);

	void startRendering();

	void updateGL();

 protected:
	void paintGL();

	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void resizeEvent(QResizeEvent* event) override;

 signals:

 private:
	std::unordered_map<int, int> qt2fsKeys;
	Editor* fred;
	bool _isRendering = false;
};

class RenderWidget: public QWidget {
	RenderWindow* _window = nullptr;

 Q_OBJECT

 public:
	RenderWidget(QWidget* parent);

	RenderWindow* getWindow() const;
};

} // namespace fred
} // namespace fso
