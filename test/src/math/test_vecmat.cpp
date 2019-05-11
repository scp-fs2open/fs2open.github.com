#include "math/staticrand.h"
#include "math/vecmat.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class VecmatTest : public test::FSTestFixture {
  public:
	VecmatTest() : test::FSTestFixture() { pushModDir("vecmat"); }

  protected:
	void SetUp() override { test::FSTestFixture::SetUp(); }
	void TearDown() override { test::FSTestFixture::TearDown(); }
};

TEST_F(VecmatTest, test_vm_vec_add)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v3;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		vm_vec_add(&v3, &v1, &v2);

		for (size_t i = 0; i < 3; ++i) {
			auto value1 = v1.a1d[i];
			auto value2 = v2.a1d[i];
			auto value3 = v3.a1d[i];

			ASSERT_FLOAT_EQ(value3, value1 + value2);
		}
	}
}

TEST_F(VecmatTest, test_vm_vec_sub)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v3;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		vm_vec_sub(&v3, &v1, &v2);

		for (size_t i = 0; i < 3; ++i) {
			auto value1 = v1.a1d[i];
			auto value2 = v2.a1d[i];
			auto value3 = v3.a1d[i];

			ASSERT_FLOAT_EQ(value3, value1 - value2);
		}
	}
}

TEST_F(VecmatTest, test_vm_vec_add2)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v1backup;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		v1backup.xyz = v1.xyz;

		vm_vec_add2(&v1, &v2);

		for (size_t i = 0; i < 3; ++i) {
			auto value1 = v1.a1d[i];
			auto value2 = v2.a1d[i];
			auto value3 = v1backup.a1d[i];

			ASSERT_FLOAT_EQ(value1, value2 + value3);
		}
	}
}

TEST_F(VecmatTest, test_vm_vec_sub2)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v1backup;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		v1backup.xyz = v1.xyz;

		vm_vec_sub2(&v1, &v2);

		for (size_t i = 0; i < 3; ++i) {
			auto value1       = v1.a1d[i];
			auto value2       = v2.a1d[i];
			auto value1Backup = v1backup.a1d[i];

			ASSERT_FLOAT_EQ(value1, value1Backup - value2);
		}
	}
}

TEST_F(VecmatTest, test_vm_vec_avg)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d vArray[2];
		vec3d vAvg;

		for (size_t i = 0; i < 2; i++) {
			static_randvec_unnormalized(rand32(), &vArray[i]);
		}

		vm_vec_avg(&vAvg, &vArray[0], &vArray[1]);

		float accX = 0.0f, accY = 0.0f, accZ = 0.0f;

		for (size_t i = 0; i < 2; ++i) {
			accX += vArray[i].xyz.x;
			accY += vArray[i].xyz.y;
			accZ += vArray[i].xyz.z;
		}

		accX /= 2;
		accY /= 2;
		accZ /= 2;

		ASSERT_FLOAT_EQ(vAvg.xyz.x, accX);
		ASSERT_FLOAT_EQ(vAvg.xyz.y, accY);
		ASSERT_FLOAT_EQ(vAvg.xyz.z, accZ);
	}
}

TEST_F(VecmatTest, test_vm_vec_avg3)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d vArray[3];
		vec3d vAvg;

		for (size_t i = 0; i < 3; i++) {
			static_randvec_unnormalized(rand32(), &vArray[i]);
		}

		vm_vec_avg3(&vAvg, &vArray[0], &vArray[1], &vArray[2]);

		float accX = 0.0f, accY = 0.0f, accZ = 0.0f;

		for (size_t i = 0; i < 3; ++i) {
			accX += vArray[i].xyz.x;
			accY += vArray[i].xyz.y;
			accZ += vArray[i].xyz.z;
		}

		accX /= 3;
		accY /= 3;
		accZ /= 3;

		ASSERT_FLOAT_EQ(vAvg.xyz.x, accX);
		ASSERT_FLOAT_EQ(vAvg.xyz.y, accY);
		ASSERT_FLOAT_EQ(vAvg.xyz.z, accZ);
	}
}

TEST_F(VecmatTest, test_vm_vec_avg4)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d vArray[4];
		vec3d vAvg;

		for (size_t i = 0; i < 4; i++) {
			static_randvec_unnormalized(rand32(), &vArray[i]);
		}

		vm_vec_avg4(&vAvg, &vArray[0], &vArray[1], &vArray[2], &vArray[3]);

		float accX = 0.0f, accY = 0.0f, accZ = 0.0f;

		for (size_t i = 0; i < 4; ++i) {
			accX += vArray[i].xyz.x;
			accY += vArray[i].xyz.y;
			accZ += vArray[i].xyz.z;
		}

		accX /= 4;
		accY /= 4;
		accZ /= 4;

		ASSERT_FLOAT_EQ(vAvg.xyz.x, accX);
		ASSERT_FLOAT_EQ(vAvg.xyz.y, accY);
		ASSERT_FLOAT_EQ(vAvg.xyz.z, accZ);
	}
}

TEST_F(VecmatTest, test_vm_vec_avg_n)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d vArray[1000];
		vec3d vAvg;

		for (size_t i = 0; i < 1000; i++) {
			static_randvec_unnormalized(rand32(), &vArray[i]);
		}

		vm_vec_avg_n(&vAvg, 1000, vArray);

		float accX = 0.0f, accY = 0.0f, accZ = 0.0f;

		for (size_t i = 0; i < 1000; ++i) {
			accX += vArray[i].xyz.x;
			accY += vArray[i].xyz.y;
			accZ += vArray[i].xyz.z;
		}

		accX /= 1000;
		accY /= 1000;
		accZ /= 1000;

		ASSERT_FLOAT_EQ(vAvg.xyz.x, accX);
		ASSERT_FLOAT_EQ(vAvg.xyz.y, accY);
		ASSERT_FLOAT_EQ(vAvg.xyz.z, accZ);
	}
}

TEST_F(VecmatTest, test_vm_vec_scale)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v1Unscaled;

		static_randvec_unnormalized(rand32(), &v1);

		v1Unscaled.xyz = v1.xyz;

		auto rand_scale = static_randf(rand32());

		vm_vec_scale(&v1, rand_scale);

		ASSERT_FLOAT_EQ(v1.xyz.x, v1Unscaled.xyz.x * rand_scale);
		ASSERT_FLOAT_EQ(v1.xyz.y, v1Unscaled.xyz.y * rand_scale);
		ASSERT_FLOAT_EQ(v1.xyz.z, v1Unscaled.xyz.z * rand_scale);
	}
}

TEST_F(VecmatTest, test_vm_vec4_scale)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec4 v1, v1Unscaled;

		for (size_t i = 0; i < 4; ++i) {
			v1.a1d[i]         = static_randf(rand32());
			v1Unscaled.a1d[i] = v1.a1d[i];
		}

		auto rand_scale = static_randf(rand32());

		vm_vec_scale(&v1, rand_scale);

		ASSERT_FLOAT_EQ(v1.xyzw.x, v1Unscaled.xyzw.x * rand_scale);
		ASSERT_FLOAT_EQ(v1.xyzw.y, v1Unscaled.xyzw.y * rand_scale);
		ASSERT_FLOAT_EQ(v1.xyzw.z, v1Unscaled.xyzw.z * rand_scale);
		ASSERT_FLOAT_EQ(v1.xyzw.w, v1Unscaled.xyzw.w * rand_scale);
	}
}

TEST_F(VecmatTest, test_vm_vec_copy_scale)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v1Unscaled, v2;

		static_randvec_unnormalized(rand32(), &v1);

		v1Unscaled.xyz = v1.xyz;

		auto rand_scale = static_randf(rand32());

		vm_vec_copy_scale(&v2, &v1, rand_scale);

		ASSERT_FLOAT_EQ(v2.xyz.x, v1Unscaled.xyz.x * rand_scale);
		ASSERT_FLOAT_EQ(v2.xyz.y, v1Unscaled.xyz.y * rand_scale);
		ASSERT_FLOAT_EQ(v2.xyz.z, v1Unscaled.xyz.z * rand_scale);
	}
}

TEST_F(VecmatTest, test_vm_vec_scale_add)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v3;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		auto rand_scale = static_randf(rand32());

		vm_vec_scale_add(&v3, &v1, &v2, rand_scale);

		ASSERT_FLOAT_EQ(v3.xyz.x, v1.xyz.x + (rand_scale * v2.xyz.x));
		ASSERT_FLOAT_EQ(v3.xyz.y, v1.xyz.y + (rand_scale * v2.xyz.y));
		ASSERT_FLOAT_EQ(v3.xyz.z, v1.xyz.z + (rand_scale * v2.xyz.z));
	}
}

TEST_F(VecmatTest, test_vm_vec_scale_sub)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v3;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		auto rand_scale = static_randf(rand32());

		vm_vec_scale_sub(&v3, &v1, &v2, rand_scale);

		ASSERT_FLOAT_EQ(v3.xyz.x, v1.xyz.x - (rand_scale * v2.xyz.x));
		ASSERT_FLOAT_EQ(v3.xyz.y, v1.xyz.y - (rand_scale * v2.xyz.y));
		ASSERT_FLOAT_EQ(v3.xyz.z, v1.xyz.z - (rand_scale * v2.xyz.z));
	}
}

TEST_F(VecmatTest, test_vm_vec_scale_add2)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, v1Unscaled;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		v1Unscaled.xyz = v1.xyz;

		auto rand_scale = static_randf(rand32());

		vm_vec_scale_add2(&v1, &v2, rand_scale);

		ASSERT_FLOAT_EQ(v1.xyz.x, v1Unscaled.xyz.x + (rand_scale * v2.xyz.x));
		ASSERT_FLOAT_EQ(v1.xyz.y, v1Unscaled.xyz.y + (rand_scale * v2.xyz.y));
		ASSERT_FLOAT_EQ(v1.xyz.z, v1Unscaled.xyz.z + (rand_scale * v2.xyz.z));
	}
}

TEST_F(VecmatTest, test_vm_vec_scale2)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v1Unscaled;

		static_randvec_unnormalized(rand32(), &v1);

		v1Unscaled.xyz = v1.xyz;

		auto rand_scale_n = static_randf(rand32());
		auto rand_scale_d = static_randf(rand32());

		vm_vec_scale2(&v1, rand_scale_n, rand_scale_d);

		ASSERT_FLOAT_EQ(v1.xyz.x, v1Unscaled.xyz.x * (rand_scale_n / rand_scale_d));
		ASSERT_FLOAT_EQ(v1.xyz.y, v1Unscaled.xyz.y * (rand_scale_n / rand_scale_d));
		ASSERT_FLOAT_EQ(v1.xyz.z, v1Unscaled.xyz.z * (rand_scale_n / rand_scale_d));
	}
}

TEST_F(VecmatTest, test_vm_vec_mag)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v;

		static_randvec_unnormalized(rand32(), &v);

		auto magnitude = fl_sqrt((v.xyz.x * v.xyz.x) + (v.xyz.y * v.xyz.y) + (v.xyz.z * v.xyz.z));

		ASSERT_FLOAT_EQ(magnitude, vm_vec_mag(&v));
	}
}

TEST_F(VecmatTest, test_vm_vec_mag_squared) {
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v;

		static_randvec_unnormalized(rand32(), &v);

		auto magnitude = (v.xyz.x * v.xyz.x) + (v.xyz.y * v.xyz.y) + (v.xyz.z * v.xyz.z);

		ASSERT_FLOAT_EQ(magnitude, vm_vec_mag_squared(&v));
	}
}

TEST_F(VecmatTest, test_vm_vec_dist) {
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, t;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		auto distance = vm_vec_dist(&v1, &v2);

		vm_vec_sub(&t, &v1, &v2);
		auto test_distance = vm_vec_mag(&t);

		ASSERT_FLOAT_EQ(distance, test_distance);
	}
}

TEST_F(VecmatTest, test_vm_vec_dist_squared)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, t;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		auto distance = vm_vec_dist_squared(&v1, &v2);

		vm_vec_sub(&t, &v1, &v2);
		auto test_distance = vm_vec_mag_squared(&t);

		ASSERT_FLOAT_EQ(distance, test_distance);
	}
}

TEST_F(VecmatTest, test_vm_vec_mag_quick) {
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v;

		static_randvec_unnormalized(rand32(), &v);

		auto magnitude = vm_vec_mag(&v);

		ASSERT_NEAR(magnitude, vm_vec_mag_quick(&v), 0.1f);
	}
}

TEST_F(VecmatTest, test_vm_vec_dist_quick)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, t;

		static_randvec_unnormalized(rand32(), &v1);
		static_randvec_unnormalized(rand32(), &v2);

		auto distance = vm_vec_dist_quick(&v1, &v2);
		auto real_distance = vm_vec_dist(&v1, &v2);

		ASSERT_NEAR(distance, real_distance, 0.12f);
	}
}