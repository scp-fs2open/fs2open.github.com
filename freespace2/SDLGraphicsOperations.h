
#ifndef _SDL_GRAPHICS_OPERATIONS
#define _SDL_GRAPHICS_OPERATIONS
#pragma once

#include "osapi/osapi.h"

class SDLGraphicsOperations: public os::GraphicsOperations {
 public:
	~SDLGraphicsOperations() override;

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(const os::OpenGLContextAttributes& attrs,
												  uint32_t width, uint32_t height) override;

	void makeOpenGLContextCurrent(os::OpenGLContext* ctx) override;
};

#endif // _SDL_GRAPHICS_OPERATIONS
