#pragma once

#include "globalincs/pstypes.h"

namespace table_viewer {

SCP_string get_table_entry_text(const char* table_filename,
	const char* modular_pattern,
	const char* entry_name,
	const char* missing_table_message = nullptr,
	const char* entry_prefix = "$Name:");

SCP_string get_complete_table_text(const char* table_filename,
	const char* modular_pattern,
	const char* missing_table_message = nullptr);

} // namespace table_viewer
