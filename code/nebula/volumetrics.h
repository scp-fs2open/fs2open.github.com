#pragma once

#include "globalincs/pstypes.h"

template<size_t n>
using volume_grid_base = std::array<std::array<std::array<bool, n>, n>, n>;

template<size_t n>
class volume_grid_colored {
	std::unique_ptr<ubyte[]> data;
	int bitmap_handle;

public:
	volume_grid_colored(ubyte r, ubyte g, ubyte b, const volume_grid_base<n + 1>& base) {
		data = make_unique<ubyte[]>(n * n * n * 4);
		for (size_t x = 0; x < n; x++) {
			for (size_t y = 0; y < n; y++) {
				for (size_t z = 0; z < n; z++) {
					data[x * n * n * 4 + y * n * 4 + z * 4] = b;
					data[x * n * n * 4 + y * n * 4 + z * 4 + 1] = g;
					data[x * n * n * 4 + y * n * 4 + z * 4 + 2] = r;
					ubyte sum = 0;
					if (base[x][y][z])
						sum++;
					if (base[x][y][z+1])
						sum++;
					if (base[x][y+1][z])
						sum++;
					if (base[x][y+1][z+1])
						sum++;
					if (base[x+1][y][z])
						sum++;
					if (base[x+1][y][z+1])
						sum++;
					if (base[x+1][y+1][z])
						sum++;
					if (base[x+1][y+1][z+1])
						sum++;
					data[x * n * n * 4 + y * n * 4 + z * 4 + 3] = sum * 31;
				}
			}
		}
		bitmap_handle = bm_create_3d(32, n, n, n, data.get());
	}

	inline int getBitmapHandle() const { return bitmap_handle; };
};

volume_grid_colored<32> fromSphere(ubyte r, ubyte g, ubyte b);