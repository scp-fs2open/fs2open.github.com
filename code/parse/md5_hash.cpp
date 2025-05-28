#include "md5_hash.h"

#include "globalincs/vmallocator.h"

#include "md5.h"

SCP_string md5_hash(const char* text, size_t length)
{
	MD5 md5;
	md5.update(text, static_cast<MD5::size_type>(length));
	md5.finalize();
	return md5.hexdigest();
}

SCP_string md5_hash(const SCP_string& text)
{
	return md5_hash(text.c_str(), text.size());
}
