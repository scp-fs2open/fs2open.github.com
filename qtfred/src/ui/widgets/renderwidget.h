#pragma once

#include <memory>
#include <unordered_map>

#include <QWindow>
#include <QWidget>
#include <mission/FredRenderer.h>

#include "osapi/osapi.h"

namespace fso {
namespace fred {

class Editor;

class RenderWindow: public QWindow {
 Q_OBJECT
 public:
	explicit RenderWindow(QWidget* parent = 0);
	~RenderWindow();

	void setEditor(Editor* editor, FredRenderer* renderer);

	void initializeGL(const QSurfaceFormat& surfaceFmt);

	void startRendering();

	void updateGL();

 protected:
	void paintGL();

	bool event(QEvent* evt) override;

	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void resizeEvent(QResizeEvent* event) override;

	void exposeEvent(QExposeEvent* event) override;

	bool eventFilter(QObject* watched, QEvent* event) override;
 private:
	std::unique_ptr<QCursor> _standardCursor;
	std::unique_ptr<QCursor> _moveCursor;
	std::unique_ptr<QCursor> _rotateCursor;

	std::unordered_map<int, int> qt2fsKeys;
	Editor* fred = nullptr;
	FredRenderer* _renderer = nullptr;
	bool _isRendering = false;
};

class RenderWidget: public QWidget {
	RenderWindow* _window = nullptr;

 Q_OBJECT

 public:
	explicit RenderWidget(QWidget* parent);

	RenderWindow* getWindow() const;

 protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
};

} // namespace fred
} // namespace fso
