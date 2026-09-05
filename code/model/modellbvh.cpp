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

// See lbvh_node::child's doc comment: submodel_index >= 0 always, so this is always < 0.
int32_t encode_leaf(int submodel_index)
{
	return -submodel_index - 1;
}

// One entry per item, after sorting by Morton code.
struct SortedEntry {
	uint32_t code;
	int32_t item_index; // indexes the `items` parameter threaded through build_range()
};

// A just-built subtree: either a leaf (encode_leaf() of some submodel, no node pushed) or an
// internal node (a non-negative index freshly pushed to tree.nodes), plus that subtree's own box --
// needed regardless of which case, since a leaf has no node of its own to look the box up from
// later, and the caller (a would-be parent) always needs it for its own slot.
struct Subtree {
	int32_t code;
	vec3d box_min, box_max;
};

// pbrt-style sequential LBVH build: recursively partitions [start, end) of the Morton-sorted range
// by the highest bit that still differs across it (binary search, since the range is sorted),
// descending one bit per level until a partition isolates a single item. A bit that doesn't
// discriminate anything in the current range (e.g. two submodels that happen to land in the same
// Morton cell) is skipped rather than forcing an uneven split; running out of bits entirely with more
// than one item left (duplicate codes) falls back to a plain median split so recursion still
// terminates.
Subtree build_range(lbvh_tree &tree, const SCP_vector<SortedEntry> &sorted, const SCP_vector<lbvh_item> &items,
	int32_t start, int32_t end, int bit)
{
	if (end - start == 1) {
		const lbvh_item &it = items[sorted[start].item_index];
		return {encode_leaf(it.submodel_index), it.box_min, it.box_max};
	}

	int32_t split;
	if (bit < 0) {
		split = start + (end - start) / 2;
	} else {
		uint32_t mask = 1u << bit;
		if ((sorted[start].code & mask) == (sorted[end - 1].code & mask)) {
			return build_range(tree, sorted, items, start, end, bit - 1);
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
		split = lo;
	}

	Subtree left = build_range(tree, sorted, items, start, split, bit - 1);
	Subtree right = build_range(tree, sorted, items, split, end, bit - 1);

	lbvh_node node;
	node.min[0] = left.box_min;
	node.max[0] = left.box_max;
	node.child[0] = left.code;
	node.min[1] = right.box_min;
	node.max[1] = right.box_max;
	node.child[1] = right.code;
	tree.nodes.push_back(node);

	Subtree result;
	result.code = static_cast<int32_t>(tree.nodes.size()) - 1;
	result.box_min = box_min_of(left.box_min, right.box_min);
	result.box_max = box_max_of(left.box_max, right.box_max);
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

	tree.nodes.reserve(static_cast<size_t>(n - 1));
	Subtree root = build_range(tree, sorted, items, 0, n, 29);
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

		for (int i = 0; i < 2; ++i) {
			float bmin[3] = {node.min[i].xyz.x, node.min[i].xyz.y, node.min[i].xyz.z};
			float bmax[3] = {node.max[i].xyz.x, node.max[i].xyz.y, node.max[i].xyz.z};
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
