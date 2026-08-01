
#include <gtest/gtest.h>
#include <random>

#include "utils/HeapAllocator.h"

using namespace util;

namespace {
void dummyResizer(size_t) {}
}

TEST(HeapAllocatorTests, simpleAllocate) {
	HeapAllocator allocator(dummyResizer);

	ASSERT_EQ((size_t)0, allocator.numAllocations());

	auto offset = allocator.allocate(200);

	ASSERT_EQ((size_t)1, allocator.numAllocations());

	allocator.free(offset);

	ASSERT_EQ((size_t)0, allocator.numAllocations());
}

TEST(HeapAllocatorTests, usedBytesAndHeapSize) {
	HeapAllocator allocator(dummyResizer);

	ASSERT_EQ((size_t)0, allocator.usedBytes());
	ASSERT_GT(allocator.heapSize(), (size_t)0);

	auto initialHeapSize = allocator.heapSize();

	auto offsetA = allocator.allocate(200);
	ASSERT_EQ((size_t)200, allocator.usedBytes());

	auto offsetB = allocator.allocate(300);
	ASSERT_EQ((size_t)500, allocator.usedBytes());

	// Allocations within the initial heap size shouldn't have triggered a resize
	ASSERT_EQ(initialHeapSize, allocator.heapSize());

	allocator.free(offsetA);
	ASSERT_EQ((size_t)300, allocator.usedBytes());

	allocator.free(offsetB);
	ASSERT_EQ((size_t)0, allocator.usedBytes());

	// Force at least one resize and make sure usedBytes tracks the live allocation, not the
	// (now larger) heap size
	auto offsetC = allocator.allocate(30 * 1024 * 1024);
	ASSERT_EQ((size_t)30 * 1024 * 1024, allocator.usedBytes());
	ASSERT_GT(allocator.heapSize(), initialHeapSize);
	ASSERT_GE(allocator.heapSize(), allocator.usedBytes());

	allocator.free(offsetC);
	ASSERT_EQ((size_t)0, allocator.usedBytes());
}

TEST(HeapAllocatorTests, manySmallAllocations) {
	HeapAllocator allocator(dummyResizer);

	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<size_t> sizeDist(1, 50000);

	SCP_vector<size_t> offsets;
	// Allocate enough small ranges to require a resize at some point
	for (auto i = 0; i < 1500; ++i) {
		auto size = sizeDist(gen);

		// Use multiples of 52 since that is what the model code uses as the vertex stride
		size_t x = allocator.allocate(52 * size);

		// Make sure that all offsets are aligned properly
		ASSERT_EQ((size_t)0, x % 52);

		offsets.push_back(x);
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());


	// Free some ranges in a non-contiguous manner to test range freeing and merging
	while (offsets.size() > 512) {
		std::uniform_int_distribution<size_t> dis(0, offsets.size() - 1);
		auto el = std::next(offsets.begin(), dis(gen));

		allocator.free(*el);
		offsets.erase(el);

		ASSERT_EQ(offsets.size(), allocator.numAllocations());
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());

	// Allocate some more ranges to test range reuse
	for (auto i = 0; i < 1500; ++i) {
		auto size = sizeDist(gen);

		// Use multiples of 52 since that is what the model code uses as the vertex stride
		size_t x = allocator.allocate(52 * size);

		// Make sure that all offsets are aligned properly
		ASSERT_EQ((size_t)0, x % 52);

		offsets.push_back(x);
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());

	// And now empty the entire allocator
	while (!offsets.empty()) {
		std::uniform_int_distribution<size_t> dis(0, offsets.size() - 1);
		auto el = std::next(offsets.begin(), dis(gen));

		allocator.free(*el);
		offsets.erase(el);

		ASSERT_EQ(offsets.size(), allocator.numAllocations());
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());
	ASSERT_EQ((size_t)0, offsets.size());
}

TEST(HeapAllocatorTests, largeAllocations) {
	HeapAllocator allocator(dummyResizer);

	SCP_vector<size_t> offsets;
	// Allocate enough small ranges to require a resize at some point
	for (auto i = 0; i < 100; ++i) {
		offsets.push_back(allocator.allocate(10 * 1024 * 1024));
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());

	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

	// Free some ranges in a non-contiguous manner to test range freeing and merging
	while (offsets.size() > 50) {
		std::uniform_int_distribution<size_t> dis(0, offsets.size() - 1);
		auto el = std::next(offsets.begin(), dis(gen));

		allocator.free(*el);
		offsets.erase(el);

		ASSERT_EQ(offsets.size(), allocator.numAllocations());
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());

	// Allocate some more ranges to test range reuse
	for (auto i = 0; i < 100; ++i) {
		offsets.push_back(allocator.allocate(10 * 1024 * 1024));
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());

	// And now empty the entire allocator
	while (!offsets.empty()) {
		std::uniform_int_distribution<size_t> dis(0, offsets.size() - 1);
		auto el = std::next(offsets.begin(), dis(gen));

		allocator.free(*el);
		offsets.erase(el);

		ASSERT_EQ(offsets.size(), allocator.numAllocations());
	}

	ASSERT_EQ(offsets.size(), allocator.numAllocations());
	ASSERT_EQ((size_t)0, offsets.size());
}
