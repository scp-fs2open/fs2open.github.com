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

// One entry per item, after sorting by Morton code -- kept separate from lbvh_tree::items (which
// stays in caller-supplied order) so leaves can reference the original item index directly.
struct SortedEntry {
	uint32_t code;
	int32_t item_index;
};

// pbrt-style sequential LBVH build: recursively partitions [start, end) of the Morton-sorted range
// by the highest bit that still differs across it (binary search, since the range is sorted),
// descending one bit per level until a partition isolates a single item. A bit that doesn't
// discriminate anything in the current range (e.g. two submodels that happen to land in the same
// Morton cell) is skipped rather than forcing an uneven split; running out of bits entirely with more
// than one item left (duplicate codes) falls back to a plain median split so recursion still
// terminates. Builds tree.nodes depth-first as it unwinds; returns the index of the subtree's root.
int32_t build_range(lbvh_tree &tree, const SCP_vector<SortedEntry> &sorted, int32_t start, int32_t end, int bit)
{
	if (end - start == 1) {
		int32_t item_index = sorted[start].item_index;
		lbvh_node leaf;
		leaf.min = tree.items[item_index].box_min;
		leaf.max = tree.items[item_index].box_max;
		leaf.item = item_index;
		tree.nodes.push_back(leaf);
		return static_cast<int32_t>(tree.nodes.size()) - 1;
	}

	int32_t split;
	if (bit < 0) {
		split = start + (end - start) / 2;
	} else {
		uint32_t mask = 1u << bit;
		if ((sorted[start].code & mask) == (sorted[end - 1].code & mask)) {
			return build_range(tree, sorted, start, end, bit - 1);
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

	int32_t left = build_range(tree, sorted, start, split, bit - 1);
	int32_t right = build_range(tree, sorted, split, end, bit - 1);

	// Copy child boxes by value before push_back() -- it may reallocate tree.nodes, which would
	// invalidate a reference taken beforehand.
	vec3d lmin = tree.nodes[left].min, lmax = tree.nodes[left].max;
	vec3d rmin = tree.nodes[right].min, rmax = tree.nodes[right].max;

	lbvh_node node;
	node.left = left;
	node.right = right;
	node.min = box_min_of(lmin, rmin);
	node.max = box_max_of(lmax, rmax);
	tree.nodes.push_back(node);
	return static_cast<int32_t>(tree.nodes.size()) - 1;
}

} // namespace

lbvh_tree lbvh_build(SCP_vector<lbvh_item> items)
{
	lbvh_tree tree;
	tree.items = std::move(items);
	int32_t n = static_cast<int32_t>(tree.items.size());
	if (n == 0) {
		return tree;
	}

	vec3d scene_min = tree.items[0].box_min;
	vec3d scene_max = tree.items[0].box_max;
	for (int32_t i = 1; i < n; ++i) {
		scene_min = box_min_of(scene_min, tree.items[i].box_min);
		scene_max = box_max_of(scene_max, tree.items[i].box_max);
	}
	vec3d extent = scene_max - scene_min;

	SCP_vector<SortedEntry> sorted(n);
	for (int32_t i = 0; i < n; ++i) {
		vec3d centroid = (tree.items[i].box_min + tree.items[i].box_max) * 0.5f;
		float nx = extent.xyz.x > 0.0f ? (centroid.xyz.x - scene_min.xyz.x) / extent.xyz.x : 0.0f;
		float ny = extent.xyz.y > 0.0f ? (centroid.xyz.y - scene_min.xyz.y) / extent.xyz.y : 0.0f;
		float nz = extent.xyz.z > 0.0f ? (centroid.xyz.z - scene_min.xyz.z) / extent.xyz.z : 0.0f;
		sorted[i] = {morton3(quantize_10bit(nx), quantize_10bit(ny), quantize_10bit(nz)), i};
	}
	std::sort(sorted.begin(), sorted.end(), [](const SortedEntry &a, const SortedEntry &b) {
		return a.code != b.code ? a.code < b.code : a.item_index < b.item_index;
	});

	tree.nodes.reserve(static_cast<size_t>(2 * n - 1));
	tree.root = build_range(tree, sorted, 0, n, 29);
	return tree;
}

void lbvh_visit(const lbvh_tree &tree, const vec3d &origin, const vec3d &dir, float &t_max, float radius,
	const std::function<void(int32_t submodel_index)> &visitor)
{
	if (tree.root < 0) {
		return;
	}

	vec3d inv_dir;
	inv_dir.xyz.x = dir.xyz.x != 0.0f ? 1.0f / dir.xyz.x : FLT_MAX;
	inv_dir.xyz.y = dir.xyz.y != 0.0f ? 1.0f / dir.xyz.y : FLT_MAX;
	inv_dir.xyz.z = dir.xyz.z != 0.0f ? 1.0f / dir.xyz.z : FLT_MAX;

	int32_t stack[64];
	int sp = 0;
	stack[sp++] = tree.root;

	while (sp > 0) {
		int32_t idx = stack[--sp];
		const lbvh_node &node = tree.nodes[idx];

		float bmin[3] = {node.min.xyz.x, node.min.xyz.y, node.min.xyz.z};
		float bmax[3] = {node.max.xyz.x, node.max.xyz.y, node.max.xyz.z};
		if (!bvh_detail::ray_aabb_visit(origin, inv_dir, bmin, bmax, t_max, radius)) {
			continue;
		}

		if (node.left < 0 && node.right < 0) {
			visitor(tree.items[node.item].submodel_index);
		} else {
			Assertion(sp < 62, "modellbvh traversal stack overflow -- tree unexpectedly deep");
			if (node.left >= 0) {
				stack[sp++] = node.left;
			}
			if (node.right >= 0) {
				stack[sp++] = node.right;
			}
		}
	}
}
