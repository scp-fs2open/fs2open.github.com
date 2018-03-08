#pragma once

#include "globalincs/pstypes.h"

#include <functional>

namespace util {

/**
 * @brief A generic heap manager
 *
 * This class does not allocate memory! It only keeps track of where memory is stored and which memory ranges may be
 * reused later. This needs some kind of underlying memory manager before it can do anything.
 */
class HeapAllocator {
 public:
	/**
	 * @brief A function that can resize a generic heap structure.
	 *
	 * This takes the new size of the heap as its only parameter.
	 */
	typedef std::function<void(size_t)> HeapResizer;

 private:
	struct MemoryRange {
		size_t offset = 0;
		size_t size = 0;

		bool operator<(const MemoryRange& other) const;
		bool operator==(const MemoryRange& other) const;
	};

	size_t _heapSize = 0;
	size_t _lastSizeIncraese = 0;

	HeapResizer _heapResizer;

	SCP_vector<MemoryRange> _freeRanges;
	SCP_vector<MemoryRange> _allocatedRanges;

	void addFreeRange(const MemoryRange& range);
	void addAllocatedRange(const MemoryRange& range);

	/**
	 * @brief Checks if all free ranges are merged properly.
	 */
	void checkRangesMerged();

	static bool check_connected_range(const MemoryRange& left, const MemoryRange& right);
 public:
	explicit HeapAllocator(const HeapResizer& creatorFunction);
	~HeapAllocator() = default;

	/**
	 * @brief Allocates the specified amount of memory
	 * @param size The size of the memory block to allocate
	 * @return The offset at which the specified amount of memory can be used
	 */
	size_t allocate(size_t size);

	/**
	 * @brief Frees the memory starting at the specified offset.
	 * @param offset The offset at which to free memory. This value must have been returned by allocate previously!
	 */
	void free(size_t offset);

	/**
	 * @brief Retrieves the amount of allocations currently active
	 * @return The active allocations in this heap.
	 */
	size_t numAllocations() const;
};

}
