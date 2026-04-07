#include "BriefingMapWidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QtGui/QOpenGLContext>
#include <QtWidgets/QHBoxLayout>

#include "FredApplication.h"
#include "anim/animplay.h"
#include "bmpman/bmpman.h"
#include "mission/dialogs/BriefingEditorDialogModel.h"
#include "mission/EditorViewport.h"
#include "ui/ControlBindings.h"

#include "graphics/2d.h"
#include "render/3d.h"
#include "mission/missionbriefcommon.h"

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

// ---- BriefingMapWindow ----

BriefingMapWindow::BriefingMapWindow(QWidget* parentWidget)
	: QWindow(static_cast<QWindow*>(nullptr)), _parentWidget(parentWidget)
{
	setSurfaceType(QWindow::OpenGLSurface);
}

void BriefingMapWindow::initializeGL(const QSurfaceFormat& fmt) {
	setFormat(fmt);
	create();
}

bool BriefingMapWindow::event(QEvent* evt) {
	switch (evt->type()) {
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseMove:
		// Forward input events to the parent widget
		qGuiApp->sendEvent(_parentWidget, evt);
		return true;
	default:
		return QWindow::event(evt);
	}
}

void BriefingMapWindow::exposeEvent(QExposeEvent* event) {
	if (isExposed()) {
		requestUpdate();
		event->accept();
	} else {
		QWindow::exposeEvent(event);
	}
}

// ---- BriefingViewport ----

BriefingViewport::BriefingViewport(BriefingMapWindow* window) : _window(window) {
}

SDL_Window* BriefingViewport::toSDLWindow() {
	return nullptr;
}

std::pair<uint32_t, uint32_t> BriefingViewport::getSize() {
	return std::make_pair(static_cast<uint32_t>(_window->width()),
		static_cast<uint32_t>(_window->height()));
}

void BriefingViewport::swapBuffers() {
	if (_window->isExposed()) {
		QOpenGLContext::currentContext()->swapBuffers(_window);
	}
}

void BriefingViewport::setState(os::ViewportState /*state*/) {
}

void BriefingViewport::minimize() {
}

void BriefingViewport::restore() {
}

QSurface* BriefingViewport::getRenderSurface() {
	return _window;
}

// ---- BriefingMapWidget ----

BriefingMapWidget::BriefingMapWidget(QWidget* parent,
	dialogs::BriefingEditorDialogModel* model,
	EditorViewport* viewport)
	: QWidget(parent), _model(model), _viewport(viewport)
{
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	_window = new BriefingMapWindow(this);

	auto layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->addWidget(QWidget::createWindowContainer(_window, this));
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	_renderTimer = new QTimer(this);
	_renderTimer->setInterval(33); // ~30 fps
	connect(_renderTimer, &QTimer::timeout, this, &BriefingMapWidget::renderFrame);

	fredApp->runAfterInit([this]() { initBriefingMap(); });
}

BriefingMapWidget::~BriefingMapWidget() {
	_renderTimer->stop();
}

void BriefingMapWidget::initBriefingMap() {
	// Get the surface format from the current GL context and initialize our window
	auto* currentCtx = QOpenGLContext::currentContext();
	if (currentCtx) {
		_window->initializeGL(currentCtx->format());
	}

	// brief_render_map() calls anim_render_all(), which requires anim_init()
	// to have initialized the render/free lists.
	anim_init();

	_diagnosticContext.reset(new QOpenGLContext());
	if (currentCtx) {
		_diagnosticContext->setShareContext(currentCtx);
		_diagnosticContext->setFormat(currentCtx->format());
	} else {
		_diagnosticContext->setFormat(_window->requestedFormat());
	}
	if (!_diagnosticContext->create()) {
		mprintf(("BriefingMapWidget: failed to create dedicated diagnostic GL context.\n"));
		_diagnosticContext.reset();
	} else {
		mprintf(("BriefingMapWidget: dedicated diagnostic GL context created.\n"));
	}

	// Create our os::Viewport wrapper so we can use gr_use_viewport() / gr_flip()
	_briefingViewport = std::unique_ptr<BriefingViewport>(new BriefingViewport(_window));

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

	_initialized = true;
	_renderTimer->start();
	mprintf(("BriefingMapWidget: init complete, timer started for render diagnostics.\n"));
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

void BriefingMapWidget::drawSelectedIconOutline() {
	if (Briefing == nullptr || _currentStage < 0 || _currentStage >= Briefing->num_stages) {
		return;
	}

	const auto selectedIcons = _model->getLineSelection();
	auto& stage = Briefing->stages[_currentStage];
	if (selectedIcons.empty()) {
		return;
	}

	gr_set_color(255, 255, 255);
	for (const auto selected : selectedIcons) {
		if (selected < 0 || selected >= stage.num_icons) {
			continue;
		}

		auto& icon = stage.icons[selected];
		const auto left = icon.x - 2;
		const auto top = icon.y - 2;
		const auto right = left + icon.w + 4;
		const auto bottom = top + icon.h + 4;
		const auto width = right - left;
		const auto height = bottom - top;
		const auto cornerLen = std::max(3, std::min(width, height) / 4);

		// Top-left
		gr_line(left, top, left + cornerLen, top);
		gr_line(left, top, left, top + cornerLen);
		// Top-right
		gr_line(right - cornerLen, top, right, top);
		gr_line(right, top, right, top + cornerLen);
		// Bottom-left
		gr_line(left, bottom, left + cornerLen, bottom);
		gr_line(left, bottom - cornerLen, left, bottom);
		// Bottom-right
		gr_line(right - cornerLen, bottom, right, bottom);
		gr_line(right, bottom - cornerLen, right, bottom);
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
	_movementSpeedScale = std::max(0.01f, scale);
}

void BriefingMapWidget::setRotationSpeedScale(float scale) {
	_rotationSpeedScale = std::max(0.01f, scale);
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

QWindow* BriefingMapWidget::getRenderWindow() const {
	return _window;
}

void BriefingMapWidget::renderFrame() {
	if (!_initialized) {
		if (!_loggedNotInitialized) {
			mprintf(("BriefingMapWidget: render skipped because widget is not initialized.\n"));
			_loggedNotInitialized = true;
		}
		return;
	}

	if (!_window->isExposed()) {
		if (!_loggedNotExposed) {
			mprintf(("BriefingMapWidget: render skipped because window is not exposed.\n"));
			_loggedNotExposed = true;
		}
		return;
	}

	if (!_briefingViewport) {
		if (!_loggedNoViewport) {
			mprintf(("BriefingMapWidget: render skipped because briefing viewport is null.\n"));
			_loggedNoViewport = true;
		}
		return;
	}

	// Guard against re-entrancy: swapBuffers() can pump the Qt event loop on Windows,
	// which may fire our timer again while we're still mid-render.
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

	// Render briefing content through the normal graphics pipeline so gr_flip()
	// performs any required FBO resolve/blit before presenting.
	gr_use_viewport(_briefingViewport.get());
	auto* context = QOpenGLContext::currentContext();

	if (context == nullptr) {
		if (!_loggedNoContext) {
			mprintf(("BriefingMapWidget: no current OpenGL context after gr_use_viewport().\n"));
			_loggedNoContext = true;
		}
		// Restore main viewport frame before bailing out
		if (mainFrameWasActive) {
			auto* mainView = _viewport->renderer->getTargetViewport();
			auto mainSize = mainView->getSize();
			gr_use_viewport(mainView);
			gr_screen_resize(static_cast<int>(mainSize.first), static_cast<int>(mainSize.second));
			g3_start_frame(0);
			g3_set_view_matrix(&_viewport->eye_pos, &_viewport->eye_orient, 0.5f);
		}
		_rendering = false;
		return;
	}

	if (context->surface() != _window) {
		if (!_loggedSurfaceMismatch) {
			mprintf(("BriefingMapWidget: surface mismatch before clear (current=%p target=%p).\n",
				static_cast<void*>(context->surface()),
				static_cast<void*>(_window)));
			_loggedSurfaceMismatch = true;
		}
	}

	auto viewSize = _briefingViewport->getSize();
	const int w = static_cast<int>(viewSize.first);
	const int h = static_cast<int>(viewSize.second);
	_lastRenderWidth = w;
	_lastRenderHeight = h;

	gr_screen_resize(w, h);

	brief_screen savedBscreen = bscreen;
	bscreen.map_x1 = 0;
	bscreen.map_y1 = 0;
	bscreen.map_x2 = w;
	bscreen.map_y2 = h;
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

		if ((_debugFrameCounter % 120) == 0 && stage_valid) {
			const auto& stage = Briefing->stages[_currentStage];
			mprintf(("BriefingMapWidget: stage=%d/%d draw_grid=%d num_icons=%d num_lines=%d cam_time=%d\n",
				_currentStage,
				Briefing->num_stages,
				stage.draw_grid ? 1 : 0,
				stage.num_icons,
				stage.num_lines,
				stage.camera_time));
		}

		if (stage_valid) {
			const float frametime = 0.033f;
			applyBoundCameraControls(frametime);
			Brief_text_wipe_time_elapsed += frametime;
			brief_camera_move(frametime, _currentStage);
			updateEditorHighlightPlayback();
			brief_render_map(_currentStage, frametime);
			updateEditorHighlightPlayback();
			drawSelectedIconOutline();
			maybeRenderCutTransition(frametime, w, h);
			cameraChanged(brief_get_current_cam_pos(), brief_get_current_cam_orient());
		}
	}

	Briefing = savedBriefing;
	bscreen = savedBscreen;

	gr_flip();
	_debugFrameCounter++;

	if ((_debugFrameCounter % 120) == 0) {
		mprintf(("BriefingMapWidget: rendered briefing frame=%u size=%dx%d current_surface=%p\n",
			_debugFrameCounter,
			w,
			h,
			static_cast<void*>(context->surface())));
	}

	// Restore the main viewport's persistent frame so that mouse-interaction
	// helpers (select_object → g3_point_to_vec) continue to work between the
	// main viewport's own render calls.
	if (mainFrameWasActive) {
		auto* mainView = _viewport->renderer->getTargetViewport();
		auto mainSize = mainView->getSize();
		gr_use_viewport(mainView);
		gr_screen_resize(static_cast<int>(mainSize.first), static_cast<int>(mainSize.second));
		g3_start_frame(0);
		g3_set_view_matrix(&_viewport->eye_pos, &_viewport->eye_orient, 0.5f);
	}

	_rendering = false;
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
	auto& bindings = ControlBindings::instance();
	vec3d camPos = brief_get_current_cam_pos();
	matrix camOrient = brief_get_current_cam_orient();
	const auto oldPos = camPos;
	const auto oldOrient = camOrient;

	vec3d movementVec = ZERO_VECTOR;
	angles rotangs{};

	movementVec.xyz.x += bindings.isPressed(ControlAction::MoveLeft) ? -1.0f : 0.0f;
	movementVec.xyz.x += bindings.isPressed(ControlAction::MoveRight) ? 1.0f : 0.0f;
	movementVec.xyz.y += bindings.isPressed(ControlAction::MoveForward) ? 1.0f : 0.0f;
	movementVec.xyz.y += bindings.isPressed(ControlAction::MoveBackward) ? -1.0f : 0.0f;
	movementVec.xyz.z += bindings.isPressed(ControlAction::MoveUp) ? 1.0f : 0.0f;
	movementVec.xyz.z += bindings.isPressed(ControlAction::MoveDown) ? -1.0f : 0.0f;
	rotangs.h += bindings.isPressed(ControlAction::YawLeft) ? -0.1f * _rotationSpeedScale : 0.0f;
	rotangs.h += bindings.isPressed(ControlAction::YawRight) ? 0.1f * _rotationSpeedScale : 0.0f;
	rotangs.p += bindings.isPressed(ControlAction::PitchUp) ? -0.1f * _rotationSpeedScale : 0.0f;
	rotangs.p += bindings.isPressed(ControlAction::PitchDown) ? 0.1f * _rotationSpeedScale : 0.0f;

	const auto frameScale = std::max(frametime * 30.0f * _movementSpeedScale, 0.0f);
	if (movementVec.xyz.x != 0.0f) {
		vm_vec_scale_add2(&camPos, &camOrient.vec.rvec, movementVec.xyz.x * frameScale);
	}
	if (movementVec.xyz.y != 0.0f) {
		vm_vec_scale_add2(&camPos, &camOrient.vec.fvec, movementVec.xyz.y * frameScale);
	}
	if (movementVec.xyz.z != 0.0f) {
		vm_vec_scale_add2(&camPos, &camOrient.vec.uvec, movementVec.xyz.z * frameScale);
	}

	if (rotangs.p != 0.0f || rotangs.h != 0.0f || rotangs.b != 0.0f) {
		matrix rotmat;
		matrix newmat;
		vm_angles_2_matrix(&rotmat, &rotangs);
		vm_matrix_x_matrix(&newmat, &camOrient, &rotmat);
		camOrient = newmat;
	}

	if (vm_vec_cmp(&oldPos, &camPos) || vm_matrix_cmp(&oldOrient, &camOrient)) {
		applyCameraPoseLikeKeyboardControls(camPos, camOrient, true);
	}
}

void BriefingMapWidget::mousePressEvent(QMouseEvent* event) {
	if (!_initialized || event->button() != Qt::LeftButton)
		return;

	_lastMousePos = event->pos();
	_dragStartMousePos = event->localPos();

	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || _currentStage < 0 || _currentStage >= briefPtr->num_stages || _lastRenderWidth <= 0 || _lastRenderHeight <= 0 ||
		width() <= 0 || height() <= 0) {
		_draggingIcon = false;
		_dragIconIndex = -1;
		return;
	}

	const auto mouseX = static_cast<float>(event->localPos().x()) * (static_cast<float>(_lastRenderWidth) / static_cast<float>(width()));
	const auto mouseY = static_cast<float>(event->localPos().y()) * (static_cast<float>(_lastRenderHeight) / static_cast<float>(height()));

	auto& stage = briefPtr->stages[_currentStage];
	int hitIndex = -1;
	for (int i = stage.num_icons - 1; i >= 0; --i) {
		auto& icon = stage.icons[i];

		int iconW = 0, iconH = 0;
		brief_common_get_icon_dimensions(&iconW, &iconH, &icon);
		const auto scaledW = static_cast<float>((icon.w > 0) ? icon.w : fl2i(static_cast<float>(iconW) * icon.scale_factor));
		const auto scaledH = static_cast<float>((icon.h > 0) ? icon.h : fl2i(static_cast<float>(iconH) * icon.scale_factor));
		const auto left = static_cast<float>(icon.x);
		const auto top = static_cast<float>(icon.y);

		if (mouseX >= left && mouseX <= left + scaledW && mouseY >= top && mouseY <= top + scaledH) {
			hitIndex = i;
			break;
		}
	}

	if (hitIndex >= 0) {
		_draggingIcon = true;
		_dragIconIndex = hitIndex;
		_dragStartIconPos = stage.icons[hitIndex].pos;
		brief_move_icon_reset();
		Q_EMIT iconSelected(hitIndex, (event->modifiers() & Qt::ShiftModifier) != 0);
	} else {
		_draggingIcon = false;
		_dragIconIndex = -1;
		if ((event->modifiers() & Qt::ShiftModifier) == 0) {
			Q_EMIT iconSelected(-1, false);
		}
	}
}

void BriefingMapWidget::mouseMoveEvent(QMouseEvent* event) {
	if (!_initialized || !_draggingIcon || _dragIconIndex < 0 || !(event->buttons() & Qt::LeftButton))
		return;

	auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
	if (!briefPtr || _currentStage < 0 || _currentStage >= briefPtr->num_stages || _dragIconIndex >= briefPtr->stages[_currentStage].num_icons ||
		_lastRenderWidth <= 0 || _lastRenderHeight <= 0 || width() <= 0 || height() <= 0) {
		return;
	}

	const auto scaleX = static_cast<float>(_lastRenderWidth) / static_cast<float>(width());
	const auto scaleY = static_cast<float>(_lastRenderHeight) / static_cast<float>(height());
	const auto deltaX = static_cast<float>(event->localPos().x() - _dragStartMousePos.x()) * scaleX;
	const auto deltaY = static_cast<float>(event->localPos().y() - _dragStartMousePos.y()) * scaleY;

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

	_lastMousePos = event->pos();
}

void BriefingMapWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (!_initialized || event->button() != Qt::LeftButton)
		return;

	_draggingIcon = false;
	_dragIconIndex = -1;
}

} // namespace fso::fred
