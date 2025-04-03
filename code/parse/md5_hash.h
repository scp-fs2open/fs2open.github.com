#pragma once

#include "globalincs/vmallocator.h"

// Hashes a block of text as raw bytes
SCP_string md5_hash(const char* text, size_t length);

// Hashes an SCP_string
SCP_string md5_hash(const SCP_string& text);
