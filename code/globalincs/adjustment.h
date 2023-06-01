// clang-format on
#pragma once

#include "globalincs/pstypes.h"

#include "parse/parsehi.h"

/**
 * @brief To hold configuration of, and handle application of, modifiers to numeric values.
 *
 * Was originally made for lighting_profiles, but was extracted into it's own thing for potential reuse.
 */
struct adjustment {
	float base;
	bool only_positive = true;

	float handle(float input) const;
	void reset();
	void set_adjust(const float in);
	void set_multiplier(const float in);
	void stack_multiplier(const float in);
	void set_maximum(const float in);
	void set_minimum(const float in);
	void stack_minimum(const float in);

	static bool parse(const char* filename,
		const char* valuename,
		const SCP_string& profile_name,
		adjustment* value_target,
		bool required = false);
	bool read_multiplier(float* out) const;
	bool read_adjust(float* out) const;

	adjustment() = default ;
	adjustment(const adjustment& rhs);
	//adjustment(const adjustment& lhs, const adjustment& rhs, const float& fact);
	adjustment& operator= (const adjustment& rhs);

private:
	bool has_adjust = false;
	float adjust;
	bool has_multiplier=false;
	float multipier;
	bool has_minimum = false;
	float minimum;
	bool has_maximum = false;
	float maximum;
};