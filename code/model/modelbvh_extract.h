#pragma once

#include "model/model.h"
#include "model/modelbvh.h"

// Bridges the loaded-model world (polymodel/bsp_collision_tree) and the standalone modelbvh
// module. Deliberately kept separate from modelbvh.h/.cpp, which have zero dependency on POF/
// model types by design (see modelbvh.h) -- this is the one place that's allowed to depend on
// both.

// Extracts a flat, fan-triangulated triangle soup for one submodel, suitable for bvh_build().
// Reads from the submodel's already-parsed bsp_collision_tree (built by model_load()) rather than
// re-walking the raw BSP opcode stream, so it reuses exactly the same polygon/vertex data the
// existing BSP collision tree is built from. Returns an empty vector if the submodel has no
// collision tree or no geometry. Triangle coordinates are in the submodel's local space, matching
// bsp_collision_tree's own space.
SCP_vector<bvh_triangle> model_bvh_extract_submodel_triangles(polymodel* pm, int submodel_num);
