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

namespace json {
namespace detail {

class object_iterator {
  public:
	using value_type = std::tuple<const char*, json_t*>;
	using reference = value_type&;
	using pointer = value_type*;

	object_iterator(json_t* obj, void* iter);

	object_iterator& operator++();
	value_type operator*() const;

	friend bool operator==(const object_iterator& lhs, const object_iterator& rhs);
	friend bool operator!=(const object_iterator& lhs, const object_iterator& rhs);

  private:
	json_t* m_obj = nullptr;
	void* m_iter = nullptr;
};

class array_iterator {
  public:
	using value_type = json_t*;
	using reference = value_type&;
	using pointer = value_type*;

	array_iterator(json_t* obj, size_t index);

	array_iterator& operator++();
	value_type operator*() const;

	friend bool operator==(const array_iterator& lhs, const array_iterator& rhs);
	friend bool operator!=(const array_iterator& lhs, const array_iterator& rhs);
  private:
	json_t* m_array = nullptr;
	size_t m_index = 0;
};

} // namespace detail

class object_range {
  public:
	object_range(json_t* obj);

	detail::object_iterator begin();

	detail::object_iterator end();

  private:
	json_t* m_obj = nullptr;
};

class array_range {
  public:
	array_range(json_t* array);

	detail::array_iterator begin();

	detail::array_iterator end();

  private:
	json_t* m_array = nullptr;
};

} // namespace json
