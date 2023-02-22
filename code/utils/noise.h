#pragma once

#include "globalincs/vmallocator.h"
#include "utils/RandomRange.h"

namespace util {
namespace noise {


template<size_t dimensions>
class noise {
public:
	static_assert(dimensions > 0, "Noise must have at least one dimension!");
	using coordinates = std::array<float, dimensions>;

	virtual float sample(const coordinates& pos) const = 0;
};


template<size_t dimensions>
class worley : noise<dimensions> {
	SCP_vector<coordinates> points;
	size_t ptPerEdge;
	bool seamless;

public:
	worley(size_t _ptPerEdge, bool _seamless) : points(), ptPerEdge(_ptPerEdge), seamless(_seamless) {
		auto rnd = util::UniformFloatRange(0.0f, 1.0f);

		size_t pointCnt = 1;
		for (size_t i = 0; i < dimensions; i++)
			pointCnt *= ptPerEdge;

		for (size_t i = 0; i < pointCnt; i++) {
			coordinates coords;
			for (size_t j = 0; j < dimensions; j++)
				coords[j] = rnd.next();
			points.emplace_back(std::move(coords));
		}
	}

	float sample(const coordinates& pos) const override {
		float dist = FLT_MAX;

		for (const coordinates& coord : points) {
			float localdist = 0;
			for (size_t i = 0; i < dimensions; i++) {
				float deltaCoord = seamless ? fminf(fabsf(coord[i] - pos[i]), 1.0f - fabsf(coord[i] - pos[i])) : coord[i] - pos[i];
				localdist += deltaCoord * deltaCoord;
			}
			if (localdist < dist)
				dist = localdist;
		}

		//Normalize based on expected distance
		dist = sqrtf(dist) * static_cast<float>(ptPerEdge); 
		CLAMP(dist, 0.0f, 1.0f);
		return dist;
	}
};


}
}