#include "model/modelbvh_extract.h"

#include <algorithm>

SCP_vector<bvh_triangle> model_bvh_extract_submodel_triangles(polymodel* pm, int submodel_num)
{
	SCP_vector<bvh_triangle> triangles;

	if (pm == nullptr || submodel_num < 0 || submodel_num >= pm->n_models)
		return triangles;

	bsp_info& sm = pm->submodel[submodel_num];
	bsp_collision_tree* tree = model_get_bsp_collision_tree(sm.collision_tree_index);

	if (tree == nullptr || tree->n_leaves <= 0 || tree->point_list == nullptr || tree->vert_list == nullptr)
		return triangles;

	int poly_index = 0;
	for (int li = 0; li < tree->n_leaves; ++li) {
		const bsp_collision_leaf& leaf = tree->leaf_list[li];
		int nv = leaf.num_verts;
		if (nv < 3)
			continue;

		const vec3d& v0 = tree->point_list[tree->vert_list[leaf.vert_start].vertnum];

		// Fan triangulation, pivot = first vertex, matching the render path's
		// bsp_polygon_data::generate_triangles convention (see modelbvh.h/stage-1 notes).
		for (int i = 1; i < nv - 1; ++i) {
			const vec3d& vi = tree->point_list[tree->vert_list[leaf.vert_start + i].vertnum];
			const vec3d& vi1 = tree->point_list[tree->vert_list[leaf.vert_start + i + 1].vertnum];

			bvh_triangle tri;
			tri.v0 = v0;
			tri.v1 = vi;
			tri.v2 = vi1;
			tri.tmap_num = leaf.tmap_num;
			tri.original_index = poly_index;
			triangles.push_back(tri);
		}

		++poly_index;
	}

	return triangles;
}

SCP_vector<bvh_leaf_primitive> model_bvh_extract_leaf_primitives(polymodel* pm, int submodel_num)
{
	SCP_vector<bvh_leaf_primitive> primitives;

	if (pm == nullptr || submodel_num < 0 || submodel_num >= pm->n_models)
		return primitives;

	bsp_info& sm = pm->submodel[submodel_num];
	bsp_collision_tree* tree = model_get_bsp_collision_tree(sm.collision_tree_index);

	if (tree == nullptr || tree->n_leaves <= 0 || tree->point_list == nullptr || tree->vert_list == nullptr)
		return primitives;

	primitives.reserve(tree->n_leaves);
	for (int li = 0; li < tree->n_leaves; ++li) {
		const bsp_collision_leaf& leaf = tree->leaf_list[li];
		int nv = leaf.num_verts;
		if (nv < 3)
			continue;

		bvh_leaf_primitive prim;
		prim.bmin = tree->point_list[tree->vert_list[leaf.vert_start].vertnum];
		prim.bmax = prim.bmin;
		for (int i = 1; i < nv; ++i) {
			const vec3d& v = tree->point_list[tree->vert_list[leaf.vert_start + i].vertnum];
			prim.bmin.xyz.x = std::min(prim.bmin.xyz.x, v.xyz.x);
			prim.bmin.xyz.y = std::min(prim.bmin.xyz.y, v.xyz.y);
			prim.bmin.xyz.z = std::min(prim.bmin.xyz.z, v.xyz.z);
			prim.bmax.xyz.x = std::max(prim.bmax.xyz.x, v.xyz.x);
			prim.bmax.xyz.y = std::max(prim.bmax.xyz.y, v.xyz.y);
			prim.bmax.xyz.z = std::max(prim.bmax.xyz.z, v.xyz.z);
		}
		prim.payload = li;
		primitives.push_back(prim);
	}

	return primitives;
}
