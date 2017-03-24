#pragma once

#include "graphics/opengl/gropengl.h"

gr_sync gr_opengl_sync_fence();
bool gr_opengl_sync_wait(gr_sync sync, uint64_t timeoutns);
void gr_opengl_sync_delete(gr_sync sync);
