//
//

#include "UniformAligner.h"

namespace {

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

}

namespace graphics {
namespace util {

UniformAligner::UniformAligner(size_t dataSize, size_t headerSize) :
	_numElements(0), _dataSize(dataSize), _headerSize(headerSize) {
	// Make sure that the header space is already reserved
	resize(0);
}
void UniformAligner::setAlignment(size_t align) {
	// Clear the buffer since changing the alignment invalidates the stored data
	clear();

	_requiredAlignment = align;
}
void UniformAligner::resize(size_t num_elements) {
	size_t final_size =
		alignSize(_headerSize, _requiredAlignment) + alignSize(_dataSize, _requiredAlignment) * num_elements;

	_buffer.resize(final_size);
	_numElements = num_elements;
}
void* UniformAligner::addElement() {
	_buffer.insert(_buffer.end(), alignSize(_dataSize, _requiredAlignment), 0);
	++_numElements;

	return getElement(_numElements - 1);
}
void* UniformAligner::getElement(size_t index) {
	size_t offset = alignSize(_headerSize, _requiredAlignment) + alignSize(_dataSize, _requiredAlignment) * index;

	Assertion(offset < _buffer.size(), "Invalid index specified!");

	return reinterpret_cast<void*>(_buffer.data() + offset);
}
size_t UniformAligner::getOffset(size_t index) {
	size_t offset = alignSize(_headerSize, _requiredAlignment) + alignSize(_dataSize, _requiredAlignment) * index;

	Assertion(offset < _buffer.size(), "Invalid index specified!");

	return offset;
}
void* UniformAligner::nextElement(void* currentEl) {
	uint8_t* current = reinterpret_cast<uint8_t*>(currentEl);

	current += alignSize(_dataSize, _requiredAlignment);

	return reinterpret_cast<void*>(current);
}
size_t UniformAligner::getNumElements() {
	return _numElements;
}
size_t UniformAligner::getSize() {
	return _buffer.size();
}
void* UniformAligner::getData() {
	return _buffer.data();
}
void UniformAligner::clear() {
	resize(0);
}
}
}
