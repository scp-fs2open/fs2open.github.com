#if defined(_MSC_VER) && _MSC_VER <= 1920
	// work around MSVC 2015 and 2017 compiler bug
	// https://bugreports.qt.io/browse/QTBUG-72073
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "BriefingMapWidget.h"

#include <cstdlib>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QtGui/QOpenGLContext>
#include <QtWidgets/QHBoxLayout>

#include "FredApplication.h"
#include "anim/animplay.h"
#include "mission/dialogs/BriefingEditorDialogModel.h"
#include "mission/EditorViewport.h"

#include "graphics/2d.h"
#include "render/3d.h"
#include "mission/missionbriefcommon.h"

namespace fso::fred {

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

	// Mirror in-game transition timing logic so icon movement/fades and camera
	// transitions behave as expected when stepping stages in the editor preview.
	if (previousStage >= 0 && previousStage < briefPtr->num_stages && stageNum != previousStage) {
		const auto& prev = briefPtr->stages[previousStage];

		// Cut transitions (or large jumps) are immediate stage changes.
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

	auto& stage = briefPtr->stages[stageNum];
	brief_set_new_stage(&stage.camera_pos, &stage.camera_orient, transitionTime, stageNum);
	brief_reset_icons(stageNum);

	_currentStage = stageNum;

	Briefing = savedBriefing;
}

int BriefingMapWidget::getCurrentStage() const {
	return _currentStage;
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

	// brief_render_map() enters a 3D frame internally. If another frame is
	// already active, it will assert (G3_count != 0), so skip this tick.
	if (g3_in_frame()) {
		if (!_loggedInFrameSkip) {
			mprintf(("BriefingMapWidget: g3_in_frame() is true; ending active frame before briefing render.\n"));
			_loggedInFrameSkip = true;
		}
		g3_end_frame();
		if (g3_in_frame()) {
			return;
		}
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

		const float frametime = 0.033f;
		Brief_text_wipe_time_elapsed += frametime;
		brief_camera_move(frametime, _currentStage);
		brief_render_map(_currentStage, frametime);
		cameraChanged(brief_get_current_cam_pos(), brief_get_current_cam_orient());
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

	_rendering = false;
}

void BriefingMapWidget::keyPressEvent(QKeyEvent* event) {
	if (!_initialized)
		return;

	// Temporary keyboard camera controls
	vec3d camPos = brief_get_current_cam_pos();
	matrix camOrient = brief_get_current_cam_orient();
	bool moved = false;
	const float PAN_SPEED = 50.0f;
	const float ZOOM_SPEED = 100.0f;

	switch (event->key()) {
	case Qt::Key_Left:
		vm_vec_scale_add2(&camPos, &camOrient.vec.rvec, -PAN_SPEED);
		moved = true;
		break;
	case Qt::Key_Right:
		vm_vec_scale_add2(&camPos, &camOrient.vec.rvec, PAN_SPEED);
		moved = true;
		break;
	case Qt::Key_Up:
		vm_vec_scale_add2(&camPos, &camOrient.vec.uvec, PAN_SPEED);
		moved = true;
		break;
	case Qt::Key_Down:
		vm_vec_scale_add2(&camPos, &camOrient.vec.uvec, -PAN_SPEED);
		moved = true;
		break;
	case Qt::Key_Plus:
	case Qt::Key_Equal:
		vm_vec_scale_add2(&camPos, &camOrient.vec.fvec, ZOOM_SPEED);
		moved = true;
		break;
	case Qt::Key_Minus:
		vm_vec_scale_add2(&camPos, &camOrient.vec.fvec, -ZOOM_SPEED);
		moved = true;
		break;
	case Qt::Key_PageUp:
		camPos.xyz.y += PAN_SPEED;
		moved = true;
		break;
	case Qt::Key_PageDown:
		camPos.xyz.y -= PAN_SPEED;
		moved = true;
		break;
	default:
		QWidget::keyPressEvent(event);
		return;
	}

	if (moved) {
		// Apply instant camera move by setting new stage with time=0
		auto* briefPtr = _model->getWipBriefingPtr(_model->getCurrentTeam());
		if (briefPtr && _currentStage >= 0 && _currentStage < briefPtr->num_stages) {
			auto& stage = briefPtr->stages[_currentStage];
			stage.camera_pos = camPos;

			briefing* savedBriefing = Briefing;
			Briefing = briefPtr;
			// Reset Last_new_stage so brief_set_new_stage accepts the same stage
			brief_reset_last_new_stage();
			brief_set_new_stage(&camPos, &camOrient, 0, _currentStage);
			Briefing = savedBriefing;

			_model->setCameraPosition(camPos);
			cameraChanged(camPos, camOrient);
		}
	}
}

void BriefingMapWidget::mousePressEvent(QMouseEvent* event) {
	if (!_initialized || event->button() != Qt::LeftButton)
		return;

	_lastMousePos = event->pos();

	// Hit-test icons: check which icon (if any) is under the cursor
	// For now, just store the click position for potential drag operations
	// Full icon hit-testing will be implemented once we can retrieve projected icon positions
	_draggingIcon = false;
	_dragIconIndex = -1;
}

void BriefingMapWidget::mouseMoveEvent(QMouseEvent* event) {
	if (!_initialized)
		return;

	_lastMousePos = event->pos();

	// Icon dragging will be implemented once we have icon screen positions from the renderer
}

void BriefingMapWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (!_initialized || event->button() != Qt::LeftButton)
		return;

	_draggingIcon = false;
	_dragIconIndex = -1;
}

} // namespace fso::fred
