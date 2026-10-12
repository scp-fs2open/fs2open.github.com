#include <gtest/gtest.h>

#include "globalincs/linklist.h"
#include "missioneditor/common.h"
#include "object/object.h"

class ObjectBoundsTest : public ::testing::Test {
 protected:
	void SetUp() override {
		list_init(&obj_used_list);
	}

	void TearDown() override {
		for (int i = 0; i < _count; ++i) {
			Objects[i].clear();
		}
		list_init(&obj_used_list);
	}

	void add(float x, bool marked) {
		object* objp = &Objects[_count++];
		objp->clear();
		objp->pos = vm_vec_new(x, 0.0f, 0.0f);
		if (marked) {
			objp->flags.set(Object::Object_Flags::Marked);
		}
		list_append(&obj_used_list, objp);
	}

	int _count = 0;
};

TEST_F(ObjectBoundsTest, no_objects) {
	vec3d center = vm_vec_new(1.0f, 2.0f, 3.0f);
	float radius = 4.0f;

	EXPECT_FALSE(get_object_bounds(false, &center, &radius));
	EXPECT_EQ(center.xyz.x, 1.0f);
	EXPECT_EQ(radius, 4.0f);
}

TEST_F(ObjectBoundsTest, box_midpoint_not_average) {
	add(0.0f, false);
	add(1.0f, false);
	add(10.0f, false);
	vec3d center;
	float radius;

	ASSERT_TRUE(get_object_bounds(false, &center, &radius));
	EXPECT_FLOAT_EQ(center.xyz.x, 5.0f);
	EXPECT_FLOAT_EQ(radius, 5.0f);
}

TEST_F(ObjectBoundsTest, marked_only) {
	add(0.0f, true);
	add(10.0f, true);
	add(100.0f, false);
	vec3d center;
	float radius;

	ASSERT_TRUE(get_object_bounds(true, &center, &radius));
	EXPECT_FLOAT_EQ(center.xyz.x, 5.0f);
	EXPECT_FLOAT_EQ(radius, 5.0f);

	ASSERT_TRUE(get_object_bounds(false, &center, &radius));
	EXPECT_FLOAT_EQ(center.xyz.x, 50.0f);
	EXPECT_FLOAT_EQ(radius, 50.0f);
}

TEST_F(ObjectBoundsTest, nothing_marked) {
	add(5.0f, false);
	vec3d center = vm_vec_new(1.0f, 2.0f, 3.0f);
	float radius = 4.0f;

	EXPECT_FALSE(get_object_bounds(true, &center, &radius));
	EXPECT_EQ(center.xyz.x, 1.0f);
	EXPECT_EQ(radius, 4.0f);
}
