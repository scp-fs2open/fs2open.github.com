
#include "libs/jansson.h"

namespace {
int json_write_callback(const char *buffer, size_t size, void *data) {
	CFILE* cfp = (CFILE*)data;

	if ((size_t)cfwrite(buffer, 1, (int)size, cfp) != size) {
		return -1; // Error
	} else {
		return 0; // Success
	}
}

size_t json_read_callback(void *buffer, size_t buflen, void *data) {
	auto fp = (CFILE*)data;

	return (size_t)cfread(buffer, 1, (int)buflen, fp);
}
}

int json_dump_cfile(const json_t* json, CFILE* file, size_t flags) {
	return json_dump_callback(json, json_write_callback, file, flags);
}
json_t* json_load_cfile(CFILE* cfile, size_t flags, json_error_t* error) {
	return json_load_callback(json_read_callback, cfile, flags, error);
}
