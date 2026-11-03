#include "texcache/tex_cache.h"
#include "util/FSTestFixture.h"

#include <chrono>
#include <filesystem>
#include <limits>

extern float tex_cache_size;

class TextureCacheTest : public test::FSTestFixture {
protected:
	float saved_cache_size = tex_cache_size;
	const char* key = "unittest-texture-cache";
	const char* source_name = "unittest-texture-cache.dds";

	void SetUp() override
	{
		test::FSTestFixture::SetUp();
		tex_cache_size = 3.0f;
	}

	void TearDown() override
	{
		cf_delete("tex-unittest-texture-cache.cache", CF_TYPE_CACHE, CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
		cf_delete(source_name, CF_TYPE_CACHE, CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
		tex_cache_size = saved_cache_size;
		test::FSTestFixture::TearDown();
	}

	void write_source(const SCP_vector<ubyte>& data)
	{
		std::unique_ptr<CFILE> fp(cfopen(source_name, "wb", CF_TYPE_CACHE, false,
			CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT));
		ASSERT_NE(nullptr, fp);
		ASSERT_EQ(static_cast<int>(data.size()), cfwrite(data.data(), 1, static_cast<int>(data.size()), fp.get()));
	}

	SCP_string source_key()
	{
		const auto location = cf_find_file_location(source_name, CF_TYPE_CACHE,
			CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
		std::unique_ptr<CFILE> fp(cfopen(location.full_name.c_str(), "rb", CF_TYPE_CACHE));
		EXPECT_NE(nullptr, fp);
		if (!fp) {
			return {};
		}
		cfseek(fp.get(), 128, CF_SEEK_SET);
		auto result = tex_cache_make_key(fp.get(), location.full_name.c_str(), TEX_CACHE_TYPE_ETC2, 0, 0, 16);
		EXPECT_EQ(128, cftell(fp.get()));
		return result;
	}
};

TEST_F(TextureCacheTest, portable_round_trip_and_prune)
{
	// FSTestFixture runs in portable mode, without a user root.
	SCP_vector<ubyte> payload(1024, 42);
	SCP_vector<ubyte> loaded(payload.size());
	tex_cache_store(key, 32, 32, 1, 0, payload.data(), payload.size());
	ASSERT_TRUE(tex_cache_try_load(key, 32, 32, 1, 0, loaded.size(), loaded.data()));
	EXPECT_EQ(payload, loaded);

	tex_cache_size = 0.0f;
	tex_cache_prune();
	EXPECT_FALSE(cf_find_file_location("tex-unittest-texture-cache.cache", CF_TYPE_CACHE,
		CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT).found);
}

TEST_F(TextureCacheTest, repaint_after_checksum_window_changes_key)
{
	SCP_vector<ubyte> data(128 * 1024, 42);
	write_source(data);
	const auto location = cf_find_file_location(source_name, CF_TYPE_CACHE,
		CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	ASSERT_TRUE(location.found);
	const auto original_time = std::filesystem::last_write_time(location.full_name.c_str());
	const auto original_key = source_key();
	ASSERT_FALSE(original_key.empty());
	std::filesystem::last_write_time(location.full_name.c_str(), original_time + std::chrono::seconds(10));
	EXPECT_NE(original_key, source_key());

	// Same name, length and first 64 KB; only a lower part of the texture changes.
	data[96 * 1024] = 43;
	write_source(data);
	std::filesystem::last_write_time(location.full_name.c_str(), original_time + std::chrono::seconds(20));
	EXPECT_NE(original_key, source_key());
}

TEST_F(TextureCacheTest, oversized_stored_payload_is_rejected)
{
	std::unique_ptr<CFILE> fp(cfopen("tex-unittest-texture-cache.cache", "wb", CF_TYPE_CACHE, false,
		CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT));
	ASSERT_NE(nullptr, fp);
	const uint header[] = {0x32435445u, 1, 0, 0, 4, 4, 1, 16, std::numeric_limits<uint>::max(), 0};
	for (auto field : header) {
		cfwrite_uint(field, fp.get());
	}
	fp.reset();

	ubyte loaded[16] = {};
	EXPECT_FALSE(tex_cache_try_load(key, 4, 4, 1, 0, sizeof(loaded), loaded));
}
