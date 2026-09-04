#include "model/modelbvh.h"

#include "math/vecmat.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <utility>

namespace {

// Exact-bitwise vertex dedup key for the shared vertex pool built in bvh_build() below -- see that
// function's comment for why bitwise (not epsilon) comparison is correct here.
struct VertexKey {
	uint32_t bx, by, bz;
	bool operator==(const VertexKey& o) const { return bx == o.bx && by == o.by && bz == o.bz; }
};
struct VertexKeyHash {
	size_t operator()(const VertexKey& k) const
	{
		size_t h = std::hash<uint32_t>()(k.bx);
		h ^= std::hash<uint32_t>()(k.by) + 0x9e3779b9 + (h << 6) + (h >> 2);
		h ^= std::hash<uint32_t>()(k.bz) + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}
};
uint32_t bits_of(float f)
{
	uint32_t u;
	std::memcpy(&u, &f, sizeof(u));
	return u;
}

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
	SCP_vector<uint32_t> i0, i1, i2;
	SCP_vector<int> tmap_num, original_index, leaf_index;
	SCP_vector<bvh_uv> uv0, uv1, uv2;

	size_t reserve_n = tree.triangle_count();
	i0.reserve(reserve_n);
	i1.reserve(reserve_n);
	i2.reserve(reserve_n);
	tmap_num.reserve(reserve_n);
	original_index.reserve(reserve_n);
	leaf_index.reserve(reserve_n);
	uv0.reserve(reserve_n);
	uv1.reserve(reserve_n);
	uv2.reserve(reserve_n);

	auto append_real = [&](size_t src) {
		i0.push_back(tree.i0[src]);
		i1.push_back(tree.i1[src]);
		i2.push_back(tree.i2[src]);
		tmap_num.push_back(tree.tmap_num[src]);
		original_index.push_back(tree.original_index[src]);
		leaf_index.push_back(tree.leaf_index[src]);
		uv0.push_back(tree.uv0[src]);
		uv1.push_back(tree.uv1[src]);
		uv2.push_back(tree.uv2[src]);
	};

	auto append_degenerate = [&](size_t src) {
		// Same metadata as the source triangle, but v1/v2 collapsed onto v0 -- since v0's vertex is
		// already in the shared pool, this just reuses its index three times, no new pool entries.
		i0.push_back(tree.i0[src]);
		i1.push_back(tree.i0[src]);
		i2.push_back(tree.i0[src]);
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
			int new_start = static_cast<int>(i0.size());

			for (int t = start; t < start + count; ++t)
				append_real(static_cast<size_t>(t));

			int padded_count = ((count + BVH_N - 1) / BVH_N) * BVH_N;
			for (int p = count; p < padded_count; ++p)
				append_degenerate(static_cast<size_t>(start + count - 1));

			node.child[i] = new_start;
			node.count[i] = padded_count;
		}
	}

	tree.i0 = std::move(i0);
	tree.i1 = std::move(i1);
	tree.i2 = std::move(i2);
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

	tree.i0.reserve(n);
	tree.i1.reserve(n);
	tree.i2.reserve(n);
	tree.tmap_num.reserve(n);
	tree.original_index.reserve(n);
	tree.leaf_index.reserve(n);
	tree.uv0.reserve(n);
	tree.uv1.reserve(n);
	tree.uv2.reserve(n);

	// Shared vertex pool: each distinct position is stored once, on first reference while walking
	// triangles in the SAH build's leaf order (`indices`), so the pool ends up ordered by
	// first-touch-in-leaf-order. Dedup is exact-bitwise -- every vertex here is a POF-authored float
	// copied verbatim by the caller into a bvh_triangle, so a genuine shared vertex is bit-identical
	// wherever it's referenced, and bitwise dedup is far cheaper than an epsilon comparison.
	SCP_unordered_map<VertexKey, uint32_t, VertexKeyHash> vertex_pool;
	vertex_pool.reserve(static_cast<size_t>(n));
	auto intern_vertex = [&](const vec3d& v) -> uint32_t {
		VertexKey key{bits_of(v.xyz.x), bits_of(v.xyz.y), bits_of(v.xyz.z)};
		auto it = vertex_pool.find(key);
		if (it != vertex_pool.end())
			return it->second;
		uint32_t idx = static_cast<uint32_t>(tree.vx.size());
		tree.vx.push_back(v.xyz.x);
		tree.vy.push_back(v.xyz.y);
		tree.vz.push_back(v.xyz.z);
		vertex_pool.emplace(key, idx);
		return idx;
	};

	for (int i = 0; i < n; ++i) {
		const bvh_triangle& t = triangles[indices[i]];
		tree.i0.push_back(intern_vertex(t.v0));
		tree.i1.push_back(intern_vertex(t.v1));
		tree.i2.push_back(intern_vertex(t.v2));
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

bool ray_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& origin, const vec3d& dir,
	float best_t, float& out_t, int32_t& out_triangle_index)
{
	constexpr float EPS = 1e-8f;

	bool found = false;

	for (int32_t chunk = start; chunk < start + count; chunk += BVH_N) {
		// Vertex components are resolved with a per-lane scalar index load from the shared pool
		// first (there's no hardware gather on this project's SIMD baseline -- confirmed AVX2-only,
		// and weak even there before Skylake -- so a per-lane scalar load is the right tool, not a
		// missed-vectorization gap). Separate fixed-BVH_N-length arrays and per-stage loops below so
		// the autovectorizer has the best chance of turning the arithmetic into SIMD, every lane
		// computed unconditionally with no data-dependent control flow across lanes.
		float v0x[BVH_N], v0y[BVH_N], v0z[BVH_N];
		float v1x[BVH_N], v1y[BVH_N], v1z[BVH_N];
		float v2x[BVH_N], v2y[BVH_N], v2z[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			int32_t idx = chunk + i;
			uint32_t a = tree.i0[idx], b = tree.i1[idx], c = tree.i2[idx];
			v0x[i] = tree.vx[a];
			v0y[i] = tree.vy[a];
			v0z[i] = tree.vz[a];
			v1x[i] = tree.vx[b];
			v1y[i] = tree.vy[b];
			v1z[i] = tree.vz[b];
			v2x[i] = tree.vx[c];
			v2y[i] = tree.vy[c];
			v2z[i] = tree.vz[c];
		}

		float e1x[BVH_N], e1y[BVH_N], e1z[BVH_N];
		float e2x[BVH_N], e2y[BVH_N], e2z[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			e1x[i] = v1x[i] - v0x[i];
			e1y[i] = v1y[i] - v0y[i];
			e1z[i] = v1z[i] - v0z[i];
			e2x[i] = v2x[i] - v0x[i];
			e2y[i] = v2y[i] - v0y[i];
			e2z[i] = v2z[i] - v0z[i];
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
			tvecx[i] = origin.xyz.x - v0x[i];
			tvecy[i] = origin.xyz.y - v0y[i];
			tvecz[i] = origin.xyz.z - v0z[i];
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

bool sphere_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& sphere_p0,
	const vec3d& sphere_dir, float radius, float best_t, float& out_t, int32_t& out_triangle_index)
{
	// Query-wide scalars (same sphere for every triangle in this leaf, so these are computed once
	// per call, not per lane) -- mirrors fvi_polyedge_sphereline()'s own vs_sqr hoist.
	const float xs0x = sphere_p0.xyz.x, xs0y = sphere_p0.xyz.y, xs0z = sphere_p0.xyz.z;
	const float vsx = sphere_dir.xyz.x, vsy = sphere_dir.xyz.y, vsz = sphere_dir.xyz.z;
	const float vs_sqr = vsx * vsx + vsy * vsy + vsz * vsz;
	const float Rs2 = radius * radius;
	const float Rs_point2 = 0.2f * radius;

	// Edge-cylinder + vertex-sphere test for one triangle edge (va -> vb), batched over BVH_N lanes.
	// A faithful per-lane translation of fvi_polyedge_sphereline()'s per-edge body: every goto
	// becomes an unconditional bool computed for all lanes, combined with && / || instead of control
	// flow. hitOut/timeOut receive this edge's contribution only (the caller min-reduces across the
	// triangle's 3 edges, same as the scalar function's running best_sphere_time).
	auto test_edge = [&](const float avx[BVH_N], const float avy[BVH_N], const float avz[BVH_N],
						  const float bvx[BVH_N], const float bvy[BVH_N], const float bvz[BVH_N], bool hitOut[BVH_N],
						  float timeOut[BVH_N]) {
		float vex[BVH_N], vey[BVH_N], vez[BVH_N];
		float dxx[BVH_N], dxy[BVH_N], dxz[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			vex[i] = bvx[i] - avx[i];
			vey[i] = bvy[i] - avy[i];
			vez[i] = bvz[i] - avz[i];
			dxx[i] = xs0x - avx[i];
			dxy[i] = xs0y - avy[i];
			dxz[i] = xs0z - avz[i];
		}

		float dx_dot_ve[BVH_N], dx_dot_vs[BVH_N], dx_sqr[BVH_N], ve_dot_vs[BVH_N], ve_sqr[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			dx_dot_ve[i] = dxx[i] * vex[i] + dxy[i] * vey[i] + dxz[i] * vez[i];
			dx_dot_vs[i] = dxx[i] * vsx + dxy[i] * vsy + dxz[i] * vsz;
			dx_sqr[i] = dxx[i] * dxx[i] + dxy[i] * dxy[i] + dxz[i] * dxz[i];
			ve_dot_vs[i] = vex[i] * vsx + vey[i] * vsy + vez[i] * vsz;
			ve_sqr[i] = vex[i] * vex[i] + vey[i] * vey[i] + vez[i] * vez[i];
		}

		// Stage 1: closest-approach time along the sphere's own line (fvi_polyedge_sphereline's
		// first quadratic, "A*x*x + 2*B*x + C = 0" solved for the sphere parameter).
		float A[BVH_N], B[BVH_N], C[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			A[i] = ve_dot_vs[i] * ve_dot_vs[i] - ve_sqr[i] * vs_sqr;
			B[i] = dx_dot_ve[i] * ve_dot_vs[i] - dx_dot_vs[i] * ve_sqr[i];
			C[i] = dx_dot_ve[i] * dx_dot_ve[i] + Rs2 * ve_sqr[i] - dx_sqr[i] * ve_sqr[i];
		}

		float disc1[BVH_N];
		for (int i = 0; i < BVH_N; ++i)
			disc1[i] = B[i] * B[i] - A[i] * C[i];

		bool stage1_disc_pos[BVH_N];
		for (int i = 0; i < BVH_N; ++i)
			stage1_disc_pos[i] = disc1[i] > 0.0f;

		float root1[BVH_N];
		bool stage1_in_range[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			float root = stage1_disc_pos[i] ? std::sqrt(disc1[i]) : 0.0f;
			float invA = 1.0f / A[i];
			float r1 = (-B[i] + root) * invA;
			float r2 = (-B[i] - root) * invA;
			float lo = std::min(r1, r2);
			if (lo < 0.0f && lo >= -0.05f)
				lo = 0.000001f;
			root1[i] = lo;
			stage1_in_range[i] = stage1_disc_pos[i] && lo <= 1.0f && lo >= 0.0f;
		}

		// Stage 2: edge-parameter roots at the stage-1 sphere time, cross-validated against the
		// sphere's actual surface distance (the "q" check in fvi_polyedge_sphereline) -- this is
		// what distinguishes a genuine hit on the bounded edge segment from the stage-1 solve simply
		// finding where the *infinite* edge line comes within Rs of the sphere's line.
		float A2[BVH_N], B2[BVH_N], C2[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			A2[i] = A[i] * ve_sqr[i];
			B2[i] = ve_sqr[i] * (dx_dot_ve[i] * vs_sqr - dx_dot_vs[i] * ve_dot_vs[i]);
			C2[i] = 2.0f * dx_dot_ve[i] * dx_dot_vs[i] * ve_dot_vs[i] + Rs2 * ve_dot_vs[i] * ve_dot_vs[i] -
					dx_sqr[i] * ve_dot_vs[i] * ve_dot_vs[i] - dx_dot_ve[i] * dx_dot_ve[i] * vs_sqr;
		}

		bool seg_hit[BVH_N];
		float seg_time[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			float disc2 = B2[i] * B2[i] - A2[i] * C2[i];
			float root2sqrt = (disc2 < 0.0f) ? 0.0f : std::sqrt(disc2);
			float invA2 = 1.0f / A2[i];
			float root1b = (-B2[i] + root2sqrt) * invA2;
			float root2b = (-B2[i] - root2sqrt) * invA2;

			float ehx1 = avx[i] + vex[i] * root1b, ehy1 = avy[i] + vey[i] * root1b, ehz1 = avz[i] + vez[i] * root1b;
			float ehx2 = avx[i] + vex[i] * root2b, ehy2 = avy[i] + vey[i] * root2b, ehz2 = avz[i] + vez[i] * root2b;
			float shx = xs0x + vsx * root1[i], shy = xs0y + vsy * root1[i], shz = xs0z + vsz * root1[i];

			float q1 = (ehx1 - shx) * (ehx1 - shx) + (ehy1 - shy) * (ehy1 - shy) + (ehz1 - shz) * (ehz1 - shz);
			float q2 = (ehx2 - shx) * (ehx2 - shx) + (ehy2 - shy) * (ehy2 - shy) + (ehz2 - shz) * (ehz2 - shz);

			bool pass1 = root1b >= 0.0f && root1b <= 1.0f && std::fabs(q1 - Rs2) < Rs_point2;
			bool pass2 = root2b >= 0.0f && root2b <= 1.0f && std::fabs(q2 - Rs2) < Rs_point2;

			seg_hit[i] = stage1_disc_pos[i] && stage1_in_range[i] && (pass1 || pass2);
			seg_time[i] = root1[i];
		}

		// TryVertex fallback: direct sphere-vs-point test against both of this edge's endpoints
		// (check_sphere_point() in fvi.cpp, inlined). Attempted whenever the stage-1 quadratic had
		// real roots but the segment test above didn't confirm a hit -- matches every path that
		// reaches fvi_polyedge_sphereline()'s TryVertex: label.
		for (int i = 0; i < BVH_N; ++i) {
			bool need_vertex = stage1_disc_pos[i] && !seg_hit[i];

			float dxb_x = xs0x - bvx[i], dxb_y = xs0y - bvy[i], dxb_z = xs0z - bvz[i];
			float dxb_sqr = dxb_x * dxb_x + dxb_y * dxb_y + dxb_z * dxb_z;
			float dxb_dot_vs = dxb_x * vsx + dxb_y * vsy + dxb_z * vsz;

			bool v0hit = false, v1hit = false;
			float sphere_v0 = FLT_MAX, sphere_v1 = FLT_MAX;

			float discv0 = dx_dot_vs[i] * dx_dot_vs[i] - vs_sqr * (dx_sqr[i] - Rs2);
			if (discv0 > 0.0f) {
				float root = std::sqrt(discv0);
				float r1 = (-dx_dot_vs[i] + root) / vs_sqr;
				float r2 = (-dx_dot_vs[i] - root) / vs_sqr;
				float lo = std::min(r1, r2);
				if (lo > 0.0f && lo < 1.0f) {
					v0hit = true;
					sphere_v0 = lo;
				}
			}

			float discv1 = dxb_dot_vs * dxb_dot_vs - vs_sqr * (dxb_sqr - Rs2);
			if (discv1 > 0.0f) {
				float root = std::sqrt(discv1);
				float r1 = (-dxb_dot_vs + root) / vs_sqr;
				float r2 = (-dxb_dot_vs - root) / vs_sqr;
				float lo = std::min(r1, r2);
				if (lo > 0.0f && lo < 1.0f) {
					v1hit = true;
					sphere_v1 = lo;
				}
			}

			bool vertex_hit = v0hit || v1hit;
			float vertex_time = (v0hit && v1hit) ? std::min(sphere_v0, sphere_v1) : (v0hit ? sphere_v0 : sphere_v1);

			hitOut[i] = seg_hit[i] || (need_vertex && vertex_hit);
			timeOut[i] = seg_hit[i] ? seg_time[i] : vertex_time;
		}
	};

	bool found = false;

	for (int32_t chunk = start; chunk < start + count; chunk += BVH_N) {
		float v0x[BVH_N], v0y[BVH_N], v0z[BVH_N];
		float v1x[BVH_N], v1y[BVH_N], v1z[BVH_N];
		float v2x[BVH_N], v2y[BVH_N], v2z[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			int32_t idx = chunk + i;
			uint32_t a = tree.i0[idx], b = tree.i1[idx], c = tree.i2[idx];
			v0x[i] = tree.vx[a];
			v0y[i] = tree.vy[a];
			v0z[i] = tree.vz[a];
			v1x[i] = tree.vx[b];
			v1y[i] = tree.vy[b];
			v1z[i] = tree.vz[b];
			v2x[i] = tree.vx[c];
			v2y[i] = tree.vy[c];
			v2z[i] = tree.vz[c];
		}

		float e1x[BVH_N], e1y[BVH_N], e1z[BVH_N];
		float e2x[BVH_N], e2y[BVH_N], e2z[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			e1x[i] = v1x[i] - v0x[i];
			e1y[i] = v1y[i] - v0y[i];
			e1z[i] = v1z[i] - v0z[i];
			e2x[i] = v2x[i] - v0x[i];
			e2y[i] = v2y[i] - v0y[i];
			e2z[i] = v2z[i] - v0z[i];
		}

		// Face normal (raw cross product + magnitude; normalize with a guarded reciprocal so a
		// degenerate/zero-area padding triangle -- see bvh_build() -- yields nx=ny=nz=0 rather than
		// NaN, which then safely fails every downstream test in this function).
		float nx[BVH_N], ny[BVH_N], nz[BVH_N];
		bool degenerate[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			float rx = e1y[i] * e2z[i] - e1z[i] * e2y[i];
			float ry = e1z[i] * e2x[i] - e1x[i] * e2z[i];
			float rz = e1x[i] * e2y[i] - e1y[i] * e2x[i];
			float magsq = rx * rx + ry * ry + rz * rz;
			degenerate[i] = magsq <= 1e-16f;
			float inv_mag = degenerate[i] ? 0.0f : 1.0f / std::sqrt(magsq);
			nx[i] = rx * inv_mag;
			ny[i] = ry * inv_mag;
			nz[i] = rz * inv_mag;
		}

		// fvi_sphere_plane(), batched: face/slab touch-time test against each triangle's own plane.
		bool active[BVH_N];
		float face_t[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			float vs_dot_norm = vsx * nx[i] + vsy * ny[i] + vsz * nz[i];
			bool backface = vs_dot_norm > 0.0f; // MC_COLLIDE_ALL never reaches this function -- see header comment
			float D = -(nx[i] * v0x[i] + ny[i] * v0y[i] + nz[i] * v0z[i]);
			float xs0_dot_norm = nx[i] * xs0x + ny[i] * xs0y + nz[i] * xs0z;
			bool plane_valid = std::fabs(vs_dot_norm) > 1e-30f;
			float inv_vs = plane_valid ? 1.0f / vs_dot_norm : 0.0f;
			float t1raw = (-D - xs0_dot_norm + radius) * inv_vs;
			float t2raw = (-D - xs0_dot_norm - radius) * inv_vs;
			float t1 = std::min(t1raw, t2raw);
			float t2 = std::max(t1raw, t2raw);
			bool fvi_ok = plane_valid && t1 < 1.0f && t2 > 0.0f;
			active[i] = !degenerate[i] && !backface && fvi_ok;
			face_t[i] = t1;
		}

		// Face containment (barycentric), only meaningful where active[i] && face_t[i] >= 0 -- see
		// mc_check_triangle_sphereline_face()'s check_face/on_face.
		bool on_face[BVH_N];
		for (int i = 0; i < BVH_N; ++i) {
			bool check_face = active[i] && face_t[i] >= 0.0f;

			float t1 = face_t[i];
			float vtx = xs0x + vsx * t1, vty = xs0y + vsy * t1, vtz = xs0z + vsz * t1;
			float dist = (vtx - v0x[i]) * nx[i] + (vty - v0y[i]) * ny[i] + (vtz - v0z[i]) * nz[i];
			float hpx = vtx - dist * nx[i], hpy = vty - dist * ny[i], hpz = vtz - dist * nz[i];

			float vpx = hpx - v0x[i], vpy = hpy - v0y[i], vpz = hpz - v0z[i];
			float d00 = e1x[i] * e1x[i] + e1y[i] * e1y[i] + e1z[i] * e1z[i];
			float d01 = e1x[i] * e2x[i] + e1y[i] * e2y[i] + e1z[i] * e2z[i];
			float d11 = e2x[i] * e2x[i] + e2y[i] * e2y[i] + e2z[i] * e2z[i];
			float d20 = vpx * e1x[i] + vpy * e1y[i] + vpz * e1z[i];
			float d21 = vpx * e2x[i] + vpy * e2y[i] + vpz * e2z[i];
			float denom = d00 * d11 - d01 * d01;

			bool bary_ok = false;
			if (std::fabs(denom) >= 1e-12f) {
				constexpr float BARY_EPS = 1e-4f;
				float gamma_v1 = (d11 * d20 - d01 * d21) / denom;
				float gamma_v2 = (d00 * d21 - d01 * d20) / denom;
				float gamma_v0 = 1.0f - gamma_v1 - gamma_v2;
				bary_ok = gamma_v0 >= -BARY_EPS && gamma_v1 >= -BARY_EPS && gamma_v2 >= -BARY_EPS;
			}
			on_face[i] = check_face && bary_ok;
		}

		bool edgeHit0[BVH_N], edgeHit1[BVH_N], edgeHit2[BVH_N];
		float edgeTime0[BVH_N], edgeTime1[BVH_N], edgeTime2[BVH_N];
		test_edge(v0x, v0y, v0z, v1x, v1y, v1z, edgeHit0, edgeTime0);
		test_edge(v1x, v1y, v1z, v2x, v2y, v2z, edgeHit1, edgeTime1);
		test_edge(v2x, v2y, v2z, v0x, v0y, v0z, edgeHit2, edgeTime2);

		for (int i = 0; i < BVH_N; ++i) {
			bool check_edges = active[i]; // == fvi_ok, see header comment derivation
			bool any_hit;
			float t;

			if (on_face[i]) {
				any_hit = true;
				t = face_t[i];
			} else if (check_edges) {
				any_hit = edgeHit0[i] || edgeHit1[i] || edgeHit2[i];
				t = FLT_MAX;
				if (edgeHit0[i])
					t = std::min(t, edgeTime0[i]);
				if (edgeHit1[i])
					t = std::min(t, edgeTime1[i]);
				if (edgeHit2[i])
					t = std::min(t, edgeTime2[i]);
			} else {
				any_hit = false;
				t = FLT_MAX;
			}

			if (any_hit && t < best_t) {
				best_t = t;
				out_t = t;
				out_triangle_index = chunk + i;
				found = true;
			}
		}
	}

	return found;
}
