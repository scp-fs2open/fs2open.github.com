#include <util/FSTestFixture.h>

#include <bmpman/bmpman.h>
#include <bmpman/bm_internal.h>
#include <globalincs/systemvars.h>
#include <graphics/2d.h>
#include <mod_table/mod_table.h>
#include <osapi/dialogs.h>

#include <utility>

namespace {

struct RecordedFree {
	int handle;
	bool release;
};

class BmpmanEvictTest : public test::FSTestFixture {
 public:
	BmpmanEvictTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS | INIT_MOD_TABLE) {
		pushModDir("bmpman");
	}

 protected:
	SCP_vector<RecordedFree> frees;

	void SetUp() override {
		test::FSTestFixture::SetUp();

		// the fixture runs as a standalone server, where bm_load substitutes placeholder images
		saved_standalone = Is_standalone;
		Is_standalone = 0;

		// eviction only runs on OpenGL (Vulkan flushes on its own); the stub renderer's function pointers stay in place
		saved_mode = gr_screen.mode;
		gr_screen.mode = GraphicsAPI::OpenGL;

		saved_free = gr_screen.gf_bm_free_data;
		// a lock's loader also calls the renderer with release=false to drop stale data; only release frees are evictions
		gr_screen.gf_bm_free_data = [this](bitmap_slot* slot, bool release) {
			if (release)
				frees.push_back({slot->entry.handle, release});
		};
	}

	void TearDown() override {
		gr_screen.gf_bm_free_data = saved_free;
		gr_screen.mode = saved_mode;
		Is_standalone = saved_standalone;

		test::FSTestFixture::TearDown();
	}

	static void page_in() {
		bm_page_in_start();
		bm_page_in_stop();
	}

	static int load_still() {
		int handle = bm_load("attacker");
		EXPECT_GE(handle, 0);
		return handle;
	}

	static int load_anim(int* nframes) {
		int handle = bm_load_animation("CursorWeb", nframes);
		EXPECT_GE(handle, 0);
		EXPECT_GT(*nframes, 1);
		return handle;
	}

 private:
	int saved_standalone = 0;
	GraphicsAPI saved_mode = GraphicsAPI::Stub;
	std::function<void(bitmap_slot*, bool)> saved_free;
};

TEST_F(BmpmanEvictTest, knob_default_is_one) {
	EXPECT_EQ(1, Gpu_texture_evict_after_missions);
}

TEST_F(BmpmanEvictTest, knob_parsed_in_graphics_settings) {
	EXPECT_EQ(3, Gpu_texture_evict_after_missions);
}

// parses the table itself: the clamp warns, and under unit tests a warning throws in debug builds
class BmpmanEvictKnobTest : public test::FSTestFixture {
 public:
	BmpmanEvictKnobTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("bmpman");
	}
};

TEST_F(BmpmanEvictKnobTest, knob_above_255_is_clamped) {
#ifndef NDEBUG
	ASSERT_THROW(mod_table_init(), os::dialogs::WarningException);
#else
	mod_table_init();
#endif
	EXPECT_EQ(255, Gpu_texture_evict_after_missions);
}

TEST_F(BmpmanEvictTest, knob_zero_disables) {
	Gpu_texture_evict_after_missions = 0;
	int handle = load_still();

	for (int i = 0; i < 3; i++)
		page_in();

	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);
}

TEST_F(BmpmanEvictTest, never_used_still_freed_at_first_page_in) {
	Gpu_texture_evict_after_missions = 1;
	int handle = load_still();

	page_in();

	ASSERT_EQ(1u, frees.size());
	EXPECT_EQ(handle, frees[0].handle);
	EXPECT_TRUE(frees[0].release);
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);
}

TEST_F(BmpmanEvictTest, used_mark_keeps_texture_for_one_page_in) {
	Gpu_texture_evict_after_missions = 1;
	int handle = load_still();

	bm_mark_used(handle);
	page_in();

	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(0, bm_get_entry(handle)->used_since_page_in);
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);

	page_in();

	ASSERT_EQ(1u, frees.size());
	EXPECT_EQ(handle, frees[0].handle);
}

TEST_F(BmpmanEvictTest, lock_and_unlock_count_as_use) {
	Gpu_texture_evict_after_missions = 1;
	int handle = load_still();

	ASSERT_NE(nullptr, bm_lock(handle, 16, BMP_TEX_OTHER));
	bm_unlock(handle);
	page_in();

	EXPECT_TRUE(frees.empty());

	page_in();

	ASSERT_EQ(1u, frees.size());
	EXPECT_EQ(handle, frees[0].handle);
}

TEST_F(BmpmanEvictTest, preload_keeps_and_resets_counter) {
	Gpu_texture_evict_after_missions = 2;
	int handle = load_still();

	page_in();
	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(1, bm_get_entry(handle)->unused_page_ins);

	bm_page_in_start();
	bm_page_in_texture(handle);
	bm_page_in_stop();

	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);
}

TEST_F(BmpmanEvictTest, n_two_frees_on_second_unused_page_in) {
	Gpu_texture_evict_after_missions = 2;
	int handle = load_still();

	page_in();
	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(1, bm_get_entry(handle)->unused_page_ins);

	page_in();
	ASSERT_EQ(1u, frees.size());
	EXPECT_EQ(handle, frees[0].handle);
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);
}

TEST_F(BmpmanEvictTest, animation_freed_whole_with_release) {
	Gpu_texture_evict_after_missions = 1;
	int nframes = 0;
	int handle = load_anim(&nframes);

	page_in();

	ASSERT_EQ(static_cast<size_t>(nframes), frees.size());
	for (int i = 0; i < nframes; i++) {
		EXPECT_EQ(handle + i, frees[i].handle);
		EXPECT_TRUE(frees[i].release);
	}
}

TEST_F(BmpmanEvictTest, locked_frame_counts_as_use_for_whole_animation) {
	Gpu_texture_evict_after_missions = 1;
	int nframes = 0;
	int handle = load_anim(&nframes);

	ASSERT_NE(nullptr, bm_lock(handle + 1, 16, BMP_TEX_OTHER));
	page_in();
	page_in();

	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);

	bm_unlock(handle + 1);
	page_in();

	EXPECT_EQ(static_cast<size_t>(nframes), frees.size());
}

TEST_F(BmpmanEvictTest, user_bitmap_never_freed) {
	Gpu_texture_evict_after_missions = 1;
	ubyte pixels[4 * 4 * 4] = {0};
	int handle = bm_create(32, 4, 4, pixels);
	ASSERT_GE(handle, 0);

	page_in();
	page_in();

	EXPECT_TRUE(frees.empty());
	bm_release(handle);
}

TEST_F(BmpmanEvictTest, no_eviction_outside_opengl) {
	Gpu_texture_evict_after_missions = 1;
	gr_screen.mode = GraphicsAPI::Vulkan;
	int handle = load_still();

	page_in();
	page_in();

	EXPECT_TRUE(frees.empty());
	EXPECT_EQ(0, bm_get_entry(handle)->unused_page_ins);
}

TEST_F(BmpmanEvictTest, volume_bitmap_never_freed) {
	Gpu_texture_evict_after_missions = 1;
	ubyte pixels[4 * 4 * 4 * 4] = {0};
	int handle = bm_create_3d(32, 4, 4, 4, pixels);
	ASSERT_GE(handle, 0);

	page_in();
	page_in();

	EXPECT_TRUE(frees.empty());
	bm_release(handle);
}

TEST_F(BmpmanEvictTest, mark_on_frame_lands_on_first_frame) {
	int nframes = 0;
	int handle = load_anim(&nframes);

	bm_mark_used(handle + 1);

	EXPECT_EQ(1, bm_get_entry(handle)->used_since_page_in);
	EXPECT_EQ(0, bm_get_entry(handle + 1)->used_since_page_in);
}

TEST_F(BmpmanEvictTest, mark_on_frame_of_non_array_animation_lands_on_first_frame) {
	int nframes = 0;
	int handle = load_anim(&nframes);

	// frames that differ in size never share a texture array; forcing the flag avoids a loader warning, which throws here
	bm_get_entry(handle)->info.ani.is_array = false;

	bm_mark_used(handle + 1);

	EXPECT_EQ(1, bm_get_entry(handle)->used_since_page_in);
	EXPECT_EQ(0, bm_get_entry(handle + 1)->used_since_page_in);
}

TEST_F(BmpmanEvictTest, marks_consumed_after_preloading_page_in) {
	Gpu_texture_evict_after_missions = 1;
	int nframes = 0;
	int handle = load_anim(&nframes);

	bm_page_in_start();
	bm_page_in_texture(handle);
	bm_page_in_stop();

	EXPECT_TRUE(frees.empty());
	for (int i = 0; i < nframes; i++)
		EXPECT_EQ(0, bm_get_entry(handle + i)->used_since_page_in) << "frame " << i;

	page_in();

	EXPECT_EQ(static_cast<size_t>(nframes), frees.size());
}

TEST_F(BmpmanEvictTest, mark_invalid_handle_is_safe) {
	bm_mark_used(-1);
}

} // namespace
