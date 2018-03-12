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
class RenderWidget;

class RenderWindow: public QWindow {
 Q_OBJECT
 public:
	explicit RenderWindow(RenderWidget* parent = nullptr);
	~RenderWindow();

	void setEditor(Editor* editor, FredRenderer* renderer);

	void initializeGL(const QSurfaceFormat& surfaceFmt);

	void startRendering();

 protected:
	void paintGL();

	bool event(QEvent* evt) override;

	void resizeEvent(QResizeEvent* event) override;

	void exposeEvent(QExposeEvent* event) override;

 private:
	RenderWidget* _renderWidget = nullptr;

	Editor* fred = nullptr;
	FredRenderer* _renderer = nullptr;
};

class RenderWidget: public QWidget {
 Q_OBJECT

	RenderWindow* _window = nullptr;

	std::unique_ptr<QCursor> _standardCursor;
	std::unique_ptr<QCursor> _moveCursor;
	std::unique_ptr<QCursor> _rotateCursor;

	std::unordered_map<int, int> qt2fsKeys;
	Editor* fred = nullptr;
	EditorViewport* _viewport = nullptr;

	CursorMode _cursorMode = CursorMode::Selecting;

	bool _usingMarkingBox = false;
	Marking_box _markingBox;

	QPoint _lastMouse;

 public:
	explicit RenderWidget(QWidget* parent);

	void setSurfaceFormat(const QSurfaceFormat& fmt);
	QSurface* getRenderSurface() const;

	void setEditor(Editor* editor, EditorViewport* viewport);

	void setCursorMode(CursorMode mode);

	void renderFrame();
 protected:
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;

	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent*) override;

	void contextMenuEvent(QContextMenuEvent* event) override;
	void updateCursor() const;
};

} // namespace fred
} // namespace fso
