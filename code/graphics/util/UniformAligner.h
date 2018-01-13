#pragma once

#include "globalincs/pstypes.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace graphics {
namespace util {

/**
 * @brief Aligns data so that each element starts at a specific offset. Useful for storing uniform buffer data
 */
class UniformAligner {
 private:
	static_assert(sizeof(uint8_t) == 1, "A uint8_t must be exactly one byte!");

	size_t _requiredAlignment = 1;

	std::vector<uint8_t> _buffer;
	size_t _numElements;

	size_t _dataSize = 0;
	size_t _headerSize = 0;
 public:
	UniformAligner(size_t dataSize, size_t headerSize = 0);

	void setAlignment(size_t align);

	void resize(size_t num_elements);

	void clear();

	void* addElement();

	template<typename T>
	T* addTypedElement() {
		Assertion(sizeof(T) == _dataSize,
				  "Sizes of template parameter and runtime size do not match! This probably uses the wrong type.");

		return reinterpret_cast<T*>(addElement());
	}

	template<typename THeader>
	THeader* getHeader() {
		Assertion(sizeof(THeader) == _headerSize, "Header size does not match requested header type!");

		return reinterpret_cast<THeader*>(_buffer.data());
	}

	void* getElement(size_t index);

	template<typename T>
	T* getTypedElement(size_t index) {
		Assertion(sizeof(T) == _dataSize,
				  "Sizes of template parameter and runtime size do not match! This probably uses the wrong type.");

		return reinterpret_cast<T*>(getElement(index));
	}

	size_t getOffset(size_t index);

	/**
	 * @brief Gets the offset of the last element in the aligner
	 * @return The offset in bytes
	 */
	size_t getCurrentOffset();

	template<typename T>
	T* nextTypedElement(T* currentEl) {
		Assertion(sizeof(T) == _dataSize,
				  "Sizes of template parameter and runtime size do not match! This probably uses the wrong type.");

		return reinterpret_cast<T*>(nextElement(reinterpret_cast<void*>(currentEl)));
	}

	void* nextElement(void* currentEl);

	size_t getNumElements();

	/**
	 * @brief Gets the size in bytes of the data in this aligner
	 * @return The size in bytes
	 */
	size_t getSize();

	void* getData();
};

}
}

