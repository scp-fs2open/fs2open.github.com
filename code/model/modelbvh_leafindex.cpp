#include "model/modelbvh_leafindex.h"

#include <algorithm>
#include <cfloat>

// Same binary-SAH-build-then-collapse-then-flatten algorithm as modelbvh.cpp, adapted for
// bvh_leaf_primitive (AABB + payload) instead of bvh_triangle. See modelbvh.cpp for the detailed
// algorithm commentary; this file intentionally mirrors its structure rather than sharing code,
// to avoid risking modelbvh.cpp's already-validated (stage 1 unit tests + real-POF golden-parity,
// see collision_bvh_rewrite_plan project notes) behavior via a generalizing refactor.

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
		// those sentinel extremes. Bins with zero items hit this during the SAH prefix-sum sweep
		// in build_range() below.
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

vec3d centroid_of(const bvh_leaf_primitive& p)
{
	vec3d c;
	c.xyz.x = (p.bmin.xyz.x + p.bmax.xyz.x) * 0.5f;
	c.xyz.y = (p.bmin.xyz.y + p.bmax.xyz.y) * 0.5f;
	c.xyz.z = (p.bmin.xyz.z + p.bmax.xyz.z) * 0.5f;
	return c;
}

struct BinBuildNode {
	AABB bounds;
	int start = 0, count = 0;
	int left = -1, right = -1;
};

constexpr int LEAF_THRESHOLD = 2;
constexpr int NUM_BINS = 16;

int build_range(const SCP_vector<AABB>& item_bounds, const SCP_vector<vec3d>& item_centroid, SCP_vector<int>& indices,
	SCP_vector<BinBuildNode>& bin_nodes, int start, int count)
{
	AABB bounds;
	AABB cbounds;
	for (int i = start; i < start + count; ++i) {
		bounds.grow(item_bounds[indices[i]]);
		cbounds.grow(item_centroid[indices[i]]);
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
		return make_leaf();

	float cmin = cbounds.bmin.a1d[axis];
	float cmax = cbounds.bmax.a1d[axis];

	int bin_count[NUM_BINS] = {};
	AABB bin_bounds[NUM_BINS];
	for (int i = start; i < start + count; ++i) {
		float t = (item_centroid[indices[i]].a1d[axis] - cmin) / (cmax - cmin);
		int bin = std::min(NUM_BINS - 1, std::max(0, static_cast<int>(t * NUM_BINS)));
		bin_count[bin]++;
		bin_bounds[bin].grow(item_bounds[indices[i]]);
	}

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
	int best_split = -1;
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
	auto mid = std::partition(begin, end, [&](int idx) { return item_centroid[idx].a1d[axis] < split_pos; });
	int left_n = static_cast<int>(mid - begin);

	if (left_n == 0 || left_n == count) {
		std::nth_element(begin, begin + count / 2, end,
			[&](int a, int b) { return item_centroid[a].a1d[axis] < item_centroid[b].a1d[axis]; });
		left_n = count / 2;
	}

	int left = build_range(item_bounds, item_centroid, indices, bin_nodes, start, left_n);
	int right = build_range(item_bounds, item_centroid, indices, bin_nodes, start + left_n, count - left_n);

	BinBuildNode node;
	node.bounds = bounds;
	node.left = left;
	node.right = right;
	bin_nodes.push_back(node);
	return static_cast<int>(bin_nodes.size()) - 1;
}

struct CollapsedChild {
	bool is_leaf = false;
	AABB bounds;
	int start = 0, count = 0;
	int bin_node = -1;
};

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
			break;

		int expand_bin_node = children[best].bin_node;
		children.erase(children.begin() + best);
		const BinBuildNode& expand = bin_nodes[expand_bin_node];
		children.push_back(to_child(expand.left));
		children.push_back(to_child(expand.right));
	}
}

int emit_node(const SCP_vector<BinBuildNode>& bin_nodes, int bin_node_index, SCP_vector<bvh_node>& out_nodes)
{
	SCP_vector<CollapsedChild> children;
	const BinBuildNode& self = bin_nodes[bin_node_index];
	if (self.left < 0) {
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
			int child_idx = emit_node(bin_nodes, c.bin_node, out_nodes);
			out_nodes[idx].count[i] = 0;
			out_nodes[idx].child[i] = child_idx;
		}
	}

	return idx;
}

} // namespace

bvh_leaf_tree bvh_build_leaves(SCP_vector<bvh_leaf_primitive> primitives)
{
	bvh_leaf_tree tree;
	int n = static_cast<int>(primitives.size());
	if (n == 0)
		return tree;

	SCP_vector<AABB> item_bounds(n);
	SCP_vector<vec3d> item_centroid(n);
	for (int i = 0; i < n; ++i) {
		AABB b;
		b.bmin = primitives[i].bmin;
		b.bmax = primitives[i].bmax;
		item_bounds[i] = b;
		item_centroid[i] = centroid_of(primitives[i]);
	}

	SCP_vector<int> indices(n);
	for (int i = 0; i < n; ++i)
		indices[i] = i;

	SCP_vector<BinBuildNode> bin_nodes;
	int root_bin = build_range(item_bounds, item_centroid, indices, bin_nodes, 0, n);

	tree.root = emit_node(bin_nodes, root_bin, tree.nodes);

	tree.items.reserve(n);
	for (int i = 0; i < n; ++i)
		tree.items.push_back(primitives[indices[i]]);

	return tree;
}

namespace bvh_leafindex_detail {

bool ray_aabb_leafindex(const vec3d& origin, const vec3d& inv_dir, const float bmin[3], const float bmax[3], float t_max)
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
	return true;
}

} // namespace bvh_leafindex_detail
