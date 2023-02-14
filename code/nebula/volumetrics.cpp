#include "volumetrics.h"

#include "bmpman/bmpman.h"

#include <memory>

template<size_t n>
using volume_grid_base = std::array<std::array<std::array<bool, n>, n>, n>;

using volume_grid_base_coarse = volume_grid_base<33>;

static volume_grid_base_coarse volume_grid_base_sphere() {
	volume_grid_base_coarse result;

	for (size_t x = 0; x < std::tuple_size<volume_grid_base_coarse>::value; x++) {
		for (size_t y = 0; y < std::tuple_size<volume_grid_base_coarse>::value; y++) {
			for (size_t z = 0; z < std::tuple_size<volume_grid_base_coarse>::value; z++) {
				float dx = (static_cast<float>(x) - static_cast<float>(std::tuple_size<volume_grid_base_coarse>::value / 2)) / static_cast<float>(std::tuple_size<volume_grid_base_coarse>::value / 2);
				float dy = (static_cast<float>(y) - static_cast<float>(std::tuple_size<volume_grid_base_coarse>::value / 2)) / static_cast<float>(std::tuple_size<volume_grid_base_coarse>::value / 2);
				float dz = (static_cast<float>(z) - static_cast<float>(std::tuple_size<volume_grid_base_coarse>::value / 2)) / static_cast<float>(std::tuple_size<volume_grid_base_coarse>::value / 2);
				result[x][y][z] = dx * dx + dy * dy + dz * dz < 1.0f;
			}
		}
	}

	return result;
}

template<size_t n>
class volume_grid_colored {
	std::unique_ptr<ubyte[]> data;
	int bitmap_handle;

public:
	volume_grid_colored(ubyte r, ubyte g, ubyte b, const volume_grid_base<n + 1>& base) {
		data = std::make_unique<ubyte[]>(n * n * n * 4);



		bitmap_handle = bm_create_3d(4, n, n, n, data.get());
	}
};