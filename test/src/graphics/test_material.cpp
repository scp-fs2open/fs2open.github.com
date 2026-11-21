#include <gtest/gtest.h>
#include <graphics/material.h>

// USE_NORMAL_MAP means "read the scene normal buffer": set when the decal brings no normal map of its own.
TEST(DecalMaterial, SceneNormalFlagSetOnlyWithoutDecalNormalMap)
{
	decal_material without_normal;
	without_normal.set_texture_map(TM_BASE_TYPE, 0);
	without_normal.set_texture_map(TM_NORMAL_TYPE, -1);
	EXPECT_EQ(without_normal.get_shader_flags(), static_cast<uint>(SDR_FLAG_DECAL_USE_NORMAL_MAP));

	decal_material with_normal;
	with_normal.set_texture_map(TM_BASE_TYPE, 0);
	with_normal.set_texture_map(TM_NORMAL_TYPE, 1);
	EXPECT_EQ(with_normal.get_shader_flags(), 0u);
}

TEST(ModelMaterial, SpecularSelectionPrefersReflectAndReportsEncoding)
{
	model_material mat;
	bool is_spec_gloss = true;
	EXPECT_EQ(mat.get_specular_map(&is_spec_gloss), -1);
	EXPECT_FALSE(is_spec_gloss);

	mat.set_texture_map(TM_SPECULAR_TYPE, 0);
	EXPECT_EQ(mat.get_specular_map(&is_spec_gloss), 0);
	EXPECT_FALSE(is_spec_gloss);

	mat.set_texture_map(TM_SPEC_GLOSS_TYPE, 1);
	EXPECT_EQ(mat.get_specular_map(&is_spec_gloss), 1);
	EXPECT_TRUE(is_spec_gloss);
	EXPECT_EQ(mat.get_specular_map(), 1);

	mat.set_texture_map(TM_SPECULAR_TYPE, -1);
	mat.set_texture_map(TM_SPEC_GLOSS_TYPE, 0);
	EXPECT_EQ(mat.get_specular_map(&is_spec_gloss), 0);
	EXPECT_TRUE(is_spec_gloss);

	mat.set_texture_map(TM_SPEC_GLOSS_TYPE, -1);
	EXPECT_EQ(mat.get_specular_map(&is_spec_gloss), -1);
	EXPECT_FALSE(is_spec_gloss);
}
