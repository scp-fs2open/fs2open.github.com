#include "BriefingMapWidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include "FredApplication.h"
#include "anim/animplay.h"
#include "bmpman/bmpman.h"
#include "mission/dialogs/BriefingEditorDialogModel.h"
#include "mission/EditorViewport.h"
#include "ui/ControlBindings.h"
#include "ui/Theme.h"

#include "graphics/2d.h"
#include "render/3d.h"
#include "mission/missionbriefcommon.h"

#include <algorithm>
#include <cmath>

namespace fso::fred {

namespace {
void ensure_highlight_anim_loaded(brief_icon& icon) {
	if ((icon.flags & BI_HIGHLIGHT) == 0) {
		return;
	}

	auto* iconInfo = brief_get_icon_info(&icon);
	if (iconInfo == nullptr) {
		return;
	}

	auto& sourceAnim = iconInfo->highlight;
	if (sourceAnim.filename[0] == '\0' || !stricmp(NOX("none"), sourceAnim.filename)) {
		return;
	}

	if (sourceAnim.first_frame < 0) {
		hud_anim_load(&sourceAnim);
	}

	if (sourceAnim.first_frame >= 0) {
		icon.highlight_anim = sourceAnim;
	}
}
}

// ---- BriefingViewport ----

BriefingViewport::BriefingViewport(QOffscreenSurface* surface) : _surface(surface) {
}

SDL_Window* BriefingViewport::toSDLWindow() {
	return nullptr;
}

std::pair<uint32_t, uint32_t> BriefingViewport::getSize() {
	// The reference resolution the briefing is composed at (also the off-screen render target size).
	// Pinned to the retail GR_1024 briefing map size, what the game and Lua's ui.drawBriefingMap()
	// default to, rather than the FRED-only "$FRED Briefing window resolution" table setting: the game
	// ignores that setting, so honoring it here would only make the editor preview diverge from the
	// shipped briefing. This is the single source of truth for the render size (see renderFrame()).
	return std::make_pair(static_cast<uint32_t>(Brief_grid_coords[GR_1024][2]),
		static_cast<uint32_t>(Brief_grid_coords[GR_1024][3]));
}

void BriefingViewport::swapBuffers() {
	// Nothing to present: the briefing is read back from the render target, not swapped to a window.
}

void BriefingViewport::setState(os::ViewportState /*state*/) {
}

void BriefingViewport::minimize() {
}

void BriefingViewport::restore() {
}

QSurface* BriefingViewport::getRenderSurface() {
	return _surface;
}

// ---- BriefingMapWidget ----

BriefingMapWidget::BriefingMapWidget(QWidget* parent,
	dialogs::BriefingEditorDialogModel* model,
	EditorViewport* viewport)
	: QWidget(parent), _model(model), _viewport(viewport)
{
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	// Opaque: we always fill the whole widget (image + black letterbox bars) in paintEvent.
	setAttribute(Qt::WA_OpaquePaintEvent, true);

	_renderTimer = new QTimer(this);
	_renderTimer->setInterval(33); // ~30 fps
	connect(_renderTimer, &QTimer::timeout, this, &BriefingMapWidget::renderFrame);

	fredApp->runAfterInit([this]() { initBriefingMap(); });
}

BriefingMapWidget::~BriefingMapWidget() {
	_renderTimer->stop();
	if (_renderTarget >= 0) {
		bm_release(_renderTarget);
		_renderTarget = -1;
	}
}

void BriefingMapWidget::initBriefingMap() {
	// The briefing renders through an off-screen surface (no visible window): the engine's GL context
	// is made current on it so we can render into a render target, which we then read back and paint.
	auto* currentCtx = QOpenGLContext::currentContext();
	_surface = new QOffscreenSurface(nullptr, this);
	if (currentCtx) {
		_surface->setFormat(currentCtx->format());
	}
	_surface->create();

	// brief_render_map() calls anim_render_all(), which requires anim_init()
	// to have initialized the render/free lists.
	anim_init();

	// Create our os::Viewport wrapper so we can use gr_use_viewport().
	_briefingViewport = std::unique_ptr<BriefingViewport>(new BriefingViewport(_surface));

	// Initialize the briefing rendering subsystem.
	// This mirrors what brief_init(true) does in the Lua API path:
	// set the Briefing pointer, init the map (camera, grid, animations),
	// set the initial camera target, and reset icon state.
	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (briefPtr) {
		briefing* savedBriefing = Briefing;
		Briefing = briefPtr;

		brief_init_map();

		if (Briefing->num_stages > 0) {
			brief_set_new_stage(&Briefing->stages[0].camera_pos,
				&Briefing->stages[0].camera_orient, 0, 0);
			brief_reset_icons(0);
		}

		Briefing = savedBriefing;
	}

	// Initialize the camera controller physics so it is ready before the first frame.
	// The dialog's combo-box handlers will call setMovementSpeedScale/setRotationSpeedScale
	// after the widget is shown, which will re-apply the correct speed values.
	_cameraController.resetViewPhysics();

	_initialized = true;
	_renderTimer->start();
}

void BriefingMapWidget::setStage(int stageNum) {
	if (!_initialized)
		return;

	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || stageNum < 0 || stageNum >= briefPtr->num_stages)
		return;

	// Save the Briefing pointer, set it to our WIP data
	briefing* savedBriefing = Briefing;
	Briefing = briefPtr;

	const auto previousStage = _currentStage;
	int transitionTime = briefPtr->stages[stageNum].camera_time;

	if (previousStage >= 0 && previousStage < briefPtr->num_stages && stageNum != previousStage) {
		const auto& prev = briefPtr->stages[previousStage];

		if (std::abs(stageNum - previousStage) > 1) {
			transitionTime = 0;
		} else if (stageNum > previousStage) {
			if (prev.flags & BS_FORWARD_CUT) {
				transitionTime = 0;
			}
		} else {
			if (prev.flags & BS_BACKWARD_CUT) {
				transitionTime = 0;
			} else {
				transitionTime = prev.camera_time;
			}
		}
	}

	if (shouldUseCutTransition(previousStage, stageNum, briefPtr)) {
		_pendingCutStage = stageNum;
		_cutFadeIn = true;
		_cutFadeFrame = 0;
	} else {
		applyStageTransition(stageNum, transitionTime);
	}

	Briefing = savedBriefing;
}

bool BriefingMapWidget::shouldUseCutTransition(int fromStage, int toStage, const briefing* briefPtr) {
	if (briefPtr == nullptr || fromStage < 0 || toStage < 0 || fromStage == toStage) {
		return false;
	}

	if (fromStage >= briefPtr->num_stages || toStage >= briefPtr->num_stages) {
		return false;
	}

	if (std::abs(toStage - fromStage) > 1) {
		return true;
	}

	const auto& prev = briefPtr->stages[fromStage];
	if (toStage > fromStage) {
		return (prev.flags & BS_FORWARD_CUT) != 0;
	}

	return (prev.flags & BS_BACKWARD_CUT) != 0;
}

void BriefingMapWidget::applyStageTransition(int stageNum, int transitionTime) {
	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || stageNum < 0 || stageNum >= briefPtr->num_stages) {
		return;
	}

	briefing* savedBriefing = Briefing;
	Briefing = briefPtr;

	auto& stage = briefPtr->stages[stageNum];
	brief_set_new_stage(&stage.camera_pos, &stage.camera_orient, transitionTime, stageNum);
	// Editor behavior: start highlights as soon as the camera reaches its target
	// instead of waiting for briefing text wipe timing.
	Brief_text_wipe_time_elapsed = BRIEF_TEXT_WIPE_TIME + 1.0f;
	brief_reset_icons(stageNum);
	_currentStage = stageNum;
	_cameraController.resetViewPhysics(); // clear residual velocity from previous stage

	Briefing = savedBriefing;
}

void BriefingMapWidget::updateEditorHighlightPlayback() const {
	if (Briefing == nullptr || _currentStage < 0 || _currentStage >= Briefing->num_stages) {
		return;
	}

	auto& stage = Briefing->stages[_currentStage];
	for (int i = 0; i < stage.num_icons; ++i) {
		auto& icon = stage.icons[i];
		if ((icon.flags & BI_SHOWHIGHLIGHT) == 0 || (icon.flags & BI_HIGHLIGHT) == 0) {
			continue;
		}

		auto& anim = icon.highlight_anim;
		if (anim.first_frame < 0) {
			ensure_highlight_anim_loaded(icon);
			if (icon.highlight_anim.first_frame < 0) {
				continue;
			}
		}

		if (anim.filename[0] == '\0') {
			continue;
		}

		int animW = 0;
		int animH = 0;
		bm_get_info(anim.first_frame, &animW, &animH, nullptr);
		if (icon.scale_factor != 1.0f) {
			animW = fl2i(static_cast<float>(animW) * icon.scale_factor);
			animH = fl2i(static_cast<float>(animH) * icon.scale_factor);
		}

		const int x = fl2i(i2fl(icon.x) + icon.w / 2.0f - animW / 2.0f);
		const int y = fl2i(i2fl(icon.y) + icon.h / 2.0f - animH / 2.0f);
		icon.hold_x = x;
		icon.hold_y = y;
	}
}

QPixmap BriefingMapWidget::checkerboardTile() {
	// A subtle, theme-appropriate checkerboard so the letterbox/pillarbox bars read as matte rather
	// than part of the (pure-black) render. Two close grey shades keep it quiet; the real signal is the
	// contrast between the black map and the grey bars. Rebuilt only when the editor theme flips.
	const bool dark = currentThemeIsDark();
	if (!_checkerTile.isNull() && dark == _checkerTileDark) {
		return _checkerTile;
	}
	_checkerTileDark = dark;

	constexpr int square = 8;
	const QColor light = dark ? QColor(0x36, 0x36, 0x36) : QColor(0xDC, 0xDC, 0xDC);
	const QColor darkc = dark ? QColor(0x2A, 0x2A, 0x2A) : QColor(0xC8, 0xC8, 0xC8);

	QPixmap tile(square * 2, square * 2);
	{
		QPainter p(&tile);
		p.fillRect(tile.rect(), darkc);
		p.fillRect(0, 0, square, square, light);
		p.fillRect(square, square, square, square, light);
	}
	_checkerTile = tile;
	return _checkerTile;
}

void BriefingMapWidget::paintEvent(QPaintEvent* /*event*/) {
	QPainter painter(this);
	// Matte bars: the briefing image drawn over _blitRect below is opaque, so the checkerboard only
	// shows through in the letterbox/pillarbox area around it.
	painter.fillRect(rect(), QBrush(checkerboardTile()));

	if (_frameImage.isNull()) {
		return;
	}

	// Aspect-fit the reference-resolution briefing image into the widget, centered (letterboxed).
	const QSize scaled = _frameImage.size().scaled(size(), Qt::KeepAspectRatio);
	_blitRect = QRect(QPoint((width() - scaled.width()) / 2, (height() - scaled.height()) / 2), scaled);

	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.drawImage(_blitRect, _frameImage);

	drawSelectionBrackets(painter);
}

void BriefingMapWidget::drawSelectionBrackets(QPainter& painter) {
	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (briefPtr == nullptr || _currentStage < 0 || _currentStage >= briefPtr->num_stages) {
		return;
	}
	if (_blitRect.width() <= 0 || _blitRect.height() <= 0 || _lastRenderWidth <= 0 || _lastRenderHeight <= 0) {
		return;
	}

	const auto selectedIcons = _model->getLineSelection();
	auto& stage = briefPtr->stages[_currentStage];
	if (selectedIcons.empty()) {
		return;
	}

	// Icon coordinates are in reference-resolution space; map them through the letterbox rectangle so
	// the (crisp, overlaid) brackets land on the on-screen icons.
	const double scale = static_cast<double>(_blitRect.width()) / static_cast<double>(_lastRenderWidth);
	const auto mapX = [&](double refX) { return _blitRect.x() + static_cast<int>(std::lround(refX * scale)); };
	const auto mapY = [&](double refY) { return _blitRect.y() + static_cast<int>(std::lround(refY * scale)); };

	painter.setPen(QPen(Qt::white, 1));
	for (const auto selected : selectedIcons) {
		if (selected < 0 || selected >= stage.num_icons) {
			continue;
		}

		auto& icon = stage.icons[selected];
		const int left = mapX(icon.x - 2);
		const int top = mapY(icon.y - 2);
		const int right = mapX(icon.x + icon.w + 2);
		const int bottom = mapY(icon.y + icon.h + 2);
		const int cornerLen = std::max(3, std::min(right - left, bottom - top) / 4);

		// Top-left
		painter.drawLine(left, top, left + cornerLen, top);
		painter.drawLine(left, top, left, top + cornerLen);
		// Top-right
		painter.drawLine(right - cornerLen, top, right, top);
		painter.drawLine(right, top, right, top + cornerLen);
		// Bottom-left
		painter.drawLine(left, bottom, left + cornerLen, bottom);
		painter.drawLine(left, bottom - cornerLen, left, bottom);
		// Bottom-right
		painter.drawLine(right - cornerLen, bottom, right, bottom);
		painter.drawLine(right, bottom - cornerLen, right, bottom);
	}
}

void BriefingMapWidget::maybeRenderCutTransition(float frametime, int width, int height) {
	(void)frametime;

	if (!_cutFadeIn) {
		return;
	}

	constexpr int CutFadeFrameCount = 8;
	const auto fadeProgress = static_cast<float>(_cutFadeFrame + 1) / static_cast<float>(CutFadeFrameCount);
	color fadeColor;
	gr_init_alphacolor(&fadeColor, 255, 255, 255, fl2i(fadeProgress * 255.0f));
	gr_set_color_fast(&fadeColor);
	gr_rect(0, 0, width, height, GR_RESIZE_NONE);

	_cutFadeFrame++;
	if (_cutFadeFrame >= CutFadeFrameCount) {
		_cutFadeIn = false;
		_cutFadeFrame = 0;

		if (_pendingCutStage >= 0) {
			applyStageTransition(_pendingCutStage, 0);
			_pendingCutStage = -1;
		}
		return;
	}
}

int BriefingMapWidget::getCurrentStage() const {
	return _currentStage;
}

void BriefingMapWidget::notifyIconVisualsChanged() {
	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || _currentStage < 0 || _currentStage >= briefPtr->num_stages) {
		return;
	}

	briefing* savedBriefing = Briefing;
	Briefing = briefPtr;

	auto& stage = briefPtr->stages[_currentStage];
	brief_reset_last_new_stage();
	brief_set_new_stage(&stage.camera_pos, &stage.camera_orient, 0, _currentStage);
	brief_reset_icons(_currentStage);

	const auto selected = _model->getCurrentIconIndex();
	if (selected >= 0 && selected < stage.num_icons) {
		auto& icon = stage.icons[selected];
		if (icon.flags & BI_HIGHLIGHT) {
			ensure_highlight_anim_loaded(icon);
			icon.highlight_anim.time_elapsed = 0.0f;
			icon.flags |= BI_SHOWHIGHLIGHT;
			brief_cancel_pending_highlight_anims();
		} else {
			icon.flags &= ~BI_SHOWHIGHLIGHT;
		}
	}

	Briefing = savedBriefing;
}

void BriefingMapWidget::applyCameraToCurrentStage(const vec3d& pos, const matrix& orient) {
	applyCameraPoseLikeKeyboardControls(pos, orient, true);
}

void BriefingMapWidget::setMovementSpeedScale(float scale) {
	// Dialog sends 4.0, 8.0, 16.0; map to physicsSpeed 1, 2, 4 preserving the 1:2:4 ratio.
	_cameraController.setPhysicsSpeed(std::max(1, fl2ir(scale / 4.0f)));
}

void BriefingMapWidget::setRotationSpeedScale(float scale) {
	// Dialog sends 0.0625, 0.125, 0.25; map to physicsRot 8, 15, 30 (max_rotvel *= physicsRot/30).
	_cameraController.setPhysicsRot(std::max(1, fl2ir(scale * 120.0f)));
}

void BriefingMapWidget::applyCameraPoseLikeKeyboardControls(const vec3d& camPos, const matrix& camOrient, bool updateModel) {
	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || _currentStage < 0 || _currentStage >= briefPtr->num_stages) {
		return;
	}

	auto& stage = briefPtr->stages[_currentStage];
	stage.camera_pos = camPos;
	stage.camera_orient = camOrient;

	briefing* savedBriefing = Briefing;
	Briefing = briefPtr;
	brief_reset_last_new_stage();
	brief_set_new_stage(&stage.camera_pos, &stage.camera_orient, 0, _currentStage);
	Briefing = savedBriefing;

	if (updateModel) {
		_model->setCameraPosition(camPos);
		_model->setCameraOrientation(camOrient);
	}

	cameraChanged(camPos, camOrient);
}

void BriefingMapWidget::renderFrame() {
	if (!_initialized || !_briefingViewport || _surface == nullptr) {
		return;
	}

	// Guard against re-entrancy.
	if (_rendering)
		return;

	// The main viewport's FredRenderer intentionally leaves a g3 frame open
	// after gr_flip() so that mouse-interaction helpers (select_object, etc.)
	// can use g3_point_to_vec between renders.  We must end that persistent
	// frame before brief_render_map() starts its own, then re-open it when
	// we are done so the main viewport stays in the state it expects.
	const bool mainFrameWasActive = (g3_in_frame() != 0);
	if (mainFrameWasActive) {
		g3_end_frame();
	}

	_rendering = true;

	// Make the engine's GL context current on our off-screen surface so we can render the briefing.
	gr_use_viewport(_briefingViewport.get());
	auto* context = QOpenGLContext::currentContext();

	if (context == nullptr) {
		if (!_loggedNoContext) {
			mprintf(("BriefingMapWidget: no current OpenGL context after gr_use_viewport().\n"));
			_loggedNoContext = true;
		}
		restoreMainViewportFrame(mainFrameWasActive);
		_rendering = false;
		return;
	}

	// Reference resolution the briefing is composed at (see BriefingViewport::getSize).
	const auto refSize = _briefingViewport->getSize();
	const int resW = static_cast<int>(refSize.first);
	const int resH = static_cast<int>(refSize.second);

	// Lazily (re)create the offscreen render target at the reference resolution. Rendering the
	// briefing at a fixed canonical size and scaling the finished image to fit the widget is what
	// makes this a faithful WYSIWYG view: icon sizes, grid, line thickness and text all scale together.
	if (_renderTarget < 0 || _renderTargetW != resW || _renderTargetH != resH) {
		if (_renderTarget >= 0) {
			bm_release(_renderTarget);
		}
		_renderTarget = bm_make_render_target(resW, resH,
			BMP_FLAG_RENDER_TARGET_DYNAMIC | BMP_FLAG_RENDER_TARGET_DEPTH_ATTACHMENT);
		_renderTargetW = resW;
		_renderTargetH = resH;
	}

	if (_renderTarget < 0) {
		if (!_loggedNoRenderTarget) {
			mprintf(("BriefingMapWidget: failed to create %dx%d render target.\n", resW, resH));
			_loggedNoRenderTarget = true;
		}
		restoreMainViewportFrame(mainFrameWasActive);
		_rendering = false;
		return;
	}

	// Icon coordinates come out of brief_render_map() in reference-resolution space.
	_lastRenderWidth = resW;
	_lastRenderHeight = resH;

	// ---- Render the briefing into the off-screen target and read it back into a QImage ----
	// bm_set_render_target() switches gr_screen to the target's own dimensions for us.
	if (bm_set_render_target(_renderTarget)) {
		brief_screen savedBscreen = bscreen;
		bscreen.map_x1 = 0;
		bscreen.map_y1 = 0;
		bscreen.map_x2 = resW;
		bscreen.map_y2 = resH;
		bscreen.resize = GR_RESIZE_NONE;

		briefing* savedBriefing = Briefing;
		Briefing = _model->getWipBriefingPtr(_model->getCurrentTeam());

		gr_reset_clip();
		gr_clear();

		if (Briefing != nullptr) {
			const bool stage_valid = (_currentStage >= 0 && _currentStage < Briefing->num_stages);
			if (!stage_valid) {
				mprintf(("BriefingMapWidget: invalid stage index %d (num_stages=%d)\n", _currentStage, Briefing->num_stages));
			}

			if (stage_valid) {
				const float frametime = 0.033f;
				applyBoundCameraControls(frametime);
				Brief_text_wipe_time_elapsed += frametime;
				brief_camera_move(frametime, _currentStage);
				updateEditorHighlightPlayback();
				brief_render_map(_currentStage, frametime);
				updateEditorHighlightPlayback();
				maybeRenderCutTransition(frametime, resW, resH);
				cameraChanged(brief_get_current_cam_pos(), brief_get_current_cam_orient());
			}
		}

		// Read the finished frame back while the render target is still bound. The briefing is an
		// opaque scene, so use RGBX (ignore the alpha byte) to avoid the background reading as
		// transparent. FSO already renders the target top-down, so no vertical flip is needed.
		QImage frame(resW, resH, QImage::Format_RGBX8888);
		context->functions()->glReadPixels(0, 0, resW, resH, GL_RGBA, GL_UNSIGNED_BYTE, frame.bits());
		_frameImage = frame.copy();

		Briefing = savedBriefing;
		bscreen = savedBscreen;

		bm_set_render_target(-1);
	}

	// Restore the main viewport's persistent frame so its mouse-interaction helpers keep working.
	restoreMainViewportFrame(mainFrameWasActive);

	_rendering = false;

	// Repaint the widget with the freshly read-back frame (selection brackets are drawn there).
	update();
}

void BriefingMapWidget::restoreMainViewportFrame(bool wasActive) {
	if (!wasActive) {
		return;
	}
	auto* mainView = _viewport->renderer->getTargetViewport();
	auto mainSize = mainView->getSize();
	gr_use_viewport(mainView);
	gr_screen_resize(static_cast<int>(mainSize.first) * devicePixelRatio(),
		static_cast<int>(mainSize.second) * devicePixelRatio());
	g3_start_frame(0);
	g3_set_view_matrix(&_viewport->camera.eye_pos, &_viewport->camera.eye_orient, 0.5f);
}

bool BriefingMapWidget::event(QEvent* evt) {
	if (evt->type() == QEvent::ShortcutOverride) {
		auto* keyEvent = static_cast<QKeyEvent*>(evt);
		if (ControlBindings::instance().matches(keyEvent)) {
			evt->accept();
			return true;
		}
	}

	return QWidget::event(evt);
}

void BriefingMapWidget::keyPressEvent(QKeyEvent* event) {
	if (!_initialized) {
		QWidget::keyPressEvent(event);
		return;
	}

	if (!ControlBindings::instance().handleKeyPress(event)) {
		QWidget::keyPressEvent(event);
		return;
	}
	event->accept();
}

void BriefingMapWidget::keyReleaseEvent(QKeyEvent* event) {
	if (!_initialized) {
		QWidget::keyReleaseEvent(event);
		return;
	}

	if (!ControlBindings::instance().handleKeyRelease(event)) {
		QWidget::keyReleaseEvent(event);
		return;
	}
	event->accept();
}

void BriefingMapWidget::applyBoundCameraControls(float frametime) {
	// Sync from briefing globals each frame so externally-driven moves
	// (stage transitions, paste, coordinates dialog) are picked up before
	// applying user input via the shared CameraController.
	_cameraController.view_pos    = brief_get_current_cam_pos();
	_cameraController.view_orient = brief_get_current_cam_orient();

	if (_cameraController.processControls(
			&_cameraController.view_pos,
			&_cameraController.view_orient,
			frametime,
			false)) {
		applyCameraPoseLikeKeyboardControls(
			_cameraController.view_pos,
			_cameraController.view_orient,
			true);
	}
}

void BriefingMapWidget::mousePressEvent(QMouseEvent* event) {
	if (!_initialized || event->button() != Qt::LeftButton)
		return;

	_dragStartMousePos = event->position();

	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || _currentStage < 0 || _currentStage >= briefPtr->num_stages || _lastRenderWidth <= 0 || _lastRenderHeight <= 0 ||
		_blitRect.width() <= 0 || _blitRect.height() <= 0) {
		_draggingIcon = false;
		_dragIconIndex = -1;
		return;
	}

	// Map the logical mouse position into reference-resolution space: shift by the letterbox origin,
	// then scale from the (logical) blit rectangle to the render-target size. QWidget coordinates and
	// _blitRect are both logical, so no device-pixel-ratio factor is needed here.
	const auto mouseX = (static_cast<float>(event->position().x()) - static_cast<float>(_blitRect.x())) *
						(static_cast<float>(_lastRenderWidth) / static_cast<float>(_blitRect.width()));
	const auto mouseY = (static_cast<float>(event->position().y()) - static_cast<float>(_blitRect.y())) *
						(static_cast<float>(_lastRenderHeight) / static_cast<float>(_blitRect.height()));

	auto& stage = briefPtr->stages[_currentStage];

	// Collect every icon under the cursor, top-most first (higher index = drawn later = on top).
	SCP_vector<int> hits;
	for (int i = stage.num_icons - 1; i >= 0; --i) {
		auto& icon = stage.icons[i];

		int iconW = 0, iconH = 0;
		brief_common_get_icon_dimensions(&iconW, &iconH, &icon);
		const auto scaledW = static_cast<float>((icon.w > 0) ? icon.w : fl2i(static_cast<float>(iconW) * icon.scale_factor));
		const auto scaledH = static_cast<float>((icon.h > 0) ? icon.h : fl2i(static_cast<float>(iconH) * icon.scale_factor));
		const auto left = static_cast<float>(icon.x);
		const auto top = static_cast<float>(icon.y);

		if (mouseX >= left && mouseX <= left + scaledW && mouseY >= top && mouseY <= top + scaledH) {
			hits.push_back(i);
		}
	}

	const bool shiftHeld = (event->modifiers() & Qt::ShiftModifier) != 0;

	if (hits.empty()) {
		_draggingIcon = false;
		_dragIconIndex = -1;
		if (!shiftHeld) {
			Q_EMIT iconSelected(-1, false);
		}
		return;
	}

	int pickedIndex = hits.front(); // default: the top-most icon under the cursor
	if (!shiftHeld) {
		// Rolling select: if the currently-selected icon is one of the stacked hits, advance to the
		// next one underneath (wrapping bottom -> top) so repeated clicks cycle the whole stack.
		const int current = _model->getCurrentIconIndex();
		for (size_t k = 0; k < hits.size(); ++k) {
			if (hits[k] == current) {
				pickedIndex = hits[(k + 1) % hits.size()];
				break;
			}
		}
	}

	_draggingIcon = true;
	_dragIconIndex = pickedIndex;
	_dragStartIconPos = stage.icons[pickedIndex].pos;
	brief_move_icon_reset();
	Q_EMIT iconSelected(pickedIndex, shiftHeld);
}

void BriefingMapWidget::mouseMoveEvent(QMouseEvent* event) {
	if (!_initialized || !_draggingIcon || _dragIconIndex < 0 || !(event->buttons() & Qt::LeftButton))
		return;

	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || _currentStage < 0 || _currentStage >= briefPtr->num_stages || _dragIconIndex >= briefPtr->stages[_currentStage].num_icons ||
		_lastRenderWidth <= 0 || _lastRenderHeight <= 0 || _blitRect.width() <= 0 || _blitRect.height() <= 0) {
		return;
	}

	// Convert the logical mouse delta into reference-resolution pixels (scaled from the logical
	// letterbox rectangle to the render-target size). The letterbox offset cancels in a delta.
	const auto scaleX = static_cast<float>(_lastRenderWidth) / static_cast<float>(_blitRect.width());
	const auto scaleY = static_cast<float>(_lastRenderHeight) / static_cast<float>(_blitRect.height());
	const auto deltaX = static_cast<float>(event->position().x() - _dragStartMousePos.x()) * scaleX;
	const auto deltaY = static_cast<float>(event->position().y() - _dragStartMousePos.y()) * scaleY;

	const auto camPos = brief_get_current_cam_pos();
	const auto camOrient = brief_get_current_cam_orient();
	const auto& currentIcon = briefPtr->stages[_currentStage].icons[_dragIconIndex];

	vec3d toIcon;
	vm_vec_sub(&toIcon, &currentIcon.pos, &camPos);
	const auto depth = vm_vec_dot(&toIcon, &camOrient.vec.fvec);
	if (depth <= 1.0f) {
		return;
	}

	const auto horizontalFov = g3_get_hfov(Proj_fov);
	const auto worldPerPixelX = (2.0f * depth * std::tan(horizontalFov / 2.0f)) / static_cast<float>(_lastRenderWidth);
	const auto worldPerPixelY = worldPerPixelX;
	constexpr float DragResponseScale = 1.5f; // This is kind hacky but it makes the drag feel more responsive without having to move the mouse as far, which is nice given the precision required to drag small icons.

	vec3d newPos = _dragStartIconPos;
	vm_vec_scale_add2(&newPos, &camOrient.vec.rvec, deltaX * worldPerPixelX * DragResponseScale);
	vm_vec_scale_add2(&newPos, &camOrient.vec.uvec, -deltaY * worldPerPixelY * DragResponseScale);
	_model->setIconPosition(newPos);
}

void BriefingMapWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (!_initialized || event->button() != Qt::LeftButton)
		return;

	_draggingIcon = false;
	_dragIconIndex = -1;
}

} // namespace fso::fred
