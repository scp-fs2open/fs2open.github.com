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
		bitmap_handle = bm_create_3d(32, n, n, n, data.get());
	}

	inline int getBitmapHandle() const { return bitmap_handle; };
};

volume_grid_colored<32> fromSphere(ubyte r, ubyte g, ubyte b);