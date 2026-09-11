#include "VulkanTexture.h"

#include <algorithm>
#include "VulkanBuffer.h"
#include "VulkanDeletionQueue.h"
#include "VulkanRenderer.h"
#include "gr_vulkan.h"

#include "graphics/util/pixel_swizzle.h"
#include "bmpman/bmpman.h"
#include "ddsutils/ddsutils.h"
#include "ddsutils/bcdec.h"
#include "globalincs/systemvars.h"


namespace graphics::vulkan {

// ========== gr_screen function pointer implementations ==========

int vulkan_preload(int bitmap_num, int /*is_aabitmap*/)
{
	auto* texManager = getTextureManager();

	// Check if texture is already loaded
	auto* slot = texManager->getTextureSlot(bitmap_num);
	if (slot && slot->imageView) {
		return 1;  // Already loaded
	}

	// Determine lock parameters based on compression type.
	// For compressed DDS textures, lock with the matching DXT/BC7 flags to get
	// raw compressed data with all pre-baked mipmap levels.
	int compType = bm_is_compressed(bitmap_num);
	int lockBpp = 32;
	ubyte lockFlags = BMP_TEX_XPARENT;

	switch (compType) {
	case DDS_DXT1:
		lockBpp = 24;
		lockFlags = BMP_TEX_DXT1;
		break;
	case DDS_DXT3:
		lockBpp = 32;
		lockFlags = BMP_TEX_DXT3;
		break;
	case DDS_DXT5:
		lockBpp = 32;
		lockFlags = BMP_TEX_DXT5;
		break;
	case DDS_BC7:
		lockBpp = 32;
		lockFlags = BMP_TEX_BC7;
		break;
	case DDS_CUBEMAP_DXT1:
		lockBpp = 24;
		lockFlags = BMP_TEX_CUBEMAP;
		break;
	case DDS_CUBEMAP_DXT3:
	case DDS_CUBEMAP_DXT5:
		lockBpp = 32;
		lockFlags = BMP_TEX_CUBEMAP;
		break;
	default:
		// Uncompressed — use 32bpp decompressed
		compType = 0;
		break;
	}

	bitmap* bmp = bm_lock(bitmap_num, static_cast<ubyte>(lockBpp), lockFlags);
	if (!bmp) {
		static int warnCount = 0;
		if (warnCount < 10) {
			nprintf(("vulkan", "vulkan_preload: Failed to lock bitmap %d (compType=%d)\n", bitmap_num, compType));
			warnCount++;
		}
		return 0;
	}

	// Upload the texture
	bool success = texManager->bm_data(bitmap_num, bmp, compType);

	// Unlock bitmap
	bm_unlock(bitmap_num);

	if (success) {
		static int successCount = 0;
		if (successCount < 10) {
			nprintf(("vulkan", "vulkan_preload: Successfully uploaded texture %d (compressed=%d)\n",
				bitmap_num, compType));
			successCount++;
		}
	}

	return success ? 1 : 0;
}

void vulkan_bm_create(bitmap_slot* slot)
{
	auto* texManager = getTextureManager();
	texManager->bm_create(slot);
}

void vulkan_bm_free_data(bitmap_slot* slot, bool release)
{
	auto* texManager = getTextureManager();
	texManager->bm_free_data(slot, release);
}

void vulkan_bm_init(bitmap_slot* slot)
{
	auto* texManager = getTextureManager();
	texManager->bm_init(slot);
}

bool vulkan_bm_data(int handle, bitmap* bm)
{
	auto* texManager = getTextureManager();
	int compType = bm_is_compressed(handle);
	return texManager->bm_data(handle, bm, compType);
}

void vulkan_bm_page_in_start()
{
	// Flush all GPU texture resources so that textures not needed in the next
	// mission are freed. Matches the OpenGL pattern (opengl_tcache_flush in
	// opengl_preload_init, currently commented out there). Textures that ARE
	// needed will be re-uploaded on demand during level load / first use.
	// Without this, Vulkan VkImage/VMA allocations accumulate across missions
	// because bm_unload_fast() only frees CPU-side pixel data.
	auto* texManager = getTextureManager();
	texManager->flushTextures();
}

int vulkan_bm_make_render_target(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags)
{
	auto* texManager = getTextureManager();
	return texManager->bm_make_render_target(handle, width, height, bpp, mm_lvl, flags);
}

int vulkan_bm_set_render_target(int handle, int face)
{
	auto* texManager = getTextureManager();
	return texManager->bm_set_render_target(handle, face);
}

void vulkan_update_texture(int bitmap_handle, int bpp, const ubyte* data, int width, int height)
{
	auto* texManager = getTextureManager();
	texManager->update_texture(bitmap_handle, bpp, data, width, height);
}

ubyte* vulkan_get_bitmap_from_texture(const int bitmap_num, int* width_out, int* height_out)
{
	auto* texManager = getTextureManager();
	return texManager->get_bitmap_from_texture(bitmap_num, width_out, height_out);
}

} // namespace graphics::vulkan

