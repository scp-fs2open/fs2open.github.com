#pragma once


#include "graphics/2d.h"
#include "UniformAligner.h"

namespace graphics {
namespace util {

class UniformBufferManager;

/**
 * @brief Manages a uniform buffer memory range
 *
 * This class will be used by the uniform buffer manager for allowing user code direct access to uniform buffer memory.
 * This also contains a UniformAligner which can be used for making sure that the data is aligned properly for GPU
 * access.
 */
class UniformBuffer {
	UniformBufferManager* _parent = nullptr;
	size_t _parent_offset         = 0;

	gr_buffer_handle _buffer_handle;

	UniformAligner _aligner;

  public:
	UniformBuffer();
	UniformBuffer(UniformBufferManager* parent, size_t parent_offset, void* data_buffer, size_t buffer_size,
	              size_t element_size, size_t header_size, size_t element_alignment);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;

	UniformBuffer(UniformBuffer&&) = default;
	UniformBuffer& operator=(UniformBuffer&&) = default;

	inline UniformAligner& aligner() {
		return _aligner;
	}

	/**
	 * @brief Gets the buffer handle for use with gr_bind_uniform_buffer.
	 * @return The buffer handle
	 */
	gr_buffer_handle bufferHandle();

	/**
	 * @brief Submits the data from the uniform aligner to the underlying buffer object.
	 *
	 * This should be called when you are done building the uniform data.
	 */
	void submitData();

	/**
	 * @brief Given the offset into the memory buffer, returns the total offset in the buffer object
	 *
	 * This should be used for retrieving the correct offset for gr_bind_uniform_buffer.
	 *
	 * @param localOffset The offset into the local memory buffer (e.g. the return value of
	 * 	UniformAligner::getCurrentOffset)
	 * @return The absolute offset in the uniform buffer object.
	 */
	size_t getBufferOffset(size_t localOffset);

	/**
	 * @brief Gets the absolute offset to the specified element in the aligner
	 * This should be used for retrieving the correct offset for gr_bind_uniform_buffer.
	 *
	 * @param index The element index to be retrieved
	 * @return The absolute offset in the uniform buffer object.
	 */
	size_t getAlignerElementOffset(size_t index);

	/**
	 * @brief Gets the absolute offset of the current element in the aligner
	 * This should be used for retrieving the correct offset for gr_bind_uniform_buffer.
	 *
	 * @return The absolute offset in the uniform buffer object.
	 */
	size_t getCurrentAlignerOffset();
};

}
}

