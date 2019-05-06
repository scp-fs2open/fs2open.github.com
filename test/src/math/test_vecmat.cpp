#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "math/vecmat.h"
#include "math/staticrand.h"

class VecmatTest : public test::FSTestFixture {
public:
	VecmatTest() : test::FSTestFixture() {
		pushModDir("vecmat");
	}

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	void TearDown() override {
		test::FSTestFixture::TearDown();
	}
};

TEST_F(VecmatTest, test_vm_vec_add) {
	vec3d v1, v2, v3;

	vm_vec_make(&v1, 0.0f, 0.0f, 0.0f);
	vm_vec_make(&v2, 1.0f, 1.0f, 1.0f);

	vm_vec_add(&v3, &v1, &v2);

	for (size_t i = 0; i < 3; ++i) {
		ASSERT_GE(v3.a1d[i], v1.a1d[i]);
		ASSERT_GE(v3.a1d[i], v2.a1d[i]);
	}
}