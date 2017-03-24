

#include "UniformBufferManager.h"
#include "uniform_structs.h"

namespace {

size_t getElementSize(uniform_block_type type) {
	switch (type) {
	case uniform_block_type::Lights:
		return sizeof(graphics::deferred_light_data);
	case uniform_block_type::NUM_BLOCK_TYPES:
	default:
		Assertion(false, "Invalid block type encountered!");
		return 0;
	}
}

size_t getHeaderSize(uniform_block_type type) {
	switch (type) {
	case uniform_block_type::Lights:
		return 0;
	case uniform_block_type::NUM_BLOCK_TYPES:
	default:
		Assertion(false, "Invalid block type encountered!");
		return 0;
	}
}

}

namespace graphics {
namespace util {


UniformBufferManager::UniformBufferManager(uniform_block_type uniform_type) : _type(uniform_type) {
}
UniformBuffer* UniformBufferManager::getBuffer() {
	if (!_retiredBuffers.empty()) {
		// Reuse one of the free buffers
		auto buffer = _retiredBuffers.back();

		// Sanity check
		Assertion(!buffer->isInUse(), "Retired buffer is in use!");

		_retiredBuffers.pop_back();
		_usedBuffers.push_back(buffer);
		return buffer;
	}

	// No free buffers available, create a new one
	std::unique_ptr<UniformBuffer> buffer(new UniformBuffer(getElementSize(_type), getHeaderSize(_type)));
	auto ptr = buffer.get();
	_buffers.push_back(std::move(buffer));
	_usedBuffers.push_back(ptr);

	return ptr;
}
void UniformBufferManager::retireBuffers() {
	for (auto& buffer : _usedBuffers) {
		if (!buffer->isInUse()) {
			// This buffer will be retired soon so add it to the list before removing it from the used list
			_retiredBuffers.push_back(buffer);
		}
	}

	// Remove the buffers from the list
	_usedBuffers.erase(std::remove_if(_usedBuffers.begin(), _usedBuffers.end(), [](UniformBuffer* buffer) {
		return !buffer->isInUse();
	}), _usedBuffers.end());
}

}
}
