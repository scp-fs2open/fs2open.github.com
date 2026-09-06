#include "texcache/tex_cache.h"
#include "cfile/cfile.h"
#include "options/Option.h"

#include "lz4.h"
#include <md5.h>

#include <cstring>
#include <algorithm>

#define TEX_CACHE_VERSION_TAG "tex-v1"
static constexpr uint TEX_CACHE_MAGIC = 0x32435445u;
static constexpr uint TEX_CACHE_VERSION = 1u;
static constexpr uint TEX_CACHE_FLAG_LZ4 = 0x1u;
static constexpr int TEX_CACHE_CHKSUM_WINDOW = 65536;

static const uint32_t CACHE_LOCATION_FLAGS = CF_LOCATION_ROOT_USER | CF_LOCATION_TYPE_ROOT;

static SCP_string tex_cache_filename(const SCP_string &key)
{
	return SCP_string("tex-") + key + ".cache";
}

static SCP_string tex_cache_temp_filename(const SCP_string &key)
{
	return SCP_string("tex-") + key + ".cache.tmp";
}

SCP_string tex_cache_make_key(CFILE *dds, tex_cache_type cache_type, uint src_fourcc, int dst_format, uint tex_size)
{
	const int saved_pos = cftell(dds);

	cfseek(dds, 0, CF_SEEK_SET);

	uint src_crc = 0;
	cf_chksum_long(dds, &src_crc, TEX_CACHE_CHKSUM_WINDOW);

	const uint src_len = (uint)cfilelength(dds);

	cfseek(dds, saved_pos, CF_SEEK_SET);

	const char* name = cf_get_filename(dds);
	if (name == nullptr) {
		name = "";
	}

	MD5 md5;
	md5.update(TEX_CACHE_VERSION_TAG, (MD5::size_type)strlen(TEX_CACHE_VERSION_TAG));
	md5.update(reinterpret_cast<const char*>(&cache_type), sizeof(cache_type));
	md5.update(name, (MD5::size_type)strlen(name));
	md5.update(reinterpret_cast<const char*>(&src_crc), sizeof(src_crc));
	md5.update(reinterpret_cast<const char*>(&src_len), sizeof(src_len));
	md5.update(reinterpret_cast<const char*>(&src_fourcc), sizeof(src_fourcc));
	md5.update(reinterpret_cast<const char*>(&dst_format), sizeof(dst_format));
	md5.update(reinterpret_cast<const char*>(&tex_size), sizeof(tex_size));
	md5.finalize();

	return md5.hexdigest();
}

bool tex_cache_try_load(const SCP_string &key, uint expected_width, uint expected_height, uint expected_mips, int expected_format, size_t expected_size, ubyte *out_data)
{
	const SCP_string name = tex_cache_filename(key);

	CFILE *fp = cfopen(name.c_str(), "rb", CF_TYPE_CACHE, false, CACHE_LOCATION_FLAGS);

	if (fp == nullptr) {
		return false;
	}

	const uint magic = cfread_uint(fp);
	const uint version = cfread_uint(fp);
	const uint flags = cfread_uint(fp);
	const uint format = cfread_uint(fp);
	const uint width = cfread_uint(fp);
	const uint height = cfread_uint(fp);
	const uint mips = cfread_uint(fp);
	const uint payload_size = cfread_uint(fp);
	const uint stored_size = cfread_uint(fp);
	const uint payload_crc = cfread_uint(fp);

	const bool header_ok = (magic == TEX_CACHE_MAGIC) &&
	                       (version == TEX_CACHE_VERSION) &&
	                       ((flags & ~TEX_CACHE_FLAG_LZ4) == 0) &&
	                       (format == (uint)expected_format) &&
	                       (width == expected_width) &&
	                       (height == expected_height) &&
	                       (mips == expected_mips) &&
	                       (payload_size == (uint)expected_size);

	if (!header_ok) {
		cfclose(fp);
		return false;
	}

	SCP_vector<ubyte> stored(stored_size);
	const int got = cfread(stored.data(), 1, (int)stored_size, fp);

	cfclose(fp);

	if (got != (int)stored_size) {
		cf_delete(name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
		return false;
	}

	bool ok = false;

	if (flags & TEX_CACHE_FLAG_LZ4) {
		const int dec = LZ4_decompress_safe(reinterpret_cast<const char *>(stored.data()),
		                                    reinterpret_cast<char *>(out_data),
		                                    (int)stored_size, (int)payload_size);
		ok = (dec == (int)payload_size);
	} else if (stored_size == payload_size) {
		memcpy(out_data, stored.data(), payload_size);
		ok = true;
	}

	if (!ok) {
		cf_delete(name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
		return false;
	}

	const uint crc = cf_add_chksum_long(0, out_data, payload_size);

	if (crc != payload_crc) {
		cf_delete(name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
		return false;
	}

	return true;
}

void tex_cache_store(const SCP_string &key, uint width, uint height, uint mips, int format, const ubyte *payload, size_t payload_size)
{
	if (payload == nullptr || payload_size == 0) {
		return;
	}

	const uint payload_crc = cf_add_chksum_long(0, const_cast<ubyte *>(payload), payload_size);

	const int bound = LZ4_compressBound((int)payload_size);

	SCP_vector<ubyte> compressed(bound > 0 ? (size_t)bound : 1);

	const int csize = LZ4_compress_fast(reinterpret_cast<const char *>(payload),
	                                  reinterpret_cast<char *>(compressed.data()),
	                                  (int)payload_size, bound, 1);

	uint flags = 0;
	const ubyte *out_buf = payload;
	size_t out_size = payload_size;

	if ((csize > 0) && ((size_t)csize < payload_size)) {
		flags = TEX_CACHE_FLAG_LZ4;
		out_buf = compressed.data();
		out_size = (size_t)csize;
	}

	const SCP_string temp_name = tex_cache_temp_filename(key);
	const SCP_string final_name = tex_cache_filename(key);

	CFILE *fp = cfopen(temp_name.c_str(), "wb", CF_TYPE_CACHE, false, CACHE_LOCATION_FLAGS);

	if (fp == nullptr) {
		return;
	}

	cfwrite_uint(TEX_CACHE_MAGIC, fp);
	cfwrite_uint(TEX_CACHE_VERSION, fp);
	cfwrite_uint(flags, fp);
	cfwrite_uint((uint)format, fp);
	cfwrite_uint(width, fp);
	cfwrite_uint(height, fp);
	cfwrite_uint(mips, fp);
	cfwrite_uint((uint)payload_size, fp);
	cfwrite_uint((uint)out_size, fp);
	cfwrite_uint(payload_crc, fp);

	const int written = cfwrite(out_buf, 1, (int)out_size, fp);

	cfclose(fp);

	if (written != (int)out_size) {
		cf_delete(temp_name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
		return;
	}

	cf_rename(temp_name.c_str(), final_name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
}

auto CacheSizeLimit = options::OptionBuilder<float>("Graphics.TexCacheSize",
	std::pair<const char*, int>{"Texture Transcode Cache", 1933},
	std::pair<const char*, int>{"Texture transcode cache size in GBs. 0 for unlimited. Mostly relevant for mobile GPUs lacking S3TC / BC7 support.", 1934})
	.category(std::make_pair("Graphics", 1825))
	.level(options::ExpertLevel::Advanced)
	.range(0.0f, 20.0f)
	.default_val(3.0f)
	.importance(0)
	.finish();

void tex_cache_prune()
{
	// Clear any incomplete .tmp file
	SCP_vector<SCP_string> temps;

	cf_get_file_list(temps, CF_TYPE_CACHE, "tex-*.cache.tmp", CF_SORT_NONE, nullptr, CACHE_LOCATION_FLAGS);

	for (const auto& temp : temps) {
		const SCP_string tname = temp + ".tmp";
		cf_delete(tname.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
	}

	auto max_bytes = (size_t)(CacheSizeLimit->getValue() * 1000000000);

	// unlimited cache
	if (max_bytes <= 0)
		return;

	// Check and clear cache
	SCP_vector<SCP_string> names;
	SCP_vector<file_list_info> info;

	cf_get_file_list(names, CF_TYPE_CACHE, "tex-*.cache", CF_SORT_NONE, &info, CACHE_LOCATION_FLAGS);

	const size_t count = names.size();

	if (count == 0) {
		return;
	}

	size_t total = 0;
	for (size_t i = 0; i < count; ++i) {
		total += info[i].size;
	}

	if (total <= max_bytes) {
		return;
	}

	// Free a minimum of 20%, not the entire cache.
	const size_t target = (max_bytes / 5) * 4;

	SCP_vector<size_t> order(count);
	for (size_t i = 0; i < count; ++i) {
		order[i] = i;
	}

	std::sort(order.begin(), order.end(), [&info](size_t a, size_t b) {
		return info[a].write_time < info[b].write_time;
	});

	for (size_t k = 0; (k < count) && (total > target); ++k) {
		const size_t idx = order[k];
		const SCP_string fname = names[idx] + ".cache";

		if (cf_delete(fname.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS)) {
			total -= info[idx].size;
		}
	}
}