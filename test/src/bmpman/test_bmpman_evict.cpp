#include <util/FSTestFixture.h>

#include <bmpman/bmpman.h>
#include <bmpman/bm_internal.h>
#include <globalincs/systemvars.h>
#include <graphics/2d.h>

namespace {

struct RecordedFree {
	int handle;
	bool release;
};

class BmpmanEvictTest : public test::FSTestFixture {
 public:
	BmpmanEvictTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS | INIT_MOD_TABLE) {}

 protected:
	SCP_vector<RecordedFree> frees;

	void SetUp() override {
		test::FSTestFixture::SetUp();

		// the fixture runs as a standalone server, where bm_load substitutes placeholder images
		saved_standalone = Is_standalone;
		Is_standalone = 0;

		// the free runs only on OpenGL (Vulkan flushes on its own); the stub renderer's hooks stay in place
		saved_mode = gr_screen.mode;
		gr_screen.mode = GraphicsAPI::OpenGL;

		saved_free = gr_screen.gf_bm_free_data;
		// a lock's loader also calls the renderer with release=false to drop stale data; only release frees
		// are the frees under test
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

	// counts the recorded frees of the frames [handle, handle + nframes), so other fixture bitmaps cannot skew a test
	size_t frees_in(int handle, int nframes = 1) const {
		size_t n = 0;
		for (const auto& f : frees) {
			if (f.handle >= handle && f.handle < handle + nframes)
				n++;
		}
		return n;
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

TEST_F(BmpmanEvictTest, not_preloaded_still_freed_at_page_in) {
	int handle = load_still();

	page_in();

	ASSERT_EQ(1u, frees_in(handle));
	for (const auto& f : frees) {
		if (f.handle == handle) {
			EXPECT_TRUE(f.release);
		}
	}

	// the entry survives the free: the next lock reloads the pixels from the file
	ASSERT_NE(nullptr, bm_lock(handle, 16, BMP_TEX_OTHER));
	bm_unlock(handle);
}

TEST_F(BmpmanEvictTest, freed_again_at_every_page_in) {
	int handle = load_still();

	page_in();
	page_in();

	EXPECT_EQ(2u, frees_in(handle));
}

TEST_F(BmpmanEvictTest, preloaded_still_kept) {
	int handle = load_still();

	// the stub renderer's preload reports failure; the free lives in the other branch either way
	bm_page_in_start();
	bm_page_in_texture(handle);
	bm_page_in_stop();

	EXPECT_EQ(0u, frees_in(handle));
}

TEST_F(BmpmanEvictTest, lock_then_unlock_does_not_keep) {
	int handle = load_still();

	ASSERT_NE(nullptr, bm_lock(handle, 16, BMP_TEX_OTHER));
	bm_unlock(handle);
	page_in();

	EXPECT_EQ(1u, frees_in(handle));
}

TEST_F(BmpmanEvictTest, animation_freed_whole_with_release) {
	int nframes = 0;
	int handle = load_anim(&nframes);

	page_in();

	ASSERT_EQ(static_cast<size_t>(nframes), frees_in(handle, nframes));
	for (int i = 0; i < nframes; i++) {
		EXPECT_EQ(1u, frees_in(handle + i)) << "frame " << i;
	}
	for (const auto& f : frees) {
		if (f.handle >= handle && f.handle < handle + nframes) {
			EXPECT_TRUE(f.release);
		}
	}

	for (int i = 0; i < nframes; i++) {
		ASSERT_NE(nullptr, bm_lock(handle + i, 16, BMP_TEX_OTHER)) << "frame " << i;
		bm_unlock(handle + i);
	}
}

TEST_F(BmpmanEvictTest, locked_frame_keeps_whole_animation) {
	int nframes = 0;
	int handle = load_anim(&nframes);

	ASSERT_NE(nullptr, bm_lock(handle + 1, 16, BMP_TEX_OTHER));
	page_in();

	EXPECT_EQ(0u, frees_in(handle, nframes));

	bm_unlock(handle + 1);
	page_in();

	EXPECT_EQ(static_cast<size_t>(nframes), frees_in(handle, nframes));
}

TEST_F(BmpmanEvictTest, later_frame_preloaded_alone_frees_then_rebuilds) {
	int nframes = 0;
	int handle = load_anim(&nframes);

	// frames are visited in handle order: the first frame's free runs before the preload of frame 1 rebuilds the array
	bm_page_in_start();
	bm_page_in_texture(handle + 1, 1);
	bm_page_in_stop();

	EXPECT_EQ(static_cast<size_t>(nframes), frees_in(handle, nframes));
}

TEST_F(BmpmanEvictTest, first_frame_preloaded_alone_keeps_animation) {
	int nframes = 0;
	int handle = load_anim(&nframes);

	bm_page_in_start();
	bm_page_in_texture(handle, 1);
	bm_page_in_stop();

	EXPECT_EQ(0u, frees_in(handle, nframes));
}

TEST_F(BmpmanEvictTest, user_bitmap_never_freed) {
	ubyte pixels[4 * 4 * 4] = {0};
	int handle = bm_create(32, 4, 4, pixels);
	ASSERT_GE(handle, 0);

	page_in();

	EXPECT_EQ(0u, frees_in(handle));
	bm_release(handle);
}

TEST_F(BmpmanEvictTest, volume_bitmap_never_freed) {
	ubyte pixels[4 * 4 * 4 * 4] = {0};
	int handle = bm_create_3d(32, 4, 4, 4, pixels);
	ASSERT_GE(handle, 0);

	page_in();

	EXPECT_EQ(0u, frees_in(handle));
	bm_release(handle);
}

TEST_F(BmpmanEvictTest, render_target_never_freed) {
	ubyte pixels[4 * 4 * 4] = {0};
	int handle = bm_create(32, 4, 4, pixels);
	ASSERT_GE(handle, 0);

	// the stub renderer cannot make a render target, so the slot is retyped by hand for the page-in filter
	bm_get_entry(handle)->type = BM_TYPE_RENDER_TARGET_STATIC;
	page_in();
	bm_get_entry(handle)->type = BM_TYPE_RENDER_TARGET_DYNAMIC;
	page_in();

	EXPECT_EQ(0u, frees_in(handle));

	bm_get_entry(handle)->type = BM_TYPE_USER;
	bm_release(handle);
}

TEST_F(BmpmanEvictTest, no_eviction_outside_opengl) {
	gr_screen.mode = GraphicsAPI::Vulkan;
	int handle = load_still();

	page_in();

	EXPECT_EQ(0u, frees_in(handle));
}

} // namespace
