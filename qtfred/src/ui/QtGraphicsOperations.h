#pragma once

#include <osapi/osapi.h>
#include "mission/editor.h"
#include "FredView.h"

#include <QtGui/QOpenGLContext>

namespace fso {
namespace fred {

class QtOpenGLContext: public os::OpenGLContext {
	std::unique_ptr<QOpenGLContext> _context;
 public:
	QtOpenGLContext(std::unique_ptr<QOpenGLContext>&& context);
	~QtOpenGLContext();

	os::OpenGLLoadProc getLoaderFunction() override;
	void setSwapInterval(int status) override;

	void makeCurrent(QSurface* surface);
};

class QtViewport: public os::Viewport {
	std::unique_ptr<FredView> _viewportWindow;
	os::ViewPortProperties _viewProps;
 public:
	QtViewport(std::unique_ptr<FredView>&& window, const os::ViewPortProperties& viewProps);
	~QtViewport();

	SDL_Window* toSDLWindow() override;
	std::pair<uint32_t, uint32_t> getSize() override;
	void swapBuffers() override;
	void setState(os::ViewportState state) override;
	void minimize() override;
	void restore() override;

	const os::ViewPortProperties& getViewProperties() const;
	FredView* getWindow();
};

class QtGraphicsOperations: public os::GraphicsOperations {
	Editor* _editor = nullptr;

	QtOpenGLContext* _lastContext = nullptr;
 public:
	QtGraphicsOperations(Editor* editor);
	~QtGraphicsOperations();

	std::unique_ptr<os::OpenGLContext>
	createOpenGLContext(os::Viewport* viewport, const os::OpenGLContextAttributes& gl_attrs) override;

	void makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) override;

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override;
};

}
}
