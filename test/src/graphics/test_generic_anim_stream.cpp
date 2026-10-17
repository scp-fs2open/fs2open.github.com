#include <util/FSTestFixture.h>

#include <bmpman/bm_internal.h>
#include <bmpman/bmpman.h>
#include <globalincs/systemvars.h>
#include <graphics/2d.h>
#include <graphics/generic.h>

#include <cstring>

namespace {

class GenericAnimStreamTest : public test::FSTestFixture {
 public:
	GenericAnimStreamTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS | INIT_MOD_TABLE) {}

 protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();

		// the fixture runs as a standalone server, where bm_load substitutes placeholder images
		saved_standalone = Is_standalone;
		Is_standalone = 0;

		saved_capable = gr_screen.gf_is_capable;
	}

	void TearDown() override {
		gr_screen.gf_is_capable = saved_capable;
		Is_standalone = saved_standalone;

		test::FSTestFixture::TearDown();
	}

	// the pixels of one frame file, read through the same loader and flags the stream uses; loaded as a private slot so
	// the helper can never adopt a live stream's renamed next-frame slot
	static SCP_vector<ubyte> frame_pixels(const char* name) {
		SCP_vector<ubyte> out;
		int handle = bm_load_duplicate(name);
		EXPECT_GE(handle, 0);
		if (handle < 0) {
			return out;
		}

		bitmap* bmp = bm_lock(handle, 32, BMP_TEX_NONCOMP, true);
		if (bmp != nullptr && bmp->data != 0) {
			auto* data = reinterpret_cast<const ubyte*>(bmp->data);
			out.assign(data, data + bmp->w * bmp->h * 4);
		}
		bm_unlock(handle);
		bm_release(handle);
		return out;
	}

 private:
	int saved_standalone = 0;
	std::function<bool(gr_capability)> saved_capable;
};

TEST_F(GenericAnimStreamTest, eff_stream_owns_a_user_bitmap_holding_frame_zero) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));

	EXPECT_EQ(BM_TYPE_EFF, ga.type);
	EXPECT_EQ(3, ga.num_frames);
	ASSERT_GE(ga.bitmap_id, 0);
	EXPECT_EQ(BM_TYPE_USER, bm_get_type(ga.bitmap_id));
	ASSERT_NE(nullptr, ga.buffer);

	auto frame0 = frame_pixels("streamtest_0000");
	ASSERT_EQ(static_cast<size_t>(ga.width * ga.height * 4), frame0.size());
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_unload(&ga);
}

TEST_F(GenericAnimStreamTest, eff_stream_buffer_follows_the_current_frame) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_NE(nullptr, ga.buffer);

	// three frames over 0.3 s: 0.15 s of play lands on frame 1
	generic_anim_render(&ga, 0.15f, 0, 0);
	EXPECT_EQ(1, ga.current_frame);

	auto frame1 = frame_pixels("streamtest_0001");
	EXPECT_TRUE(bm_is_valid(ga.eff.next_frame)) << "the comparison read must not take the stream's slot";
	ASSERT_EQ(static_cast<size_t>(ga.width * ga.height * 4), frame1.size());
	EXPECT_EQ(0, memcmp(ga.buffer, frame1.data(), frame1.size()));

	generic_anim_unload(&ga);
}

TEST_F(GenericAnimStreamTest, eff_stream_unload_releases_bitmap_and_buffer) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	int handle = ga.bitmap_id;
	ASSERT_TRUE(bm_is_valid(handle));

	generic_anim_unload(&ga);

	EXPECT_EQ(nullptr, ga.buffer);
	EXPECT_EQ(-1, ga.bitmap_id);
	EXPECT_FALSE(bm_is_valid(handle));
}

TEST_F(GenericAnimStreamTest, eff_stream_skips_compressed_frames_and_keeps_the_buffer) {
	// with S3TC reported as supported the loader keeps DXT frames compressed instead of converting them to 32 bit
	gr_screen.gf_is_capable = [](gr_capability capability) { return capability == gr_capability::CAPABILITY_S3TC; };

	generic_anim ga;
	generic_anim_init(&ga, "streamdxt");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_NE(nullptr, ga.buffer);
	EXPECT_EQ(BM_TYPE_USER, bm_get_type(ga.bitmap_id));

	// the loader hands DXT frames back still compressed, so neither frame can be copied into the 32-bit buffer
	size_t bytes = static_cast<size_t>(ga.width) * ga.height * 4;
	SCP_vector<ubyte> zeros(bytes, 0);
	EXPECT_EQ(0, memcmp(ga.buffer, zeros.data(), bytes));

	generic_anim_render(&ga, 0.15f, 0, 0);
	EXPECT_EQ(1, ga.current_frame);
	EXPECT_EQ(0, memcmp(ga.buffer, zeros.data(), bytes));

	generic_anim_unload(&ga);
}

TEST_F(GenericAnimStreamTest, eff_stream_skips_a_compressed_frame_and_keeps_the_last_good_one) {
	gr_screen.gf_is_capable = [](gr_capability capability) { return capability == gr_capability::CAPABILITY_S3TC; };

	// frame 0 is a PNG, frame 1 a DXT1 file: the frame files are found by name, so a stream can mix formats
	generic_anim ga;
	generic_anim_init(&ga, "streammix");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_NE(nullptr, ga.buffer);

	auto frame0 = frame_pixels("streammix_0000");
	ASSERT_EQ(static_cast<size_t>(ga.width * ga.height * 4), frame0.size());
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_render(&ga, 0.15f, 0, 0);
	EXPECT_EQ(1, ga.current_frame);
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_unload(&ga);
}

// locks a frame file at 32 bpp and returns its bytes as the loader delivered them; bpp_out receives the loader's depth,
// which for PNG is the file's channel count times eight regardless of the depth asked for
SCP_vector<ubyte> frame_raw(const char* name, int* bpp_out) {
	int handle = bm_load_duplicate(name);
	EXPECT_TRUE(handle >= 0) << name;
	if (handle < 0) {
		return {};
	}
	bitmap* bmp = bm_lock(handle, 32, BMP_TEX_NONCOMP, true);
	EXPECT_NE(nullptr, bmp);
	if (bmp == nullptr) {
		return {};
	}
	*bpp_out = bmp->bpp;
	auto bytes = reinterpret_cast<const ubyte*>(bmp->data);
	SCP_vector<ubyte> out(bytes, bytes + static_cast<size_t>(bmp->w) * bmp->h * (bmp->bpp / 8));
	bm_unlock(handle);
	bm_release(handle);
	return out;
}

// an RGB PNG without alpha comes back from the loader at 24 bpp; the stream's 32 bpp buffer gets it with alpha 255
TEST_F(GenericAnimStreamTest, eff_stream_expands_rgb_frames_to_32_bit) {
	generic_anim ga;
	generic_anim_init(&ga, "streamrgb");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_NE(nullptr, ga.buffer);

	for (int frame = 0; frame < 2; frame++) {
		if (frame == 1) {
			generic_anim_render(&ga, 0.15f, 0, 0);
			ASSERT_EQ(1, ga.current_frame);
		}
		int bpp = 0;
		auto raw = frame_raw(frame == 0 ? "streamrgb_0000" : "streamrgb_0001", &bpp);
		ASSERT_EQ(24, bpp);
		SCP_vector<ubyte> expected;
		for (size_t px = 0; px < raw.size() / 3; px++) {
			expected.push_back(raw[px * 3]);
			expected.push_back(raw[px * 3 + 1]);
			expected.push_back(raw[px * 3 + 2]);
			expected.push_back(255);
		}
		ASSERT_EQ(static_cast<size_t>(ga.width * ga.height * 4), expected.size());
		EXPECT_EQ(0, memcmp(ga.buffer, expected.data(), expected.size())) << "frame " << frame;
	}

	generic_anim_unload(&ga);
}

// a HUD-color stream draws through an 8 bpp AABITMAP; the frame's color is folded to luminance the way the OpenGL
// update path did for the old file bitmap (mean of the three color channels, scaled by alpha)
TEST_F(GenericAnimStreamTest, eff_stream_hud_color_stream_holds_8_bit_luminance) {
	generic_anim ga;
	generic_anim_init(&ga, "streamhud");
	ga.use_hud_color = true;
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_NE(nullptr, ga.buffer);
	ASSERT_TRUE(bm_is_valid(ga.bitmap_id));
	EXPECT_EQ(BM_TYPE_USER, bm_get_type(ga.bitmap_id));

	int bpp = 0;
	auto raw = frame_raw("streamhud_0000", &bpp);
	ASSERT_EQ(32, bpp);
	SCP_vector<ubyte> expected;
	for (size_t px = 0; px < raw.size() / 4; px++) {
		int luminance = raw[px * 4] + raw[px * 4 + 1] + raw[px * 4 + 2];
		expected.push_back(static_cast<ubyte>((luminance / 3) * (raw[px * 4 + 3] / 255.0f)));
	}
	ASSERT_EQ(static_cast<size_t>(ga.width * ga.height), expected.size());
	// solid red at alpha 128: (255 / 3) * (128 / 255) truncates to 42
	EXPECT_EQ(42, expected[0]);
	EXPECT_EQ(0, memcmp(ga.buffer, expected.data(), expected.size()));

	// the advance path runs the same conversion: frame 1 is white at alpha 128, (765 / 3) * (128 / 255) truncates
	// to 128
	generic_anim_render(&ga, 0.15f, 0, 0);
	ASSERT_EQ(1, ga.current_frame);
	EXPECT_EQ(128, ga.buffer[0]);
	for (size_t px = 0; px < expected.size(); px++) {
		EXPECT_EQ(128, ga.buffer[px]) << "pixel " << px;
	}

	generic_anim_unload(&ga);
}

// the talking-head code streams the same generic_anim again without unloading it when the head file has not changed
TEST_F(GenericAnimStreamTest, eff_stream_restream_reuses_its_bitmap_and_buffer) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	int first_bitmap = ga.bitmap_id;
	ASSERT_TRUE(first_bitmap >= 0);
	ASSERT_NE(nullptr, ga.buffer);
	// a released slot can be handed out again under the same handle, so the entry's signature (unique per creation)
	// is what shows the bitmap was kept rather than remade
	int first_signature = bm_get_entry(first_bitmap)->signature;

	generic_anim_render(&ga, 0.15f, 0, 0);
	ASSERT_EQ(1, ga.current_frame);

	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_EQ(first_bitmap, ga.bitmap_id);
	EXPECT_EQ(first_signature, bm_get_entry(ga.bitmap_id)->signature);
	auto frame0 = frame_pixels("streamtest_0000");
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_unload(&ga);
	EXPECT_EQ(nullptr, ga.buffer);
	EXPECT_FALSE(bm_is_valid(first_bitmap));
}

// streambad_0000.png has a valid header and truncated image data, so it loads but cannot be locked
TEST_F(GenericAnimStreamTest, eff_stream_fails_cleanly_when_frame_zero_is_corrupt) {
	generic_anim ga;
	generic_anim_init(&ga, "streambad");
	EXPECT_EQ(-1, generic_anim_stream(&ga));
	EXPECT_EQ(nullptr, ga.buffer);
	EXPECT_TRUE(ga.bitmap_id < 0);

	// the frame file's load count is balanced: a fresh load gets a handle that can be released normally
	int handle = bm_load("streambad_0000");
	ASSERT_TRUE(handle >= 0);
	EXPECT_TRUE(bm_release(handle));
	EXPECT_FALSE(bm_is_valid(handle));
}

// streamsize_0001.png is 4x4 where frame 0 is 8x8: a frame of another size is not copied into the 8x8 buffer. The
// smaller direction is used on purpose: bm_reload keeps the slot's first-file size, so a BIGGER later frame overflows
// inside the loader before this code runs (scp-fs2open #7883, a bmpman guard), which no check here can prevent
TEST_F(GenericAnimStreamTest, eff_stream_skips_a_frame_of_another_size) {
	generic_anim ga;
	generic_anim_init(&ga, "streamsize");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_EQ(8, ga.width);
	ASSERT_EQ(8, ga.height);

	auto frame0 = frame_pixels("streamsize_0000");
	generic_anim_render(&ga, 0.15f, 0, 0);
	EXPECT_EQ(1, ga.current_frame);
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_unload(&ga);
}

// without S3TC the loader decodes DXT frames to 32 bpp on read; those are plain pixels and are copied like any other
TEST_F(GenericAnimStreamTest, eff_stream_copies_dxt_frames_the_loader_has_decoded) {
	gr_screen.gf_is_capable = [](gr_capability) { return false; };

	generic_anim ga;
	generic_anim_init(&ga, "streamdxt");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_NE(nullptr, ga.buffer);

	for (int frame = 0; frame < 2; frame++) {
		if (frame == 1) {
			generic_anim_render(&ga, 0.15f, 0, 0);
			ASSERT_EQ(1, ga.current_frame);
		}
		int bpp = 0;
		auto decoded = frame_raw(frame == 0 ? "streamdxt_0000" : "streamdxt_0001", &bpp);
		ASSERT_EQ(32, bpp);
		ASSERT_EQ(static_cast<size_t>(ga.width * ga.height * 4), decoded.size());
		EXPECT_NE(0, decoded[0] + decoded[1] + decoded[2]) << "a decoded red block has color";
		EXPECT_EQ(0, memcmp(ga.buffer, decoded.data(), decoded.size())) << "frame " << frame;
	}

	generic_anim_unload(&ga);
}

// streambadlater_0001.png is truncated: the lock fails, the stream keeps frame 0 and does not touch a null bitmap
TEST_F(GenericAnimStreamTest, eff_stream_keeps_frame_zero_when_a_later_frame_is_corrupt) {
	generic_anim ga;
	generic_anim_init(&ga, "streambadlater");
	ASSERT_EQ(0, generic_anim_stream(&ga));

	auto frame0 = frame_pixels("streambadlater_0000");
	generic_anim_render(&ga, 0.15f, 0, 0);
	EXPECT_EQ(1, ga.current_frame);
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_unload(&ga);
}

// a re-stream loads the next-frame file again; the previous load is released first, so the count does not climb
TEST_F(GenericAnimStreamTest, eff_stream_restream_balances_the_next_frame_load) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_TRUE(ga.eff.next_frame >= 0);
	EXPECT_EQ(1, bm_get_entry(ga.eff.next_frame)->load_count);

	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_TRUE(ga.eff.next_frame >= 0);
	EXPECT_EQ(1, bm_get_entry(ga.eff.next_frame)->load_count);

	generic_anim_unload(&ga);
}

// a struct whose last stream was another format (type and buffer left by the ANI path) is not the EFF path's to free;
// the old buffer is left alone and a fresh pair is made
TEST_F(GenericAnimStreamTest, eff_stream_does_not_free_a_buffer_another_stream_type_owns) {
	static ubyte foreign_buffer[16] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ga.type = BM_TYPE_ANI;
	ga.buffer = foreign_buffer;
	ga.width = 4;
	ga.height = 4;

	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_NE(foreign_buffer, ga.buffer);
	EXPECT_EQ(7, foreign_buffer[0]);

	generic_anim_unload(&ga);
}

// a restart at another depth replaces the buffer and bitmap; the next-frame load from the first run is still released
TEST_F(GenericAnimStreamTest, eff_stream_restart_at_another_depth_balances_the_next_frame_load) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	ASSERT_EQ(1, bm_get_entry(ga.eff.next_frame)->load_count);

	ga.use_hud_color = true;
	ASSERT_EQ(0, generic_anim_stream(&ga));
	// the replaced bitmap may come back in the same slot, so the handle value is not checked; the depth is
	EXPECT_EQ(8, ga.eff.bpp);
	EXPECT_EQ(1, bm_get_entry(ga.eff.next_frame)->load_count);

	generic_anim_unload(&ga);
}

// bm_lock's failure path already drops the stream's load of frame 0; another owner's load of the same file must survive
TEST_F(GenericAnimStreamTest, eff_stream_failure_keeps_another_owners_load_of_frame_zero) {
	int shared = bm_load("streambad_0000");
	ASSERT_TRUE(shared >= 0);

	generic_anim ga;
	generic_anim_init(&ga, "streambad");
	EXPECT_EQ(-1, generic_anim_stream(&ga));

	// the stream reads frame 0 through a private slot, so the other owner's slot and load are untouched
	EXPECT_TRUE(bm_is_valid(shared));
	EXPECT_EQ(1, bm_get_entry(shared)->load_count);
	EXPECT_TRUE(bm_release(shared));
	EXPECT_FALSE(bm_is_valid(shared));
}

// a failed stream must leave the struct in a state generic_anim_unload accepts (the Lua object unloads unconditionally)
TEST_F(GenericAnimStreamTest, eff_stream_failure_leaves_a_struct_unload_accepts) {
	generic_anim ga;
	generic_anim_init(&ga, "streambad");
	ASSERT_EQ(-1, generic_anim_stream(&ga));
	EXPECT_EQ(0, ga.num_frames);
	generic_anim_unload(&ga);

	// streammissing.eff exists but has no streammissing_0000 image: the first-frame-not-found exit
	generic_anim missing;
	generic_anim_init(&missing, "streammissing");
	EXPECT_EQ(-1, generic_anim_stream(&missing));
	EXPECT_EQ(0, missing.num_frames);
	EXPECT_EQ(nullptr, missing.buffer);
	generic_anim_unload(&missing);

	// the same exit on a struct that owns a previous EFF run releases that run's pair
	generic_anim owner;
	generic_anim_init(&owner, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&owner));
	int owned_bitmap = owner.bitmap_id;
	int owned_next = owner.eff.next_frame;
	strcpy_s(owner.filename, "streammissing");
	EXPECT_EQ(-1, generic_anim_stream(&owner));
	EXPECT_FALSE(bm_is_valid(owned_bitmap));
	EXPECT_FALSE(bm_is_valid(owned_next));
	EXPECT_EQ(nullptr, owner.buffer);
	generic_anim_unload(&owner);
}

// bm_reload renames a stream's next-frame slot to the frame it holds, and bm_load finds slots by name: a second
// stream of the same file must not take, copy and release the first stream's slot when that one sits on frame 0
TEST_F(GenericAnimStreamTest, eff_stream_frame_zero_load_never_takes_another_streams_slot) {
	generic_anim first;
	generic_anim_init(&first, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&first));
	generic_anim_render(&first, 0.15f, 0, 0);
	generic_anim_render(&first, 0.1f, 0, 0);
	generic_anim_render(&first, 0.1f, 0, 0);
	ASSERT_EQ(0, first.current_frame) << "three advances on a looping three-frame stream wrap to frame 0";
	int slot = first.eff.next_frame;
	ASSERT_TRUE(bm_is_valid(slot));

	generic_anim second;
	generic_anim_init(&second, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&second));
	EXPECT_TRUE(bm_is_valid(slot)) << "the first stream's next-frame slot survives the second stream's start";

	auto frame1 = frame_pixels("streamtest_0001");
	generic_anim_render(&first, 0.1f, 0, 0);
	EXPECT_EQ(1, first.current_frame);
	EXPECT_EQ(0, memcmp(first.buffer, frame1.data(), frame1.size())) << "the first stream still advances";

	generic_anim_unload(&second);
	generic_anim_unload(&first);
}

// a one-frame EFF has no next-frame file: the stream, a restart and the unload all tolerate the -1 handle
TEST_F(GenericAnimStreamTest, eff_stream_one_frame_eff_streams_restarts_and_unloads) {
	generic_anim ga;
	generic_anim_init(&ga, "streamone");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_EQ(1, ga.num_frames);
	EXPECT_TRUE(ga.eff.next_frame < 0);
	auto frame0 = frame_pixels("streamone_0000");
	EXPECT_EQ(0, memcmp(ga.buffer, frame0.data(), frame0.size()));

	generic_anim_render(&ga, 0.5f, 0, 0);
	EXPECT_EQ(0, ga.current_frame);

	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_TRUE(ga.eff.next_frame < 0);
	generic_anim_unload(&ga);
	EXPECT_EQ(nullptr, ga.buffer);
}

// a restart that fails before the EFF branch (file not found) must leave the ownership record intact, so the next
// successful restart still reuses or releases the first run's pair instead of orphaning it
TEST_F(GenericAnimStreamTest, eff_stream_failed_lookup_keeps_the_ownership_record) {
	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	int first_bitmap = ga.bitmap_id;
	int first_signature = bm_get_entry(first_bitmap)->signature;

	strcpy_s(ga.filename, "streamnone");
	ASSERT_EQ(-1, generic_anim_stream(&ga));
	EXPECT_TRUE(bm_is_valid(first_bitmap)) << "a failed lookup touches nothing";

	strcpy_s(ga.filename, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_EQ(first_signature, bm_get_entry(ga.bitmap_id)->signature);
	EXPECT_EQ(1, bm_get_entry(ga.eff.next_frame)->load_count);

	generic_anim_unload(&ga);
	EXPECT_FALSE(bm_is_valid(first_bitmap));
}

// on a fresh struct eff.next_frame is whatever the union held (0 after init, often a live bitmap); a failure exit must
// not release a handle this stream never loaded
TEST_F(GenericAnimStreamTest, eff_stream_failure_never_releases_a_handle_it_did_not_load) {
	int bystander = bm_load("streamtest_0002");
	ASSERT_TRUE(bystander >= 0);

	generic_anim ga;
	generic_anim_init(&ga, "streambad");
	ga.eff.next_frame = bystander;
	EXPECT_EQ(-1, generic_anim_stream(&ga));

	EXPECT_TRUE(bm_is_valid(bystander));
	EXPECT_EQ(1, bm_get_entry(bystander)->load_count);
	EXPECT_TRUE(bm_release(bystander));
	generic_anim_unload(&ga);
}

// a struct tagged by another stream type whose EFF attempt fails before frame 0 must not become "owned" and have its
// foreign buffer freed by the successful attempt that follows
TEST_F(GenericAnimStreamTest, eff_stream_failed_attempt_does_not_adopt_a_foreign_buffer) {
	static ubyte foreign_buffer[16] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
	generic_anim ga;
	generic_anim_init(&ga, "streammissing");
	ga.type = BM_TYPE_ANI;
	ga.buffer = foreign_buffer;
	EXPECT_EQ(-1, generic_anim_stream(&ga));

	strcpy_s(ga.filename, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_NE(foreign_buffer, ga.buffer);
	EXPECT_EQ(9, foreign_buffer[0]);
	generic_anim_unload(&ga);
}

// the renderer upload runs once per advance and once more only when a restart keeps a bitmap that already has a texture
TEST_F(GenericAnimStreamTest, eff_stream_uploads_once_per_advance_and_on_a_kept_bitmap) {
	static int uploads = 0;
	uploads = 0;
	auto saved_update = gr_screen.gf_update_texture;
	gr_screen.gf_update_texture = [](int, int, const ubyte*, int, int) { uploads++; };

	generic_anim ga;
	generic_anim_init(&ga, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_EQ(0, uploads) << "a fresh bitmap has no texture to refresh";

	generic_anim_render(&ga, 0.15f, 0, 0);
	ASSERT_EQ(1, ga.current_frame);
	EXPECT_EQ(1, uploads);

	ASSERT_EQ(0, generic_anim_stream(&ga));
	EXPECT_EQ(2, uploads) << "a kept bitmap is refreshed with frame 0";

	generic_anim_unload(&ga);
	gr_screen.gf_update_texture = saved_update;
}

// two streams of one EFF must not share a next-frame slot: with a shared slot the first stream's unload frees it (the
// per-frame unload has already taken both loads to zero) and the second stream's next bm_reload fails, freezing it
TEST_F(GenericAnimStreamTest, eff_stream_next_frame_slot_is_private_to_each_stream) {
	generic_anim first;
	generic_anim_init(&first, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&first));
	generic_anim second;
	generic_anim_init(&second, "streamtest");
	ASSERT_EQ(0, generic_anim_stream(&second));
	EXPECT_NE(first.eff.next_frame, second.eff.next_frame);

	generic_anim_render(&first, 0.15f, 0, 0);
	generic_anim_render(&second, 0.15f, 0, 0);
	ASSERT_EQ(1, first.current_frame);
	ASSERT_EQ(1, second.current_frame);

	generic_anim_unload(&first);
	EXPECT_TRUE(bm_is_valid(second.eff.next_frame)) << "the second stream's slot survives the first stream's unload";

	auto frame2 = frame_pixels("streamtest_0002");
	generic_anim_render(&second, 0.1f, 0, 0);
	EXPECT_EQ(2, second.current_frame);
	EXPECT_EQ(0, memcmp(second.buffer, frame2.data(), frame2.size())) << "the second stream still advances";
	generic_anim_unload(&second);
}

} // namespace
