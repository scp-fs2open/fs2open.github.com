
#ifndef _SDL_GRAPHICS_OPERATIONS
#define _SDL_GRAPHICS_OPERATIONS
#pragma once

#include "osapi/osapi.h"
#include "osapi/vulkan_surface.h"

class SDLGraphicsOperations: public os::GraphicsOperations, public os::VulkanSurfaceProvider {
 public:
	SDLGraphicsOperations();
	~SDLGraphicsOperations() override;

	std::unique_ptr<os::OpenGLContext> createOpenGLContext(os::Viewport* viewport,
														   const os::OpenGLContextAttributes& gl_attrs) override;

	void makeOpenGLContextCurrent(os::Viewport* view, os::OpenGLContext* ctx) override;

	std::unique_ptr<os::Viewport> createViewport(const os::ViewPortProperties& props) override;

	os::VulkanSurfaceProvider* getVulkanSupport() override { return this; }

	void* getVulkanProcAddr() override;

	bool getVulkanInstanceExtensions(SCP_vector<SCP_string>& extensions) override;

	uint64_t createVulkanSurface(os::Viewport* view, void* vkInstance) override;

	void destroyVulkanSurface(void* vkInstance, uint64_t surface) override;

 private:
	bool _vulkanLibraryLoaded = false;
};

#endif // _SDL_GRAPHICS_OPERATIONS
