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

struct sexp_container
{
	// meta-character for containers in text replacement, etc.
	static constexpr char DELIM = '&';
	static const SCP_string DELIM_STR;
	// applies to list data, map keys, and map data
	static constexpr int VALUE_MAX_LENGTH = NAME_LENGTH - 1; // leave space for null char
	// leave space for leading/trailing '&' for container multidimensionality
	static constexpr int NAME_MAX_LENGTH = VALUE_MAX_LENGTH - 2;

	SCP_string container_name;
	ContainerType type = ContainerType::LIST | ContainerType::STRING_DATA;
	int opf_type = OPF_ANYTHING;

	SCP_list<SCP_string> list_data;
	SCP_unordered_map<SCP_string, SCP_string> map_data;

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

	bool name_matches(const sexp_container &container) const;
	bool empty() const;
	int size() const;

	// get type without persistence flags
	ContainerType get_non_persistent_type() const;
	// matching is performed only on non-persistence flags
	bool type_matches(const sexp_container &container) const;
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

	const char *name;
	const Modifier modifier;
};

using ListModifier = list_modifier::Modifier;

ListModifier get_list_modifier(const char *text, bool accept_prefix = false);
const char *get_list_modifier_name(ListModifier modifier);

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
