#include "model/modellbvh.h"

#include "math/vecmat.h"

#include <algorithm>
#include <cfloat>
#include <utility>

namespace {

// Standard 10-bit Morton (Z-order) bit-spreading: inserts two zero bits after each of v's low 10
// bits, so 3 independently-spread axis values can be OR'd together (offset by 0/1/2 bits) into one
// 30-bit interleaved code.
uint32_t expand_bits_10(uint32_t v)
{
	v = (v | (v << 16)) & 0x030000FFu;
	v = (v | (v << 8)) & 0x0300F00Fu;
	v = (v | (v << 4)) & 0x030C30C3u;
	v = (v | (v << 2)) & 0x09249249u;
	return v;
}

uint32_t morton3(uint32_t x, uint32_t y, uint32_t z)
{
	return expand_bits_10(x) | (expand_bits_10(y) << 1) | (expand_bits_10(z) << 2);
}

uint32_t quantize_10bit(float v)
{
	v = std::clamp(v, 0.0f, 1.0f);
	return std::min(1023u, static_cast<uint32_t>(v * 1024.0f));
}

vec3d box_min_of(const vec3d &a, const vec3d &b)
{
	vec3d r;
	r.xyz.x = std::min(a.xyz.x, b.xyz.x);
	r.xyz.y = std::min(a.xyz.y, b.xyz.y);
	r.xyz.z = std::min(a.xyz.z, b.xyz.z);
	return r;
}

vec3d box_max_of(const vec3d &a, const vec3d &b)
{
	vec3d r;
	r.xyz.x = std::max(a.xyz.x, b.xyz.x);
	r.xyz.y = std::max(a.xyz.y, b.xyz.y);
	r.xyz.z = std::max(a.xyz.z, b.xyz.z);
	return r;
}

// See lbvh_node::child's doc comment: submodel_index >= 0 always, so this is always < 0 (and never
// INT32_MIN for any submodel_index that will ever exist -- that value is reserved for "empty slot").
int32_t encode_leaf(int submodel_index)
{
	return -submodel_index - 1;
}

// One entry per item, after sorting by Morton code.
struct SortedEntry {
	uint32_t code;
	int32_t item_index; // indexes the `items` parameter threaded through the build
};

// A just-built subtree: either a leaf (encode_leaf() of some submodel, no node pushed) or an
// internal node (a non-negative index freshly pushed to tree.nodes), plus that subtree's own box --
// needed regardless of which case, since a leaf has no node of its own to look the box up from
// later, and the caller (a would-be parent slot) always needs it.
struct Subtree {
	int32_t code;
	vec3d box_min, box_max;
};

// Where [start, end) (sorted by Morton code) should split: the highest bit that still differs
// across the range, found by binary search since the range is sorted, skipping any leading bit that
// doesn't discriminate anything within it (e.g. two submodels landing in the same Morton cell).
// bit_used is the bit the split actually happened on, or -1 if every bit was exhausted (duplicate
// codes), in which case pos is a plain median split so the caller still makes progress.
struct SplitPoint {
	int32_t pos;
	int bit_used;
};

SplitPoint find_split(const SCP_vector<SortedEntry> &sorted, int32_t start, int32_t end, int bit)
{
	while (bit >= 0) {
		uint32_t mask = 1u << bit;
		if ((sorted[start].code & mask) == (sorted[end - 1].code & mask)) {
			--bit;
			continue;
		}
		int32_t lo = start, hi = end - 1;
		while (lo < hi) {
			int32_t mid = (lo + hi) / 2;
			if (sorted[mid].code & mask) {
				hi = mid;
			} else {
				lo = mid + 1;
			}
		}
		return {lo, bit};
	}
	return {start + (end - start) / 2, -1};
}

Subtree build_node(lbvh_tree &tree, const SCP_vector<SortedEntry> &sorted, const SCP_vector<lbvh_item> &items,
	int32_t start, int32_t end, int bit);

// Resolves one child slot's range into either a leaf (single item) or a further BVH_N-wide node
// (built recursively), and writes its box/child code into `node`'s slot i.
Subtree fill_slot(lbvh_tree &tree, const SCP_vector<SortedEntry> &sorted, const SCP_vector<lbvh_item> &items,
	lbvh_node &node, int slot, int32_t start, int32_t end, int bit)
{
	Subtree sub;
	if (end - start == 1) {
		const lbvh_item &it = items[sorted[start].item_index];
		sub = {encode_leaf(it.submodel_index), it.box_min, it.box_max};
	} else {
		sub = build_node(tree, sorted, items, start, end, bit);
	}
	node.minx[slot] = sub.box_min.xyz.x;
	node.miny[slot] = sub.box_min.xyz.y;
	node.minz[slot] = sub.box_min.xyz.z;
	node.maxx[slot] = sub.box_max.xyz.x;
	node.maxy[slot] = sub.box_max.xyz.y;
	node.maxz[slot] = sub.box_max.xyz.z;
	node.child[slot] = sub.code;
	return sub;
}

// Builds one BVH_N(=4)-wide node over [start, end) (end - start > 1): splits the Morton-sorted range
// in two, then splits each half again wherever it still holds more than one item -- collapsing what
// would be two binary LBVH levels into a single 4-wide node, the same "greedily collapsed" shape
// bvh_build() uses for the triangle module (see modelbvh.h's own doc comment on BVH_N). A range that
// already dropped to one item after the first split becomes a leaf slot directly instead of forcing
// a pointless second split, so a node always ends up with 2-4 real children, never fewer.
Subtree build_node(lbvh_tree &tree, const SCP_vector<SortedEntry> &sorted, const SCP_vector<lbvh_item> &items,
	int32_t start, int32_t end, int bit)
{
	SplitPoint top = find_split(sorted, start, end, bit);
	int32_t mid = top.pos;
	int next_bit = top.bit_used - 1;

	int32_t range_start[BVH_N], range_end[BVH_N];
	int range_bit[BVH_N];
	int n_ranges = 0;
	auto add_range = [&](int32_t s, int32_t e, int b) {
		range_start[n_ranges] = s;
		range_end[n_ranges] = e;
		range_bit[n_ranges] = b;
		++n_ranges;
	};

	if (mid - start > 1) {
		SplitPoint ls = find_split(sorted, start, mid, next_bit);
		add_range(start, ls.pos, ls.bit_used - 1);
		add_range(ls.pos, mid, ls.bit_used - 1);
	} else {
		add_range(start, mid, next_bit);
	}
	if (end - mid > 1) {
		SplitPoint rs = find_split(sorted, mid, end, next_bit);
		add_range(mid, rs.pos, rs.bit_used - 1);
		add_range(rs.pos, end, rs.bit_used - 1);
	} else {
		add_range(mid, end, next_bit);
	}

	lbvh_node node;
	vec3d node_min, node_max;
	for (int slot = 0; slot < BVH_N; ++slot) {
		if (slot < n_ranges) {
			Subtree sub = fill_slot(tree, sorted, items, node, slot, range_start[slot], range_end[slot],
				range_bit[slot]);
			if (slot == 0) {
				node_min = sub.box_min;
				node_max = sub.box_max;
			} else {
				node_min = box_min_of(node_min, sub.box_min);
				node_max = box_max_of(node_max, sub.box_max);
			}
		} else {
			// Impossible box (min > max), matching bvh_node's own padding convention -- fails the
			// slab test for free even without consulting child[slot] first.
			node.minx[slot] = node.miny[slot] = node.minz[slot] = FLT_MAX;
			node.maxx[slot] = node.maxy[slot] = node.maxz[slot] = -FLT_MAX;
			node.child[slot] = INT32_MIN;
		}
	}

	tree.nodes.push_back(node);

	Subtree result;
	result.code = static_cast<int32_t>(tree.nodes.size()) - 1;
	result.box_min = node_min;
	result.box_max = node_max;
	return result;
}

} // namespace

lbvh_tree lbvh_build(SCP_vector<lbvh_item> items)
{
	lbvh_tree tree;
	int32_t n = static_cast<int32_t>(items.size());
	if (n == 0) {
		return tree;
	}

	vec3d scene_min = items[0].box_min;
	vec3d scene_max = items[0].box_max;
	for (int32_t i = 1; i < n; ++i) {
		scene_min = box_min_of(scene_min, items[i].box_min);
		scene_max = box_max_of(scene_max, items[i].box_max);
	}
	vec3d extent = scene_max - scene_min;

	SCP_vector<SortedEntry> sorted(n);
	for (int32_t i = 0; i < n; ++i) {
		vec3d centroid = (items[i].box_min + items[i].box_max) * 0.5f;
		float nx = extent.xyz.x > 0.0f ? (centroid.xyz.x - scene_min.xyz.x) / extent.xyz.x : 0.0f;
		float ny = extent.xyz.y > 0.0f ? (centroid.xyz.y - scene_min.xyz.y) / extent.xyz.y : 0.0f;
		float nz = extent.xyz.z > 0.0f ? (centroid.xyz.z - scene_min.xyz.z) / extent.xyz.z : 0.0f;
		sorted[i] = {morton3(quantize_10bit(nx), quantize_10bit(ny), quantize_10bit(nz)), i};
	}
	std::sort(sorted.begin(), sorted.end(), [](const SortedEntry &a, const SortedEntry &b) {
		return a.code != b.code ? a.code < b.code : a.item_index < b.item_index;
	});

	if (n == 1) {
		const lbvh_item &it = items[sorted[0].item_index];
		tree.root = encode_leaf(it.submodel_index);
		tree.root_min = it.box_min;
		tree.root_max = it.box_max;
		return tree;
	}

	tree.nodes.reserve(static_cast<size_t>((n + BVH_N - 2) / (BVH_N - 1)));
	Subtree root = build_node(tree, sorted, items, 0, n, 29);
	tree.root = root.code;
	tree.root_min = root.box_min;
	tree.root_max = root.box_max;
	return tree;
}

void lbvh_visit(const lbvh_tree &tree, const vec3d &origin, const vec3d &dir, float &t_max, float radius,
	const std::function<void(int32_t submodel_index)> &visitor)
{
	if (tree.root == INT32_MIN) {
		return;
	}

	vec3d inv_dir;
	inv_dir.xyz.x = dir.xyz.x != 0.0f ? 1.0f / dir.xyz.x : FLT_MAX;
	inv_dir.xyz.y = dir.xyz.y != 0.0f ? 1.0f / dir.xyz.y : FLT_MAX;
	inv_dir.xyz.z = dir.xyz.z != 0.0f ? 1.0f / dir.xyz.z : FLT_MAX;

	{
		float bmin[3] = {tree.root_min.xyz.x, tree.root_min.xyz.y, tree.root_min.xyz.z};
		float bmax[3] = {tree.root_max.xyz.x, tree.root_max.xyz.y, tree.root_max.xyz.z};
		if (!bvh_detail::ray_aabb_visit(origin, inv_dir, bmin, bmax, t_max, radius)) {
			return;
		}
	}

	if (tree.root < 0) {
		visitor(-tree.root - 1);
		return;
	}

	int32_t stack[64];
	int sp = 0;
	stack[sp++] = tree.root;

	while (sp > 0) {
		int32_t idx = stack[--sp];
		const lbvh_node &node = tree.nodes[idx];

		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] == INT32_MIN) {
				continue;
			}

			float bmin[3] = {node.minx[i], node.miny[i], node.minz[i]};
			float bmax[3] = {node.maxx[i], node.maxy[i], node.maxz[i]};
			if (!bvh_detail::ray_aabb_visit(origin, inv_dir, bmin, bmax, t_max, radius)) {
				continue;
			}

			if (node.child[i] < 0) {
				visitor(-node.child[i] - 1);
			} else {
				Assertion(sp < 64, "modellbvh traversal stack overflow -- tree unexpectedly deep");
				stack[sp++] = node.child[i];
			}
		}
	}
}
