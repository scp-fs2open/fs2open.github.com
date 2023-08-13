#pragma once

SCP_vector<const char*> gr_opengl_openxr_get_extensions();
bool gr_opengl_openxr_test_capabilities();
bool gr_opengl_openxr_create_session();
int64_t gr_opengl_openxr_get_swapchain_format(const SCP_vector<int64_t>& allowed);
bool gr_opengl_openxr_acquire_swapchain_buffers();
bool gr_opengl_openxr_flip();