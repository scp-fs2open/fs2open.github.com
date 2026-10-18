#include <gtest/gtest.h>
#include <model/model.h>
#include <osapi/dialogs.h>

#include "util/FSTestFixture.h"
#include "util/test_util.h"

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

extern void resolve_submodel_index(const polymodel *pm, const char *requester, const char *field, int &submodel_index, const SCP_vector<SCP_string> &submodel_list);

class ResolveSubmodelIndexTest : public test::FSTestFixture {
public:
	ResolveSubmodelIndexTest() { pushModDir("model"); }

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();

		pm = new polymodel();
		pm->n_models = 3;
		pm->submodel = make_shared<bsp_info[]>(3);
		strcpy_s(pm->submodel[0].name, "hull");
		strcpy_s(pm->submodel[1].name, "turret");
		strcpy_s(pm->submodel[2].name, "barrel");
	}
	void TearDown() override {
		test::FSTestFixture::TearDown();

		pm->submodel.reset();
		delete pm;
	}

	int resolve(const char *name) {
		SCP_vector<SCP_string> names{ name };
		int index = 0;
		resolve_submodel_index(pm, "test", "$look_at", index, names);
		return index;
	}

	polymodel *pm = nullptr;
};

TEST_F(ResolveSubmodelIndexTest, names_and_numbers_in_range) {
	EXPECT_EQ(resolve("turret"), 1);
	EXPECT_EQ(resolve("0"), 0);
	EXPECT_EQ(resolve("2"), 2);
	EXPECT_EQ(resolve("-1"), -1);
}

TEST_F(ResolveSubmodelIndexTest, numbers_out_of_range_warn) {
	DEBUG_TEST();

	ASSERT_THROW(resolve("3"), os::dialogs::WarningException);
	ASSERT_THROW(resolve("-2"), os::dialogs::WarningException);
	ASSERT_THROW(resolve("4294967296"), os::dialogs::WarningException);
	ASSERT_THROW(resolve("+3"), os::dialogs::WarningException);
}

extern bool sldc_tree_is_valid(const ubyte *sldc, uint tree_size);
extern bool slc2_tree_is_valid(const ubyte *tree, int tree_size, int num_tris);

namespace {
void add_slc2_split(SCP_vector<ubyte> &tree, uint front, uint back) {
	int header[2] = { 0, 40 };
	float bounds[6] = {};
	uint offsets[2] = { front, back };
	tree.insert(tree.end(), reinterpret_cast<ubyte *>(header), reinterpret_cast<ubyte *>(header) + sizeof(header));
	tree.insert(tree.end(), reinterpret_cast<ubyte *>(bounds), reinterpret_cast<ubyte *>(bounds) + sizeof(bounds));
	tree.insert(tree.end(), reinterpret_cast<ubyte *>(offsets), reinterpret_cast<ubyte *>(offsets) + sizeof(offsets));
}

void add_slc2_leaf(SCP_vector<ubyte> &tree, const SCP_vector<uint> &tris) {
	int header[2] = { 1, static_cast<int>(36 + 4 * tris.size()) };
	float bounds[6] = {};
	uint count = static_cast<uint>(tris.size());
	tree.insert(tree.end(), reinterpret_cast<ubyte *>(header), reinterpret_cast<ubyte *>(header) + sizeof(header));
	tree.insert(tree.end(), reinterpret_cast<ubyte *>(bounds), reinterpret_cast<ubyte *>(bounds) + sizeof(bounds));
	tree.insert(tree.end(), reinterpret_cast<const ubyte *>(&count), reinterpret_cast<const ubyte *>(&count) + sizeof(count));
	for (uint tri : tris)
		tree.insert(tree.end(), reinterpret_cast<const ubyte *>(&tri), reinterpret_cast<const ubyte *>(&tri) + sizeof(tri));
}

void add_sldc_node(SCP_vector<ubyte> &tree, ubyte type, uint size, uint back) {
	tree.push_back(type);
	tree.insert(tree.end(), reinterpret_cast<ubyte *>(&size), reinterpret_cast<ubyte *>(&size) + sizeof(size));
	tree.insert(tree.end(), 24, 0);
	if (type == 0) {
		uint front = 0;
		tree.insert(tree.end(), reinterpret_cast<ubyte *>(&front), reinterpret_cast<ubyte *>(&front) + sizeof(front));
		tree.insert(tree.end(), reinterpret_cast<ubyte *>(&back), reinterpret_cast<ubyte *>(&back) + sizeof(back));
	} else {
		tree.insert(tree.end(), size - 29, 0);
	}
}

bool slc2_valid(const SCP_vector<ubyte> &tree, int num_tris) {
	return slc2_tree_is_valid(tree.data(), static_cast<int>(tree.size()), num_tris);
}
}

TEST(ShieldCollisionTreeTest, slc2_valid_trees) {
	SCP_vector<ubyte> tree;
	add_slc2_split(tree, 40, 84);
	add_slc2_leaf(tree, { 0, 1 });
	add_slc2_leaf(tree, {});
	EXPECT_TRUE(slc2_valid(tree, 2));
}

TEST(ShieldCollisionTreeTest, slc2_bad_trees) {
	SCP_vector<ubyte> tree;

	add_slc2_split(tree, 40, 84);
	add_slc2_leaf(tree, { 0, 2 });
	add_slc2_leaf(tree, {});
	EXPECT_FALSE(slc2_valid(tree, 2)) << "triangle number past the shield mesh";

	tree.clear();
	add_slc2_split(tree, 0, 40);
	add_slc2_leaf(tree, { 0 });
	EXPECT_FALSE(slc2_valid(tree, 1)) << "child pointing at its own node";

	tree.clear();
	add_slc2_split(tree, 40, 40);
	add_slc2_leaf(tree, { 0 });
	EXPECT_FALSE(slc2_valid(tree, 1)) << "two children sharing a node";

	tree.clear();
	add_slc2_split(tree, 40, 80);
	add_slc2_leaf(tree, { 0 });
	EXPECT_FALSE(slc2_valid(tree, 1)) << "child past the end of the tree";

	tree.clear();
	add_slc2_leaf(tree, { 0, 0 });
	tree.resize(tree.size() - 4);
	EXPECT_FALSE(slc2_valid(tree, 1)) << "polygon list past the end of the tree";

	EXPECT_FALSE(slc2_valid({}, 1)) << "empty tree";

	tree.clear();
	add_slc2_split(tree, 40, 44);
	add_slc2_leaf(tree, {0, 0});
	EXPECT_FALSE(slc2_valid(tree, 1)) << "child starting inside another node";
}

TEST(ShieldCollisionTreeTest, slc2_depth_limit) {
	auto build = [](int levels) {
		SCP_vector<ubyte> tree;
		for (int i = 0; i < levels; i++) {
			add_slc2_split(tree, 80, 40);
			add_slc2_leaf(tree, { 0 });
		}
		add_slc2_leaf(tree, { 0 });
		return tree;
	};
	EXPECT_TRUE(slc2_valid(build(255), 1));
	EXPECT_FALSE(slc2_valid(build(256), 1));
}

TEST(ShieldCollisionTreeTest, sldc_trees) {
	SCP_vector<ubyte> tree;
	add_sldc_node(tree, 0, 37, 37 + 33);
	add_sldc_node(tree, 1, 33, 0);
	add_sldc_node(tree, 1, 33, 0);
	EXPECT_TRUE(sldc_tree_is_valid(tree.data(), static_cast<uint>(tree.size())));

	tree.clear();
	add_sldc_node(tree, 1, 29, 0);
	tree[1] = 28;
	add_sldc_node(tree, 1, 33, 0);
	EXPECT_FALSE(sldc_tree_is_valid(tree.data(), static_cast<uint>(tree.size()))) << "node smaller than its header";

	tree.clear();
	add_sldc_node(tree, 1, 33, 0);
	tree.resize(tree.size() + 3);
	std::unique_ptr<ubyte[]> exact(new ubyte[tree.size()]);
	memcpy(exact.get(), tree.data(), tree.size());
	EXPECT_FALSE(sldc_tree_is_valid(exact.get(), static_cast<uint>(tree.size())))
		<< "trailing bytes that are not a node";

	tree.clear();
	add_sldc_node(tree, 0, 37, 37 + 33);
	tree.push_back(0);
	tree[1] = 38;
	add_sldc_node(tree, 1, 33, 0);
	add_sldc_node(tree, 1, 33, 0);
	EXPECT_FALSE(sldc_tree_is_valid(tree.data(), static_cast<uint>(tree.size()))) << "split that is not 37 bytes";

	tree.clear();
	add_sldc_node(tree, 0, 37, 37 + 33 + 33);
	add_sldc_node(tree, 1, 33, 0);
	add_sldc_node(tree, 1, 33, 0);
	EXPECT_FALSE(sldc_tree_is_valid(tree.data(), static_cast<uint>(tree.size())))
		<< "back child at the end of the tree";

	tree.clear();
	add_sldc_node(tree, 0, 37, 0);
	add_sldc_node(tree, 1, 33, 0);
	EXPECT_FALSE(sldc_tree_is_valid(tree.data(), static_cast<uint>(tree.size()))) << "split with no back child";

	tree.clear();
	add_sldc_node(tree, 0, 37, 200);
	add_sldc_node(tree, 1, 33, 0);
	EXPECT_FALSE(sldc_tree_is_valid(tree.data(), static_cast<uint>(tree.size()))) << "back child past the end";

	tree.clear();
	add_sldc_node(tree, 1, 400, 0);
	EXPECT_FALSE(sldc_tree_is_valid(tree.data(), 100)) << "node larger than the tree";
}
