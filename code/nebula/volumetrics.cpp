#include "volumetrics.h"

#include "bmpman/bmpman.h"

#include <memory>

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

volume_grid_colored<32> fromSphere(ubyte r, ubyte g, ubyte b) {
	return volume_grid_colored<32>(r, g, b, volume_grid_base_sphere());
}