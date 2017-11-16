//
//

#include "UniformAligner.h"

namespace graphics {
namespace util {

size_t alignSize(size_t size, size_t align) {
	if (align == 0) {
		return size;
	}

	auto remainder = size % align;
	if (remainder == 0) {
		return size;
	}

	return size + align - remainder;
}

UniformAligner::UniformAligner() = default;
UniformAligner::UniformAligner(uint8_t* buffer,
							   size_t buffer_size,
							   size_t dataSize,
							   size_t headerSize,
							   size_t element_alignment) :
	_requiredAlignment(element_alignment),
	_buffer(buffer),
	_buffer_size(buffer_size),
	_numElements(0),
	_dataSize(dataSize),
	_headerSize(headerSize) {
	_buffer_offset = getOffset(0); // Set the initial offset to the first data element
}
void* UniformAligner::addElement() {
	Assertion(_buffer_offset + _dataSize <= _buffer_size, "Not enough space in the buffer for adding another element! "
		"Check the number of elements the buffer was allocated with!");

	auto el_offset = _buffer_offset;

	_buffer_offset += alignSize(_dataSize, _requiredAlignment);
	++_numElements;

	return reinterpret_cast<void*>(_buffer + el_offset);
}
void* UniformAligner::getElement(size_t index) {
	size_t offset = alignSize(_headerSize, _requiredAlignment) + alignSize(_dataSize, _requiredAlignment) * index;

	Assertion(offset < _buffer_size, "Invalid index specified!");

	return reinterpret_cast<void*>(_buffer + offset);
}
size_t UniformAligner::getOffset(size_t index) {
	size_t offset = alignSize(_headerSize, _requiredAlignment) + alignSize(_dataSize, _requiredAlignment) * index;

	Assertion(offset < _buffer_size, "Invalid index specified!");

	return offset;
}
void* UniformAligner::nextElement(void* currentEl) {
	auto current = reinterpret_cast<uint8_t*>(currentEl);

	current += alignSize(_dataSize, _requiredAlignment);

	return reinterpret_cast<void*>(current);
}
size_t UniformAligner::getNumElements() {
	return _numElements;
}
size_t UniformAligner::getSize() {
	return _buffer_size;
}
void* UniformAligner::getData() {
	return _buffer;
}
size_t UniformAligner::getCurrentOffset() {
	return getOffset(_numElements - 1);
}
size_t UniformAligner::getBufferSize(size_t num_elements, size_t alignment, size_t dataSize, size_t headerSize) {
	auto alignedHeaderSize = alignSize(headerSize, alignment);
	auto alignedElementSize = alignSize(dataSize, alignment);

	return alignedHeaderSize + num_elements * alignedElementSize;
}
}
}
