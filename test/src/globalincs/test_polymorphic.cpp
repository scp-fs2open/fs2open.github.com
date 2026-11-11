#include <gtest/gtest.h>

#include "globalincs/vmallocator.h"

namespace {
struct Base {
	int base_value = 1;

	virtual ~Base() = default;
	virtual int kind() const { return 0; }
};

struct Derived : Base {
	int derived_value = 2;

	Derived() = default;
	explicit Derived(int value) : derived_value(value) {}

	int kind() const override { return 1; }
};
} // namespace

TEST(PolymorphicTests, defaultIsEmpty) {
	SCP_polymorphic<Base> empty;

	ASSERT_FALSE(empty);
	ASSERT_EQ(nullptr, empty.get());

	SCP_polymorphic<Base> copy(empty);
	ASSERT_FALSE(copy);
}

TEST(PolymorphicTests, makeForwardsArguments) {
	auto made = SCP_polymorphic<Base>::make<Derived>(42);

	ASSERT_TRUE(made);
	ASSERT_EQ(1, made->kind());
	ASSERT_EQ(42, static_cast<Derived*>(made.get())->derived_value);
}

TEST(PolymorphicTests, copyKeepsDynamicType) {
	auto original = SCP_polymorphic<Base>::make<Derived>(42);

	SCP_polymorphic<Base> copy(original);

	ASSERT_TRUE(copy);
	ASSERT_NE(original.get(), copy.get());
	ASSERT_EQ(1, copy->kind());
	ASSERT_EQ(42, static_cast<Derived*>(copy.get())->derived_value);
}

TEST(PolymorphicTests, copyIsDeep) {
	auto original = SCP_polymorphic<Base>::make<Derived>();
	SCP_polymorphic<Base> copy;
	copy = original;

	static_cast<Derived*>(copy.get())->derived_value = 7;

	ASSERT_EQ(2, static_cast<Derived*>(original.get())->derived_value);
	ASSERT_EQ(7, static_cast<Derived*>(copy.get())->derived_value);
}

TEST(PolymorphicTests, moveTransfersOwnership) {
	auto original = SCP_polymorphic<Base>::make<Derived>();
	auto object = original.get();

	SCP_polymorphic<Base> moved(std::move(original));

	ASSERT_EQ(object, moved.get());
	ASSERT_FALSE(original); // NOLINT(bugprone-use-after-move)
}

TEST(PolymorphicTests, vectorCopyKeepsTypes) {
	SCP_vector<SCP_polymorphic<Base>> original;
	original.push_back(SCP_polymorphic<Base>::make<Base>());
	original.push_back(SCP_polymorphic<Base>::make<Derived>());

	auto copy = original;

	ASSERT_EQ(0, copy[0]->kind());
	ASSERT_EQ(1, copy[1]->kind());
	ASSERT_NE(original[1].get(), copy[1].get());
}
