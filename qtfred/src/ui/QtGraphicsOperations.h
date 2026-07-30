#pragma once

#include <osapi/osapi.h>
#include <osapi/vulkan_surface.h>
#include "mission/Editor.h"
#include "FredView.h"

#define WIN32_LEAN_AND_MEAN
#include <QtGui/qtguiglobal.h>
#include <QtGui/QOpenGLContext>

// Vulkan needs both halves: the engine's backend has to be compiled in, and Qt has to have been
// built with Vulkan support (it is a compile-time feature there, so the header itself is absent
// otherwise).
#if defined(WITH_VULKAN) && QT_CONFIG(vulkan)
#define QTFRED_HAS_VULKAN 1
#include <QtCore/QPointer>
#include <QtGui/QVulkanInstance>
#else
#define QTFRED_HAS_VULKAN 0
#endif

namespace fso {
namespace fred {

/**
 * @brief Whether qtFRED's presentable windows must be built as Vulkan surfaces
 *
 * Set from ViewPortProperties::enable_vulkan when the renderer asks for its viewport, which is
 * before any window that can be presented to is constructed.
 */
bool fredUsingVulkanSurfaces();

#if QTFRED_HAS_VULKAN
/**
 * @brief The QVulkanInstance qtFRED's Vulkan surfaces are created against, or nullptr
 *
 * A QWindow has to know both its surface type and its instance before it is first realized, and a
 * window living inside QWidget::createWindowContainer() cannot be un-realized and rebuilt later:
 * destroy() takes the container's embedding with it and the window stops being composited into the
 * widget at all (which looks exactly like a render path that draws nothing). So any window that
 * might be presented to has to be constructed as a Vulkan surface from the start, which means
 * reaching the instance from outside the graphics operations.
 *
 * Valid from the first surface creation -- which happens during gr_init(), long before any dialog
 * can be opened -- until the graphics operations are destroyed.
 */
QVulkanInstance* fredVulkanInstance();
#endif

class QtOpenGLContext: public os::OpenGLContext {
	std::unique_ptr<QOpenGLContext> _context;
 public:
	QtOpenGLContext(std::unique_ptr<QOpenGLContext>&& context);
	~QtOpenGLContext() override;

	os::OpenGLLoadProc getLoaderFunction() override;
	bool setSwapInterval(int status) override;

	void makeCurrent(QSurface* surface);
};

class QtSurfaceViewport : public os::Viewport {
public:
	~QtSurfaceViewport() override = default;

	virtual QSurface* getRenderSurface() = 0;
};

class QtViewport: public QtSurfaceViewport {
	std::unique_ptr<FredView> _viewportWindow;
	os::ViewPortProperties _viewProps;
 public:
	QtViewport(std::unique_ptr<FredView>&& window, const os::ViewPortProperties& viewProps);
	~QtViewport() override;

	SDL_Window* toSDLWindow() override;
	std::pair<uint32_t, uint32_t> getSize() override;
	void swapBuffers() override;
	void setState(os::ViewportState state) override;
	void minimize() override;
	void restore() override;
	QSurface* getRenderSurface() override;

	const os::ViewPortProperties& getViewProperties() const;
	FredView* getWindow();
};

class QtGraphicsOperations: public os::GraphicsOperations, public os::VulkanSurfaceProvider {
	Editor* _editor = nullptr;

	QtOpenGLContext* _lastContext = nullptr;

	bool _vulkanLibraryLoaded = false;

#if QTFRED_HAS_VULKAN
	// Adopts the engine's VkInstance rather than creating one, so Qt can hand out surfaces for its
	// own windows while the renderer keeps the instance it built (with the debug-messenger and
	// validation-feature structs chained into vkCreateInstance, which QVulkanInstance can't express).
	std::unique_ptr<QVulkanInstance> _vulkanInstance;

	// Surfaces belong to the QWindow Qt created them for, and qtFRED tears the renderer down
	// (gr_close) before the windows (os_cleanup), so we have to release them explicitly while the
	// instance is still alive. QPointer so a window that somehow went first reads as null.
	SCP_unordered_map<uint64_t, QPointer<QWindow>> _vulkanSurfaceWindows;
#endif

 public:
	QtGraphicsOperations(Editor* editor);
	~QtGraphicsOperations() override;

	std::unique_ptr<os::OpenGLContext>
	createOpenGLContext(os::Viewport* viewport, const os::OpenGLContextAttributes& gl_attrs) override;

	void makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) override;

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override;

	os::VulkanSurfaceProvider* getVulkanSupport() override;

	void* getVulkanProcAddr() override;

	bool getVulkanInstanceExtensions(SCP_vector<SCP_string>& extensions) override;

	uint64_t createVulkanSurface(os::Viewport* view, void* vkInstance) override;

	void destroyVulkanSurface(void* vkInstance, uint64_t surface) override;
};

}
}
