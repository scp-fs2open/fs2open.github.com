#include "model/modelbvh_extract.h"

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
		bvh_uv uv0{tree->vert_list[leaf.vert_start].u, tree->vert_list[leaf.vert_start].v};

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
			// The source leaf's index at extraction time -- not poly_index (a skip-compacted
			// counter with different semantics). Not used by the live collision path (tmap_num is
			// already copied onto the triangle above), kept for build-time traceability/debugging.
			tri.leaf_index = li;
			tri.uv0 = uv0;
			tri.uv1 = {tree->vert_list[leaf.vert_start + i].u, tree->vert_list[leaf.vert_start + i].v};
			tri.uv2 = {tree->vert_list[leaf.vert_start + i + 1].u, tree->vert_list[leaf.vert_start + i + 1].v};
			triangles.push_back(tri);
		}

		++poly_index;
	}

	return triangles;
}
