#ifndef _TEX_CACHE_H
#define _TEX_CACHE_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

enum tex_cache_type {
	TEX_CACHE_TYPE_ETC2 = 1000,
	TEX_CACHE_TYPE_BC
};

// Clear the transcode cache if the total file size is over the limit
void tex_cache_prune();

// Get the MD5 key hash for the dds texture
SCP_string tex_cache_make_key(CFILE *dds, tex_cache_type cache_type, uint src_fourcc, int dst_format, uint tex_size);

// Load a transcoded texture from cache if present
bool tex_cache_try_load(const SCP_string &key, uint expected_width, uint expected_height, uint expected_mips, int expected_format, size_t expected_size, ubyte *out_data);

// Save a texture to cache for later use
void tex_cache_store(const SCP_string &key, uint width, uint height, uint mips, int format, const ubyte *payload, size_t payload_size);

#endif
