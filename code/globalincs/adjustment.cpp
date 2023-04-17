// clang-format on
#include "globalincs/adjustment.h"

#include "globalincs/pstypes.h"

//*************************************************
//			adjustment funcs
//*************************************************

/**
 * @brief Processes an input according to the user's configuration and returns the result.
 */
float adjustment::handle(float input)
{
	if (only_positive && input < 0.0f)
		input = base;
	if (has_multiplier)
		input *= multipier;
	// handling adjust after multiplier makes it possible to multiply by 0 and still adjust.
	if (has_adjust)
		input += adjust;
	if (has_minimum)
		input = MAX(input, minimum);
	if (has_maximum)
		input = MIN(input, maximum);
	return input;
}

void adjustment::reset()
{
	base = 1.0f;
	has_adjust = false;
	has_minimum = false;
	has_maximum = false;
	has_multiplier = false;
	only_positive = true;
}

void adjustment::set_adjust(float in)
{
	has_adjust = true;
	adjust = in;
}

void adjustment::set_multiplier(float in)
{
	has_multiplier = true;
	multipier = in;
}

void adjustment::stack_multiplier(float in)
{
	if (has_multiplier) {
		multipier *= in;
	} else {
		multipier = in;
		has_multiplier = true;
	}
}

void adjustment::set_maximum(float in)
{
	has_maximum = true;
	maximum = in;
}

void adjustment::set_minimum(float in)
{
	has_minimum = true;
	minimum = in;
}

void adjustment::stack_minimum(float in)
{
	if (has_minimum) {
		minimum = MAX(minimum, in);
	} else {
		minimum = in;
		has_minimum = true;
	}
}
bool adjustment::read_adjust(float* out) const
{
	*out = adjust;
	return has_adjust;
}
bool adjustment::read_multiplier(float* out) const
{
	*out = multipier;
	return has_multiplier;
}
/**
 * @brief for use during the parsing of a light profile to attempt to read in an LPV
 *
 * @param filename for error reporting primairly
 * @param valuename Text of the field
 * @param profile_name The name of the profile being parsed, for error reporting
 * @param value_target The profile object parsing into
 * @param required If false it is an error for this to not find a value.
 * @return true if a succesful parse, false otherwise
 */
bool adjustment::parse(const char* filename,
	const char* valuename,
	const SCP_string& profile_name,
	adjustment* value_target,
	bool required)
{
	if (optional_string(valuename)) {
		bool parsed = true;
		int parses = 0;
		while (parsed) {
			parsed = false;
			if (parse_optional_float_into("+default:", &value_target->base)) {
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+maximum:", &value_target->maximum)) {
				value_target->has_maximum = true;
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+minimum:", &value_target->minimum)) {
				value_target->has_minimum = true;
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+multiplier:", &value_target->multipier)) {
				value_target->has_multiplier = true;
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+adjust:", &value_target->adjust)) {
				value_target->has_adjust = true;
				parsed = true;
				parses++;
			}
		}
		if (parses == 0) {
			Warning(LOCATION,
				"Adjustment config '%s' in file '%s' profile '%s' parsed but set no properties, possible "
				"malformed table.",
				valuename,
				filename,
				profile_name.c_str());
		} else if (parses > 5) {
			Warning(LOCATION,
				"Adjustment config '%s' in file '%s' section '%s' parsed too many properties, possible "
				"malformed table.",
				valuename,
				filename,
				profile_name.c_str());
		}
		return true;

	} else if (required) {
		Error(LOCATION,
			"Expected adjustment config value '%s' in file '%s' section '%s' not found",
			valuename,
			filename,
			profile_name.c_str());
	}
	return false;
}

adjustment::adjustment(const adjustment& rhs){
	*this = rhs;
	/*
	has_adjust = rhs.has_adjust;
	adjust = rhs.adjust;
	has_multiplier = rhs.has_multiplier;
	multipier = rhs.multipier;
	has_minimum = rhs.has_minimum;
	minimum = rhs.minimum;
	has_maximum = rhs.has_maximum;
	maximum = rhs.maximum;*/
}
adjustment& adjustment::operator= (const adjustment& rhs){
	has_adjust = rhs.has_adjust;
	adjust = rhs.adjust;
	has_multiplier = rhs.has_multiplier;
	multipier = rhs.multipier;
	has_minimum = rhs.has_minimum;
	minimum = rhs.minimum;
	has_maximum = rhs.has_maximum;
	maximum = rhs.maximum;
	return *this;
}
/*
adjustment::adjustment(const adjustment& l, const adjustment& r, const float& fact) {
	float f = fact;
	CLAMP(f, 0.0f, 1.0f);
	//TODO: implement this shit


}*/