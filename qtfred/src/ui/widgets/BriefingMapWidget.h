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
	void iconCreateRequested(vec3d worldPos);   // Ctrl+click: make a new icon at this world position
	void deleteSelectedIconsRequested();        // Delete key: remove the selected icon(s)
	void nudgeIconsRequested(vec3d worldDelta); // arrow keys: move the selected icon(s) by this offset
	// drag-box selection: the icons enclosed by the rubber band (additive = add to the current selection)
	void iconsSelectedInBox(SCP_vector<int> indices, bool additive);

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
	void drawSelectionMarquee(QPainter& painter);
	// Emit iconsSelectedInBox() for every icon whose center falls inside the rubber band (widget coords).
	void selectIconsInBox(const QPointF& startLogical, const QPointF& endLogical, bool additive);
	QPixmap checkerboardTile(); // subtle, theme-appropriate matte for the letterbox bars
	void applyCameraPoseLikeKeyboardControls(const vec3d& camPos, const matrix& camOrient, bool updateModel);
	void applyBoundCameraControls(float frametime);
	// Unproject a mouse position (in render-target/reference-resolution pixels) onto the briefing grid
	// plane, giving the world position under the cursor for placing a new icon.
	vec3d worldPosAtMouse(float mouseRefX, float mouseRefY) const;

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
	// When a plain click lands on a member of a multi-selection we keep the selection (so a drag moves the
	// whole group); if the click turns out not to be a drag, this collapses the selection to that icon.
	int _pendingCollapseIndex = -1;

	// Drag-box (rubber band) selection, started by pressing on empty space. Positions are logical widget
	// coordinates. _boxSelectActive turns on once the drag passes the click threshold.
	bool _boxSelectPending = false;
	bool _boxSelectActive = false;
	bool _boxSelectAdditive = false;
	QPointF _boxStartPos;
	QPointF _boxCurrentPos;
	// Render size icon coordinates are expressed in (the reference/render-target resolution).
	int _lastRenderWidth = 0;
	int _lastRenderHeight = 0;
	// Projection scale (Matrix_scale) the briefing last rendered with, captured so we can unproject
	// clicks accurately even though the live g3 state belongs to the main editor viewport by then.
	vec3d _lastMatrixScale = ZERO_VECTOR;

	// Briefing cut transition state (forward/backward cut + jump cuts)
	bool _cutFadeIn = false;
	int _cutFadeFrame = 0;
	int _pendingCutStage = -1;

};

} // namespace fso::fred
