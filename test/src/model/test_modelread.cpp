#include <gtest/gtest.h>
#include <model/model.h>

#include "util/FSTestFixture.h"

#define EXPECT_VECMAT_NEAR(global,vector) EXPECT_NEAR(error(&global,vector), 0.0f, 0.001f);

float error(vec3d* val, vec3d target) {
	float totalError = 0;
	for (int i = 0; i < 3; i++) {
		float diff = val->a1d[i] - target.a1d[i];
		totalError += diff * diff;
	}
	return totalError;
}
extern float error(matrix* val, matrix target);

class SubmodelLocalizeTest : public test::FSTestFixture {
public:
	SubmodelLocalizeTest() { pushModDir("model"); }

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
		
		pm = new polymodel();
		pmi = new polymodel_instance();

		pm->submodel = make_shared<bsp_info[]>(3);
		pmi->submodel = new submodel_instance[3];

		pm->submodel[0].parent = -1;
		pm->submodel[1].parent = 0;
		pm->submodel[2].parent = 1;

		pm->submodel[0].offset = vec3d{ {{0, 0, 0}} };
		pm->submodel[1].offset = vec3d{ {{0, 1, 0}} };
		pm->submodel[2].offset = vec3d{ {{0, 1, 0}} };

		pm->submodel[0].depth = 1;
		pm->submodel[1].depth = 2;
		pm->submodel[2].depth = 3;

		pmi->submodel[0].canonical_orient = vmd_identity_matrix;
		angles ang{ PI_2, 0.0f, 0.0f };
		vm_angles_2_matrix(&pmi->submodel[1].canonical_orient, &ang);
		ang = angles{ -PI_2, 0.0f, PI_2 };
		vm_angles_2_matrix(&pmi->submodel[2].canonical_orient, &ang);
	}

	void TearDown() override {
		test::FSTestFixture::TearDown();

		pm->submodel.reset();
		delete[] pmi->submodel;
		delete pm;
		delete pmi;
	}

	polymodel *pm;
	polymodel_instance *pmi;
};

TEST_F(SubmodelLocalizeTest, submodel_instance_localize_roundtrip) {

	vec3d global;
	vec3d roundtrip;
	vec3d local{ {{0.0f, 1.0f, 0.0f}} };
	model_instance_local_to_global_point(&global, &local, pm, pmi, 2);
	model_instance_global_to_local_point(&roundtrip, &global, pm, pmi, 2);

	EXPECT_VECMAT_NEAR(global, (vec3d{ {{-1.0f, 1.0f, 1.0f}} }));
	EXPECT_VECMAT_NEAR(roundtrip, local);
}

TEST_F(SubmodelLocalizeTest, submodel_instance_localize_roundtrip_world) {

	vec3d globalPos{ {{0.0f, 5.0f, 0.0f}} };
	matrix globalOrient;
	angles globalRot{ 0.0f, PI_2, 0.0f };
	vm_angles_2_matrix(&globalOrient, &globalRot);

	vec3d global;
	vec3d roundtrip;
	vec3d local{ {{0.0f, 1.0f, 0.0f}} };
	model_instance_local_to_global_point(&global, &local, pm, pmi, 2, &globalOrient, &globalPos);
	model_instance_global_to_local_point(&roundtrip, &global, pm, pmi, 2, &globalOrient, &globalPos);

	EXPECT_VECMAT_NEAR(global, (vec3d{ {{-1.0f, 4.0f, 1.0f}} }));
	EXPECT_VECMAT_NEAR(roundtrip, local);

	matrix globalMat;
	matrix roundtripMat;
	matrix localMat = globalOrient;
	model_instance_local_to_global_point_orient(&global, &globalMat, &local, &localMat, pm, pmi, 2, &globalOrient, &globalPos);
	model_instance_global_to_local_point_orient(&roundtrip, &roundtripMat, &global, &globalMat, pm, pmi, 2, &globalOrient, &globalPos);

	EXPECT_VECMAT_NEAR(global, (vec3d{ {{-1.0f, 4.0f, 1.0f}} }));
	EXPECT_VECMAT_NEAR(roundtrip, local);
	EXPECT_VECMAT_NEAR(roundtripMat, localMat);
}

extern void do_movement_sanity_checks(polymodel *pm, int submodel, bool is_rotation, bool is_turret);

class TurretAxisTest : public test::FSTestFixture {
public:
	TurretAxisTest() { pushModDir("model"); }

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();

		pm = new polymodel();
		pm->n_models = 3;
		pm->submodel = make_shared<bsp_info[]>(3);

		pm->submodel[0].parent = -1;
		pm->submodel[0].first_child = 1;
		pm->submodel[1].parent = 0;
		pm->submodel[1].first_child = 2;
		pm->submodel[2].parent = 1;

		// the reader has already switched off the movement of a barrel marked "other" with no $rotation_axis
		pm->submodel[2].rotation_type = MOVEMENT_TYPE_NONE;
		pm->submodel[2].rotation_axis_id = MOVEMENT_AXIS_OTHER;
	}

	void TearDown() override {
		test::FSTestFixture::TearDown();

		pm->submodel.reset();
		delete pm;
	}

	polymodel *pm;
};

TEST_F(TurretAxisTest, barrel_marked_other_without_axis_gets_default) {
	do_movement_sanity_checks(pm, 1, true, false);
	do_movement_sanity_checks(pm, 2, true, true);

	EXPECT_VECMAT_NEAR(pm->submodel[2].rotation_axis, vmd_x_vector);
	EXPECT_EQ(pm->submodel[2].rotation_axis_id, MOVEMENT_AXIS_X);
	EXPECT_EQ(pm->submodel[2].rotation_type, MOVEMENT_TYPE_TURRET);
}

TEST_F(TurretAxisTest, barrel_with_custom_axis_but_no_movement_still_rotates) {
	pm->submodel[2].rotation_axis = vec3d{ {{0.0f, 0.6f, 0.8f}} };

	do_movement_sanity_checks(pm, 1, true, false);
	do_movement_sanity_checks(pm, 2, true, true);

	EXPECT_TRUE(vm_vec_is_normalized(&pm->submodel[2].rotation_axis));
	EXPECT_EQ(pm->submodel[2].rotation_type, MOVEMENT_TYPE_TURRET);
}

TEST_F(TurretAxisTest, barrel_with_custom_axis_keeps_it) {
	vec3d custom{ {{0.0f, 0.6f, 0.8f}} };
	pm->submodel[2].rotation_type = MOVEMENT_TYPE_TURRET;
	pm->submodel[2].rotation_axis = custom;

	do_movement_sanity_checks(pm, 1, true, false);
	do_movement_sanity_checks(pm, 2, true, true);

	EXPECT_VECMAT_NEAR(pm->submodel[2].rotation_axis, custom);
	EXPECT_EQ(pm->submodel[2].rotation_axis_id, MOVEMENT_AXIS_OTHER);
	EXPECT_EQ(pm->submodel[2].rotation_type, MOVEMENT_TYPE_TURRET);
}
