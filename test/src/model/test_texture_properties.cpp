#include <gtest/gtest.h>
#include <model/model.h>
#include <model/modelreplace.h>

#include "util/FSTestFixture.h"

// The fixtures are a three-texture cube saved by POF-Tools; txpr_count_*.pof and txpr_overrun.pof are byte-patched
// copies of txpr_cube.pof (count or last string length changed, an unknown chunk appended) that POF-Tools can't write.

class ModelTexturePropertiesTest : public test::FSTestFixture {
public:
	ModelTexturePropertiesTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {
		pushModDir("model");
	}

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
		// model_init() may only run once per process; this initializes on first use and otherwise frees
		model_free_all();
	}

	void TearDown() override {
		model_free_all();
		test::FSTestFixture::TearDown();
	}

	static SCP_string long_properties() {
		SCP_string props = "$curve=";
		for (int i = 0; i < 1199; i++) {
			props += "0.5, ";
		}
		return props + "x";
	}

	static const SCP_vector<SCP_string>& load_properties(const char* filename) {
		int model_num = model_load(filename, nullptr, ErrorType::NONE);
		EXPECT_GE(model_num, 0);
		return model_get(model_num)->texture_properties;
	}
};

TEST_F(ModelTexturePropertiesTest, read_from_txpr) {
	auto& props = load_properties("txpr_cube.pof");

	ASSERT_EQ(props.size(), 3u);
	EXPECT_EQ(props[0], "$hull=1");
	EXPECT_EQ(props[1], "");
	EXPECT_EQ(props[2].size(), 6003u);
	EXPECT_EQ(props[2], long_properties());
}

TEST_F(ModelTexturePropertiesTest, empty_without_txpr) {
	auto& props = load_properties("no_txpr_cube.pof");

	ASSERT_EQ(props.size(), 3u);
	EXPECT_TRUE(props[0].empty());
	EXPECT_TRUE(props[1].empty());
	EXPECT_TRUE(props[2].empty());
}

TEST_F(ModelTexturePropertiesTest, count_below_textures_warns_and_pads) {
#ifndef NDEBUG
	GTEST_SKIP() << "in debug test builds Warning() throws mid-load and leaves parsing paused for later tests";
#else
	int warnings = Global_warning_count;
	auto& props = load_properties("txpr_count_low.pof");

	EXPECT_EQ(Global_warning_count, warnings + 1);
	ASSERT_EQ(props.size(), 3u);
	EXPECT_EQ(props[0], "$hull=1");
	EXPECT_TRUE(props[1].empty());
	EXPECT_TRUE(props[2].empty());
#endif
}

TEST_F(ModelTexturePropertiesTest, count_above_strings_stops_at_chunk_end) {
	int warnings = Global_warning_count;
	auto& props = load_properties("txpr_count_high.pof");

	EXPECT_EQ(Global_warning_count, warnings);
	ASSERT_EQ(props.size(), 3u);
	EXPECT_EQ(props[0], "$hull=1");
	EXPECT_EQ(props[1], "");
	EXPECT_EQ(props[2], long_properties());
}

TEST_F(ModelTexturePropertiesTest, string_past_chunk_end_is_dropped) {
#ifndef NDEBUG
	GTEST_SKIP() << "in debug test builds Warning() throws mid-load and leaves parsing paused for later tests";
#else
	int warnings = Global_warning_count;
	auto& props = load_properties("txpr_overrun.pof");

	EXPECT_EQ(Global_warning_count, warnings + 2);
	ASSERT_EQ(props.size(), 3u);
	EXPECT_EQ(props[0], "$hull=1");
	EXPECT_EQ(props[1], "");
	EXPECT_TRUE(props[2].empty());
#endif
}

TEST_F(ModelTexturePropertiesTest, virtual_pof_keeps_added_textures_properties) {
	virtual_pof_init();
	int model_num = model_load("vp_texprops.pof", nullptr, ErrorType::NONE);
	ASSERT_GE(model_num, 0);
	auto pm = model_get(model_num);

	ASSERT_EQ(pm->n_textures, 6);
	ASSERT_EQ(pm->texture_properties.size(), 6u);
	EXPECT_TRUE(pm->texture_properties[0].empty());
	EXPECT_TRUE(pm->texture_properties[1].empty());
	EXPECT_TRUE(pm->texture_properties[2].empty());
	EXPECT_EQ(pm->texture_properties[3], "$hull=1");
	EXPECT_EQ(pm->texture_properties[4], "");
	EXPECT_EQ(pm->texture_properties[5], long_properties());
}
