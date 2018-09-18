#pragma once

#include "cfile/cfile.h"
#include <jansson.h>
#include <stdexcept>

int json_dump_cfile(const json_t *json, CFILE* file, size_t flags);

json_t *json_load_cfile(CFILE* cfile, size_t flags, json_error_t *error);

SCP_string json_dump_string(const json_t* json, size_t flags);

SCP_string json_dump_string_new(json_t* json, size_t flagse);

// This allows using std::unique_ptr with json_t values
namespace std {
template <>
class default_delete<json_t> {
  public:
	void operator()(json_t* ptr) const
	{
		if (ptr) {
			json_decref(ptr);
		}
	}
};
}; // namespace std

class json_exception : public std::runtime_error {
  public:
	explicit json_exception(const json_error_t& error);
};
