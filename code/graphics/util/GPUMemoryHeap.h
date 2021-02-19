#pragma once

#include "globalincs/pstypes.h"
#include "utils/HeapAllocator.h"
#include "graphics/2d.h"


namespace graphics {
namespace util {

/**
 * @brief A custom heap structure that uses a GPU buffer for storing the data
 *
 * @details This also keeps a client side copy of the data since that is required if the buffer is resized since the
 * original data will not be kept in that case.
 */
class GPUMemoryHeap {
	std::unique_ptr<::util::HeapAllocator> _allocator;
	gr_buffer_handle _bufferHandle;

	void* _dataBuffer = nullptr;
	size_t _bufferSize = 0;

	void resizeBuffer(size_t newSize);

	void* bufferPointer(size_t offset);
 public:
	explicit GPUMemoryHeap(GpuHeap heap_type);
	~GPUMemoryHeap();

	GPUMemoryHeap(const GPUMemoryHeap&) = delete;
	GPUMemoryHeap& operator=(const GPUMemoryHeap&) = delete;

	/**
	 * @brief Store some data in a GPU heap
	 * @param size The size of the data to store
	 * @param data The data to store
	 * @return The offset in bytes from the beginning of the buffer where the stored data begins
	 */
	size_t allocateGpuData(size_t size, void* data);

	/**
	 * @brief Frees up some GPU data from this heap
	 * @param offset The offset at which to free the data
	 */
	void freeGpuData(size_t offset);

	/**
	 * @brief Gets the handle of the used buffer for use in rendering operations
	 *
	 * @warning Do no change the contents of this buffer! The contents are managed by this class and may not be changed
	 * by external code.
	 *
	 * @return The graphics code buffer handle.
	 */
	gr_buffer_handle bufferHandle();
};

}
}

