#include "volumetrics.h"

#include "bmpman/bmpman.h"

volumetric_nebula::~volumetric_nebula() {
	if (isVolumeBitmapValid()) {
		bm_release(volumeBitmapHandle);
	}
}

int volumetric_nebula::getSteps() const {
	return steps; //potentially adjust for graphics settings in F2 menu
}

int volumetric_nebula::getGlobalLightSteps() const {
	return globalLightSteps; //potentially adjust for graphics settings in F2 menu
}

float volumetric_nebula::getVisibility() const {
	return visibility;
}

float volumetric_nebula::getAlphaLim() const {
	return alphaLim;
}

float volumetric_nebula::getEmissiveSpread() const {
	return emissiveSpread;
}

float volumetric_nebula::getEmissiveIntensity() const {
	return emissiveIntensity;
}

float volumetric_nebula::getEmissiveFalloff() const {
	return emissiveFalloff;
}

float volumetric_nebula::getHenyeyGreensteinCoeff() const {
	return henyeyGreensteinCoeff;
}

float volumetric_nebula::getGlobalLightDistanceFactor() const {
	return globalLightDistanceFactor;
}

bool volumetric_nebula::isVolumeBitmapValid() const {
	return volumeBitmapHandle >= 0;
}

void volumetric_nebula::renderVolumeBitmap(float r, float g, float b) {
	Assertion(!isVolumeBitmapValid(), "Volume bitmap was already rendered!");

	int n = 1 << resolution;
	int nSample = (n << (oversampling - 1)) + 1;
	auto volumeSampleCache = make_unique<bool[]>(nSample * nSample * nSample);

	//TODO replace with proper raycasts
	for (size_t x = 0; x < nSample; x++) {
		for (size_t y = 0; y < nSample; y++) {
			for (size_t z = 0; z < nSample; z++) {
				float dx = (static_cast<float>(x) - static_cast<float>(nSample / 2)) / static_cast<float>(nSample / 2);
				float dy = (static_cast<float>(y) - static_cast<float>(nSample / 2)) / static_cast<float>(nSample / 2);
				float dz = (static_cast<float>(z) - static_cast<float>(nSample / 2)) / static_cast<float>(nSample / 2);
				volumeSampleCache[x * nSample * nSample + y * nSample + z] = dx * dx + dy * dy + dz * dz < 0.9f;
			}
		}
	}

	volumeBitmapData = make_unique<ubyte[]>(n * n * n * 4);
	for (size_t x = 0; x < n; x++) {
		for (size_t y = 0; y < n; y++) {
			for (size_t z = 0; z < n; z++) {
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4] = static_cast<ubyte>(b * 255.0f);
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 1] = static_cast<ubyte>(g * 255.0f);
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 2] = static_cast<ubyte>(r * 255.0f);
				
				float sum = 0.0f;
				for (size_t sx = x * oversampling; sx <= (x + 1) * oversampling; sx++) {
					for (size_t sy = y * oversampling; sy <= (y + 1) * oversampling; sy++) {
						for (size_t sz = z * oversampling; sz <= (z + 1) * oversampling; sz++) {
							if (volumeSampleCache[sx * nSample * nSample + sy * nSample + sz])
								sum += 1.0f;
						}
					}
				}
				
				volumeBitmapData[x * n * n * 4 + y * n * 4 + z * 4 + 3] = static_cast<ubyte>(sum * 255.0f / static_cast<float>((1 << oversampling) * (1 << oversampling) * (1 << oversampling)));
			}
		}
	}

	volumeBitmapHandle = bm_create_3d(32, n, n, n, volumeBitmapData.get());
}

int volumetric_nebula::getVolumeBitmapHandle() const {
	Assertion(volumeBitmapHandle >= 0, "Tried to access volume bitmap without creating it!");
	return volumeBitmapHandle;
}
