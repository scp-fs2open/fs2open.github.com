#include "volumetrics.h"

#include "bmpman/bmpman.h"
#include "model/model.h"

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

	int modelnum = model_load(hullPof.c_str(), 0, nullptr);

	mc_info mc;

	mc.model_num = modelnum;
	mc.orient = &vmd_identity_matrix;
	mc.pos = &vmd_zero_vector;
	mc.p1 = &vmd_zero_vector;
	//mc.radius = 0.1f;

	mc.flags = MC_CHECK_MODEL /* | MC_CHECK_SPHERELINE*/ | MC_COLLIDE_ALL;

	vec3d bl;
	vm_vec_copy_scale(&bl, &size, -0.5f);

	for (size_t x = 0; x < nSample; x++) {
		for (size_t y = 0; y < nSample; y++) {
			vec3d start = bl;
			start += vec3d{ {static_cast<float>(x) * size.xyz.x / static_cast<float>(n << (oversampling - 1)),
							 static_cast<float>(y) * size.xyz.y / static_cast<float>(n << (oversampling - 1)),
							 0.0f } };
			vec3d end = start;
			end.xyz.z += size.xyz.z;

			mc.p0 = &start;
			mc.p1 = &end;
			mc.hit_points_all.clear();
			mc.hit_submodels_all.clear();
			model_collide(&mc);

			SCP_multiset<size_t> collisionZIndices;
			for(const vec3d& hitpnt : mc.hit_points_all)
				collisionZIndices.emplace(static_cast<size_t>((hitpnt.xyz.z - bl.xyz.z) / size.xyz.z * static_cast<float>(n << (oversampling - 1))));

			size_t hitcnt = 0;
			auto hitpntit = collisionZIndices.cbegin();
			for (size_t z = 0; z < nSample; z++) {
				while (hitpntit != collisionZIndices.cend() && *hitpntit < z) {
					++hitpntit;
					++hitcnt;
				}
				volumeSampleCache[x * nSample * nSample + y * nSample + z] = hitcnt % 2 != 0;
			}
		}
	}

	model_unload(modelnum);

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
