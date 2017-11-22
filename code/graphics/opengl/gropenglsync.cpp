#include "graphics/opengl/gropenglsync.h"


gr_sync gr_opengl_sync_fence() {
	auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	// This shouldn't ever fail
	Assertion(fence != nullptr, "Fence creation failed!");

	return (gr_sync) fence;
}
bool gr_opengl_sync_wait(gr_sync sync, uint64_t timeoutns) {
	Assertion(sync != nullptr, "Invalid sync object specified!");
	Assertion(glIsSync((GLsync) sync), "Pointer was specified which is not a sync object");

	// We only need GL_SYNC_FLUSH_COMMANDS_BIT if we actually wait for the sync object. Otherwise we only check if the
	// object is signaled which does not require a flush.
	auto res = glClientWaitSync((GLsync) sync, timeoutns != 0 ? GL_SYNC_FLUSH_COMMANDS_BIT : 0, timeoutns);

	// Return true if the sync object has been signaled in any way
	return res == GL_ALREADY_SIGNALED || res == GL_CONDITION_SATISFIED;
}
void gr_opengl_sync_delete(gr_sync sync) {
	Assertion(sync != nullptr, "Invalid sync object specified!");
	Assertion(glIsSync((GLsync) sync), "Pointer was specified which is not a sync object");

	glDeleteSync((GLsync) sync);
}
