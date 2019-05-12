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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

		vm_vec_copy_scale(&v1backup, &v1, 1.0f);

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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

		vm_vec_copy_scale(&v1backup, &v1, 1.0f);

		vm_vec_sub2(&v1, &v2);

		for (size_t i = 0; i < 3; ++i) {
			auto value1	   = v1.a1d[i];
			auto value2	   = v2.a1d[i];
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
			static_randvec(rand32(), &vArray[i]);
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
			static_randvec(rand32(), &vArray[i]);
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
			static_randvec(rand32(), &vArray[i]);
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
			static_randvec(rand32(), &vArray[i]);
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

		static_randvec(rand32(), &v1);

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
			v1.a1d[i]		 = static_randf(rand32());
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

		static_randvec(rand32(), &v1);

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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

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

		static_randvec(rand32(), &v1);
		static_randvec(rand32(), &v2);

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

		static_randvec(rand32(), &v1);

		v1Unscaled.xyz = v1.xyz;

		auto rand_scale_n = static_randf(rand32());
		auto rand_scale_d = static_randf(rand32());

		vm_vec_scale2(&v1, rand_scale_n, rand_scale_d);

		ASSERT_FLOAT_EQ(v1.xyz.x, v1Unscaled.xyz.x * (rand_scale_n / rand_scale_d));
		ASSERT_FLOAT_EQ(v1.xyz.y, v1Unscaled.xyz.y * (rand_scale_n / rand_scale_d));
		ASSERT_FLOAT_EQ(v1.xyz.z, v1Unscaled.xyz.z * (rand_scale_n / rand_scale_d));
	}
}

TEST_F(VecmatTest, test_vm_vec_project_parallel)
{
	//tests that vm_vec_project_parallel, correctly projects a
	//vector onto the unit vectors
	//expect that only the component of the respective unit vector remains
	vec3d v1;
	static_randvec(rand32(), &v1);

	vec3d unit1 = {};
	unit1.xyz.x = 1.0f;

	vec3d unit2 = {};
	unit2.xyz.y = 1.0f;

	vec3d unit3 = {};
	unit3.xyz.z = 1.0f;

	vec3d resx = {};
	float magx = vm_vec_projection_parallel(&resx, &v1, &unit1);
	ASSERT_FLOAT_EQ(magx, resx.xyz.x);
	ASSERT_FLOAT_EQ(resx.xyz.x, v1.xyz.x);
	ASSERT_FLOAT_EQ(resx.xyz.y, 0.0f);
	ASSERT_FLOAT_EQ(resx.xyz.z, 0.0f);

	vec3d resy = {};
	float magy = vm_vec_projection_parallel(&resy, &v1, &unit2);
	ASSERT_FLOAT_EQ(magy, resy.xyz.y);
	ASSERT_FLOAT_EQ(resy.xyz.x, 0.0f);
	ASSERT_FLOAT_EQ(resy.xyz.y, v1.xyz.y);
	ASSERT_FLOAT_EQ(resy.xyz.z, 0.0f);


	vec3d resz = {};
	float magz = vm_vec_projection_parallel(&resz, &v1, &unit3);
	ASSERT_FLOAT_EQ(magz, resz.xyz.z);
	ASSERT_FLOAT_EQ(resz.xyz.x, 0.0f);
	ASSERT_FLOAT_EQ(resz.xyz.y, 0.0f);
	ASSERT_FLOAT_EQ(resz.xyz.z, v1.xyz.z);
}

TEST_F(VecmatTest, test_vm_vec_project_plane)
{
	vec3d v1;
	static_randvec(rand32(), &v1);

	vec3d unit1 = {};
	unit1.xyz.x = 1.0f;

	vec3d unit2 = {};
	unit2.xyz.y = 1.0f;

	vec3d unit3 = {};
	unit3.xyz.z = 1.0f;

	vec3d resx = {};
	vm_vec_projection_onto_plane(&resx, &v1, &unit1);
	ASSERT_FLOAT_EQ(resx.xyz.x, 0.0f);
	ASSERT_FLOAT_EQ(resx.xyz.y, v1.xyz.y);
	ASSERT_FLOAT_EQ(resx.xyz.z, v1.xyz.z);

	vec3d resy = {};
	vm_vec_projection_onto_plane(&resy, &v1, &unit2);
	ASSERT_FLOAT_EQ(resy.xyz.x, v1.xyz.x);
	ASSERT_FLOAT_EQ(resy.xyz.y, 0.0f);
	ASSERT_FLOAT_EQ(resy.xyz.z, v1.xyz.z);

	vec3d resz = {};
	vm_vec_projection_onto_plane(&resz, &v1, &unit3);
	ASSERT_FLOAT_EQ(resz.xyz.x, v1.xyz.x);
	ASSERT_FLOAT_EQ(resz.xyz.y, v1.xyz.y);
	ASSERT_FLOAT_EQ(resz.xyz.z, 0.0f);
}
