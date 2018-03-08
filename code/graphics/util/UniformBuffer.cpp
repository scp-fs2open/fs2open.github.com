#include "UniformBuffer.h"

namespace graphics {
namespace util {

UniformBuffer::UniformBuffer(size_t element_size, size_t header_size) : _aligner(element_size, header_size) {
	int offsetAlignment;
	bool success = gr_get_property(gr_property::UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offsetAlignment);

	Assertion(success, "Uniform buffer usage requires a backend which allows to query the offset alignment!");
	_aligner.setAlignment(static_cast<size_t>(offsetAlignment));

	// This uses Dynamic since that matches our usage pattern the closest (update once and then use multiple times)
	_buffer_obj = gr_create_buffer(BufferType::Uniform, BufferUsageHint::Dynamic);

	Assertion(_buffer_obj >= 0, "Creation of buffer object failed!");
}
UniformBuffer::~UniformBuffer() {
	gr_delete_buffer(_buffer_obj);

	if (_sync_obj != nullptr) {
		gr_sync_delete(_sync_obj);
	}
}
void UniformBuffer::submitData() {
	if (_aligner.getSize() == 0) {
		// No data to submit, return now to avoid causing graphics errors
		return;
	}

	gr_update_buffer_data(_buffer_obj, _aligner.getSize(), _aligner.getData());
}
void UniformBuffer::finished() {
	Assertion(_sync_obj == nullptr, "Can't finish using uniform buffer while it's still in use!");

	// We use fences to determine if the GPU is still using this buffer. That allows us to use multiple buffers and
	// avoid implicit synchronization which might be performed by the driver otherwise.
	_sync_obj = gr_sync_fence();

	// Uniform data is single-use only so we can discard the old data of the aligner
	_aligner.clear();
}
bool UniformBuffer::isInUse() {
	if (_sync_obj == nullptr) {
		// Probably was signaled before, just assume that this means that the buffer is not in use anymore
		return false;
	}

	if (gr_sync_wait(_sync_obj, 0)) {
		// Fence was signaled => buffer is not in use anymore
		// Delete the sync object since we don't need it anymore
		gr_sync_delete(_sync_obj);
		_sync_obj = nullptr;
		return false;
	}

	// Fence still exists and has not been signaled yet. The buffer is still in use
	return true;
}
UniformBuffer::UniformBuffer(UniformBuffer&& other) : _aligner(0, 0) {
	*this = std::move(other);
}
UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) {
	std::swap(_buffer_obj, other._buffer_obj);
	std::swap(_aligner, other._aligner);
	std::swap(_sync_obj, other._sync_obj);

	return *this;
}

}
}
