#pragma once

#include "network/multi.h"

enum class lua_net_mode : uint8_t { RELIABLE, ORDERED, UNRELIABLE };

void process_lua_packet(ubyte* data, header* hinfo);