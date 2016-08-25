
#ifndef _SDL_GRAPHICS_OPERATIONS
#define _SDL_GRAPHICS_OPERATIONS
#pragma once

#include "osapi/osapi.h"

class SDLGraphicsOperations: public os::GraphicsOperations {
 public:
	SDLGraphicsOperations();
	~SDLGraphicsOperations() override;

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(os::Viewport* viewport,
														   const os::OpenGLContextAttributes& gl_attrs) override;

	void makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) override;

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override;
};

#endif // _SDL_GRAPHICS_OPERATIONS
