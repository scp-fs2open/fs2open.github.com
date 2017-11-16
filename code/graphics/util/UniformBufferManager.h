#pragma once

#include "graphics/2d.h"

#include "UniformBuffer.h"

namespace graphics {
namespace util {

/**
 * @brief A class for managing uniform block buffer data
 *
 * This uses the classic triple buffer approach for managing uniform data. Users of this class can request a memory
 * range for building uniform data.
 *
 * This assumes that uniform buffers use immutable storage and that buffers that are currently in use by the GPU may not
 * be deleted. This may not be true for all cases but it will make adding a new rendering backend easier.
 *
 * @warning This should not be used directly! Use gr_get_uniform_buffer instead.
 */
class UniformBufferManager {
	// Sets how many buffers should be used. This effectively means that the uniforms are triple-buffered
	static const size_t NUM_SEGMENTS = 3;

	std::array<gr_sync, NUM_SEGMENTS> _segment_fences;

	int _active_uniform_buffer = -1;
	size_t _active_buffer_size = 0;
	void* _buffer_ptr = nullptr; // Pointer to mapped data for persistently mapped buffers

	size_t _active_segment = 0;
	size_t _segment_size = 0;
	size_t _segment_offset = 0; // Offset of the next element to be added to the buffer

	int _offset_alignment = -1;

	SCP_vector<std::pair<int, gr_sync>> _retired_buffers;

	SCP_vector<uint8_t> _temp_buffer; // Buffer for building uniform data in case persistent mapping isn't supported

	void changeSegmentSize(size_t new_size);
public:
	UniformBufferManager();
	~UniformBufferManager();

	UniformBufferManager(const UniformBufferManager&) = delete;
	UniformBufferManager& operator=(const UniformBufferManager&) = delete;

	/**
	 * @brief Gets a uniform buffer for a specific block type
	 *
	 * @warning The storage pointers returned by the buffer will not be initialized and may contain old data! Make sure
	 * that you rewrite all the data you are going to use.
	 *
	 * @param type The type of the uniform data
	 * @param num_elements The number of elements to be stored in that buffer
	 * @return A uniform buffer which can be used for building the uniform buffer data
	 */
	UniformBuffer getUniformBuffer(uniform_block_type type, size_t num_elements);

	/**
	 * @brief Submit finished uniform data to this manager
	 *
	 * @warning This should not be used directly! It will be called by UniformBuffer with the correct parameters when
	 * appropriate.
	 *
	 * @param buffer The memory to submit to the buffer
	 * @param data_size The size of the memory buffer
	 * @param offset The offset into this buffer
	 */
	void submitData(void* buffer, size_t data_size, size_t offset);

	/**
	 * @brief Gets the graphics buffer handle for the currently active uniform buffer
	 *
	 * @warning This should not be used directly. Use UniformBuffer::bufferHandle().
	 *
	 * @return The uniform buffer handle
	 */
	int getActiveBufferHandle();

	/**
	 * @brief Checks the used buffer and retires any buffers that are no longer in use for later reuse
	 */
	void onFrameEnd();
};

}
}
