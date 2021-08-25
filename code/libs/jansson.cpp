
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
SCP_string json_dump_string_new(json_t* json, size_t flags)
{
	auto str = json_dump_string(json, flags);
	json_decref(json);
	return str;
}

json_exception::json_exception(const json_error_t& error) : std::runtime_error(format_error(error)) {}

namespace json {
namespace detail {
object_iterator::object_iterator(json_t* obj, void* iter) : m_obj(obj), m_iter(iter) {}
object_iterator& object_iterator::operator++()
{
	Assertion(m_iter != nullptr, "Tried to increment iterator after it was at the end!");

	m_iter = json_object_iter_next(m_obj, m_iter);
	return *this;
}
object_iterator::value_type object_iterator::operator*() const
{
	return object_iterator::value_type{json_object_iter_key(m_iter), json_object_iter_value(m_iter)};
}

bool operator==(const object_iterator& lhs, const object_iterator& rhs)
{
	return lhs.m_obj == rhs.m_obj && lhs.m_iter == rhs.m_iter;
}
bool operator!=(const object_iterator& lhs, const object_iterator& rhs)
{
	return !(rhs == lhs);
}
array_iterator::array_iterator(json_t* array, size_t index) : m_array(array), m_index(index) {}
bool operator==(const array_iterator& lhs, const array_iterator& rhs)
{
	return lhs.m_array == rhs.m_array && lhs.m_index == rhs.m_index;
}
bool operator!=(const array_iterator& lhs, const array_iterator& rhs)
{
	return !(rhs == lhs);
}
array_iterator& array_iterator::operator++()
{
	++m_index;
	return *this;
}
array_iterator::value_type array_iterator::operator*() const
{
	Assertion(m_index < json_array_size(m_array), "Invalid index encountered!");
	return json_array_get(m_array, m_index);
}
} // namespace detail

object_range::object_range(json_t* obj) : m_obj(obj)
{
	Assertion(json_typeof(m_obj) == JSON_OBJECT, "Invalid object type %d for json value.", json_typeof(m_obj));
}
detail::object_iterator object_range::begin()
{
	// Need to suppress the warning here since applying the requested change breaks on other compilers
	return detail::object_iterator(m_obj, json_object_iter(m_obj)); // NOLINT(modernize-return-braced-init-list)
}
detail::object_iterator object_range::end()
{
	return {m_obj, nullptr};
}

array_range::array_range(json_t* array) : m_array(array)
{
	Assertion(json_typeof(m_array) == JSON_ARRAY, "Invalid object type %d for json value.", json_typeof(m_array));
}
detail::array_iterator array_range::begin()
{
	return {m_array, 0};
}
detail::array_iterator array_range::end()
{
	return {m_array, json_array_size(m_array)};
}
} // namespace json
