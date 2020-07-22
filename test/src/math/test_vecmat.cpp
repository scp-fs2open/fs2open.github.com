
#include <gtest/gtest.h>
#include <math/vecmat.h>
#include <math/staticrand.h>

#include "util/FSTestFixture.h"


// "Correct" answers for matrix functions provided by Wolfram Mathematica 11
// Vector function tests oringially made by The_E

const matrix uninvertable = {{{{{{1.0f, 1.0f, 1.0f}}}, {{{1.0f, 1.0f, 1.0f}}}, {{{1.0f, 1.0f, 1.0f}}}}}};
const matrix input1 = {{{{{{4509.0f, 8600.0f, 5523.0f}}}, {{{5276.0f, 668.0f, 462.0f}}}, {{{145.0f, 8490.0f, 5111.0f}}}}}};
const matrix input2 = {{{{{{0.7703f, 0.8180f, 0.8535f}}}, {{{0.6409f, 0.0125f, 0.1777f}}}, {{{0.3785f, 0.0090f, 0.3851f}}}}}};
const matrix input3 = {{{{{{0.5118f, 27.86f, 1.0f}}}, {{{0.0f, 777.2f, 0.0596f}}}, {{{2.5f, 67.95f, 0.0058f}}}}}};

#define EXPECT_MATRIX_NEAR(out,matrix) EXPECT_NEAR(error(&out,matrix), 0.0f, 0.001f);

matrix make_matrix(float a, float b, float c, float d, float e, float f, float g, float h, float i) {
	matrix out = { {{{{{a, b, c}}}, {{{d, e, f}}}, {{{g, h, i}}}}} };
	return out;
}

float error(matrix* val, matrix target) {
	float totalError = 0;
	for (int i = 0; i < 9; i++) {
		float diff = val->a1d[i] - target.a1d[i];
		totalError += diff * diff;
	}
	return totalError;
}

class VecmatTest : public test::FSTestFixture {
public:
	VecmatTest() : test::FSTestFixture() { pushModDir("vecmat"); }

protected:
	void SetUp() override 
	{ 
		test::FSTestFixture::SetUp(); 
		srand(1); 
	}
	void TearDown() override { test::FSTestFixture::TearDown(); }
};

/*TEST_F(VecmatTest, matrixInvert) {

	matrix out;
	matrix out2;

	// make sure inverts work, of course
	EXPECT_TRUE(vm_inverse_matrix(&out, &input2));
	EXPECT_MATRIX_NEAR(out, make_matrix(
		-0.0223985f, 2.1415f, -0.938528f,
		1.25112f, 0.184007f, -2.85778f,
		-0.00722484f, -2.1091f, 3.58596f));

	//make sure multiplying by the original gives you the identity
	vm_matrix_x_matrix(&out2, &out, &input2);
	EXPECT_MATRIX_NEAR(out2, vmd_identity_matrix);

	// make sure order doesn't matter
	vm_matrix_x_matrix(&out2, &input2, &out);
	EXPECT_MATRIX_NEAR(out2, vmd_identity_matrix);

	//make sure inverting twice gets you back where you started
	EXPECT_TRUE(vm_inverse_matrix(&out2, &out));
	EXPECT_MATRIX_NEAR(out2, input2);

	//make sure uninvertable matrices give you false and a zero matrix
	EXPECT_FALSE(vm_inverse_matrix(&out, &uninvertable));
	EXPECT_MATRIX_NEAR(out, vmd_zero_matrix);
}

TEST_F(VecmatTest, matrixAdd) {

	matrix out;

	vm_matrix_add(&out, &input1, &input2);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4509.77f, 8600.82f, 5523.85f,
		5276.64f, 668.013f, 462.178f,
		145.379f, 8490.01f, 5111.39f));

	vm_matrix_add(&out, &out, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4510.28f, 8628.68f, 5524.85f,
		5276.64f, 1445.21f, 462.237f,
		147.879f, 8557.96f, 5111.39f));

	vm_matrix_add(&out, &input2, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		1.2821f, 28.678f, 1.8535f,
		0.6409f, 777.213f, 0.2373f,
		2.8785f, 67.959f, 0.3909f));
}

TEST_F(VecmatTest, matrixSub) {

	matrix out;

	vm_matrix_sub(&out, &input1, &input2);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4508.23f, 8599.18f, 5522.15f,
		5275.36f, 667.988f, 461.822f,
		144.622f, 8489.99f, 5110.61f));

	vm_matrix_sub(&out, &out, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4507.72f, 8571.32f, 5521.15f,
		5275.36f, -109.213f, 461.763f,
		142.122f, 8422.04f, 5110.61f));

	vm_matrix_sub(&out, &input2, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		0.2585f, -27.042f, -0.1465f,
		0.6409f, -777.188f, 0.1181f,
		-2.1215f, -67.941f, 0.3793f));
}

TEST_F(VecmatTest, matrixAdd2) {

	matrix out;

	out = input1;
	vm_matrix_add2(&out, &input2);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4509.77f, 8600.82f, 5523.85f,
		5276.64f, 668.013f, 462.178f,
		145.379f, 8490.01f, 5111.39f));

	vm_matrix_add2(&out, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4510.28f, 8628.68f, 5524.85f,
		5276.64f, 1445.21f, 462.237f,
		147.879f, 8557.96f, 5111.39f));

	out = input2;
	vm_matrix_add2(&out, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		1.2821f, 28.678f, 1.8535f,
		0.6409f, 777.213f, 0.2373f,
		2.8785f, 67.959f, 0.3909f));
}

TEST_F(VecmatTest, matrixSub2) {

	matrix out;

	out = input1;
	vm_matrix_sub2(&out, &input2);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4508.23f, 8599.18f, 5522.15f,
		5275.36f, 667.988f, 461.822f,
		144.622f, 8489.99f, 5110.61f));

	vm_matrix_sub2(&out, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		4507.72f, 8571.32f, 5521.15f,
		5275.36f, -109.213f, 461.763f,
		142.122f, 8422.04f, 5110.61f));

	out = input2;
	vm_matrix_sub2(&out, &input3);
	EXPECT_MATRIX_NEAR(out, make_matrix(
		0.2585f, -27.042f, -0.1465f,
		0.6409f, -777.188f, 0.1181f,
		-2.1215f, -67.941f, 0.3793f));
}

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
			auto value1 = v1.a1d[i];
			auto value2 = v2.a1d[i];
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
			v1.a1d[i] = static_randf(rand32());
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

TEST_F(VecmatTest, test_vm_vec_normalize)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v, vBackup;

		static_randvec_unnormalized(rand32(), &v);
		vBackup.xyz = v.xyz;

		auto magnitude = vm_vec_normalize(&v);
		auto test_magnitude = vm_vec_mag(&vBackup);

		ASSERT_FLOAT_EQ(magnitude, test_magnitude);

		auto inverse_magnitude = 1.0f / test_magnitude;
		vm_vec_scale(&vBackup, inverse_magnitude);

		ASSERT_FLOAT_EQ(v.xyz.x, vBackup.xyz.x);
		ASSERT_FLOAT_EQ(v.xyz.y, vBackup.xyz.y);
		ASSERT_FLOAT_EQ(v.xyz.z, vBackup.xyz.z);
	}
}

TEST_F(VecmatTest, test_vm_vec_copy_normalize)
{
	for (size_t loop = 0; loop < 1000; ++loop) {
		vec3d v1, v2, vBackup;

		static_randvec_unnormalized(rand32(), &v1);
		vBackup.xyz = v1.xyz;

		auto magnitude = vm_vec_copy_normalize(&v2, &v1);

		ASSERT_FLOAT_EQ(v1.xyz.x, vBackup.xyz.x);
		ASSERT_FLOAT_EQ(v1.xyz.y, vBackup.xyz.y);
		ASSERT_FLOAT_EQ(v1.xyz.z, vBackup.xyz.z);

		auto test_magnitude = vm_vec_mag(&vBackup);

		ASSERT_FLOAT_EQ(magnitude, test_magnitude);

		auto inverse_magnitude = 1.0f / test_magnitude;
		vm_vec_scale(&vBackup, inverse_magnitude);

		ASSERT_FLOAT_EQ(v2.xyz.x, vBackup.xyz.x);
		ASSERT_FLOAT_EQ(v2.xyz.y, vBackup.xyz.y);
		ASSERT_FLOAT_EQ(v2.xyz.z, vBackup.xyz.z);
	}
}*/


TEST_F(VecmatTest, basic_math)
{
	/*vec3d goal_f;
	vm_vec_make(&goal_f, -0.809975f, 0.494356f, 0.315519f);
	matrix orient = make_matrix(0.740481138f, 
		0.217397153f,
		0.635945082f,
		0.217397153f,
		0.817887902f,
		-0.532726824f,
		-0.635945082f,
		0.532726824f,
		0.558369040f);
	vm_orthogonalize_matrix(&orient);
	float delta_bank = 0.0f;

	vec3d rot_axis;
	vm_vec_cross(&rot_axis, &orient.vec.fvec, &goal_f);
	float dot = vm_vec_dot(&orient.vec.fvec, &goal_f);
	float mag = fmin(vm_vec_mag(&rot_axis), 1.0f);
	vec3d theta_goal;
	vm_vec_make(&theta_goal, 0, 0, delta_bank);
	vec3d local_rot_axis;
	vm_vec_rotate(&local_rot_axis, &rot_axis, &orient);
	vm_vec_copy_scale(&theta_goal, &local_rot_axis, (dot > 0 ? asinf(mag) : PI - asinf(mag)) / mag);

	theta_goal.xyz.z = delta_bank;
	rot_axis = theta_goal;

	//	normalize rotation axis and determine total rotation angle
	float theta = vm_vec_mag(&rot_axis);
	vm_vec_scale(&rot_axis, 1 / theta);

	matrix Mtemp1;
	vm_quaternion_rotate(&Mtemp1, theta, &rot_axis);
	matrix next_orient;
	vm_matrix_x_matrix(&next_orient, &orient, &Mtemp1);
	vec3d error = next_orient.vec.fvec - goal_f;
	EXPECT_LE(vm_vec_mag(&error), 1e-5);*/
}


TEST_F(VecmatTest, turn_towards_matrix)
{
	matrix goal_mat = make_matrix(frand()-0.5f,
		frand() - 0.5f,
		frand() - 0.5f,
		frand() - 0.5f,
		frand() - 0.5f,
		-frand() - 0.5f,
		-frand() - 0.5f,
		frand() - 0.5f,
		frand() - 0.5f);
	vm_orthogonalize_matrix(&goal_mat);

	matrix orient;// = IDENTITY_MATRIX;
	static_randvec(rand32(), &orient.vec.fvec);
	static_randvec(rand32(), &orient.vec.uvec);
	static_randvec(rand32(), &orient.vec.rvec);
	vm_orthogonalize_matrix(&orient);
	vec3d w_in;
	static_randvec(rand32(), &w_in);
	vm_vec_make(&w_in, 0, 0, 0);// -0.220586f, -0.441793f, -0.170271f);
	float delta_t = 0.2f;
	float delta_bank = 0.0f;
	matrix new_orient;
	vec3d w_out;
	vm_vec_make(&w_out, 0, 0, 0);
	vec3d vel_limit;
	vm_vec_make(&vel_limit, 0.5f, 0.5f, 0.5f);
	vec3d acc_limit;
	vm_vec_make(&acc_limit, 0.5f, 0.5f, 0.5f);
	int finished = 0;

	printf("goal_mat %f %f %f\n", goal_mat.vec.rvec.xyz.x, goal_mat.vec.rvec.xyz.y, goal_mat.vec.rvec.xyz.z);
	printf("goal_mat %f %f %f\n", goal_mat.vec.uvec.xyz.x, goal_mat.vec.uvec.xyz.y, goal_mat.vec.uvec.xyz.z);
	printf("goal_mat %f %f %f\n", goal_mat.vec.fvec.xyz.x, goal_mat.vec.fvec.xyz.y, goal_mat.vec.fvec.xyz.z);
	printf("orig_w %f %f %f\n", w_in.xyz.x, w_in.xyz.y, w_in.xyz.z);
	for (int i = 0; i < 100; i++) {
		if (i == 1) {
			printf("");
		}
		delta_t = frand();
		delta_t = delta_t * delta_t + 0.01f;
		vm_better_matrix_interpolate(&goal_mat, &orient, &w_in, delta_t, &new_orient, &w_out, &vel_limit, &acc_limit);
		printf("%i : %f\n", i, delta_t);
		printf("w_out %f %f %f\n", w_out.xyz.x, w_out.xyz.y, w_out.xyz.z);
		printf("new_orient %f %f %f\n", new_orient.vec.rvec.xyz.x, new_orient.vec.rvec.xyz.y, new_orient.vec.rvec.xyz.z);
		printf("new_orient %f %f %f\n", new_orient.vec.uvec.xyz.x, new_orient.vec.uvec.xyz.y, new_orient.vec.uvec.xyz.z);
		printf("new_orient %f %f %f\n", new_orient.vec.fvec.xyz.x, new_orient.vec.fvec.xyz.y, new_orient.vec.fvec.xyz.z);
		if (vm_vec_mag(&w_out) <= 0.001 && vm_vec_mag(&(new_orient.vec.fvec - goal_mat.vec.fvec)) <= 1e-5 &&
			    vm_vec_mag(&(new_orient.vec.uvec - goal_mat.vec.uvec)) <= 1e-5 &&
			    vm_vec_mag(&(new_orient.vec.rvec - goal_mat.vec.rvec)) <= 1e-5) {
			finished++;
			if (finished > 5)
				break;
		}
		orient = new_orient;
		w_in = w_out;
	}

}

TEST_F(VecmatTest, turn_towards_vector)
{	vec3d goal_vec;
	static_randvec(rand32(), &goal_vec);
	//vm_vec_make(&goal_vec, -0.0f, 0.0f, -1.0f);
	vm_vec_normalize(&goal_vec);

	matrix orient;// = IDENTITY_MATRIX;
	static_randvec(rand32(), &orient.vec.fvec);
	static_randvec(rand32(), &orient.vec.uvec);
	static_randvec(rand32(), &orient.vec.rvec);
	vm_orthogonalize_matrix(&orient);
	vec3d w_in;
	static_randvec(rand32(), &w_in);
	//vm_vec_make(&w_in, 0, 0, 0);// -0.220586f, -0.441793f, -0.170271f);
	float delta_t = 0.2f;
	float delta_bank = 0.0f;
	matrix new_orient;
	vec3d w_out;
	vm_vec_make(&w_out, 0, 0, 0);
	vec3d vel_limit;
	vm_vec_make(&vel_limit, 0.5f, 0.5f, 0.5f);
	vec3d acc_limit;
	vm_vec_make(&acc_limit, 0.5f, 0.5f, 0.5f);
	int finished = 0;

	printf("goal_vec %f %f %f\n", goal_vec.xyz.x, goal_vec.xyz.y, goal_vec.xyz.z);
	printf("orig_w %f %f %f\n", w_in.xyz.x, w_in.xyz.y, w_in.xyz.z);
	for (int i = 0; i < 100; i++) {
		if (i == 1) {
			printf("");
		}
		delta_t = frand();
		delta_t = delta_t * delta_t + 0.01f;
		vm_better_forward_interpolate(&goal_vec, &orient, &w_in, delta_t, delta_bank , &new_orient, &w_out, &vel_limit, &acc_limit);
		printf("%i : %f\n", i, delta_t);
		printf("w_out %f %f %f\n", w_out.xyz.x, w_out.xyz.y, w_out.xyz.z);
		printf("dist %f\n", vm_vec_mag(&(new_orient.vec.fvec - goal_vec)));
		printf("new_orient %f %f %f\n", new_orient.vec.rvec.xyz.x, new_orient.vec.rvec.xyz.y, new_orient.vec.rvec.xyz.z);
		printf("new_orient %f %f %f\n", new_orient.vec.uvec.xyz.x, new_orient.vec.uvec.xyz.y, new_orient.vec.uvec.xyz.z);
		printf("new_orient %f %f %f\n", new_orient.vec.fvec.xyz.x, new_orient.vec.fvec.xyz.y, new_orient.vec.fvec.xyz.z);
		if (vm_vec_mag(&w_out) <= 0.001 && vm_vec_mag(&(new_orient.vec.fvec - goal_vec)) <= 1e-5) {
			finished++;
			if (finished > 5)
				break;
		}
		orient = new_orient;
		w_in = w_out;
	}

}

//
//TEST_F(VecmatTest, turn_towards_vector)
//{
//	vec3d goal_vec;
//	//static_randvec(rand32(), &goal_vec);
//	vm_vec_make(&goal_vec, 0, 0, -1);
//	matrix orient;// = MATRIX_MAKE(0.646844f, 0.593360f, 0.479079f, 0.062571f, 0.584789f, -0.808769f, -0.760051f, 0.553124f, 0.341140f);
//	orient = IDENTITY_MATRIX;
//	/*
//	static_randvec(rand32(), &orient.vec.fvec);
//	static_randvec(rand32(), &orient.vec.uvec);
//	static_randvec(rand32(), &orient.vec.rvec);
//	vm_orthogonalize_matrix(&orient);
//	*/
//	vec3d w_in;
//	//static_randvec(rand32(), &w_in);
//	vm_vec_make(&w_in, -0.102979, 0.943805, 0.314051);
//	float delta_t = 1.0f;
//	float delta_bank = 0.0f;
//	matrix new_orient;
//	vec3d w_out;
//	vm_vec_make(&w_out, 0.0f, 0.0f, 0.0f);
//	vec3d vel_limit;
//	vm_vec_make(&vel_limit, 0.2f, 0.2f, 0.2f);
//	vec3d acc_limit;
//	vm_vec_make(&acc_limit, 0.5f, 0.5f, 0.5f);
//	float finished = 0;
//
//	printf("goal_vec %f %f %f\n", goal_vec.xyz.x, goal_vec.xyz.y, goal_vec.xyz.z);
//	printf("orig_w %f %f %f\n", w_in.xyz.x, w_in.xyz.y, w_in.xyz.z);
//	for (int i = 0; i < 300; i++) {
//		delta_t = frand()+0.01f;
//		vm_forward_interpolate(&goal_vec, &orient, &w_in, delta_t, delta_bank, &new_orient, &w_out, &vel_limit, &acc_limit, 1);
//		printf("%i\n",i);
//		printf("w_out %f %f %f\n", w_out.xyz.x, w_out.xyz.y, w_out.xyz.z);
//		printf("new_orient %f %f %f\n", new_orient.vec.rvec.xyz.x, new_orient.vec.rvec.xyz.y, new_orient.vec.rvec.xyz.z);
//		printf("new_orient %f %f %f\n", new_orient.vec.uvec.xyz.x, new_orient.vec.uvec.xyz.y, new_orient.vec.uvec.xyz.z);
//		printf("new_orient %f %f %f\n", new_orient.vec.fvec.xyz.x, new_orient.vec.fvec.xyz.y, new_orient.vec.fvec.xyz.z);
//		if (vm_vec_equal(w_out, vmd_zero_vector) &&
//			vm_vec_equal(new_orient.vec.fvec, goal_vec)) {
//			finished++;
//			if (finished > 3)
//				break;
//
//		}
//		orient = new_orient;
//		w_in = w_out;
//	}
//
//}

/*
goal_vec -0.809975 0.494356 0.315519
delta t 0.915117919

w_out 0.057321 -0.075787 0.000093
new_orient 0.646844 0.593360 0.479079
new_orient 0.062571 0.584789 -0.808769
new_orient -0.760051 0.553124 0.341140

TEST_F(VecmatTest, scalar_itnerp)
{
	float goal = 1.0f;
	float w_in = 0.6f;
	float delta_t = 0.1f;
	float vel_limit = 0.5f;
	float acc_limit = 0.5f;

	printf("goal %f\n", goal);
	printf("orig_w %f\n", w_in);
	float pos = 0;
	for (int i = 0; i < 100; i++) {
		pos += vm_scalar_interpolate(goal - pos, delta_t, &w_in, vel_limit, acc_limit);
		printf("%i w_out %f new_pos %f\n", i, w_in, pos);
		if (w_in == 0 && pos == goal)
			break;
	}

}
*/

