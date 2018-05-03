//
//

#include "QtGraphicsOperations.h"

#include "widgets/renderwidget.h"

#include <QtWidgets/QtWidgets>

#include "FredApplication.h"

namespace {

QSurfaceFormat getSurfaceFormat(const os::ViewPortProperties& viewProps, const os::OpenGLContextAttributes& glAttrs) {
	QSurfaceFormat format;

	format.setRedBufferSize(viewProps.pixel_format.red_size);
	format.setGreenBufferSize(viewProps.pixel_format.green_size);
	format.setBlueBufferSize(viewProps.pixel_format.blue_size);
	format.setAlphaBufferSize(viewProps.pixel_format.alpha_size);

	format.setDepthBufferSize(viewProps.pixel_format.depth_size);
	format.setStencilBufferSize(viewProps.pixel_format.stencil_size);

	format.setSamples(viewProps.pixel_format.multi_samples);

	format.setRenderableType(QSurfaceFormat::OpenGL);

	switch(glAttrs.profile) {
	case os::OpenGLProfile::Core:
		format.setProfile(QSurfaceFormat::CoreProfile);
		break;
	case os::OpenGLProfile::Compatibility:
		format.setProfile(QSurfaceFormat::CompatibilityProfile);
		break;
	}

	if (glAttrs.flags[os::OpenGLContextFlags::Debug]) {
		format.setOption(QSurfaceFormat::DebugContext);
	}
	if (!glAttrs.flags[os::OpenGLContextFlags::ForwardCompatible]) {
		format.setOption(QSurfaceFormat::DeprecatedFunctions);
	}

	format.setMajorVersion(glAttrs.major_version);
	format.setMinorVersion(glAttrs.minor_version);

	return format;
}

}

namespace fso {
namespace fred {


QtGraphicsOperations::QtGraphicsOperations(Editor* editor) : _editor(editor) {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		Error(LOCATION, "Couldn't init SDL video: %s", SDL_GetError());
		return;
	}
}
QtGraphicsOperations::~QtGraphicsOperations() {
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

std::unique_ptr<os::OpenGLContext>
QtGraphicsOperations::createOpenGLContext(os::Viewport* viewport, const os::OpenGLContextAttributes& gl_attrs) {
	auto qtPort = static_cast<QtViewport*>(viewport);

	std::unique_ptr<QOpenGLContext> context(new QOpenGLContext());
	context->setFormat(getSurfaceFormat(qtPort->getViewProperties(), gl_attrs));

	if (!context->create()) {
		return nullptr;
	}

	return std::unique_ptr<os::OpenGLContext>(new QtOpenGLContext(std::move(context)));
}

void QtGraphicsOperations::makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) {
	auto qtPort = static_cast<QtViewport*>(view);
	auto qtContext = static_cast<QtOpenGLContext*>(ctx);

	if (qtPort == nullptr && qtContext == nullptr) {
		if (_lastContext != nullptr) {
			_lastContext->makeCurrent(nullptr);
		}
	} else {
		qtContext->makeCurrent(qtPort->getWindow()->getRenderSurface());
	}

	// We keep track of our last context since the qt information may return contexts managed by the GUI framework
	_lastContext = qtContext;
}

QtViewport::QtViewport(std::unique_ptr<FredView>&& window, const os::ViewPortProperties& viewProps) :
	_viewProps(viewProps) {
	_viewportWindow = std::move(window);
}
QtViewport::~QtViewport() {
}

std::unique_ptr<os::Viewport> QtGraphicsOperations::createViewport(const os::ViewPortProperties& props) {
	std::unique_ptr<FredView> mw(new FredView());
	mw->getRenderWidget()->setSurfaceFormat(getSurfaceFormat(props, props.gl_attributes));

	auto viewPtr = mw.get();
	auto view = std::unique_ptr<os::Viewport>(new QtViewport(std::move(mw), props));

	auto renderer = _editor->createEditorViewport(view.get());
	viewPtr->setEditor(_editor, renderer);

	if (fredApp->isInitializeComplete()) {
		// Only show new viewports if the initialization has already been completed
		// The windows created at program start will only be shown once initialization is completed
		viewPtr->show();
	}

	return view;
}

SDL_Window* QtViewport::toSDLWindow() {
	return nullptr;
}
std::pair<uint32_t, uint32_t> QtViewport::getSize() {
	auto size = _viewportWindow->getRenderSurface()->size();

	return std::make_pair((uint32_t) size.width(), (uint32_t) size.height());
}
void QtViewport::swapBuffers() {
	if (_viewportWindow->getRenderWidget()->isVisible()) {
		QOpenGLContext::currentContext()->swapBuffers(_viewportWindow->getRenderSurface());
	}
}
void QtViewport::setState(os::ViewportState  /*state*/) {
	// Not used in FRED
}
void QtViewport::minimize() {
	_viewportWindow->showMinimized();
}
void QtViewport::restore() {
	_viewportWindow->show();
}
const os::ViewPortProperties& QtViewport::getViewProperties() const {
	return _viewProps;
}
FredView* QtViewport::getWindow() {
	return _viewportWindow.get();
}

static void* openglFunctionLoader(const char* name) {
	auto currentCtx = QOpenGLContext::currentContext();

	if (currentCtx == nullptr) {
		return nullptr;
	}

	return reinterpret_cast<void*>(currentCtx->getProcAddress(name));
}

QtOpenGLContext::QtOpenGLContext(std::unique_ptr<QOpenGLContext>&& context) : _context(std::move(context)) {
}
QtOpenGLContext::~QtOpenGLContext() {
}
os::OpenGLLoadProc QtOpenGLContext::getLoaderFunction() {
	return openglFunctionLoader;
}
void QtOpenGLContext::setSwapInterval(int) {
	// Not used at the moment, should default to vsync enabled
}
void QtOpenGLContext::makeCurrent(QSurface* surface) {
	_context->makeCurrent(surface);
}
}
}
