#include "parsehi.h"
#include "parselo.h"

/**
 * @brief Parses an optional table value into a field if the name is found
 *
 * @param field_name The name of the table field
 * @param value_target Pointer to the variable to assign any parsed value to
 *
 * @return True if a value was parsed, false if not
 */
bool parse_optional_float_into(const SCP_string& field_name, float* value_target)
{
	if (optional_string(field_name.c_str())) {
		stuff_float(value_target);
		return true;
	}
	return false;
}
