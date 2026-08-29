#include "model/modelbvh.h"

#include "math/vecmat.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace {

vec3d make_vec3d(float x, float y, float z)
{
	vec3d v;
	v.xyz.x = x;
	v.xyz.y = y;
	v.xyz.z = z;
	return v;
}

struct AABB {
	vec3d bmin = make_vec3d(FLT_MAX, FLT_MAX, FLT_MAX);
	vec3d bmax = make_vec3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	void grow(const vec3d& p)
	{
		bmin.xyz.x = std::min(bmin.xyz.x, p.xyz.x);
		bmin.xyz.y = std::min(bmin.xyz.y, p.xyz.y);
		bmin.xyz.z = std::min(bmin.xyz.z, p.xyz.z);
		bmax.xyz.x = std::max(bmax.xyz.x, p.xyz.x);
		bmax.xyz.y = std::max(bmax.xyz.y, p.xyz.y);
		bmax.xyz.z = std::max(bmax.xyz.z, p.xyz.z);
	}

	void grow(const AABB& other)
	{
		// An empty/never-grown AABB is still bmin=+FLT_MAX/bmax=-FLT_MAX (see default member
		// initializers above); growing through it unconditionally would poison this box out to
		// those sentinel extremes. Bins with zero triangles hit this during the SAH prefix-sum
		// sweep in build_range() below.
		if (!other.valid())
			return;
		grow(other.bmin);
		grow(other.bmax);
	}

	bool valid() const { return bmin.xyz.x <= bmax.xyz.x; }

	float surface_area() const
	{
		if (!valid())
			return 0.0f;
		float dx = bmax.xyz.x - bmin.xyz.x;
		float dy = bmax.xyz.y - bmin.xyz.y;
		float dz = bmax.xyz.z - bmin.xyz.z;
		return 2.0f * (dx * dy + dy * dz + dz * dx);
	}
};

AABB triangle_bounds(const bvh_triangle& tri)
{
	AABB b;
	b.grow(tri.v0);
	b.grow(tri.v1);
	b.grow(tri.v2);
	return b;
}

vec3d triangle_centroid(const bvh_triangle& tri)
{
	vec3d c = tri.v0 + tri.v1 + tri.v2;
	return c * (1.0f / 3.0f);
}

// Scratch build node for the binary SAH build. Kept separate from the public bvh_node (SoA,
// BVH_N-wide) type -- this is a plain binary tree, in arbitrary (build-order) array order.
struct BinBuildNode {
	AABB bounds;
	int start = 0, count = 0; // valid when left == -1 (leaf)
	int left = -1, right = -1; // indices into the bin_nodes array; -1 == leaf
};

constexpr int LEAF_THRESHOLD = 2;
constexpr int NUM_BINS = 16;

// Recursively builds a binary SAH tree over indices[start, start+count), partitioning `indices`
// in place. Returns the index of the created node in `bin_nodes`.
int build_range(const SCP_vector<AABB>& tri_bounds, const SCP_vector<vec3d>& tri_centroid, SCP_vector<int>& indices,
	SCP_vector<BinBuildNode>& bin_nodes, int start, int count)
{
	AABB bounds;
	AABB cbounds; // bounds of centroids, used to choose the split axis/position
	for (int i = start; i < start + count; ++i) {
		bounds.grow(tri_bounds[indices[i]]);
		cbounds.grow(tri_centroid[indices[i]]);
	}

	auto make_leaf = [&]() {
		BinBuildNode node;
		node.bounds = bounds;
		node.start = start;
		node.count = count;
		bin_nodes.push_back(node);
		return static_cast<int>(bin_nodes.size()) - 1;
	};

	if (count <= LEAF_THRESHOLD)
		return make_leaf();

	float cextent[3] = {cbounds.bmax.xyz.x - cbounds.bmin.xyz.x, cbounds.bmax.xyz.y - cbounds.bmin.xyz.y,
		cbounds.bmax.xyz.z - cbounds.bmin.xyz.z};
	int axis = 0;
	if (cextent[1] > cextent[axis])
		axis = 1;
	if (cextent[2] > cextent[axis])
		axis = 2;

	if (cextent[axis] <= 1e-8f)
		// All triangle centroids coincide (or are degenerate on every axis) -- no split can help.
		return make_leaf();

	float cmin = cbounds.bmin.a1d[axis];
	float cmax = cbounds.bmax.a1d[axis];

	// Bin triangles by centroid position along the chosen axis.
	int bin_count[NUM_BINS] = {};
	AABB bin_bounds[NUM_BINS];
	for (int i = start; i < start + count; ++i) {
		float t = (tri_centroid[indices[i]].a1d[axis] - cmin) / (cmax - cmin);
		int bin = std::min(NUM_BINS - 1, std::max(0, static_cast<int>(t * NUM_BINS)));
		bin_count[bin]++;
		bin_bounds[bin].grow(tri_bounds[indices[i]]);
	}

	// Prefix sums from the left and right to evaluate the NUM_BINS-1 split-plane candidates.
	AABB left_bounds[NUM_BINS], right_bounds[NUM_BINS];
	int left_count[NUM_BINS], right_count[NUM_BINS];
	AABB running;
	int running_count = 0;
	for (int b = 0; b < NUM_BINS; ++b) {
		running.grow(bin_bounds[b]);
		running_count += bin_count[b];
		left_bounds[b] = running;
		left_count[b] = running_count;
	}
	running = AABB();
	running_count = 0;
	for (int b = NUM_BINS - 1; b >= 0; --b) {
		running.grow(bin_bounds[b]);
		running_count += bin_count[b];
		right_bounds[b] = running;
		right_count[b] = running_count;
	}

	float leaf_cost = static_cast<float>(count) * bounds.surface_area();
	float best_cost = leaf_cost;
	int best_split = -1; // split after bin `best_split` (bins [0,best_split] go left)
	for (int split = 0; split < NUM_BINS - 1; ++split) {
		int lc = left_count[split];
		int rc = right_count[split + 1];
		if (lc == 0 || rc == 0)
			continue;
		float cost = static_cast<float>(lc) * left_bounds[split].surface_area() +
			static_cast<float>(rc) * right_bounds[split + 1].surface_area();
		if (cost < best_cost) {
			best_cost = cost;
			best_split = split;
		}
	}

	if (best_split < 0)
		return make_leaf();

	float split_pos = cmin + (static_cast<float>(best_split + 1) / static_cast<float>(NUM_BINS)) * (cmax - cmin);

	auto begin = indices.begin() + start;
	auto end = indices.begin() + start + count;
	auto mid = std::partition(begin, end, [&](int idx) { return tri_centroid[idx].a1d[axis] < split_pos; });
	int left_n = static_cast<int>(mid - begin);

	if (left_n == 0 || left_n == count) {
		// Binned SAH degenerated (can happen with many coincident/near-coincident centroids);
		// fall back to an unconditional median split so recursion always makes progress.
		std::nth_element(begin, begin + count / 2, end,
			[&](int a, int b) { return tri_centroid[a].a1d[axis] < tri_centroid[b].a1d[axis]; });
		left_n = count / 2;
	}

	int left = build_range(tri_bounds, tri_centroid, indices, bin_nodes, start, left_n);
	int right = build_range(tri_bounds, tri_centroid, indices, bin_nodes, start + left_n, count - left_n);

	BinBuildNode node;
	node.bounds = bounds;
	node.left = left;
	node.right = right;
	bin_nodes.push_back(node);
	return static_cast<int>(bin_nodes.size()) - 1;
}

// One resolved child slot for a to-be-emitted BVH_N-wide node: either a leaf range or a pointer
// to a BinBuildNode subtree that still needs its own N-wide children resolved.
struct CollapsedChild {
	bool is_leaf = false;
	AABB bounds;
	// leaf:
	int start = 0, count = 0;
	// internal:
	int bin_node = -1;
};

// Greedily expands `children` (starting from node's two binary children) up to BVH_N entries,
// each iteration expanding whichever internal entry has the largest bounding-box surface area
// (a cheap proxy for "most worth splitting further" -- equivalent in spirit to always resolving
// the highest-SAH-cost collapse first).
void resolve_children(const SCP_vector<BinBuildNode>& bin_nodes, int bin_node_index, SCP_vector<CollapsedChild>& children)
{
	const BinBuildNode& n = bin_nodes[bin_node_index];
	auto to_child = [&](int idx) {
		const BinBuildNode& c = bin_nodes[idx];
		CollapsedChild cc;
		cc.bounds = c.bounds;
		if (c.left < 0) {
			cc.is_leaf = true;
			cc.start = c.start;
			cc.count = c.count;
		} else {
			cc.is_leaf = false;
			cc.bin_node = idx;
		}
		return cc;
	};

	children.push_back(to_child(n.left));
	children.push_back(to_child(n.right));

	for (;;) {
		if (static_cast<int>(children.size()) >= BVH_N)
			break;

		int best = -1;
		float best_area = -1.0f;
		for (int i = 0; i < static_cast<int>(children.size()); ++i) {
			if (children[i].is_leaf)
				continue;
			float area = children[i].bounds.surface_area();
			if (area > best_area) {
				best_area = area;
				best = i;
			}
		}
		if (best < 0)
			break; // nothing left that can be expanded further

		int expand_bin_node = children[best].bin_node;
		children.erase(children.begin() + best);
		const BinBuildNode& expand = bin_nodes[expand_bin_node];
		children.push_back(to_child(expand.left));
		children.push_back(to_child(expand.right));
	}
}

// Recursively resolves `bin_node_index`'s BVH_N-wide children and emits them (and their
// subtrees) into `out_nodes` in depth-first/pre-order: a node's own slot is reserved and its
// subtree is appended immediately after it, before any sibling subtree.
int emit_node(const SCP_vector<BinBuildNode>& bin_nodes, int bin_node_index, SCP_vector<bvh_node>& out_nodes)
{
	SCP_vector<CollapsedChild> children;
	const BinBuildNode& self = bin_nodes[bin_node_index];
	if (self.left < 0) {
		// The whole (sub)tree is a single leaf (e.g. total triangle count <= LEAF_THRESHOLD) --
		// resolve_children assumes two binary children to start from, which a leaf doesn't have.
		// Wrap it as a single-slot node instead.
		CollapsedChild cc;
		cc.is_leaf = true;
		cc.bounds = self.bounds;
		cc.start = self.start;
		cc.count = self.count;
		children.push_back(cc);
	} else {
		resolve_children(bin_nodes, bin_node_index, children);
	}

	int idx = static_cast<int>(out_nodes.size());
	out_nodes.push_back(bvh_node{});

	// Pad remaining slots with the sentinel (impossible box) so unused slots fail the SIMD test
	// for free and are never mistaken for a real child.
	for (int i = 0; i < BVH_N; ++i) {
		out_nodes[idx].minx[i] = out_nodes[idx].miny[i] = out_nodes[idx].minz[i] = FLT_MAX;
		out_nodes[idx].maxx[i] = out_nodes[idx].maxy[i] = out_nodes[idx].maxz[i] = -FLT_MAX;
		out_nodes[idx].count[i] = 0;
		out_nodes[idx].child[i] = -1;
	}

	for (int i = 0; i < static_cast<int>(children.size()); ++i) {
		const CollapsedChild& c = children[i];
		out_nodes[idx].minx[i] = c.bounds.bmin.xyz.x;
		out_nodes[idx].miny[i] = c.bounds.bmin.xyz.y;
		out_nodes[idx].minz[i] = c.bounds.bmin.xyz.z;
		out_nodes[idx].maxx[i] = c.bounds.bmax.xyz.x;
		out_nodes[idx].maxy[i] = c.bounds.bmax.xyz.y;
		out_nodes[idx].maxz[i] = c.bounds.bmax.xyz.z;

		if (c.is_leaf) {
			out_nodes[idx].count[i] = c.count;
			out_nodes[idx].child[i] = c.start;
		} else {
			// Recurse first (appends the child subtree contiguously), then backfill -- `idx`'s
			// slot in out_nodes is already fixed by index and safe to write after growth since
			// we index by value (idx), never hold a reference/pointer across this call.
			int child_idx = emit_node(bin_nodes, c.bin_node, out_nodes);
			out_nodes[idx].count[i] = 0;
			out_nodes[idx].child[i] = child_idx;
		}
	}

	return idx;
}

bool ray_aabb(const vec3d& origin, const vec3d& inv_dir, const float bmin[3], const float bmax[3], float t_max, float& out_tmin)
{
	float o[3] = {origin.xyz.x, origin.xyz.y, origin.xyz.z};
	float id[3] = {inv_dir.xyz.x, inv_dir.xyz.y, inv_dir.xyz.z};

	float tmin = 0.0f;
	float tmax = t_max;
	for (int axis = 0; axis < 3; ++axis) {
		float t0 = (bmin[axis] - o[axis]) * id[axis];
		float t1 = (bmax[axis] - o[axis]) * id[axis];
		if (t0 > t1)
			std::swap(t0, t1);
		tmin = std::max(tmin, t0);
		tmax = std::min(tmax, t1);
		if (tmin > tmax)
			return false;
	}
	out_tmin = tmin;
	return true;
}

// Standard Moller-Trumbore ray-triangle test.
bool ray_triangle(const vec3d& origin, const vec3d& dir, const bvh_triangle& tri, float& out_t)
{
	constexpr float EPS = 1e-8f;
	vec3d e1 = tri.v1 - tri.v0;
	vec3d e2 = tri.v2 - tri.v0;

	vec3d pvec;
	vm_vec_cross(&pvec, &dir, &e2);
	float det = vm_vec_dot(&e1, &pvec);
	if (std::fabs(det) < EPS)
		return false;
	float inv_det = 1.0f / det;

	vec3d tvec = origin - tri.v0;
	float u = vm_vec_dot(&tvec, &pvec) * inv_det;
	if (u < 0.0f || u > 1.0f)
		return false;

	vec3d qvec;
	vm_vec_cross(&qvec, &tvec, &e1);
	float v = vm_vec_dot(&dir, &qvec) * inv_det;
	if (v < 0.0f || u + v > 1.0f)
		return false;

	float t = vm_vec_dot(&e2, &qvec) * inv_det;
	if (t < 0.0f)
		return false;

	out_t = t;
	return true;
}

} // namespace

bvh_tree bvh_build(SCP_vector<bvh_triangle> triangles)
{
	bvh_tree tree;
	int n = static_cast<int>(triangles.size());
	if (n == 0)
		return tree;

	SCP_vector<AABB> tri_bounds(n);
	SCP_vector<vec3d> tri_centroid(n);
	for (int i = 0; i < n; ++i) {
		tri_bounds[i] = triangle_bounds(triangles[i]);
		tri_centroid[i] = triangle_centroid(triangles[i]);
	}

	SCP_vector<int> indices(n);
	for (int i = 0; i < n; ++i)
		indices[i] = i;

	SCP_vector<BinBuildNode> bin_nodes;
	int root_bin = build_range(tri_bounds, tri_centroid, indices, bin_nodes, 0, n);

	tree.root = emit_node(bin_nodes, root_bin, tree.nodes);

	tree.triangles.reserve(n);
	for (int i = 0; i < n; ++i)
		tree.triangles.push_back(triangles[indices[i]]);

	return tree;
}

bool bvh_ray_intersect(const bvh_tree& tree, const vec3d& origin, const vec3d& dir, float& out_t, int& out_triangle_index)
{
	if (tree.nodes.empty())
		return false;

	vec3d inv_dir = make_vec3d(dir.xyz.x != 0.0f ? 1.0f / dir.xyz.x : FLT_MAX, dir.xyz.y != 0.0f ? 1.0f / dir.xyz.y : FLT_MAX,
		dir.xyz.z != 0.0f ? 1.0f / dir.xyz.z : FLT_MAX);

	bool found = false;
	float best_t = FLT_MAX;
	int best_tri = -1;

	// Simple explicit-stack traversal; not optimized (no front-to-back ordering) -- this only
	// needs to be correct, since it exists to validate the build in tests.
	SCP_vector<int> stack;
	stack.push_back(tree.root);
	while (!stack.empty()) {
		int node_idx = stack.back();
		stack.pop_back();
		const bvh_node& node = tree.nodes[node_idx];

		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] < 0)
				continue;
			float bmin[3] = {node.minx[i], node.miny[i], node.minz[i]};
			float bmax[3] = {node.maxx[i], node.maxy[i], node.maxz[i]};
			float t_hit;
			if (!ray_aabb(origin, inv_dir, bmin, bmax, best_t, t_hit))
				continue;

			if (node.count[i] > 0) {
				int start = node.child[i];
				int count = node.count[i];
				for (int t = start; t < start + count; ++t) {
					float hit_t;
					if (ray_triangle(origin, dir, tree.triangles[t], hit_t) && hit_t < best_t) {
						best_t = hit_t;
						best_tri = t;
						found = true;
					}
				}
			} else {
				stack.push_back(node.child[i]);
			}
		}
	}

	if (found) {
		out_t = best_t;
		out_triangle_index = best_tri;
	}
	return found;
}
