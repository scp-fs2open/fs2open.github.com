#pragma once

#include "graphics/2d.h"

#include "UniformBuffer.h"

namespace graphics {
namespace util {

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
