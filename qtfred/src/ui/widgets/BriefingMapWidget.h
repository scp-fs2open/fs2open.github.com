#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QRect>
#include <QTimer>

#include "globalincs/pstypes.h"
#include "mission/CameraController.h"
#include "osapi/osapi.h"
#include "ui/QtGraphicsOperations.h"

class QOffscreenSurface;
class QPainter;
class QPaintEvent;
class briefing;

namespace fso::fred::dialogs {
class BriefingEditorDialogModel;
}

namespace fso::fred {

class EditorViewport;

// Lightweight os::Viewport backed by an offscreen surface so we can make the engine's GL context
// current and render the briefing into an off-screen render target. The briefing map is then read
// back into a QImage and painted into a normal QWidget, which (unlike an embedded native QWindow)
// participates in the layout and resizes/clips cleanly.
class BriefingViewport : public QtSurfaceViewport {
public:
	explicit BriefingViewport(QOffscreenSurface* surface);

	SDL_Window* toSDLWindow() override;
	std::pair<uint32_t, uint32_t> getSize() override;
	void swapBuffers() override;
	void setState(os::ViewportState state) override;
	void minimize() override;
	void restore() override;
	QSurface* getRenderSurface() override;

private:
	QOffscreenSurface* _surface = nullptr;
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
	void notifyIconVisualsChanged();
	void applyCameraToCurrentStage(const vec3d& pos, const matrix& orient);
	void setMovementSpeedScale(float scale);
	void setRotationSpeedScale(float scale);

signals:
	void iconSelected(int index, bool toggleSelection);
	void cameraChanged(vec3d pos, matrix orient);

protected:
	bool event(QEvent* evt) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

private:
	void renderFrame();
	void restoreMainViewportFrame(bool wasActive);
	void initBriefingMap();
	void applyStageTransition(int stageNum, int transitionTime);
	void maybeRenderCutTransition(float frametime, int width, int height);
	static bool shouldUseCutTransition(int fromStage, int toStage, const briefing* briefPtr);
	void updateEditorHighlightPlayback() const;
	void drawSelectionBrackets(QPainter& painter);
	QPixmap checkerboardTile(); // subtle, theme-appropriate matte for the letterbox bars
	void applyCameraPoseLikeKeyboardControls(const vec3d& camPos, const matrix& camOrient, bool updateModel);
	void applyBoundCameraControls(float frametime);

	CameraController _cameraController;

	QOffscreenSurface* _surface = nullptr; // offscreen GL surface the briefing renders through
	QImage _frameImage;                    // last rendered briefing frame (reference resolution)
	QRect _blitRect;                       // where _frameImage is drawn in the widget (logical px)
	QPixmap _checkerTile;                  // cached matte tile
	bool _checkerTileDark = false;         // theme the cached tile was built for
	QTimer* _renderTimer = nullptr;
	std::unique_ptr<BriefingViewport> _briefingViewport; // our os::Viewport for gr_use_viewport

	dialogs::BriefingEditorDialogModel* _model = nullptr;
	EditorViewport* _viewport = nullptr;

	int _currentStage = 0;
	bool _initialized = false;
	bool _rendering = false; // re-entrancy guard
	bool _loggedNoContext = false;
	bool _loggedNoRenderTarget = false;

	// Offscreen render target: the briefing is rendered at the reference resolution, read back into
	// _frameImage, and painted (scaled + letterboxed) into the widget, so the view is a faithful copy
	// of the canonical briefing that resizes cleanly.
	int _renderTarget = -1;
	int _renderTargetW = 0;
	int _renderTargetH = 0;

	// Mouse drag state
	bool _draggingIcon = false;
	int _dragIconIndex = -1;
	QPointF _dragStartMousePos;
	vec3d _dragStartIconPos = ZERO_VECTOR;
	// Render size icon coordinates are expressed in (the reference/render-target resolution).
	int _lastRenderWidth = 0;
	int _lastRenderHeight = 0;

	// Briefing cut transition state (forward/backward cut + jump cuts)
	bool _cutFadeIn = false;
	int _cutFadeFrame = 0;
	int _pendingCutStage = -1;

};

} // namespace fso::fred
