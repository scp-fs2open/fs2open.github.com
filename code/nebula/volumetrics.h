#pragma once

#include "globalincs/pstypes.h"

class volumetric_nebula {
	//Quality
	//How many steps are used to nebulify the volume until the visibility is reached. In theory purely quality and can be changed without changing the aesthetics.
	int steps = 15;
	//Number of steps per nebula slice to test towards the sun
	int globalLightSteps = 6;

	//General Visibility
	//The distance in meters until the target translucity is reached
	float visibility = 7.5f;
	//The target translucity. The nebula won't get more opaque than what is specified here. 0 is not possible.
	float alphaLim = 0.001f;

	//Emissive Light Fogging
	//How quickly the emissive will widen in the nebula. The larger the value, the wider will emissives be drawn even with only little nebula to obscure it.
	float emissiveSpread = 1.0f;
	//How intense emissives will be added. The higher, the brighter the emissives.
	float emissiveIntensity = 0.2f;
	//Correcting factor for emissive alpha. Values > 1 will darken the added emissive closer to the actual source, values < 1 will lighten the added emissive closer to the actual source.
	float emissiveFalloff = 1.33f;

	//Sun-based illumination
	//HG coefficient for backlit nebulae. (-1, 1), but should probably be in the range of (0.05, 0.75)
	float heyneyGreensteinCoeff = 0.2f;
	//Distance factor for global light vs nebula translucity. Values > 1 means the nebula is brighter than it ought to be when it's deeper, values < 0 means it's darker when its shallower
	float globalLightDistanceFactor = 1.0f;

public:
	int getSteps() const;
	int getGlobalLightSteps() const;

	float getVisibility() const;
	float getAlphaLim() const;

	float getEmissiveSpread() const;
	float getEmissiveIntensity() const;
	float getEmissiveFalloff() const;

	float getHeyneyGreensteinCoeff() const;
	float getGlobalLightDistanceFactor() const;
};

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