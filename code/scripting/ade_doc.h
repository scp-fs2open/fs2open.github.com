#pragma once

#include "globalincs/pstypes.h"

namespace scripting {

enum class ade_type_info_type {
	Empty,
	Simple,
	Tuple,
	Array,
	Map,
	Iterator,
	Alternative,
};

class ade_type_array;
class ade_type_map;
class ade_type_iterator;
class ade_type_alternative;

/**
 * @brief A definition of a type used in the ADE system
 */
class ade_type_info {
	ade_type_info_type _type = ade_type_info_type::Empty;

	SCP_string _simple_name;

	SCP_vector<ade_type_info> _elements;

  public:
	ade_type_info() = default;
	/*implicit*/ ade_type_info(const char* single_type); // NOLINT(hicpp-explicit-conversions)
	/*implicit*/ ade_type_info(std::initializer_list<ade_type_info> tuple_types);
	/*implicit*/ ade_type_info(const ade_type_array& listType);
	/*implicit*/ ade_type_info(const ade_type_map& listType);
	/*implicit*/ ade_type_info(const ade_type_iterator& iteratorType);
	/*implicit*/ ade_type_info(const ade_type_alternative& iteratorType);

	ade_type_info_type getType() const;

	bool isEmpty() const;

	bool isSimple() const;

	bool isTuple() const;

	bool isArray() const;

	const char* getSimpleName() const;

	const ade_type_info& arrayType() const;

	const SCP_vector<ade_type_info>& elements() const;
};

class ade_type_array {
	ade_type_info _element_type;

  public:
	explicit ade_type_array(ade_type_info elementType);

	const ade_type_info& getElementType() const;
};

class ade_type_map {
	ade_type_info _key_type;
	ade_type_info _value_type;

  public:
	explicit ade_type_map(ade_type_info keyType, ade_type_info valueType);

	const ade_type_info& getKeyType() const;

	const ade_type_info& getValueType() const;
};

class ade_type_iterator {
	ade_type_info _element_type;

  public:
	explicit ade_type_iterator(ade_type_info elementType);

	const ade_type_info& getElementType() const;
};

class ade_type_alternative {
	SCP_vector<ade_type_info> _elements;

  public:
	explicit ade_type_alternative(SCP_vector<ade_type_info> elements);

	const SCP_vector<ade_type_info>& getElementTypes() const;
};

struct argument_def {
	ade_type_info type;
	SCP_string name;
	SCP_string def_val;
	bool optional = false;
};

class argument_list_parser {
  public:
	bool parse(const SCP_string& argumentList);

	const SCP_vector<scripting::argument_def>& getArgList() const;

  private:
	SCP_vector<argument_def> _argList;
};

} // namespace scripting
