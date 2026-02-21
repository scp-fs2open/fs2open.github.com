
#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanDescriptorManager.h"
#include "VulkanPipeline.h"
#include "VulkanQuery.h"
#include "VulkanState.h"
#include "VulkanDraw.h"
#include "VulkanDeferred.h"
#include "VulkanPostProcessing.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"
#include "osapi/osapi.h"

#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/material.h"
#include "graphics/post_processing.h"
#include "graphics/grinternal.h"
#include "lighting/lighting.h"
#include "pngutils/pngutils.h"

namespace graphics {
namespace vulkan {

namespace {

std::unique_ptr<VulkanRenderer> renderer_instance;

// Sync object for tracking frame completion
struct VulkanSyncObject {
	uint64_t frameNumber;
};

// ========== Renderer-level functions ==========

void vulkan_setup_frame()
{
	auto* renderer = getRendererInstance();
	renderer->setupFrame();
}

void vulkan_flip()
{
	renderer_instance->flip();
}

bool vulkan_is_capable(gr_capability capability)
{
	switch (capability) {
	case gr_capability::CAPABILITY_ENVIRONMENT_MAP:
		return true;
	case gr_capability::CAPABILITY_NORMAL_MAP:
		return Cmdline_normal != 0;
	case gr_capability::CAPABILITY_HEIGHT_MAP:
		return Cmdline_height != 0;
	case gr_capability::CAPABILITY_SOFT_PARTICLES:
		return Gr_post_processing_enabled;
	case gr_capability::CAPABILITY_DISTORTION:
		return Gr_post_processing_enabled;
	case gr_capability::CAPABILITY_POST_PROCESSING:
		return Gr_post_processing_enabled;
	case gr_capability::CAPABILITY_DEFERRED_LIGHTING:
		return light_deferred_enabled();
	case gr_capability::CAPABILITY_SHADOWS:
		return getRendererInstance()->supportsShaderViewportLayerOutput();
	case gr_capability::CAPABILITY_THICK_OUTLINE:
		return false;
	case gr_capability::CAPABILITY_BATCHED_SUBMODELS:
		return true;
	case gr_capability::CAPABILITY_TIMESTAMP_QUERY:
		return getQueryManager() != nullptr;
	case gr_capability::CAPABILITY_SEPARATE_BLEND_FUNCTIONS:
		// Vulkan supports per-attachment blend by spec
		return true;
	case gr_capability::CAPABILITY_PERSISTENT_BUFFER_MAPPING:
		// Vulkan has persistently mappable host-visible memory
		return true;
	case gr_capability::CAPABILITY_BPTC:
		return getRendererInstance()->isTextureCompressionBCSupported();
	case gr_capability::CAPABILITY_LARGE_SHADER:
		// Always true for Vulkan: we use pre-compiled SPIR-V uber-shaders with
		// runtime branching on modelData.flags. The variant approach would require
		// compiling exponentially many SPIR-V permutations. Unbound texture slots
		// are handled via fallback descriptors, so there's no driver issue.
		return true;
	case gr_capability::CAPABILITY_INSTANCED_RENDERING:
		return true;
	case gr_capability::CAPABILITY_QUERIES_REUSABLE:
		// Vulkan queries require explicit reset between read and write.
		// The backend manages this lifecycle internally via deleteQueryObject.
		return false;
	}
	return false;
}

bool vulkan_get_property(gr_property prop, void* dest)
{
	auto* renderer = getRendererInstance();

	switch (prop) {
	case gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT:
		*reinterpret_cast<int*>(dest) = static_cast<int>(renderer->getMinUniformBufferOffsetAlignment());
		return true;
	case gr_property::UNIFORM_BUFFER_MAX_SIZE:
		*reinterpret_cast<int*>(dest) = static_cast<int>(renderer->getMaxUniformBufferSize());
		return true;
	case gr_property::MAX_ANISOTROPY:
		*reinterpret_cast<float*>(dest) = renderer->getMaxAnisotropy();
		return true;
	default:
		return false;
	}
}

void vulkan_push_debug_group(const char* name)
{
	auto* renderer = getRendererInstance();
	if (!renderer->isDebugUtilsEnabled()) {
		return;
	}

	auto* stateTracker = getStateTracker();

	vk::DebugUtilsLabelEXT label;
	label.pLabelName = name;
	label.color = {{ 1.0f, 1.0f, 1.0f, 1.0f }};
	stateTracker->getCommandBuffer().beginDebugUtilsLabelEXT(label);
}

void vulkan_pop_debug_group()
{
	auto* renderer = getRendererInstance();
	if (!renderer->isDebugUtilsEnabled()) {
		return;
	}

	auto* stateTracker = getStateTracker();
	stateTracker->getCommandBuffer().endDebugUtilsLabelEXT();
}

void vulkan_imgui_new_frame()
{
	ImGui_ImplVulkan_NewFrame();
}

void vulkan_imgui_render_draw_data()
{
	auto* renderer = getRendererInstance();
	if (renderer) {
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer->getVkCurrentCommandBuffer());
	}
}

gr_sync vulkan_sync_fence()
{
	auto* renderer = getRendererInstance();
	auto* sync = new VulkanSyncObject();
	sync->frameNumber = renderer->getCurrentFrameNumber();
	return static_cast<gr_sync>(sync);
}

bool vulkan_sync_wait(gr_sync sync, uint64_t /*timeoutns*/)
{
	if (!sync) {
		return true;
	}

	auto* renderer = getRendererInstance();
	auto* syncObj = static_cast<VulkanSyncObject*>(sync);

	// Wait on the specific frame's fence (no-op if already complete)
	renderer->waitForFrame(syncObj->frameNumber);
	return true;
}

void vulkan_sync_delete(gr_sync sync)
{
	if (sync) {
		delete static_cast<VulkanSyncObject*>(sync);
	}
}

// ========== Screen capture (save/restore, screenshots) ==========

static ubyte* Vulkan_saved_screen = nullptr;
static int Vulkan_saved_screen_id = -1;

int vulkan_save_screen()
{
	if (Vulkan_saved_screen) {
		// Already have a saved screen
		return -1;
	}

	ubyte* pixels = nullptr;
	uint32_t w, h;
	if (!renderer_instance->readbackFramebuffer(&pixels, &w, &h)) {
		return -1;
	}

	int bmpId = bm_create(32, static_cast<int>(w), static_cast<int>(h), pixels, 0);
	if (bmpId < 0) {
		vm_free(pixels);
		return -1;
	}

	Vulkan_saved_screen = pixels;
	Vulkan_saved_screen_id = bmpId;
	return Vulkan_saved_screen_id;
}

void vulkan_restore_screen(int bmp_id)
{
	gr_reset_clip();

	if (!Vulkan_saved_screen) {
		gr_clear();
		return;
	}

	Assert((bmp_id < 0) || (bmp_id == Vulkan_saved_screen_id));

	if (Vulkan_saved_screen_id < 0) {
		return;
	}

	gr_set_bitmap(Vulkan_saved_screen_id);
	gr_bitmap(0, 0, GR_RESIZE_NONE);
}

void vulkan_free_screen(int bmp_id)
{
	if (!Vulkan_saved_screen) {
		return;
	}

	vm_free(Vulkan_saved_screen);
	Vulkan_saved_screen = nullptr;

	Assert((bmp_id < 0) || (bmp_id == Vulkan_saved_screen_id));

	if (Vulkan_saved_screen_id >= 0) {
		bm_release(Vulkan_saved_screen_id);
		Vulkan_saved_screen_id = -1;
	}
}

// Swizzle BGRA→RGBA in-place for PNG output (swap chain is B8G8R8A8)
static void swizzle_bgra_to_rgba(ubyte* pixels, size_t pixelCount)
{
	for (size_t i = 0; i < pixelCount; i++) {
		size_t off = i * 4;
		std::swap(pixels[off + 0], pixels[off + 2]);
	}
}

void vulkan_print_screen(const char* filename)
{
	ubyte* pixels = nullptr;
	uint32_t w, h;
	if (!renderer_instance->readbackFramebuffer(&pixels, &w, &h)) {
		return;
	}

	swizzle_bgra_to_rgba(pixels, static_cast<size_t>(w) * h);

	char tmp[MAX_PATH_LEN];
	snprintf(tmp, MAX_PATH_LEN - 1, "screenshots/%s.png", filename);

	_mkdir(os_get_config_path("screenshots").c_str());

	if (!png_write_bitmap(os_get_config_path(tmp).c_str(), w, h, false, pixels)) {
		ReleaseWarning(LOCATION, "Failed to write screenshot to \"%s\".", os_get_config_path(tmp).c_str());
	}

	vm_free(pixels);
}

SCP_string vulkan_blob_screen()
{
	ubyte* pixels = nullptr;
	uint32_t w, h;
	if (!renderer_instance->readbackFramebuffer(&pixels, &w, &h)) {
		return "";
	}

	swizzle_bgra_to_rgba(pixels, static_cast<size_t>(w) * h);

	SCP_string result = png_b64_bitmap(w, h, false, pixels);

	vm_free(pixels);

	return "data:image/png;base64," + result;
}

// get_region: intentional no-op. The only caller is neb2_pre_render() in
// NEB2_RENDER_POF mode, which renders a 32x32 background thumbnail into a
// CPU buffer that is never actually read — the pixel data, ex_scale, and
// ey_scale it computes have no consumers. Modern nebula rendering uses
// NEB2_RENDER_HTL (fog color + gr_clear) and doesn't need get_region at all.
void vulkan_get_region(int /*front*/, int /*w*/, int /*h*/, ubyte* /*data*/) {}

void stub_dump_envmap(const char* /*filename*/) {}

std::unique_ptr<os::Viewport> stub_create_viewport(const os::ViewPortProperties& /*props*/)
{
	return std::unique_ptr<os::Viewport>();
}
void stub_use_viewport(os::Viewport* /*view*/) {}
SCP_vector<const char*> stub_openxr_get_extensions() { return {}; }
bool stub_openxr_test_capabilities() { return false; }
bool stub_openxr_create_session() { return false; }
int64_t stub_openxr_get_swapchain_format(const SCP_vector<int64_t>& /*allowed*/) { return 0; }
bool stub_openxr_acquire_swapchain_buffers() { return false; }
bool stub_openxr_flip() { return false; }

// ========== Function pointer table ==========
// Implementations are defined in their respective files:
// VulkanDraw.cpp, VulkanBuffer.cpp, VulkanTexture.cpp, VulkanShader.cpp, VulkanState.cpp

void init_function_pointers()
{
	// function pointers...
	gr_screen.gf_setup_frame = vulkan_setup_frame;
	gr_screen.gf_set_clip = vulkan_set_clip;
	gr_screen.gf_reset_clip = vulkan_reset_clip;

	gr_screen.gf_clear = vulkan_clear;

	gr_screen.gf_print_screen = vulkan_print_screen;
	gr_screen.gf_blob_screen = vulkan_blob_screen;

	gr_screen.gf_zbuffer_get = vulkan_zbuffer_get;
	gr_screen.gf_zbuffer_set = vulkan_zbuffer_set;
	gr_screen.gf_zbuffer_clear = vulkan_zbuffer_clear;

	gr_screen.gf_stencil_set = vulkan_stencil_set;
	gr_screen.gf_stencil_clear = vulkan_stencil_clear;

	gr_screen.gf_alpha_mask_set = vulkan_alpha_mask_set;

	gr_screen.gf_save_screen = vulkan_save_screen;
	gr_screen.gf_restore_screen = vulkan_restore_screen;
	gr_screen.gf_free_screen = vulkan_free_screen;

	gr_screen.gf_get_region = vulkan_get_region;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data = vulkan_bm_free_data;
	gr_screen.gf_bm_create = vulkan_bm_create;
	gr_screen.gf_bm_init = vulkan_bm_init;
	gr_screen.gf_bm_page_in_start = vulkan_bm_page_in_start;
	gr_screen.gf_bm_data = vulkan_bm_data;
	gr_screen.gf_bm_make_render_target = vulkan_bm_make_render_target;
	gr_screen.gf_bm_set_render_target = vulkan_bm_set_render_target;

	gr_screen.gf_set_cull = vulkan_set_cull;
	gr_screen.gf_set_color_buffer = vulkan_set_color_buffer;

	gr_screen.gf_set_clear_color = vulkan_set_clear_color;

	gr_screen.gf_preload = vulkan_preload;

	gr_screen.gf_set_texture_addressing = vulkan_set_texture_addressing;
	gr_screen.gf_zbias = vulkan_zbias;
	gr_screen.gf_set_fill_mode = vulkan_set_fill_mode;

	gr_screen.gf_create_buffer = vulkan_create_buffer;
	gr_screen.gf_delete_buffer = vulkan_delete_buffer;

	gr_screen.gf_update_transform_buffer = vulkan_update_transform_buffer;
	gr_screen.gf_update_buffer_data = vulkan_update_buffer_data;
	gr_screen.gf_update_buffer_data_offset = vulkan_update_buffer_data_offset;
	gr_screen.gf_map_buffer = vulkan_map_buffer;
	gr_screen.gf_flush_mapped_buffer = vulkan_flush_mapped_buffer;

	gr_screen.gf_post_process_set_effect = vulkan_post_process_set_effect;
	gr_screen.gf_post_process_set_defaults = vulkan_post_process_set_defaults;

	gr_screen.gf_post_process_begin = vulkan_post_process_begin;
	gr_screen.gf_post_process_end = vulkan_post_process_end;
	gr_screen.gf_post_process_save_zbuffer = vulkan_post_process_save_zbuffer;
	gr_screen.gf_post_process_restore_zbuffer = vulkan_post_process_restore_zbuffer;

	gr_screen.gf_scene_texture_begin = vulkan_scene_texture_begin;
	gr_screen.gf_scene_texture_end = vulkan_scene_texture_end;
	gr_screen.gf_copy_effect_texture = vulkan_copy_effect_texture;

	gr_screen.gf_deferred_lighting_begin = vulkan_deferred_lighting_begin;
	gr_screen.gf_deferred_lighting_msaa = vulkan_deferred_lighting_msaa;
	gr_screen.gf_deferred_lighting_end = vulkan_deferred_lighting_end;
	gr_screen.gf_deferred_lighting_finish = vulkan_deferred_lighting_finish;

	gr_screen.gf_calculate_irrmap = vulkan_calculate_irrmap;
	gr_screen.gf_dump_envmap = stub_dump_envmap;
	gr_screen.gf_override_fog = vulkan_override_fog;

	gr_screen.gf_imgui_new_frame = vulkan_imgui_new_frame;
	gr_screen.gf_imgui_render_draw_data = vulkan_imgui_render_draw_data;

	gr_screen.gf_set_line_width = vulkan_set_line_width;

	gr_screen.gf_sphere = vulkan_draw_sphere;

	gr_screen.gf_shadow_map_start = vulkan_shadow_map_start;
	gr_screen.gf_shadow_map_end = vulkan_shadow_map_end;

	gr_screen.gf_start_decal_pass = vulkan_start_decal_pass;
	gr_screen.gf_stop_decal_pass = vulkan_stop_decal_pass;
	gr_screen.gf_render_decals = vulkan_render_decals;

	gr_screen.gf_render_shield_impact = vulkan_render_shield_impact;

	gr_screen.gf_maybe_create_shader = vulkan_maybe_create_shader;
	gr_screen.gf_recompile_all_shaders = vulkan_recompile_all_shaders;

	gr_screen.gf_clear_states = vulkan_clear_states;

	gr_screen.gf_update_texture = vulkan_update_texture;
	gr_screen.gf_get_bitmap_from_texture = vulkan_get_bitmap_from_texture;

	gr_screen.gf_render_model = vulkan_render_model;
	gr_screen.gf_render_primitives = vulkan_render_primitives;
	gr_screen.gf_render_primitives_particle = vulkan_render_primitives_particle;
	gr_screen.gf_render_primitives_distortion = vulkan_render_primitives_distortion;
	gr_screen.gf_render_movie = vulkan_render_movie;
	gr_screen.gf_render_nanovg = vulkan_render_nanovg;
	gr_screen.gf_render_primitives_batched = vulkan_render_primitives_batched;
	gr_screen.gf_render_rocket_primitives = vulkan_render_rocket_primitives;

	gr_screen.gf_is_capable = vulkan_is_capable;
	gr_screen.gf_get_property = vulkan_get_property;

	gr_screen.gf_push_debug_group = vulkan_push_debug_group;
	gr_screen.gf_pop_debug_group = vulkan_pop_debug_group;

	gr_screen.gf_create_query_object = vulkan_create_query_object;
	gr_screen.gf_query_value = vulkan_query_value;
	gr_screen.gf_query_value_available = vulkan_query_value_available;
	gr_screen.gf_get_query_value = vulkan_get_query_value;
	gr_screen.gf_delete_query_object = vulkan_delete_query_object;

	gr_screen.gf_create_viewport = stub_create_viewport;
	gr_screen.gf_use_viewport = stub_use_viewport;

	gr_screen.gf_bind_uniform_buffer = vulkan_bind_uniform_buffer;

	gr_screen.gf_sync_fence = vulkan_sync_fence;
	gr_screen.gf_sync_wait = vulkan_sync_wait;
	gr_screen.gf_sync_delete = vulkan_sync_delete;

	gr_screen.gf_set_viewport = vulkan_set_viewport;

	gr_screen.gf_openxr_get_extensions = stub_openxr_get_extensions;
	gr_screen.gf_openxr_test_capabilities = stub_openxr_test_capabilities;
	gr_screen.gf_openxr_create_session = stub_openxr_create_session;
	gr_screen.gf_openxr_get_swapchain_format = stub_openxr_get_swapchain_format;
	gr_screen.gf_openxr_acquire_swapchain_buffers = stub_openxr_acquire_swapchain_buffers;
	gr_screen.gf_openxr_flip = stub_openxr_flip;
}

} // anonymous namespace

void initialize_function_pointers() {
	init_function_pointers();
}

bool initialize(std::unique_ptr<os::GraphicsOperations>&& graphicsOps)
{
	renderer_instance.reset(new VulkanRenderer(std::move(graphicsOps)));
	if (!renderer_instance->initialize()) {
		return false;
	}

	// Initialize ImGui SDL2 backend for input handling.
	// The Vulkan rendering backend (ImGui_ImplVulkan) is initialized
	// inside VulkanRenderer::initImGui() after all Vulkan objects are ready.
	SDL_Window* window = os::getSDLMainWindow();
	if (window) {
		ImGui_ImplSDL2_InitForVulkan(window);
	}

	gr_screen.gf_flip = vulkan_flip;

	// Initialize matrices and viewport (matching OpenGL backend initialization)
	gr_reset_matrices();
	gr_setup_viewport();

	// Start first frame so a command buffer is active before the first draw calls.
	// The engine draws the title screen during game_init(), before the main loop's
	// first gr_flip() → setupFrame(). Without this, any gr_clear/gr_bitmap before
	// the first flip would hit a null command buffer. Matches OpenGL init behavior.
	gr_setup_frame();

	mprintf(("Vulkan: Initialization complete\n"));
	return true;
}

VulkanRenderer* getRendererInstance()
{
	return renderer_instance.get();
}

void cleanup()
{
	renderer_instance->shutdown();
	renderer_instance = nullptr;
}

} // namespace vulkan
} // namespace graphics
