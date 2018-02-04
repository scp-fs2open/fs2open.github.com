#include "utils/HeapAllocator.h"

namespace {

const size_t HEAP_SIZE_INCREASE = 1 * 1024 * 1024; // Always increment in 1MB steps
const size_t HEAP_MAX_INCREASE = 20 * 1024 * 1024; // never increase heap size by more than 20MB

template<typename TIter, typename T>
TIter get_element_before(TIter first, TIter last, const T& value) {
	if (first == last) {
		// There are no elements in the range
		return last;
	}

	auto iter = std::upper_bound(first, last, value);

	if (iter == first) {
		// The element before this does not exist
		return last;
	}

	// We found the position (which may be last) so now we can decrement it once to get the element before this found position
	return std::next(iter, -1);
}

}

namespace util {
HeapAllocator::HeapAllocator(const HeapAllocator::HeapResizer& creator) : _heapResizer(creator) {
	_heapSize = HEAP_SIZE_INCREASE;
	_lastSizeIncraese = HEAP_SIZE_INCREASE;
	_heapResizer(_heapSize);

	MemoryRange range;
	range.offset = 0;
	range.size = _heapSize;

	addFreeRange(range);
}
size_t HeapAllocator::allocate(size_t size) {
	// Do a simple first-fit algorithm. Not very efficient but it should do the job well for our purposes
	for (auto iter = _freeRanges.begin(); iter != _freeRanges.end(); ++iter) {
		if (iter->size >= size) {
			// Found a match!
			auto offset = iter->offset;

			iter->offset += size;
			iter->size -= size;

			if (iter->size == 0) {
				// This range is now empty and must be removed. Since we won't be using this iterator anymore it can be
				// safely erased
				_freeRanges.erase(iter);
			}

			MemoryRange allocated;
			allocated.size = size;
			allocated.offset = offset;

			addAllocatedRange(allocated);

			return offset;
		}
	}

	// No free range found => increase size of heap

	// We increase the heap size every time we run out of memory in order to reduce reallocation times
	_lastSizeIncraese = std::min(HEAP_MAX_INCREASE, 2 * _lastSizeIncraese);

	// Make sure that our allocation can actually fit into the new block if its too large for a single increase
	auto increase = std::max(HEAP_SIZE_INCREASE, _lastSizeIncraese);
	auto lastOffset = _heapSize;

	_heapSize += increase;
	_heapResizer(_heapSize);

	MemoryRange newFree;
	newFree.offset = lastOffset;
	newFree.size = increase;

	addFreeRange(newFree);

	// Call us again but this time it should succeed
	return allocate(size);
}
void HeapAllocator::addFreeRange(const HeapAllocator::MemoryRange& range) {
	if (range.size == 0) {
		// Ignore empty ranges
		return;
	}

	Assertion(std::is_sorted(_freeRanges.begin(), _freeRanges.end()),
			  "Free ranges were not sorted before adding a free range.");

	auto left = get_element_before(_freeRanges.begin(), _freeRanges.end(), range);
	auto right = std::upper_bound(_freeRanges.begin(), _freeRanges.end(), range);

	if (left != _freeRanges.end()) {
		// This new range may be just after another
		if (left->offset + left->size == range.offset) {
			left->size += range.size;

			// After this operation the two ranges may be connected. If that happens then they need to be merged as well
			if (right != _freeRanges.end() && check_connected_range(*left, *right)) {
				left->size += right->size;
				_freeRanges.erase(right);
			}

			checkRangesMerged();

			// The range has been added by merging it with another
			return;
		}
	}

	if (right != _freeRanges.end()) {
		// This new range may be just before another
		if (range.offset + range.size == right->offset) {
			right->offset = range.offset;
			right->size += range.size;

			// After this operation the two ranges may be connected. If that happens then they need to be merged as well
			if (left != _freeRanges.end() && check_connected_range(*left, *right)) {
				left->size += right->size;
				_freeRanges.erase(right);
			}

			checkRangesMerged();

			// The range has been added by merging it with another
			return;
		}
	}

	// If we are here then we found no range to merge the new range into
	_freeRanges.insert(right, range);

	checkRangesMerged();
}
void HeapAllocator::addAllocatedRange(const HeapAllocator::MemoryRange& range) {
	Assertion(std::is_sorted(_allocatedRanges.begin(), _allocatedRanges.end()), "Allocated ranges are not sorted!");
	Assertion(!std::binary_search(_allocatedRanges.begin(), _allocatedRanges.end(), range),
			  "Allocated ranges already contain the specified range!");

	// We need to keep the vector sorted for better search performance so we insert this new range at the sorted position
	// into the vector
	auto it = std::upper_bound(_allocatedRanges.begin(), _allocatedRanges.end(), range);

	// This works even if it is .end() since .insert will just add it to the vector in that case
	_allocatedRanges.insert(it, range);

	Assertion(std::is_sorted(_allocatedRanges.begin(), _allocatedRanges.end()), "Allocated ranges are not sorted!");
}
void HeapAllocator::free(size_t offset) {
	// We use a dummy range for finding the entry in our allocated ranges
	MemoryRange range;
	range.offset = offset;

	auto it = std::lower_bound(_allocatedRanges.begin(), _allocatedRanges.end(), range);

	// Make sure that the range is valid
	Assertion(it != _allocatedRanges.end(), "Specified offset was not found in the allocated ranges!");
	Assertion(it->offset == offset, "Specified offset does not point to the start of an allocated range!");

	// Copy the range out of the vector before erasing it so we can add it to the free ranges
	auto freedRange = *it;

	_allocatedRanges.erase(it);

	addFreeRange(freedRange);
}
size_t HeapAllocator::numAllocations() const {
	return _allocatedRanges.size();
}
bool HeapAllocator::check_connected_range(const MemoryRange& left, const MemoryRange& right) {
	return left.offset + left.size == right.offset;
}
void HeapAllocator::checkRangesMerged() {
	bool first = true;
	size_t lastEnd = 0;
	for (auto& range : _freeRanges) {
		if (!first) {
			Assertion(lastEnd != range.offset, "Found unmerged ranges at offset " SIZE_T_ARG "!", lastEnd);
		}

		lastEnd = range.offset + range.size;
		first = false;
	}
}

bool HeapAllocator::MemoryRange::operator<(const HeapAllocator::MemoryRange& other) const {
	return offset < other.offset;
}
bool HeapAllocator::MemoryRange::operator==(const HeapAllocator::MemoryRange& other) const {
	return offset == other.offset;
}
}
