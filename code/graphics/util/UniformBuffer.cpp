#include "UniformBuffer.h"
#include "UniformBufferManager.h"

namespace graphics {
namespace util {

UniformBuffer::UniformBuffer() = default;
UniformBuffer::UniformBuffer(UniformBufferManager* parent, size_t parent_offset, void* data_buffer, size_t buffer_size,
                             size_t element_size, size_t header_size, size_t element_alignment)
    : _parent(parent), _parent_offset(parent_offset),
      _aligner(reinterpret_cast<uint8_t*>(data_buffer), buffer_size, element_size, header_size, element_alignment)
{
	// Cache the buffer handle here since the active handle may change between now and when bind buffer range is called
	// if the buffer needs to be resized
	_buffer_handle = _parent->getActiveBufferHandle();
}
UniformBuffer::~UniformBuffer() = default;
void UniformBuffer::submitData() { _parent->submitData(_aligner.getData(), _aligner.getSize(), _parent_offset); }
gr_buffer_handle UniformBuffer::bufferHandle() { return _buffer_handle; }
size_t UniformBuffer::getBufferOffset(size_t localOffset) { return _parent_offset + localOffset; }
size_t UniformBuffer::getAlignerElementOffset(size_t index) { return getBufferOffset(_aligner.getOffset(index)); }
size_t UniformBuffer::getCurrentAlignerOffset() { return getBufferOffset(_aligner.getCurrentOffset()); }
}
}
