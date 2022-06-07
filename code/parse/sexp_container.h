/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#pragma once

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "parse/sexp.h"

// enum class inherits from int by default
enum class ContainerType {
	NONE = 0x00,
	LIST = 0x01,
	MAP = 0x02,
	STRICTLY_TYPED_KEYS = 0x04,
	STRICTLY_TYPED_DATA = 0x08,
	NUMBER_DATA = 0x10,
	STRING_DATA = 0x20,
	NUMBER_KEYS = 0x40,
	STRING_KEYS = 0x80,

	// persistence
	SAVE_ON_MISSION_CLOSE = 0x08000000,
	SAVE_ON_MISSION_PROGRESS = 0x10000000,
	NETWORK = 0x20000000,
	SAVE_TO_PLAYER_FILE = 0x40000000,
};

using ContainerTypeInt = std::underlying_type<ContainerType>::type;

inline constexpr ContainerType operator&(ContainerType lhs, ContainerType rhs)
{
	return (ContainerType)((ContainerTypeInt)lhs & (ContainerTypeInt)rhs);
}

inline ContainerType &operator&=(ContainerType &lhs, ContainerType rhs)
{
	lhs = (ContainerType)((ContainerTypeInt)lhs & (ContainerTypeInt)rhs);
	return lhs;
}

inline constexpr ContainerType operator|(ContainerType lhs, ContainerType rhs)
{
	return (ContainerType)((ContainerTypeInt)lhs | (ContainerTypeInt)rhs);
}

inline ContainerType &operator|=(ContainerType &lhs, ContainerType rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

inline constexpr ContainerType operator^(ContainerType lhs, ContainerType rhs)
{
	return (ContainerType)((ContainerTypeInt)lhs ^ (ContainerTypeInt)rhs);
}

inline ContainerType &operator^=(ContainerType &lhs, ContainerType rhs)
{
	lhs = lhs ^ rhs;
	return lhs;
}

inline constexpr ContainerType operator~(ContainerType ct)
{
	return (ContainerType)(~(ContainerTypeInt)ct);
}

inline constexpr bool any(ContainerType ct)
{
	return ct != ContainerType::NONE;
}

inline constexpr bool none(ContainerType ct)
{
	return ct == ContainerType::NONE;
}

// custom hash function to standardize map container behavior
// to match FREDders' expectations of platform independence
struct map_container_hash
{
	// sizeof(uint32_t) will always be <= sizeof(size_t)
	uint32_t operator()(const SCP_string &str) const;
};

struct list_modifier {
	enum class Modifier
	{
		INVALID = 0,
		GET_FIRST,
		GET_LAST,
		REMOVE_FIRST,
		REMOVE_LAST,
		GET_RANDOM,
		REMOVE_RANDOM,
		AT_INDEX
	};

	const char* name;
	const Modifier modifier;
};

using ListModifier = list_modifier::Modifier;

ListModifier get_list_modifier(const char *text, bool accept_prefix = false);
const char *get_list_modifier_name(ListModifier modifier);

struct sexp_container
{
	// meta-character for containers in text replacement, etc.
	static constexpr char DELIM = '&';
	static const SCP_string DELIM_STR;
	static const SCP_string NAME_NODE_PREFIX;
	// applies to list data, map keys, and map data
	static constexpr int VALUE_MAX_LENGTH = NAME_LENGTH - 1; // leave space for null char
	// leave space for leading/trailing '&' for container multidimensionality
	static constexpr int NAME_MAX_LENGTH = VALUE_MAX_LENGTH - 2;

	SCP_string container_name;
	ContainerType type = ContainerType::LIST | ContainerType::STRING_DATA;
	int opf_type = OPF_STRING;
	bool used_in_special_arg = false;

	SCP_list<SCP_string> list_data;
	SCP_unordered_map<SCP_string, SCP_string, map_container_hash> map_data;

	bool operator==(const sexp_container &sc) const;

	inline bool is_list() const
	{
		return any(type & ContainerType::LIST);
	}

	inline bool is_map() const
	{
		return any(type & ContainerType::MAP);
	}

	inline bool is_eternal() const
	{
		return any(type & ContainerType::SAVE_TO_PLAYER_FILE);
	}

	inline bool is_persistent() const
	{
		return any(type & (ContainerType::SAVE_ON_MISSION_PROGRESS | ContainerType::SAVE_ON_MISSION_CLOSE));
	}

	ContainerType get_data_type() const;

	bool name_matches(const sexp_container &container) const;
	bool empty() const;
	int size() const;
	// for detecting attempted concurrent modification
	bool is_being_used_in_special_arg() const
	{
		return used_in_special_arg;
	}

	// get type without persistence flags
	ContainerType get_non_persistent_type() const;
	// matching is performed only on non-persistence flags
	bool type_matches(const sexp_container &container) const;

	// checks whether accessed via strings
	// meaning string data if a list or string keys if a map
	// but map data doesn't have to be strings
	bool is_of_string_type() const;
	// returns data for list container or key for map container
	const SCP_string &get_value_at_index(int index) const;

	// list modifier operations
	SCP_string apply_list_modifier(ListModifier modifier, int data_index = -1);

private:
	SCP_string list_get_first(bool remove);
	SCP_string list_get_last(bool remove);
	SCP_string list_get_random(bool remove);
	SCP_string list_get_at(int index);
	SCP_string list_apply_iterator(SCP_list<SCP_string>::iterator list_it, const char *pos_str, bool remove);
};

// management functions
void init_sexp_containers();
void update_sexp_containers(SCP_vector<sexp_container> &containers,
	const SCP_unordered_map<SCP_string, SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to>
		&renamed_containers);

// parsing functions
void stuff_sexp_list_containers();
void stuff_sexp_map_containers();

// retrieval functions
const SCP_vector<sexp_container> &get_all_sexp_containers();
const SCP_vector<list_modifier> &get_all_list_modifiers();

/**
 * Return pointer to a sexp_container by its name, or nullptr if not found
 */
sexp_container *get_sexp_container(const char *name);

// text replacement
bool sexp_container_replace_refs_with_values(SCP_string &text);
bool sexp_container_replace_refs_with_values(char *text, size_t max_len);

const char *sexp_container_CTEXT(int node);

// persistence
bool sexp_container_has_persistent_non_eternal_containers();

// SEXPs
int sexp_container_eval_status_sexp(int op_num, int node);
int sexp_container_eval_change_sexp(int op_num, int node);

// collect argument values for the "for-container" SEXPs
// if just_count is true, then argument_vector is left unmodified
// returns the number of arguments
int sexp_container_collect_data_arguments(int node,
	SCP_vector<std::pair<const char*, int>> &argument_vector,
	bool just_count = false);
int sexp_container_collect_map_key_arguments(int node,
	SCP_vector<std::pair<const char*, int>> &argument_vector,
	bool just_count = false);

// version of query_sexp_args_count() when we need to select an element within a range
// and containers are valid arguments
int sexp_container_query_sexp_args_count(int node,
	SCP_vector<int> &cumulative_arg_countss,
	bool only_valid_args = false);

// mark containers when they're being used as SEXP special arguments
void sexp_container_set_special_arg_status(int arg_handler_node, bool status);
