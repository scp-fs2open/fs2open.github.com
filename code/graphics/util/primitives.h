#pragma once

#include "globalincs/pstypes.h"

namespace graphics {
namespace util {

struct generated_mesh {
	SCP_vector<float> vertices;   // position-only, 3 floats per vertex
	SCP_vector<ushort> indices;
	unsigned int vertex_count;    // number of unique vertices generated
	unsigned int index_count;     // number of indices
};

/**
 * @brief Generate a unit sphere mesh (radius 1.0) suitable for deferred light volumes
 *
 * Based on http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
 *
 * @param rings Number of horizontal rings
 * @param segments Number of vertical segments
 * @return generated_mesh containing position-only vertices and triangle indices
 */
generated_mesh generate_sphere_mesh(int rings, int segments);

/**
 * @brief Generate a unit cylinder mesh (radius 1.0, height 1.0) suitable for deferred light volumes
 *
 * Based on http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
 *
 * @param segments Number of radial segments
 * @return generated_mesh containing position-only vertices and triangle indices
 */
generated_mesh generate_cylinder_mesh(int segments);

}
}
