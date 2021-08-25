#pragma once

#include "graphics/2d.h"

#include "UniformBuffer.h"

#include <array>

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

	gr_buffer_handle _active_uniform_buffer;
	size_t _active_buffer_size = 0;
	void* _buffer_ptr          = nullptr; // Pointer to mapped data for persistently mapped buffers

	size_t _active_segment = 0;
	size_t _segment_size   = 0;
	size_t _segment_offset = 0; // Offset of the next element to be added to the buffer

	int _offset_alignment        = -1;
	bool _use_persistent_mapping = false;

	/**
	 * @brief A list of retired uniform buffers that might still be in use by the GPU
	 * Once the sync fence is signaled it means that the buffer is not in use anymore by the GPU and can be deleted.
	 * The byte array is the associated shadow buffer which will ensure that the pointer stays valid until the buffer is
	 * deleted at which point any reference to it will surely be removed.
	 */
	SCP_vector<std::tuple<gr_sync, gr_buffer_handle, std::unique_ptr<uint8_t[]>>> _retired_buffers;

	/**
	 * @brief This is a pointer to an array which is a shadow of the uniform buffer
	 * This is needed for building the uniform buffer on the CPU side even if persistent mapping is active since small
	 * writes to the GPU take a lot of time.
	 */
	std::unique_ptr<uint8_t[]> _shadow_uniform_buffer;

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
	 * @param element_size_override Override the elemnt size
	 * @return A uniform buffer which can be used for building the uniform buffer data
	 */
	UniformBuffer getUniformBuffer(uniform_block_type type, size_t num_elements, size_t element_size_override);

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
	gr_buffer_handle getActiveBufferHandle();

	/**
	 * @brief Checks the used buffer and retires any buffers that are no longer in use for later reuse
	 */
	void onFrameEnd();

	/**
	 * @brief Gets the current size of the uniform buffer
	 * This is mostly for debugging purposes.
	 * @return The size in bytes of the uniform buffer.
	 */
	size_t getBufferSize();

	/**
	 * @brief Gets the number of bytes used in the current segment of the buffer
	 * This is mostly for debugging purposes.
	 * @return The bytes used in the segment
	 */
	size_t getCurrentlyUsedSize();
};

}
}
