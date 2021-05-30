/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#pragma once

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "parse/sexp.h"

// must inherit from int because of persistence (e.g., csg_write_int())
enum ContainerType : int {
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

inline constexpr bool operator&(ContainerType lhs, ContainerType rhs) {
	using ContainerTypeInt = std::underlying_type<ContainerType>::type;
	return static_cast<ContainerTypeInt>(lhs) & static_cast<ContainerTypeInt>(rhs);
}

inline constexpr ContainerType operator|(ContainerType lhs, ContainerType rhs) {
	using ContainerTypeInt = std::underlying_type<ContainerType>::type;
	return static_cast<ContainerType>(static_cast<ContainerTypeInt>(lhs) |
		static_cast<ContainerTypeInt>(rhs));
}

inline void operator|=(ContainerType lhs, ContainerType rhs) { lhs = lhs | rhs; }

struct sexp_container
{
	// meta-character for containers in text replacement, etc.
	static constexpr char DELIM = '&';
	// applies to list data, map keys, and map data
	static constexpr int VALUE_MAX_LENGTH = NAME_LENGTH - 1; // leave space for null char
	// leave space for leading/trailing '&' for container multidimensionality
	static constexpr int NAME_MAX_LENGTH = VALUE_MAX_LENGTH - 2;

	SCP_string container_name;
	ContainerType type = LIST | STRING_DATA;
	int opf_type = OPF_ANYTHING;

	SCP_list<SCP_string> list_data;
	SCP_unordered_map<SCP_string, SCP_string> map_data;

	inline bool is_list() const
	{
		return type & LIST;
	}

	inline bool is_map() const
	{
		return type & MAP;
	}

	inline bool is_eternal() const
	{
		return type & SAVE_TO_PLAYER_FILE;
	}

	inline bool is_persistent() const
	{
		return type & (SAVE_ON_MISSION_PROGRESS | SAVE_ON_MISSION_CLOSE);
	}

	bool empty() const;
	int size() const;
};

struct list_modifier {
	enum class Modifier
	{
		GET_FIRST,
		GET_LAST,
		REMOVE_FIRST,
		REMOVE_LAST,
		GET_RANDOM,
		REMOVE_RANDOM,
		AT_INDEX,
		INVALID
	};

	const char *name;
	const Modifier modifier;
};

using ListModifier = list_modifier::Modifier;

// management functions
void init_sexp_containers();
void update_sexp_containers(SCP_vector<sexp_container>& containers);

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
