#pragma once

#include "globalincs/pstypes.h"

class polymodel;

// Internal BSP helpers shared by the model loader and virtual POF processing.
int submodel_get_num_polys_sub(ubyte* bsp_data);
SCP_set<int> model_get_textures_used(const polymodel* pm, int submodel);