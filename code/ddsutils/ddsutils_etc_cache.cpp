#include "ddsutils/ddsutils_etc_cache.h"
#include "cfile/cfile.h"

#include <md5.h>

#include <cstring>

#define ETC2_CACHE_VERSION_TAG "etc2-v1"
#define ETC2_CACHE_MAGIC 0x32435445u
#define ETC2_CACHE_VERSION 1u
#define ETC2_CACHE_CHKSUM_WINDOW 65536

static const uint32_t CACHE_LOCATION_FLAGS = CF_LOCATION_ROOT_USER | CF_LOCATION_TYPE_ROOT;

static SCP_string etc2_cache_filename(const SCP_string &key)
{
	return SCP_string("etc2-") + key + ".cache";
}

static SCP_string etc2_cache_temp_filename(const SCP_string &key)
{
	return SCP_string("etc2-") + key + ".cache.tmp";
}

SCP_string etc2_cache_make_key(CFILE *dds, uint src_fourcc, int dst_format, uint max_size)
{
	const int saved_pos = cftell(dds);

	cfseek(dds, 0, CF_SEEK_SET);

	uint src_crc = 0;
	cf_chksum_long(dds, &src_crc, ETC2_CACHE_CHKSUM_WINDOW);

	const uint src_len = (uint)cfilelength(dds);

	cfseek(dds, saved_pos, CF_SEEK_SET);

	const char* name = cf_get_filename(dds);
	if (name == nullptr) {
		name = "";
	}

	MD5 md5;
	md5.update(ETC2_CACHE_VERSION_TAG, (MD5::size_type)strlen(ETC2_CACHE_VERSION_TAG));
	md5.update(name, (MD5::size_type)strlen(name));
	md5.update(reinterpret_cast<const char*>(&src_crc), sizeof(src_crc));
	md5.update(reinterpret_cast<const char*>(&src_len), sizeof(src_len));
	md5.update(reinterpret_cast<const char*>(&src_fourcc), sizeof(src_fourcc));
	md5.update(reinterpret_cast<const char*>(&dst_format), sizeof(dst_format));
	md5.update(reinterpret_cast<const char*>(&max_size), sizeof(max_size));
	md5.finalize();

	return md5.hexdigest();
}

bool etc2_cache_try_load(const SCP_string &key,
                         uint expected_width,
                         uint expected_height,
                         uint expected_mips,
                         int expected_format,
                         size_t expected_size,
                         ubyte *out_data)
{
	const SCP_string name = etc2_cache_filename(key);

	CFILE *fp = cfopen(name.c_str(), "rb", CF_TYPE_CACHE, false, CACHE_LOCATION_FLAGS);

	if (fp == nullptr) {
		return false;
	}

	const uint magic = cfread_uint(fp);
	const uint version = cfread_uint(fp);
	const uint format = cfread_uint(fp);
	const uint width = cfread_uint(fp);
	const uint height = cfread_uint(fp);
	const uint mips = cfread_uint(fp);
	const uint payload_size = cfread_uint(fp);
	const uint payload_crc = cfread_uint(fp);

	const bool header_ok = (magic == ETC2_CACHE_MAGIC) &&
	                       (version == ETC2_CACHE_VERSION) &&
	                       (format == (uint)expected_format) &&
	                       (width == expected_width) &&
	                       (height == expected_height) &&
	                       (mips == expected_mips) &&
	                       (payload_size == (uint)expected_size);

	if (!header_ok) {
		cfclose(fp);
		return false;
	}

	const int got = cfread(out_data, 1, (int)payload_size, fp);

	cfclose(fp);

	if (got != (int)payload_size) {
		cf_delete(name.c_str(), CF_TYPE_CACHE);
		return false;
	}

	const uint crc = cf_add_chksum_long(0, out_data, payload_size);

	if (crc != payload_crc) {
		cf_delete(name.c_str(), CF_TYPE_CACHE);
		return false;
	}

	return true;
}

void etc2_cache_store(const SCP_string &key,
                      uint width,
                      uint height,
                      uint mips,
                      int format,
                      const ubyte *payload,
                      size_t payload_size)
{
	if (payload == nullptr || payload_size == 0) {
		return;
	}

	const SCP_string temp_name = etc2_cache_temp_filename(key);
	const SCP_string final_name = etc2_cache_filename(key);

	CFILE *fp = cfopen(temp_name.c_str(), "wb", CF_TYPE_CACHE, false, CACHE_LOCATION_FLAGS);

	if (fp == nullptr) {
		return;
	}

	const uint payload_crc = cf_add_chksum_long(0, const_cast<ubyte *>(payload), payload_size);

	cfwrite_uint(ETC2_CACHE_MAGIC, fp);
	cfwrite_uint(ETC2_CACHE_VERSION, fp);
	cfwrite_uint((uint)format, fp);
	cfwrite_uint(width, fp);
	cfwrite_uint(height, fp);
	cfwrite_uint(mips, fp);
	cfwrite_uint((uint)payload_size, fp);
	cfwrite_uint(payload_crc, fp);

	const int written = cfwrite(payload, 1, (int)payload_size, fp);

	cfclose(fp);

	if (written != (int)payload_size) {
		cf_delete(temp_name.c_str(), CF_TYPE_CACHE);
		return;
	}

	cf_rename(temp_name.c_str(), final_name.c_str(), CF_TYPE_CACHE, CACHE_LOCATION_FLAGS);
}
