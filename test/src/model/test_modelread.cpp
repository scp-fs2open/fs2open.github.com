#include <gtest/gtest.h>
#include <model/model.h>

#include "util/FSTestFixture.h"

#define EXPECT_VECTOR_NEAR(global,vector) EXPECT_NEAR(error(&global,vector), 0.0f, 0.001f);

float error(vec3d* val, vec3d target) {
	float totalError = 0;
	for (int i = 0; i < 3; i++) {
		float diff = val->a1d[i] - target.a1d[i];
		totalError += diff * diff;
	}
	return totalError;
}

class SubmodelLocalizeTest : public test::FSTestFixture {
public:
	SubmodelLocalizeTest() { pushModDir("model"); }

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
		
		pm = new polymodel();
		pmi = new polymodel_instance();

		pm->submodel = new bsp_info[3];
		pmi->submodel = new submodel_instance[3];

		pm->submodel[0].parent = -1;
		pm->submodel[1].parent = 0;
		pm->submodel[2].parent = 1;

		pm->submodel[0].offset = vec3d{ {{0, 0, 0}} };
		pm->submodel[1].offset = vec3d{ {{0, 1, 0}} };
		pm->submodel[2].offset = vec3d{ {{0, 1, 0}} };

		pmi->submodel[0].canonical_orient = vmd_identity_matrix;
		angles ang{ PI_2, 0.0f, 0.0f };
		vm_angles_2_matrix(&pmi->submodel[1].canonical_orient, &ang);
		ang = angles{ -PI_2, 0.0f, PI_2 };
		vm_angles_2_matrix(&pmi->submodel[2].canonical_orient, &ang);
	}

	void TearDown() override {
		test::FSTestFixture::TearDown();

		delete[] pm->submodel;
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

	EXPECT_VECTOR_NEAR(global, (vec3d{ {{-1.0f, 1.0f, 1.0f}} }));
	EXPECT_VECTOR_NEAR(roundtrip, local);
}

TEST_F(SubmodelLocalizeTest, submodel_instance_localize_roundtrip_world) {

	vec3d globalPos{ {{0.0f, 5.0f, 0.0f}} };
	matrix globalOrient;
	angles globalRot{ 0.0f, PI_2, 0.0f};
	vm_angles_2_matrix(&globalOrient, &globalRot);

	vec3d global;
	vec3d roundtrip;
	vec3d local{ {{0.0f, 1.0f, 0.0f}} };
	model_instance_local_to_global_point(&global, &local, pm, pmi, 2, &globalOrient, &globalPos);
	model_instance_global_to_local_point(&roundtrip, &global, pm, pmi, 2, &globalOrient, &globalPos);

	EXPECT_VECTOR_NEAR(global, (vec3d{ {{-1.0f, 4.0f, 1.0f}} }));
	EXPECT_VECTOR_NEAR(roundtrip, local);
}