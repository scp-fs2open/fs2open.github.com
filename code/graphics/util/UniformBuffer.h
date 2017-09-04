#pragma once


#include "graphics/2d.h"
#include "UniformAligner.h"

namespace graphics {
namespace util {

/**
 * @brief Manages a single uniform buffer
 *
 * This is a wrapper around a buffer handle from the graphics system which also provides access to a uniform data
 * aligner. It also manages the fence object which detects if this buffer is still in use or can be reused by other
 * rendering operations.
 */
class UniformBuffer {
	int _buffer_obj = -1;

	UniformAligner _aligner;

	gr_sync _sync_obj = nullptr;
 public:
	UniformBuffer(size_t element_size, size_t header_size = 0);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;

	UniformBuffer(UniformBuffer&& other);
	UniformBuffer& operator=(UniformBuffer&& other);

	inline UniformAligner& aligner() {
		return _aligner;
	}

	inline int bufferHandle() {
		return _buffer_obj;
	}

	/**
	 * @brief Submits the data from the uniform aligner to the underlying buffer object.
	 */
	void submitData();

	/**
	 * @brief Signal that the code is done using this uniform buffer
	 *
	 * This is used for synchronizing access to this buffer. After this call is finished and until the data has been
	 * consumed by the GPU, this buffer is considered to the "in use".
	 */
	void finished();

	/**
	 * @brief Determines if the buffer is still in use by the GPU
	 * @return @c true if the GPU may still be using this buffer, @c false if not
	 */
	bool isInUse();
};

}
}

