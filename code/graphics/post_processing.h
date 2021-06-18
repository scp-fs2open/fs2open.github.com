#pragma once

#include "globalincs/pstypes.h"

#include "math/vecmat.h"

namespace graphics {

enum class PostEffectUniformType {
	Invalid,
	NoiseAmount,
	Saturation,
	Brightness,
	Contrast,
	FilmGrain,
	TvStripes,
	Cutoff,
	Tint,
	Dither,
};

struct post_effect_t {
	SCP_string name;
	PostEffectUniformType uniform_type = PostEffectUniformType::Invalid;
	SCP_string define_name;

	float intensity{0.0f};
	float default_intensity{0.0f};
	float div{1.0f};
	float add{0.0f};

	vec3d rgb = vmd_zero_vector;

	bool always_on{false};
};

struct lightshaft_parameters {
	bool on = true; // enabled by default
	float density = 0.5f;
	float weight = 0.02f;
	float falloff = 1.0f;
	float intensity = 0.5f;
	float cpintensity = 0.5f * 50 * 0.02f;
	int samplenum = 50;
};

class PostProcessingManager {
  public:
	bool parse_table();

	void clear();

	const SCP_vector<graphics::post_effect_t>& getPostEffects() const;
	SCP_vector<graphics::post_effect_t>& getPostEffects();

	const lightshaft_parameters& getLightshaftParams() const;
	lightshaft_parameters& getLightshaftParams();

	bool bloomShadersOk() const;
	void setBloomShadersOk(bool ok);

  private:
	SCP_vector<post_effect_t> m_postEffects;

	lightshaft_parameters m_lightshaftParams;
	bool m_bloomShadersOk = true;
};

} // namespace graphics

bool gr_lightshafts_enabled();
int gr_bloom_intensity();
// used by lab
void gr_set_bloom_intensity(int intensity);
