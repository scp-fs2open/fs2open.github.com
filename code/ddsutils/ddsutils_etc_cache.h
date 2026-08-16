#ifndef _DDSUTILS_ETC_CACHE_H
#define _DDSUTILS_ETC_CACHE_H

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

// Clear the transcode cache if the total file size is over the limit
void etc2_cache_prune();

// Get the MD5 file hash for the dds texture
SCP_string etc2_cache_make_key(CFILE *dds, uint src_fourcc, int dst_format, uint etc2_size);

// Load a transcoded texture from cache if present
bool etc2_cache_try_load(const SCP_string &key, uint expected_width, uint expected_height, uint expected_mips, int expected_format, size_t expected_size, ubyte *out_data);

// Save a texture to cache for later use
void etc2_cache_store(const SCP_string &key, uint width, uint height, uint mips, int format, const ubyte *payload, size_t payload_size);

#endif
