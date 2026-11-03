//
//

#if defined(_MSC_VER) && _MSC_VER <= 1920
	// work around MSVC 2015 and 2017 compiler bug
	// https://bugreports.qt.io/browse/QTBUG-72073
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "QtGraphicsOperations.h"

#include "widgets/renderwidget.h"

#include <QtWidgets/QtWidgets>

#include "FredApplication.h"

#include <SDL3/SDL_vulkan.h>

#if QTFRED_HAS_VULKAN
#include <vulkan/vulkan_core.h>
#endif

namespace {

#if QTFRED_HAS_VULKAN
/**
 * @brief The surface extension the current Qt platform plugin's windows need
 *
 * Deliberately derived from Qt rather than from SDL_Vulkan_GetInstanceExtensions(): SDL reports
 * what *its* video driver would need, and the two can disagree -- SDL happily picks Wayland while
 * Qt runs under XCB, which is exactly what qtFRED's own XWayland workaround asks users to do. The
 * surface comes from Qt, so the extension has to as well.
 *
 * @return The extension name, or nullptr if this platform has no known mapping
 */
const char* vulkanPlatformSurfaceExtension() {
	const auto platform = QGuiApplication::platformName();

	if (platform == QLatin1String("xcb")) {
		return "VK_KHR_xcb_surface";
	}
	if (platform.startsWith(QLatin1String("wayland"))) {
		return "VK_KHR_wayland_surface";
	}
	if (platform == QLatin1String("windows")) {
		return "VK_KHR_win32_surface";
	}
	if (platform == QLatin1String("cocoa")) {
		return "VK_EXT_metal_surface";
	}

	return nullptr;
}
#endif

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
	case os::OpenGLProfile::ES:
		format.setProfile(QSurfaceFormat::NoProfile);
		format.setRenderableType(QSurfaceFormat::OpenGLES);
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

namespace {
bool g_vulkanSurfaces = false;
#if QTFRED_HAS_VULKAN
// Set once QtGraphicsOperations builds the instance, cleared when it goes. Windows need to reach it
// at construction time (see fredVulkanInstance()), which is well before they could be handed one.
QVulkanInstance* g_vulkanInstance = nullptr;
#endif
} // namespace

bool fredUsingVulkanSurfaces() {
	return g_vulkanSurfaces;
}

#if QTFRED_HAS_VULKAN

QVulkanInstance* fredVulkanInstance() {
	return g_vulkanInstance;
}
#endif


QtGraphicsOperations::QtGraphicsOperations(Editor* editor) : _editor(editor) {
	if ( !SDL_InitSubSystem(SDL_INIT_VIDEO) ) {
		Error(LOCATION, "Couldn't init SDL video: %s", SDL_GetError());
		return;
	}
}
QtGraphicsOperations::~QtGraphicsOperations() {
	if (_vulkanLibraryLoaded) {
		SDL_Vulkan_UnloadLibrary();
		_vulkanLibraryLoaded = false;
	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

os::VulkanSurfaceProvider* QtGraphicsOperations::getVulkanSupport() {
#if QTFRED_HAS_VULKAN
	// gr_init() asks this before the renderer exists in order to decide whether it can honour a
	// Vulkan request, so the platform check has to live here rather than in a later step. The
	// QApplication is constructed in main() long before initialize() runs, so platformName() is
	// meaningful at this point.
	const auto* extension = vulkanPlatformSurfaceExtension();
	if (extension == nullptr) {
		mprintf(("qtFRED: no Vulkan surface extension known for Qt platform '%s'; Vulkan unavailable.\n",
			QGuiApplication::platformName().toUtf8().constData()));
		return nullptr;
	}

	return this;
#else
	return nullptr;
#endif
}

void* QtGraphicsOperations::getVulkanProcAddr() {
	// SDL is still the right thing to load the loader with: libvulkan is windowing-system
	// agnostic, so nothing here depends on SDL's video driver matching Qt's platform plugin --
	// only the surface extension does, and that comes from Qt (see vulkanPlatformSurfaceExtension).
	// SDL's per-platform library naming is well tested; ours would not be.
	if (!_vulkanLibraryLoaded) {
		if (!SDL_Vulkan_LoadLibrary(nullptr)) {
			mprintf(("qtFRED: failed to load the Vulkan library: %s\n", SDL_GetError()));
			return nullptr;
		}
		_vulkanLibraryLoaded = true;
	}

	auto procAddr = reinterpret_cast<void*>(SDL_Vulkan_GetVkGetInstanceProcAddr());
	if (procAddr == nullptr) {
		mprintf(("qtFRED: failed to get vkGetInstanceProcAddr: %s\n", SDL_GetError()));
	}

	return procAddr;
}

bool QtGraphicsOperations::getVulkanInstanceExtensions(SCP_vector<SCP_string>& extensions) {
#if QTFRED_HAS_VULKAN
	const auto* platformExtension = vulkanPlatformSurfaceExtension();
	if (platformExtension == nullptr) {
		return false;
	}

	extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.emplace_back(platformExtension);

	mprintf(("qtFRED: Vulkan surface extension for Qt platform '%s': %s\n",
		QGuiApplication::platformName().toUtf8().constData(),
		platformExtension));

	return true;
#else
	(void)extensions;
	return false;
#endif
}

uint64_t QtGraphicsOperations::createVulkanSurface(os::Viewport* view, void* vkInstance) {
#if QTFRED_HAS_VULKAN
	auto* qtView = dynamic_cast<QtSurfaceViewport*>(view);
	if (qtView == nullptr) {
		mprintf(("qtFRED: cannot create a Vulkan surface for a non-Qt viewport.\n"));
		return 0;
	}

	auto* window = dynamic_cast<QWindow*>(qtView->getRenderSurface());
	if (window == nullptr) {
		mprintf(("qtFRED: render surface is not a QWindow, cannot create a Vulkan surface.\n"));
		return 0;
	}

	if (!_vulkanInstance) {
		_vulkanInstance.reset(new QVulkanInstance());
		_vulkanInstance->setVkInstance(static_cast<VkInstance>(vkInstance));

		if (!_vulkanInstance->create()) {
			mprintf(("qtFRED: QVulkanInstance::create() failed with error %d.\n",
				_vulkanInstance->errorCode()));
			_vulkanInstance.reset();
			return 0;
		}

		g_vulkanInstance = _vulkanInstance.get();
	}

	// A window inside QWidget::createWindowContainer() cannot be rebuilt here: destroy() takes the
	// container's embedding with it and the window is never composited into the widget again, which
	// presents as a permanently blank panel rather than as an error. Windows that can be presented
	// to therefore set their surface type and instance in their own constructor (see
	// fredVulkanInstance()); all that is left to do here is realize it.
	if (window->surfaceType() != QSurface::VulkanSurface) {
		mprintf(("qtFRED: window was built as surface type %d rather than VulkanSurface; it cannot be "
		         "presented to.\n", static_cast<int>(window->surfaceType())));
		return 0;
	}

	if (window->vulkanInstance() == nullptr) {
		window->setVulkanInstance(_vulkanInstance.get());
	}

	if (window->handle() == nullptr) {
		window->create();
	}

	const auto surface = QVulkanInstance::surfaceForWindow(window);
	if (surface == VK_NULL_HANDLE) {
		mprintf(("qtFRED: QVulkanInstance::surfaceForWindow() failed.\n"));
		return 0;
	}

	const auto handle = os::vulkan_handle_value(surface);
	_vulkanSurfaceWindows[handle] = window;

	return handle;
#else
	(void)view;
	(void)vkInstance;
	return 0;
#endif
}

void QtGraphicsOperations::destroyVulkanSurface(void* vkInstance, uint64_t surface) {
#if QTFRED_HAS_VULKAN
	(void)vkInstance;

	const auto entry = _vulkanSurfaceWindows.find(surface);
	if (entry == _vulkanSurfaceWindows.end()) {
		return;
	}

	// Qt owns the surface -- it is destroyed along with the native window, and calling
	// vkDestroySurfaceKHR on it ourselves would double-free. Destroying the native window here is
	// what releases it, and it has to happen now: qtFRED's shutdown() runs gr_close() (which
	// destroys the VkInstance) before os_cleanup() frees the viewports, so leaving it to the
	// window's own destructor would tear the surface down against a dead instance.
	if (entry->second != nullptr) {
		entry->second->destroy();
	}

	_vulkanSurfaceWindows.erase(entry);
#else
	(void)vkInstance;
	(void)surface;
#endif
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
	auto qtContext = static_cast<QtOpenGLContext*>(ctx);

	if (view == nullptr && qtContext == nullptr) {
		if (_lastContext != nullptr) {
			_lastContext->makeCurrent(nullptr);
		}
	} else {
		auto qtSurfaceView = dynamic_cast<QtSurfaceViewport*>(view);
		if (qtSurfaceView != nullptr && qtContext != nullptr) {
			qtContext->makeCurrent(qtSurfaceView->getRenderSurface());
		}
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
	// Has to be set before the first window is constructed: a presentable window picks its surface
	// type in its own constructor and cannot change it afterwards.
	g_vulkanSurfaces = props.enable_vulkan;

	std::unique_ptr<FredView> mw(new FredView());

	// Under Vulkan the window must not be realized yet: its surface type and QVulkanInstance have
	// to be set first, and neither is known until the renderer has built the instance -- which
	// happens after this call. createVulkanSurface() finishes the job.
	if (!props.enable_vulkan) {
		mw->getRenderWidget()->setSurfaceFormat(getSurfaceFormat(props, props.gl_attributes));
	}

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
	if (_viewProps.enable_vulkan) {
		// Presentation is the swap chain's job, inside VulkanRenderer::flip(). There is no current
		// QOpenGLContext to ask, so this would be a null dereference rather than a no-op.
		return;
	}

	auto qSurf = dynamic_cast<QWindow*>(_viewportWindow->getRenderSurface());
	if (qSurf && qSurf->isExposed()) {
		QOpenGLContext::currentContext()->swapBuffers(qSurf);
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
QSurface* QtViewport::getRenderSurface() {
	return _viewportWindow->getRenderSurface();
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
bool QtOpenGLContext::setSwapInterval(int) {
	// Not used at the moment, should default to vsync enabled
	return true;
}
void QtOpenGLContext::makeCurrent(QSurface* surface) {
	_context->makeCurrent(surface);
}
}
}
