
#include <gtest/gtest.h>
#include <math/vecmat.h>

#include "util/FSTestFixture.h"


// "Correct" answers here provided by Wolfram Mathematica 11

const matrix uninvertable = {{{{{{1.0f, 1.0f, 1.0f}}}, {{{1.0f, 1.0f, 1.0f}}}, {{{1.0f, 1.0f, 1.0f}}}}}};
const matrix input1 = {{{{{{4509.0f, 8600.0f, 5523.0f}}}, {{{5276.0f, 668.0f, 462.0f}}}, {{{145.0f, 8490.0f, 5111.0f}}}}}};
const matrix input2 = {{{{{{0.7703f, 0.8180f, 0.8535f}}}, {{{0.6409f, 0.0125f, 0.1777f}}}, {{{0.3785f, 0.0090f, 0.3851f}}}}}};
const matrix input3 = {{{{{{0.5118f, 27.86f, 1.0f}}}, {{{0.0f, 777.2f, 0.0596f}}}, {{{2.5f, 67.95f, 0.0058f}}}}}};

#define ERROR_TOLERANCE 0.001f

#define MATRIX(a,b,c,d,e,f,g,h,i) {{{{{{a, b, c}}}, {{{d, e, f}}}, {{{g, h, i}}}}}}
#define CHECK_MATRIX(out,matrix) EXPECT_LE(error(&out,matrix), ERROR_TOLERANCE)

float error(matrix* val, matrix target) {
	float totalError = 0;
	for (int i = 0; i < 9; i++) {
		float diff = val->a1d[i] - target.a1d[i];
		totalError += diff * diff;
	}
	return totalError;
}

TEST(VecMatTest, matrixInvert) {

	matrix out;
	matrix out2;

	// make sure inverts work, of course
	EXPECT_TRUE(vm_inverse_matrix(&out, &input2));
	CHECK_MATRIX(out, MATRIX(
		-0.0223985f,2.1415f,-0.938528f,
		1.25112f,0.184007f,-2.85778f,
		-0.00722484f,-2.1091f,3.58596f));

	//make sure multiplying by the original gives you the identity
	vm_matrix_x_matrix(&out2, &out, &input2);
	CHECK_MATRIX(out2, IDENTITY_MATRIX);

	// make sure order doesn't matter
	vm_matrix_x_matrix(&out2, &input2, &out);
	CHECK_MATRIX(out2, IDENTITY_MATRIX);

	//make sure inverting twice gets you back where you started
	EXPECT_TRUE(vm_inverse_matrix(&out2, &out));
	CHECK_MATRIX(out2, input2);

	//make sure uninvertable matrices give you false and a zero matrix
	EXPECT_FALSE(vm_inverse_matrix(&out, &uninvertable));
	CHECK_MATRIX(out, ZERO_MATRIX);
}

TEST(VecMatTest, matrixAdd) {

	matrix out;

	vm_matrix_add(&out, &input1, &input2);
	CHECK_MATRIX(out, MATRIX(
		4509.77f, 8600.82f, 5523.85f,
		5276.64f, 668.013f, 462.178f, 
		145.379f, 8490.01f, 5111.39f));

	vm_matrix_add(&out, &out, &input3);
	CHECK_MATRIX(out, MATRIX(
		4510.28f, 8628.68f, 5524.85f, 
		5276.64f, 1445.21f, 462.237f, 
		147.879f, 8557.96f, 5111.39f));

	vm_matrix_add(&out, &input2, &input3);
	CHECK_MATRIX(out, MATRIX(
		1.2821f, 28.678f, 1.8535f, 
		0.6409f, 777.213f, 0.2373f, 
		2.8785f, 67.959f, 0.3909f));
}

TEST(VecMatTest, matrixSub) {

	matrix out;

	vm_matrix_sub(&out, &input1, &input2);
	CHECK_MATRIX(out, MATRIX(
		4508.23f, 8599.18f, 5522.15f, 
		5275.36f, 667.988f, 461.822f, 
		144.622f, 8489.99f, 5110.61f));

	vm_matrix_sub(&out, &out, &input3);
	CHECK_MATRIX(out, MATRIX(
		4507.72f, 8571.32f, 5521.15f, 
		5275.36f, -109.213f, 461.763f, 
		142.122f, 8422.04f, 5110.61f));

	vm_matrix_sub(&out, &input2, &input3);
	CHECK_MATRIX(out, MATRIX(
		0.2585f, -27.042f, -0.1465f, 
		0.6409f, -777.188f, 0.1181f, 
		-2.1215f, -67.941f, 0.3793f));
}

TEST(VecMatTest, matrixAdd2) {

	matrix out;

	out = input1;
	vm_matrix_add2(&out, &input2);
	CHECK_MATRIX(out, MATRIX(
		4509.77f, 8600.82f, 5523.85f,
		5276.64f, 668.013f, 462.178f, 
		145.379f, 8490.01f, 5111.39f));

	vm_matrix_add2(&out, &input3);
	CHECK_MATRIX(out, MATRIX(
		4510.28f, 8628.68f, 5524.85f, 
		5276.64f, 1445.21f, 462.237f, 
		147.879f, 8557.96f, 5111.39f));

	out = input2;
	vm_matrix_add2(&out, &input3);
	CHECK_MATRIX(out, MATRIX(
		1.2821f, 28.678f, 1.8535f, 
		0.6409f, 777.213f, 0.2373f, 
		2.8785f, 67.959f, 0.3909f));
}

TEST(VecMatTest, matrixSub2) {

	matrix out;

	out = input1;
	vm_matrix_sub2(&out, &input2);
	CHECK_MATRIX(out, MATRIX(
		4508.23f, 8599.18f, 5522.15f, 
		5275.36f, 667.988f, 461.822f, 
		144.622f, 8489.99f, 5110.61f));

	vm_matrix_sub2(&out, &input3);
	CHECK_MATRIX(out, MATRIX(
		4507.72f, 8571.32f, 5521.15f, 
		5275.36f, -109.213f, 461.763f, 
		142.122f, 8422.04f, 5110.61f));

	out = input2;
	vm_matrix_sub2(&out, &input3);
	CHECK_MATRIX(out, MATRIX(
		0.2585f, -27.042f, -0.1465f, 
		0.6409f, -777.188f, 0.1181f, 
		-2.1215f, -67.941f, 0.3793f));
}

