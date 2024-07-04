#pragma once

#include "globalincs/pstypes.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace graphics {
namespace util {

size_t alignSize(size_t size, size_t align);

/**
 * @brief Aligns data so that each element starts at a specific offset. Useful for storing uniform buffer data
 */
class UniformAligner {
 private:
	static_assert(sizeof(uint8_t) == 1, "A uint8_t must be exactly one byte!");

	size_t _requiredAlignment = 1;

	uint8_t* _buffer    = nullptr;
	size_t _buffer_size = 0;

	size_t _buffer_offset = 0;
	size_t _numElements   = 0;

	size_t _dataSize = 0;
	size_t _headerSize = 0;

  public:
	UniformAligner();
	UniformAligner(uint8_t* buffer, size_t buffer_size, size_t dataSize, size_t headerSize, size_t element_alignment);

	void* addElement();

	template <typename T>
	T* addTypedElement()
	{
		Assertion(sizeof(T) == _dataSize,
		          "Sizes of template parameter and runtime size do not match! This probably uses the wrong type.");
		static_assert(std::is_trivially_copyable<T>::value, "Element type must be trivially copyable!");

		return reinterpret_cast<T*>(addElement());
	}

	template<typename THeader>
	THeader* getHeader() {
		Assertion(sizeof(THeader) == _headerSize, "Header size does not match requested header type!");
		static_assert(std::is_trivially_copyable<THeader>::value, "Header type must be trivially copyable!");

		return reinterpret_cast<THeader*>(_buffer);
	}

	void* getElement(size_t index);

	template<typename T>
	T* getTypedElement(size_t index) {
		Assertion(sizeof(T) == _dataSize,
				  "Sizes of template parameter and runtime size do not match! This probably uses the wrong type.");
		static_assert(std::is_trivially_copyable<T>::value, "Element type must be trivially copyable!");

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
		static_assert(std::is_trivially_copyable<T>::value, "Element type must be trivially copyable!");

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

	/**
	 * @brief Determine the size a buffer must have for the specified parameters
	 * @param num_elements The number of elements to be stored in the buffer
	 * @param alignment The required alignment for the start of the individual elements
	 * @param dataSize The size of one data element.
	 * @param headerSize The size of the header data
	 * @return The size in bytes required for storing all the data
	 */
	static size_t getBufferSize(size_t num_elements, size_t alignment, size_t dataSize, size_t headerSize = 0);
};

}
}

