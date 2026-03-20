#pragma once

#include <QWidget>
#include <QWindow>
#include <QTimer>

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "ui/QtGraphicsOperations.h"

namespace fso::fred::dialogs {
class BriefingEditorDialogModel;
}

namespace fso::fred {

class EditorViewport;

class BriefingMapWindow : public QWindow {
	Q_OBJECT
public:
	explicit BriefingMapWindow(QWidget* parentWidget);

	void initializeGL(const QSurfaceFormat& fmt);

protected:
	bool event(QEvent* evt) override;
	void exposeEvent(QExposeEvent* event) override;

private:
	QWidget* _parentWidget = nullptr;
};

// Lightweight os::Viewport that wraps BriefingMapWindow so we can use
// gr_use_viewport() / gr_flip() through FSO's normal rendering pipeline.
class BriefingViewport : public QtSurfaceViewport {
public:
	explicit BriefingViewport(BriefingMapWindow* window);

	SDL_Window* toSDLWindow() override;
	std::pair<uint32_t, uint32_t> getSize() override;
	void swapBuffers() override;
	void setState(os::ViewportState state) override;
	void minimize() override;
	void restore() override;
	QSurface* getRenderSurface() override;

private:
	BriefingMapWindow* _window = nullptr;
};

class BriefingMapWidget : public QWidget {
	Q_OBJECT

public:
	explicit BriefingMapWidget(QWidget* parent,
		dialogs::BriefingEditorDialogModel* model,
		EditorViewport* viewport);
	~BriefingMapWidget() override;

	void setStage(int stageNum);
	int getCurrentStage() const;

	QWindow* getRenderWindow() const;

signals:
	void iconSelected(int index);
	void cameraChanged(vec3d pos, matrix orient);

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	void renderFrame();
	void initBriefingMap();

	BriefingMapWindow* _window = nullptr;
	QTimer* _renderTimer = nullptr;
	std::unique_ptr<BriefingViewport> _briefingViewport; // our os::Viewport for gr_use_viewport
	std::unique_ptr<QOpenGLContext> _diagnosticContext;

	dialogs::BriefingEditorDialogModel* _model = nullptr;
	EditorViewport* _viewport = nullptr;

	int _currentStage = 0;
	bool _initialized = false;
	bool _rendering = false; // re-entrancy guard
	bool _loggedNotInitialized = false;
	bool _loggedNotExposed = false;
	bool _loggedNoViewport = false;
	bool _loggedInFrameSkip = false;
	bool _loggedNoContext = false;
	bool _loggedSurfaceMismatch = false;
	bool _loggedMakeCurrentFailure = false;
	uint32_t _debugFrameCounter = 0;

	// Mouse drag state
	bool _draggingIcon = false;
	int _dragIconIndex = -1;
	QPoint _lastMousePos;
};

} // namespace fso::fred
