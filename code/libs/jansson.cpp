
#include "libs/jansson.h"
#include "jansson.h"
#include "parse/parselo.h"

namespace {
int json_write_callback(const char *buffer, size_t size, void *data) {
	auto* cfp = (CFILE*)data;

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
SCP_string format_error(const json_error_t& err)
{
	SCP_string out;
	sprintf(out, "%s(%d:%d): %s", err.source, err.line, err.column, err.text);
	return out;
}
}

int json_dump_cfile(const json_t* json, CFILE* file, size_t flags) {
	return json_dump_callback(json, json_write_callback, file, flags);
}
json_t* json_load_cfile(CFILE* cfile, size_t flags, json_error_t* error) {
	return json_load_callback(json_read_callback, cfile, flags, error);
}
SCP_string json_dump_string(const json_t* json, size_t flags)
{
	auto buf = json_dumps(json, flags);

	if (buf == nullptr) {
		return SCP_string();
	}

	SCP_string out(buf);
	free(buf);

	return out;
}
SCP_string json_dump_string_new(json_t* json, size_t flags) {
	auto str = json_dump_string(json, flags);
	json_decref(json);
	return str;
}

json_exception::json_exception(const json_error_t& error) : std::runtime_error(format_error(error)) {}
