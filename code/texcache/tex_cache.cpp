#include "texcache/tex_cache.h"
#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "options/Option.h"
#include "cmdline/cmdline.h"

#include "lz4.h"
#include <md5.h>

#include <cstring>
#include <algorithm>
#include <filesystem>
#include <limits>

// Includes a fingerprint of the bundled etcpak sources. Update it when the encoder changes.
#define TEX_CACHE_VERSION_TAG "tex-v1-etcpak-c779f41fd9eb"
static constexpr uint TEX_CACHE_MAGIC = 0x32435445u;
static constexpr uint TEX_CACHE_VERSION = 1u;
static constexpr uint TEX_CACHE_FLAG_LZ4 = 0x1u;
static constexpr int TEX_CACHE_CHKSUM_WINDOW = 65536;

//Option values
float tex_cache_size = 3.0f; // Default value
static constexpr float TEX_CACHE_OPTION_MAX_SIZE = 30.0f; // Unlimited cache
static constexpr float TEX_CACHE_OPTION_MIN_SIZE = 0.0f;  // Disabled cache
//

static const uint32_t CACHE_LOCATION_FLAGS = CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT;

static SCP_string tex_cache_filename(const SCP_string &key)
{
	return SCP_string("tex-") + key + ".cache";
}

static SCP_string tex_cache_temp_filename(const SCP_string &key)
{
	return SCP_string("tex-") + key + ".cache.tmp";
}

bool tex_cache_is_enabled()
{
	return tex_cache_size > TEX_CACHE_OPTION_MIN_SIZE && !Cmdline_no_transcode_cache;
}

SCP_string tex_cache_make_key(CFILE *dds, const char *source_name, tex_cache_type cache_type, uint src_fourcc, int dst_format, uint tex_size)
{
	const char* name = source_name;
	if (name == nullptr) {
		name = "";
	}

	const auto location = *name != '\0' ? cf_find_file_location(name, cf_get_dir_type(dds)) : CFileLocation();
	const auto src_mtime = static_cast<int64_t>(location.m_time);
	const int saved_pos = cftell(dds);

	cfseek(dds, 0, CF_SEEK_SET);

	uint src_crc = 0;
	// Fall back to a full checksum if the source metadata cannot be resolved.
	cf_chksum_long(dds, &src_crc, location.found ? TEX_CACHE_CHKSUM_WINDOW : -1);

	const uint src_len = (uint)cfilelength(dds);

	cfseek(dds, saved_pos, CF_SEEK_SET);

	MD5 md5;
	md5.update(TEX_CACHE_VERSION_TAG, (MD5::size_type)strlen(TEX_CACHE_VERSION_TAG));
	md5.update(reinterpret_cast<const char*>(&cache_type), sizeof(cache_type));
	md5.update(name, (MD5::size_type)strlen(name));
	md5.update(reinterpret_cast<const char*>(&src_mtime), sizeof(src_mtime));
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
	if (!tex_cache_is_enabled())
		return false;

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
	                       (payload_size == expected_size) &&
	                       (payload_size > 0) &&
	                       (payload_size <= static_cast<uint>(std::numeric_limits<int>::max())) &&
	                       (stored_size > 0) &&
	                       (stored_size <= payload_size) &&
	                       (stored_size == static_cast<uint>(cfilelength(fp) - cftell(fp)));

	if (!header_ok) {
		cfclose(fp);
		cf_delete(name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
		return false;
	}

	SCP_vector<ubyte> stored(stored_size);
	const int got = cfread(stored.data(), 1, (int)stored_size, fp);

	if (cfclose(fp) != 0 || got != (int)stored_size) {
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

	// Pruning uses mtime as the last-use time. Only touch fully validated, loose cache files.
	const auto location = cf_find_file_location(name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
	if (location.found && location.offset == 0 && location.data_ptr == nullptr) {
		std::error_code error;
		std::filesystem::last_write_time(std::filesystem::u8path(location.full_name.c_str()),
			std::filesystem::file_time_type::clock::now(), error);
	}

	return true;
}

void tex_cache_store(const SCP_string &key, uint width, uint height, uint mips, int format, const ubyte *payload, size_t payload_size)
{
	if (!tex_cache_is_enabled())
		return;

	if (payload == nullptr || payload_size == 0 || payload_size > static_cast<size_t>(std::numeric_limits<int>::max())) {
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

	const uint header[] = {TEX_CACHE_MAGIC, TEX_CACHE_VERSION, flags, static_cast<uint>(format), width,
		height, mips, static_cast<uint>(payload_size), static_cast<uint>(out_size), payload_crc};
	bool write_ok = true;
	for (const auto field : header) {
		if (cfwrite_uint(field, fp) != 1) {
			write_ok = false;
			break;
		}
	}
	write_ok = write_ok && cfwrite(out_buf, 1, static_cast<int>(out_size), fp) == static_cast<int>(out_size);
	const int close_result = cfclose(fp);
	if (!write_ok || close_result != 0 ||
		cf_rename(temp_name.c_str(), final_name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS) != CF_RENAME_SUCCESS) {
		cf_delete(temp_name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
	}
}

auto CacheSizeLimit = options::OptionBuilder<float>("Graphics.TexCacheSize",
	std::pair<const char*, int>{"Texture Transcode Cache", 1942},
	std::pair<const char*, int>{"Texture transcode cache size. Mostly relevant for mobile GPUs lacking S3TC / BC7 support.", 1943})
	.category(std::make_pair("Graphics", 1825))
	.level(options::ExpertLevel::Advanced)
	.range(TEX_CACHE_OPTION_MIN_SIZE, TEX_CACHE_OPTION_MAX_SIZE)
	.default_func([]() { return tex_cache_size; })
	.display([](float value) {
		if (Cmdline_no_transcode_cache || value <= TEX_CACHE_OPTION_MIN_SIZE)
			return SCP_string(XSTR("Disabled", 413));
		if (value >= TEX_CACHE_OPTION_MAX_SIZE)
			return SCP_string(XSTR("Unlimited", 1944));
		SCP_string out;
		sprintf(out, "%.1f GB", value);
		return out;
	})
	.parser([]() {
		float value;
		stuff_float(&value);
		tex_cache_size = std::clamp(value, TEX_CACHE_OPTION_MIN_SIZE, TEX_CACHE_OPTION_MAX_SIZE);
	})
	.bind_to(&tex_cache_size)
	.importance(0)
	.finish();

void tex_cache_prune()
{
	// List the same loose files that can be removed from the writable cache directory.
	SCP_string cache_dir;
	if (!cf_create_default_path_string(cache_dir, CF_TYPE_CACHE, nullptr, CACHE_LOCATION_FLAGS)) {
		return;
	}

	size_t removed_files = 0;
	uint64_t removed_bytes = 0;
	SCP_vector<SCP_string> names;
	SCP_vector<file_list_info> info;

	const auto delete_entry = [&](const SCP_string& name, size_t size) {
		if (!cf_delete(name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS)) {
			return false;
		}
		++removed_files;
		removed_bytes += size;
		return true;
	};

	const SCP_string temp_pattern = cache_dir + "tex-*.cache.tmp";
	cf_get_file_list(names, CF_TYPE_CACHE, temp_pattern.c_str(), CF_SORT_NONE, &info);
	for (size_t i = 0; i < names.size(); ++i) {
		delete_entry(names[i] + ".tmp", info[i].size);
	}

	if (tex_cache_size < TEX_CACHE_OPTION_MAX_SIZE) {
		const auto max_bytes = static_cast<uint64_t>(std::max(0.0f, tex_cache_size) * 1'000'000'000.0);
		names.clear();
		info.clear();
		const SCP_string cache_pattern = cache_dir + "tex-*.cache";
		cf_get_file_list(names, CF_TYPE_CACHE, cache_pattern.c_str(), CF_SORT_NONE, &info);

		uint64_t total = 0;
		for (const auto& entry : info) {
			total += entry.size;
		}
		const bool clear_cache = max_bytes == 0;
		if (clear_cache || total > max_bytes) {
			// Trim to 80% of the cap, or clear all entries when the cap is zero.
			const uint64_t target = (max_bytes / 5) * 4;
			SCP_vector<size_t> order(names.size());
			for (size_t i = 0; i < order.size(); ++i) {
				order[i] = i;
			}
			std::sort(order.begin(), order.end(), [&info](size_t a, size_t b) {
				return info[a].write_time < info[b].write_time;
			});
			for (const auto idx : order) {
				if (!clear_cache && total <= target) {
					break;
				}
				if (delete_entry(names[idx] + ".cache", info[idx].size)) {
					total -= info[idx].size;
				}
			}
		}
	}

	if (removed_files > 0) {
		mprintf(("Texture cache: pruned " SIZE_T_ARG " files (" UINT64_T_ARG " bytes)\n", removed_files, removed_bytes));
	}
}

void tex_cache_disable_option()
{
	options::OptionsManager::instance()->setOverride("Graphics.TexCacheSize",
		std::unique_ptr<json_t>(json_real(tex_cache_size)), "-no_transcode_cache");
}
