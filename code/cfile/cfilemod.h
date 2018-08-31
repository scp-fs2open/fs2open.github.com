#pragma once

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"

namespace cfile {

SCP_vector<ModRoot> getModRootsFromModCmdline(const SCP_string& content_root, const char* mod_cmdline);

}
