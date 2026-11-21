#pragma once

#include "globalincs/pstypes.h"

struct ModBootstrap {
	int version = 0;
	SCP_string filename;
	SCP_string launch_directory;
	SCP_string mod_id;
	SCP_string workfolder;
	SCP_vector<SCP_string> roots;
};

const char* cmdline_bootstrap_path();
void mod_bootstrap_init(const char* filename, const char* launch_directory = nullptr);
bool mod_bootstrap_init_from_cmdline(const char* launch_directory = nullptr);
void mod_bootstrap_log();
const ModBootstrap* mod_bootstrap_get();
bool mod_bootstrap_contains_path(const SCP_string& filename);
