#include "model/modelbvh.h"

#include "math/vecmat.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <utility>

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

// Floor for the SAH build's forced leaf stop, raised from the original 2 so that leaves are usually
// already close to a clean SIMD-width batch before the padding pass in bvh_build() below rounds
// them the rest of the way up. This is a floor, not a cap: the SAH cost comparison in build_range()
// can still leave a larger leaf un-split when no candidate split beats leaf_cost, so there remains
// no true hard maximum triangle count per leaf.
//
// Deliberately NOT tied to BVH_N -- this floor controls the SAH tree's *shape* (how many triangles
// get grouped per leaf, hence traversal/leaf-visitation order), while BVH_N controls the *batch
// width* the leaf-intersection SIMD loop reads at once (see BVH_N's own doc comment in modelbvh.h).
// Currently the same value (4) but the two should be varied independently, not conflated.
constexpr int LEAF_THRESHOLD = 4;
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

// Rounds every leaf's triangle range up to a multiple of BVH_N by appending degenerate
// (zero-area, v1==v2==v0) copies of the leaf's last triangle, so a SIMD leaf-intersection pass can
// always process clean BVH_N-wide chunks with no ragged remainder. Degenerate triangles have
// det==0 in the Moller-Trumbore test (v1-v0 and v2-v0 are both zero vectors), so they can never
// register a hit -- mirrors the "impossible box" padding already used for unused bvh_node child
// slots in emit_node() above. Rewrites each leaf slot's child[]/count[] in place to point at the
// repacked arrays. Must run after emit_node() has produced tree.nodes and the tree's parallel
// triangle arrays have been populated (see bvh_build()). Operates on the tree's SoA storage
// directly -- there is no separate AoS array to keep in sync.
void pad_leaves_to_simd_width(bvh_tree& tree)
{
	SCP_vector<float> v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z;
	SCP_vector<int> tmap_num, original_index, leaf_index;
	SCP_vector<bvh_uv> uv0, uv1, uv2;

	size_t reserve_n = tree.triangle_count();
	v0x.reserve(reserve_n);
	v0y.reserve(reserve_n);
	v0z.reserve(reserve_n);
	v1x.reserve(reserve_n);
	v1y.reserve(reserve_n);
	v1z.reserve(reserve_n);
	v2x.reserve(reserve_n);
	v2y.reserve(reserve_n);
	v2z.reserve(reserve_n);
	tmap_num.reserve(reserve_n);
	original_index.reserve(reserve_n);
	leaf_index.reserve(reserve_n);
	uv0.reserve(reserve_n);
	uv1.reserve(reserve_n);
	uv2.reserve(reserve_n);

	auto append_real = [&](size_t src) {
		v0x.push_back(tree.v0x[src]);
		v0y.push_back(tree.v0y[src]);
		v0z.push_back(tree.v0z[src]);
		v1x.push_back(tree.v1x[src]);
		v1y.push_back(tree.v1y[src]);
		v1z.push_back(tree.v1z[src]);
		v2x.push_back(tree.v2x[src]);
		v2y.push_back(tree.v2y[src]);
		v2z.push_back(tree.v2z[src]);
		tmap_num.push_back(tree.tmap_num[src]);
		original_index.push_back(tree.original_index[src]);
		leaf_index.push_back(tree.leaf_index[src]);
		uv0.push_back(tree.uv0[src]);
		uv1.push_back(tree.uv1[src]);
		uv2.push_back(tree.uv2[src]);
	};

	auto append_degenerate = [&](size_t src) {
		// Same metadata as the source triangle, but v1/v2 collapsed onto v0.
		v0x.push_back(tree.v0x[src]);
		v0y.push_back(tree.v0y[src]);
		v0z.push_back(tree.v0z[src]);
		v1x.push_back(tree.v0x[src]);
		v1y.push_back(tree.v0y[src]);
		v1z.push_back(tree.v0z[src]);
		v2x.push_back(tree.v0x[src]);
		v2y.push_back(tree.v0y[src]);
		v2z.push_back(tree.v0z[src]);
		tmap_num.push_back(tree.tmap_num[src]);
		original_index.push_back(tree.original_index[src]);
		leaf_index.push_back(tree.leaf_index[src]);
		uv0.push_back(tree.uv0[src]);
		uv1.push_back(tree.uv1[src]);
		uv2.push_back(tree.uv2[src]);
	};

	for (bvh_node& node : tree.nodes) {
		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] < 0 || node.count[i] <= 0)
				continue; // internal slot, or an already-unused padding slot from emit_node()

			int start = node.child[i];
			int count = node.count[i];
			int new_start = static_cast<int>(v0x.size());

			for (int t = start; t < start + count; ++t)
				append_real(static_cast<size_t>(t));

			int padded_count = ((count + BVH_N - 1) / BVH_N) * BVH_N;
			for (int p = count; p < padded_count; ++p)
				append_degenerate(static_cast<size_t>(start + count - 1));

			node.child[i] = new_start;
			node.count[i] = padded_count;
		}
	}

	tree.v0x = std::move(v0x);
	tree.v0y = std::move(v0y);
	tree.v0z = std::move(v0z);
	tree.v1x = std::move(v1x);
	tree.v1y = std::move(v1y);
	tree.v1z = std::move(v1z);
	tree.v2x = std::move(v2x);
	tree.v2y = std::move(v2y);
	tree.v2z = std::move(v2z);
	tree.tmap_num = std::move(tmap_num);
	tree.original_index = std::move(original_index);
	tree.leaf_index = std::move(leaf_index);
	tree.uv0 = std::move(uv0);
	tree.uv1 = std::move(uv1);
	tree.uv2 = std::move(uv2);
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

	tree.v0x.reserve(n);
	tree.v0y.reserve(n);
	tree.v0z.reserve(n);
	tree.v1x.reserve(n);
	tree.v1y.reserve(n);
	tree.v1z.reserve(n);
	tree.v2x.reserve(n);
	tree.v2y.reserve(n);
	tree.v2z.reserve(n);
	tree.tmap_num.reserve(n);
	tree.original_index.reserve(n);
	tree.leaf_index.reserve(n);
	tree.uv0.reserve(n);
	tree.uv1.reserve(n);
	tree.uv2.reserve(n);
	for (int i = 0; i < n; ++i) {
		const bvh_triangle& t = triangles[indices[i]];
		tree.v0x.push_back(t.v0.xyz.x);
		tree.v0y.push_back(t.v0.xyz.y);
		tree.v0z.push_back(t.v0.xyz.z);
		tree.v1x.push_back(t.v1.xyz.x);
		tree.v1y.push_back(t.v1.xyz.y);
		tree.v1z.push_back(t.v1.xyz.z);
		tree.v2x.push_back(t.v2.xyz.x);
		tree.v2y.push_back(t.v2.xyz.y);
		tree.v2z.push_back(t.v2.xyz.z);
		tree.tmap_num.push_back(t.tmap_num);
		tree.original_index.push_back(t.original_index);
		tree.leaf_index.push_back(t.leaf_index);
		tree.uv0.push_back(t.uv0);
		tree.uv1.push_back(t.uv1);
		tree.uv2.push_back(t.uv2);
	}

	pad_leaves_to_simd_width(tree);

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
			if (!bvh_detail::ray_aabb(origin, inv_dir, bmin, bmax, best_t, t_hit))
				continue;

			if (node.count[i] > 0) {
				int start = node.child[i];
				int count = node.count[i];
				for (int t = start; t < start + count; ++t) {
					float hit_t;
					if (ray_triangle(origin, dir, tree.triangle_at(t), hit_t) && hit_t < best_t) {
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

bool ray_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& origin, const vec3d& dir,
	float best_t, float& out_t, int32_t& out_triangle_index)
{
	constexpr float EPS = 1e-8f;

	bool found = false;

	for (int32_t chunk = start; chunk < start + count; chunk += BVH_N) {
		// Structured as separate fixed-BVH_N-length arrays and per-stage loops (rather than one
		// scalar per-lane loop calling vm_vec_cross/dot) so the shape matches bvh_node's own
		// SoA-array style and gives the autovectorizer the best chance of turning this into SIMD --
		// see the design note on ray_triangle_leaf_simd() in modelbvh.h. Every lane is computed
		// unconditionally; no lane is ever skipped or early-exited the way the scalar ray_triangle()
		// does, so there's no data-dependent control flow across lanes.
		float e1x[BVH_N], e1y[BVH_N], e1z[BVH_N];
		float e2x[BVH_N], e2y[BVH_N], e2z[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			int32_t idx = chunk + i;
			e1x[i] = tree.v1x[idx] - tree.v0x[idx];
			e1y[i] = tree.v1y[idx] - tree.v0y[idx];
			e1z[i] = tree.v1z[idx] - tree.v0z[idx];
			e2x[i] = tree.v2x[idx] - tree.v0x[idx];
			e2y[i] = tree.v2y[idx] - tree.v0y[idx];
			e2z[i] = tree.v2z[idx] - tree.v0z[idx];
		}

		float pvecx[BVH_N], pvecy[BVH_N], pvecz[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			pvecx[i] = dir.xyz.y * e2z[i] - dir.xyz.z * e2y[i];
			pvecy[i] = dir.xyz.z * e2x[i] - dir.xyz.x * e2z[i];
			pvecz[i] = dir.xyz.x * e2y[i] - dir.xyz.y * e2x[i];
		}

		float det[BVH_N];
		for (int i = 0; i < BVH_N; ++i)
			det[i] = e1x[i] * pvecx[i] + e1y[i] * pvecy[i] + e1z[i] * pvecz[i];

		float inv_det[BVH_N];
		for (int i = 0; i < BVH_N; ++i)
			inv_det[i] = (std::fabs(det[i]) < EPS) ? 0.0f : 1.0f / det[i];

		float tvecx[BVH_N], tvecy[BVH_N], tvecz[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			int32_t idx = chunk + i;
			tvecx[i] = origin.xyz.x - tree.v0x[idx];
			tvecy[i] = origin.xyz.y - tree.v0y[idx];
			tvecz[i] = origin.xyz.z - tree.v0z[idx];
		}

		float u[BVH_N];
		for (int i = 0; i < BVH_N; ++i)
			u[i] = (tvecx[i] * pvecx[i] + tvecy[i] * pvecy[i] + tvecz[i] * pvecz[i]) * inv_det[i];

		float qvecx[BVH_N], qvecy[BVH_N], qvecz[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			qvecx[i] = tvecy[i] * e1z[i] - tvecz[i] * e1y[i];
			qvecy[i] = tvecz[i] * e1x[i] - tvecx[i] * e1z[i];
			qvecz[i] = tvecx[i] * e1y[i] - tvecy[i] * e1x[i];
		}

		float v[BVH_N], t[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			v[i] = (dir.xyz.x * qvecx[i] + dir.xyz.y * qvecy[i] + dir.xyz.z * qvecz[i]) * inv_det[i];
			t[i] = (e2x[i] * qvecx[i] + e2y[i] * qvecy[i] + e2z[i] * qvecz[i]) * inv_det[i];
		}

		// Same barycentric tolerance as mc_check_triangle_face()'s BARY_EPS in modelcollide.cpp
		// (u/v here are the same barycentric weights that function's own separate computation
		// derives, just via Moller-Trumbore instead of a post-hoc vp0/e1/e2 solve). Needed so a ray
		// landing exactly on the shared edge between two adjacent fan triangles doesn't fail *both*
		// triangles' containment test here (each seeing itself as juuust outside by floating-point
		// noise) while the scalar mc_check_triangle_face() reference -- which only ever runs on
		// this function's own reported candidate -- would have accepted it. A zero-tolerance test
		// here would silently drop real hits at those boundaries.
		constexpr float BARY_EPS = 1e-4f;
		bool valid[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			valid[i] = std::fabs(det[i]) >= EPS && u[i] >= -BARY_EPS && u[i] <= 1.0f + BARY_EPS && v[i] >= -BARY_EPS &&
				(u[i] + v[i]) <= 1.0f + BARY_EPS && t[i] >= 0.0f && t[i] < best_t;
		}

		for (int i = 0; i < BVH_N; ++i) {
			if (valid[i] && t[i] < best_t) {
				best_t = t[i];
				out_t = t[i];
				out_triangle_index = chunk + i;
				found = true;
			}
		}
	}

	return found;
}
