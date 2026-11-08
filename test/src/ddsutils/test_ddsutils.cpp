#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "graphics/2d.h"
#include "util/FSTestFixture.h"

class DDSConversionResizeTest : public test::FSTestFixture {
protected:
	std::function<bool(gr_capability)> saved_capability_query;
	bool saved_no_cache = false;
	const char* source_name = "unittest-dds-resize.dds";

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		saved_capability_query = gr_screen.gf_is_capable;
		gr_screen.gf_is_capable = [](gr_capability capability) {
			return capability == gr_capability::CAPABILITY_ETC2;
		};
		saved_no_cache = Cmdline_no_transcode_cache;
		Cmdline_no_transcode_cache = true;
	}

	void TearDown() override
	{
		cf_delete(source_name, CF_TYPE_CACHE);
		Cmdline_no_transcode_cache = saved_no_cache;
		gr_screen.gf_is_capable = saved_capability_query;
		test::FSTestFixture::TearDown();
	}

	static void write_header(CFILE* fp, uint width, uint height, uint mips)
	{
		// A standard 124-byte DDS header with a DXT1 pixel format.
		uint header[31] = {};
		header[0] = 124;
		header[1] = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
		header[2] = height;
		header[3] = width;
		header[5] = 1;
		header[6] = mips;
		header[18] = 32;
		header[19] = DDPF_FOURCC;
		header[20] = FOURCC_DXT1;
		header[26] = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
		cfwrite_uint(DDS_FILECODE, fp);
		for (auto field : header) {
			cfwrite_uint(field, fp);
		}
	}

	static void check_dimensions(uint width, uint height, int expected_width, int expected_height,
		int expected_mips = 3, uint input_mips = 3, int expected_format = DDS_ETC2_RGB)
	{
		SCOPED_TRACE(::testing::Message() << width << "x" << height);
		std::unique_ptr<CFILE> fp(ctmpfile());
		ASSERT_NE(nullptr, fp);
		write_header(fp.get(), width, height, input_mips);
		cfseek(fp.get(), 0, CF_SEEK_SET);
		int actual_width, actual_height, actual_mips, actual_format;
		ASSERT_EQ(DDS_ERROR_NONE, dds_read_header(nullptr, fp.get(), &actual_width, &actual_height,
			nullptr, &actual_format, &actual_mips));
		EXPECT_EQ(expected_width, actual_width);
		EXPECT_EQ(expected_height, actual_height);
		EXPECT_EQ(expected_mips, actual_mips);
		EXPECT_EQ(expected_format, actual_format);
	}
};

TEST_F(DDSConversionResizeTest, keeps_small_and_narrow_etc2_textures_at_original_size)
{
	check_dimensions(128, 128, 128, 128);
	check_dimensions(256, 256, 256, 256);
	check_dimensions(1024, 256, 1024, 256);
	check_dimensions(256, 2048, 256, 2048);
}

TEST_F(DDSConversionResizeTest, halves_large_etc2_textures_and_keeps_the_4096_cap)
{
	check_dimensions(512, 512, 256, 256, 2);
	check_dimensions(1024, 512, 512, 256, 2);
	check_dimensions(1024, 1024, 512, 512, 2);
	check_dimensions(2048, 2048, 1024, 1024, 2);
	check_dimensions(4096, 4096, 2048, 2048, 2);
	check_dimensions(8192, 8192, 4096, 4096, 2);
	check_dimensions(1024, 1024, 1024, 1024, 1, 1);
}

TEST_F(DDSConversionResizeTest, software_fallback_only_resizes_above_1024)
{
	gr_screen.gf_is_capable = [](gr_capability) { return false; };
	check_dimensions(128, 128, 128, 128, 3, 3, DDS_UNCOMPRESSED);
	check_dimensions(512, 512, 512, 512, 3, 3, DDS_UNCOMPRESSED);
	check_dimensions(1024, 1024, 1024, 1024, 3, 3, DDS_UNCOMPRESSED);
	check_dimensions(2048, 2048, 1024, 1024, 2, 3, DDS_UNCOMPRESSED);
}

TEST_F(DDSConversionResizeTest, small_etc2_bitmap_matches_the_reported_header_size)
{
	std::unique_ptr<CFILE> fp(cfopen(source_name, "wb", CF_TYPE_CACHE));
	ASSERT_NE(nullptr, fp);
	write_header(fp.get(), 128, 128, 4);
	SCP_vector<ubyte> blocks(10880, 0); // DXT1 levels: 128, 64, 32 and 16.
	ASSERT_EQ(static_cast<int>(blocks.size()), cfwrite(blocks.data(), 1, static_cast<int>(blocks.size()), fp.get()));
	fp.reset();
	fp.reset(cfopen(source_name, "rb", CF_TYPE_CACHE));
	ASSERT_NE(nullptr, fp);
	int width, height, levels;
	size_t size = 0;
	ASSERT_EQ(DDS_ERROR_NONE, dds_read_header(nullptr, fp.get(), &width, &height, nullptr, nullptr, &levels, &size));
	EXPECT_EQ(128, width);
	EXPECT_EQ(128, height);
	EXPECT_EQ(4, levels);
	ASSERT_EQ(blocks.size(), size);
	fp.reset();

	SCP_vector<ubyte> data(size + 16, 0xcd);
	ubyte bpp = 0;
	ASSERT_EQ(DDS_ERROR_NONE, dds_read_bitmap(source_name, data.data(), &bpp, CF_TYPE_CACHE));
	EXPECT_EQ(24, bpp);
	for (size_t i = size; i < data.size(); ++i) {
		EXPECT_EQ(0xcd, data[i]);
	}
}
