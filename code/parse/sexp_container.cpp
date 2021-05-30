/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <algorithm>
#include <functional>
#include <iterator>

#include "globalincs/toolchain.h"
#include "parse/generic_log.h"
#include "parse/parselo.h"
#include "parse/sexp_container.h"

static SCP_vector<sexp_container> Sexp_containers;

const SCP_vector<sexp_container> &get_all_sexp_containers()
{
	return Sexp_containers;
}

static const SCP_vector<list_modifier> List_modifiers = {
	{ "Get_First",			ListModifier::GET_FIRST     },
	{ "Get_Last",			ListModifier::GET_LAST      },
	{ "Remove_First",		ListModifier::REMOVE_FIRST  },
	{ "Remove_Last",		ListModifier::REMOVE_LAST   },
	{ "Get_Random",			ListModifier::GET_RANDOM    },
	{ "Remove_Random",		ListModifier::REMOVE_RANDOM },
	{ "At",					ListModifier::AT_INDEX      }
};

const SCP_vector<list_modifier>& get_all_list_modifiers()
{
	return List_modifiers;
}

// sexp_container functions

bool sexp_container::empty() const
{
	return size() == 0;
}

int sexp_container::size() const
{
	if (is_list()) {
		return (int)list_data.size();
	} else if (is_map()) {
		return (int)map_data.size();
	} else {
		UNREACHABLE("Unknown container type %d", (int)type);
		return 0;
	}
}

// sexp_container.h functions

/**
 * Clear the SEXP Containers and related data
 */
void init_sexp_containers()
{
	Sexp_containers.clear();
}

void update_sexp_containers(SCP_vector<sexp_container> &containers)
{
	Sexp_containers = std::move(containers);
}

bool stuff_one_generic_sexp_container(SCP_string &name, ContainerType &type, int &opf_type, SCP_vector<SCP_string> &data)
{
	bool valid = true; // try to skip past bad containers by not returning until end of function
	SCP_string temp_type_string;

	data.clear();

	required_string("$Name:");
	stuff_string(name, F_NAME);

	if (name.empty()) {
		Warning(LOCATION, "SEXP Container with empty name found");
		log_printf(LOGFILE_EVENT_LOG, "SEXP Container with empty name found");
		valid = false;
	} else if (name.length() > sexp_container::NAME_MAX_LENGTH) {
		Warning(LOCATION,
			"SEXP Container name %s is longer than limit %u",
			name.c_str(),
			sexp_container::NAME_MAX_LENGTH);
		log_printf(LOGFILE_EVENT_LOG, "SEXP Container name %s is longer than limit %i",
			name.c_str(),
			sexp_container::NAME_MAX_LENGTH);
		valid = false;
	}

	required_string("$Data Type:");
	stuff_string(temp_type_string, F_NAME);
	if (!strcmp(temp_type_string.c_str(), "Number")) {
		type |= NUMBER_DATA;
		opf_type = OPF_NUMBER;
	} else if (!strcmp(temp_type_string.c_str(), "String")) {
		type |= STRING_DATA;
		opf_type = OPF_ANYTHING;
	} else {
		Warning(LOCATION, "Unknown SEXP Container type %s found", temp_type_string.c_str());
		log_printf(LOGFILE_EVENT_LOG, "Unknown SEXP Container type %s found", temp_type_string.c_str());
		valid = false;
	}

	if (optional_string("$Key Type:")) {
		Assertion((type & MAP), "$Key Type: found for container which doesn't use keys!");

		stuff_string(temp_type_string, F_NAME);
		if (!strcmp(temp_type_string.c_str(), "Number")) {
			type |= NUMBER_KEYS;
		} else if (!strcmp(temp_type_string.c_str(), "String")) {
			type |= STRING_KEYS;
		} else {
			Warning(LOCATION, "Unknown SEXP Container type %s found", temp_type_string.c_str());
			log_printf(LOGFILE_EVENT_LOG, "Unknown SEXP Container type %s found", temp_type_string.c_str());
			valid = false;
		}
	}

	if (optional_string("+Strictly Typed Keys")) {
		Assertion((type & MAP), "+Strictly Typed Keys found for container which doesn't use keys!");
		Warning(LOCATION, "Container %s is marked for strictly typed keys, which are not yet supported.", name.c_str());
		type |= STRICTLY_TYPED_KEYS;
	}

	if (optional_string("+Strictly Typed Data")) {
		Warning(LOCATION, "Container %s is marked for strictly typed data, which are not yet supported.", name.c_str());
		type |= STRICTLY_TYPED_DATA;
	}

	required_string("$Data:");
	stuff_string_list(data);

	if (optional_string("+Network Container")) {
		Warning(LOCATION, "Container %s is marked as a network container, which is not yet supported.", name.c_str());
		type |= NETWORK;
	}

	if (optional_string("+Eternal")) {
		type |= SAVE_TO_PLAYER_FILE;
	}

	// campaign-persistent
	if (optional_string("+Save On Mission Progress")) {
		type |= SAVE_ON_MISSION_PROGRESS;
	}

	// player-persistent
	if (optional_string("+Save On Mission Close")) {
		Assert(!(type & SAVE_ON_MISSION_PROGRESS));
		type |= SAVE_ON_MISSION_CLOSE;
	}

	Assert(!(type & SAVE_TO_PLAYER_FILE) ||
		((type & SAVE_ON_MISSION_PROGRESS) ^ (type & SAVE_ON_MISSION_CLOSE)));

	return valid;
}

/**
 * Stuffs sexp list type containers
 */

void stuff_sexp_list_containers()
{
	SCP_vector<SCP_string>	parsed_data;

	while (required_string_either("$End Lists", "$Name:")) {
		Sexp_containers.emplace_back();
		auto &new_list = Sexp_containers.back();

		new_list.type = LIST;
		if (stuff_one_generic_sexp_container(new_list.container_name, new_list.type, new_list.opf_type, parsed_data)) {
			std::copy(parsed_data.begin(), parsed_data.end(), back_inserter(new_list.list_data));
		} else {
			Sexp_containers.pop_back();
		}
	}
}

/**
 * Stuffs sexp map type containers
 */
void stuff_sexp_map_containers()
{
	SCP_vector<SCP_string>	parsed_data;

	while (required_string_either("$End Maps", "$Name:")) {
		Sexp_containers.emplace_back();
		auto& new_map = Sexp_containers.back();

		new_map.type = MAP;
		if (stuff_one_generic_sexp_container(new_map.container_name, new_map.type, new_map.opf_type, parsed_data)) {
			if (parsed_data.size() % 2 != 0) {
				Warning(LOCATION,
					"Data in the SEXP Map container is corrupt. Must be an even number of entries. Instead have %d",
					(int)parsed_data.size());
				log_printf(LOGFILE_EVENT_LOG,
					"Data in the SEXP Map container is corrupt. Must be an even number of entries. Instead have %d",
					(int)parsed_data.size());
				Sexp_containers.pop_back();
			} else {
				for (int i = 0; i < (int)parsed_data.size(); i += 2) {
					new_map.map_data.emplace(parsed_data[i], parsed_data[i + 1]);
				}
			}
		} else {
			Sexp_containers.pop_back();
		}
	}
}

sexp_container *get_sexp_container_generic(const char* name,
	const std::function<bool(const char*, const char*)>& match_func)
{
	for (auto &container : Sexp_containers) {
		if (match_func(container.container_name.c_str(), name)) {
			return &container;
		}
	}

	// not found
	return nullptr;
}

static bool str_match(const char* str1, const char* str2)
{
	return !stricmp(str1, str2);
}

sexp_container *get_sexp_container(const char* name) {
	return get_sexp_container_generic(name, str_match);
}

static bool str_prefix(const char* prefix, const char* str)
{
	for (; *prefix; ++prefix, ++str) {
		if (SCP_tolower(*str) != SCP_tolower(*prefix)) {
			return false;
		}
	}

	return true;
}

/**
* Tests whether this position in a character array is the start of a container name
* Returns a pointer to the container or nullptr if not found
**/
sexp_container *get_sexp_container_special(const SCP_string& text, size_t start_pos)
{
	Assert(!text.empty());
	Assert(start_pos < text.length());

	return get_sexp_container_generic(text.c_str() + start_pos, str_prefix);
}
