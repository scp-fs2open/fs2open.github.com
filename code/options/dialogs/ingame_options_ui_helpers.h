#pragma once

#include "ship/ship.h"

SCP_string get_ship_table_text_opt(ship_info* sip);

SCP_string get_directory_or_vp_opt(const char* path);

bool graphics_options_changed_opt();