#pragma once

#include "globalincs/pstypes.h"

SCP_string shader_load_source(const SCP_string& filename);
SCP_string shader_preprocess_includes(const SCP_string& filename, const SCP_string& source);
SCP_string shader_preprocess_defines(const SCP_string& filename, const SCP_string& source);
