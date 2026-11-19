#pragma once

#include "globalincs/pstypes.h"

struct ModBootstrap {
	int version = 0;
	SCP_string mod_id;
	SCP_string workfolder;
	SCP_vector<SCP_string> roots;
};

const char* cmdline_bootstrap_path();
void mod_bootstrap_init(const char* filename);
const ModBootstrap* mod_bootstrap_get();
