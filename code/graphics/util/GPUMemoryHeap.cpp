//
//

#include "GPUMemoryHeap.h"

namespace {
BufferType getBufferType(GpuHeap heap_type) {
	switch(heap_type) {
	case GpuHeap::ModelVertex:
		return BufferType::Vertex;
	case GpuHeap::ModelIndex:
		return BufferType::Index;
	case GpuHeap::NUM_VALUES:
	default:
		UNREACHABLE("Invalid heap type detected!");
		return BufferType::Vertex;
	}
}
}

namespace graphics {
namespace util {

GPUMemoryHeap::GPUMemoryHeap(GpuHeap heap_type) {
	_bufferHandle = gr_create_buffer(getBufferType(heap_type), BufferUsageHint::Static);

	_allocator.reset(new ::util::HeapAllocator([this](size_t n) { resizeBuffer(n); }));
}
GPUMemoryHeap::~GPUMemoryHeap() {
	if (_bufferHandle.isValid()) {
		gr_delete_buffer(_bufferHandle);
		_bufferHandle = gr_buffer_handle();
	}

	if (_dataBuffer != nullptr) {
		vm_free(_dataBuffer);

		_dataBuffer = nullptr;
		_bufferSize = 0;
	}
}
void GPUMemoryHeap::resizeBuffer(size_t newSize) {
	_dataBuffer = vm_realloc(_dataBuffer, newSize);
	_bufferSize = newSize;

	gr_update_buffer_data(_bufferHandle, _bufferSize, _dataBuffer);
}
void* GPUMemoryHeap::bufferPointer(size_t offset) {
	auto bytePtr = reinterpret_cast<uint8_t*>(_dataBuffer);

	return reinterpret_cast<void*>(bytePtr + offset);
}
size_t GPUMemoryHeap::allocateGpuData(size_t size, void* data) {
	auto offset = _allocator->allocate(size);

	auto dataPtr = bufferPointer(offset);
	memcpy(dataPtr, data, size);
	gr_update_buffer_data_offset(_bufferHandle, offset, size, data);

	return offset;
}
void GPUMemoryHeap::freeGpuData(size_t offset) {
	_allocator->free(offset);
	// Just leave the data in the buffers since it doesn't hurt anyone if it's kept in there
}
gr_buffer_handle GPUMemoryHeap::bufferHandle() {
	return _bufferHandle;
}

}
}
