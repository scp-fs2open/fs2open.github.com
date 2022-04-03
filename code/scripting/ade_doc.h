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
	Function,
	Generic,
	Varargs,
};

class ade_type_array;
class ade_type_map;
class ade_type_iterator;
class ade_type_alternative;
class ade_type_function;
class ade_type_generic;
class ade_type_varargs;
class ade_type_tuple;

/**
 * @brief A definition of a type used in the ADE system
 */
class ade_type_info {
	ade_type_info_type _type = ade_type_info_type::Empty;

	SCP_string _identifier;
	SCP_vector<ade_type_info> _elements;

	SCP_string _name;
  public:
	ade_type_info() = default;
	/*implicit*/ ade_type_info(const char* single_type); // NOLINT(hicpp-explicit-conversions)
	/*implicit*/ ade_type_info(SCP_string single_type);
	/*implicit*/ ade_type_info(const ade_type_tuple& tupleType);
	/*implicit*/ ade_type_info(const ade_type_array& listType);
	/*implicit*/ ade_type_info(const ade_type_map& listType);
	/*implicit*/ ade_type_info(const ade_type_iterator& iteratorType);
	/*implicit*/ ade_type_info(const ade_type_alternative& alternativeType);
	/*implicit*/ ade_type_info(const ade_type_function& functionType);
	/*implicit*/ ade_type_info(const ade_type_generic& genericType);
	/*implicit*/ ade_type_info(const ade_type_varargs& genericType);

	ade_type_info(const ade_type_info&) = default;
	ade_type_info& operator=(const ade_type_info&) = default;

	ade_type_info(ade_type_info&&) noexcept;
	ade_type_info& operator=(ade_type_info&&) noexcept;

	ade_type_info_type getType() const;

	const SCP_string& getName() const;
	void setName(const SCP_string& name);

	bool isEmpty() const;

	bool isSimple() const;

	bool isTuple() const;

	bool isArray() const;

	bool isAlternative() const;

	const char* getIdentifier() const;

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

class ade_type_tuple {
	SCP_vector<ade_type_info> _elements;

  public:
	explicit ade_type_tuple(SCP_vector<ade_type_info> elements);

	const SCP_vector<ade_type_info>& getElementTypes() const;
};

class ade_type_function {
	ade_type_info _returnType;
	SCP_vector<ade_type_info> _argumentTypes;

  public:
	explicit ade_type_function(ade_type_info returnType, SCP_vector<ade_type_info> argumentTypes);

	const ade_type_info& getReturnType() const;
	const SCP_vector<scripting::ade_type_info>& getArgumentTypes() const;
};

class ade_type_generic {
	ade_type_info _baseType;
	SCP_vector<ade_type_info> _genericTypes;

  public:
	ade_type_generic(ade_type_info baseType, SCP_vector<ade_type_info> genericTypes);

	const ade_type_info& getBaseType() const;
	const SCP_vector<scripting::ade_type_info>& getGenericTypes() const;
};

class ade_type_varargs {
	ade_type_info _baseType;

  public:
	explicit ade_type_varargs(ade_type_info baseType);

	const ade_type_info& getBaseType() const;
};

class ade_overload_list {
	SCP_vector<const char*> _arg_lists;

	void fixNullPointers();
  public:
	/*implicit*/ ade_overload_list();
	/*implicit*/ ade_overload_list(const char* arglist);
	/*implicit*/ ade_overload_list(std::initializer_list<const char*> overloads);

	const SCP_vector<const char*>& overloads();
};

} // namespace scripting
