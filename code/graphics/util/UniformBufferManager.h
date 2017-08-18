#pragma once

#include "graphics/2d.h"

#include "UniformBuffer.h"

namespace graphics {
namespace util {

/**
 * @brief A class for managing uniform buffers for a specific block type
 *
 * This class can be used for retrieving an unused uniform buffer for a specific uniform block type. External code
 * should use gr_get_uniform_buffer instead of creating an instance of this class.
 *
 * @details This system uses fence sync objects for detecting if a buffer is still in use. By using those objects it is
 * possible to avoid any synchronization operations when updating the buffer data.
 */
class UniformBufferManager {
	uniform_block_type _type;

	SCP_vector<std::unique_ptr<UniformBuffer>> _buffers; //!< All buffers that are managed by this manager

	SCP_vector<UniformBuffer*> _retiredBuffers; //!< The buffers that are currently free and can be reused
	SCP_vector<UniformBuffer*> _usedBuffers; //!< The buffers that are currently in use

public:
	explicit UniformBufferManager(uniform_block_type uniform_type);

	UniformBufferManager(const UniformBufferManager&) = delete;
	UniformBufferManager& operator=(const UniformBufferManager&) = delete;

	UniformBuffer* getBuffer();

	/**
	 * @brief Checks the used buffer and retires any buffers that are no longer in use for later reuse
	 */
	void retireBuffers();
};

}
}
