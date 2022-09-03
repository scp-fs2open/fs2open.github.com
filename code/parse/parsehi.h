#pragma once
#include "parse/parselo.h"
#include "graphics/color.h"

//parsehi is intended for higher level, frequently used combinations of parselo functions
//and also for collection of standardized parsing of complex types where appropriate

extern bool parse_optional_float_into(const SCP_string& field_name, float* value_target);

extern bool parse_optional_bool_into(const SCP_string& field_name, bool* value_target);

extern bool parse_optional_color3i_into(const SCP_string &field_name, hdr_color *out_color);