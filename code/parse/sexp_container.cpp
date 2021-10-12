/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <algorithm>
#include <iterator>

#include "globalincs/toolchain.h"
#include "parse/generic_log.h"
#include "parse/parselo.h"
#include "parse/sexp_container.h"
#include "utils/Random.h"

static SCP_vector<sexp_container> Sexp_containers;
static SCP_unordered_map<SCP_string, sexp_container*, SCP_string_lcase_hash, SCP_string_lcase_equal_to>
	Containers_by_name_map;


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

bool sexp_container::operator==(const sexp_container &sc) const
{
	return container_name == sc.container_name && type == sc.type && opf_type == sc.opf_type &&
		list_data == sc.list_data && map_data == sc.map_data;
}

bool sexp_container::name_matches(const sexp_container &container) const
{
	return !stricmp(container.container_name.c_str(), container_name.c_str());
}

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

static constexpr ContainerType CONTAINER_PERSISTENCE_FLAGS =
	ContainerType::SAVE_ON_MISSION_CLOSE | ContainerType::SAVE_ON_MISSION_PROGRESS | ContainerType::NETWORK |
	ContainerType::SAVE_TO_PLAYER_FILE;

ContainerType sexp_container::get_non_persistent_type() const
{
	return type & ~CONTAINER_PERSISTENCE_FLAGS;
}

bool sexp_container::type_matches(const sexp_container &container) const
{
	return get_non_persistent_type() == container.get_non_persistent_type();
}

// sexp_container.h functions

/**
 * Clear the SEXP Containers and related data
 */
void init_sexp_containers()
{
	Sexp_containers.clear();
	Containers_by_name_map.clear();
}

void update_sexp_containers(SCP_vector<sexp_container> &containers)
{
	Sexp_containers = std::move(containers);

	Containers_by_name_map.clear();
	for (auto &container : Sexp_containers) {
		Containers_by_name_map.emplace(container.container_name, &container);
	}
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
			"SEXP Container name %s is longer than limit %d",
			name.c_str(),
			sexp_container::NAME_MAX_LENGTH);
		log_printf(LOGFILE_EVENT_LOG,
			"SEXP Container name %s is longer than limit %d",
			name.c_str(),
			sexp_container::NAME_MAX_LENGTH);
		valid = false;
	} else if (get_sexp_container(name.c_str()) != nullptr) {
		Warning(LOCATION, "A SEXP Container named %s already exists", name.c_str());
		log_printf(LOGFILE_EVENT_LOG, "A SEXP Container named %s already exists", name.c_str());
		valid = false;
	}

	required_string("$Data Type:");
	stuff_string(temp_type_string, F_NAME);
	if (!stricmp(temp_type_string.c_str(), "Number")) {
		type |= ContainerType::NUMBER_DATA;
		opf_type = OPF_NUMBER;
	} else if (!stricmp(temp_type_string.c_str(), "String")) {
		type |= ContainerType::STRING_DATA;
		opf_type = OPF_ANYTHING;
	} else {
		Warning(LOCATION, "Unknown SEXP Container type %s found", temp_type_string.c_str());
		log_printf(LOGFILE_EVENT_LOG, "Unknown SEXP Container type %s found", temp_type_string.c_str());
		valid = false;
	}

	if (optional_string("$Key Type:")) {
		Assertion(any(type & ContainerType::MAP), "$Key Type: found for container which doesn't use keys!");

		stuff_string(temp_type_string, F_NAME);
		if (!stricmp(temp_type_string.c_str(), "Number")) {
			type |= ContainerType::NUMBER_KEYS;
		} else if (!stricmp(temp_type_string.c_str(), "String")) {
			type |= ContainerType::STRING_KEYS;
		} else {
			Warning(LOCATION, "Unknown SEXP Container type %s found", temp_type_string.c_str());
			log_printf(LOGFILE_EVENT_LOG, "Unknown SEXP Container type %s found", temp_type_string.c_str());
			valid = false;
		}
	}

	if (optional_string("+Strictly Typed Keys")) {
		Assertion(any(type & ContainerType::MAP), "+Strictly Typed Keys found for container which doesn't use keys!");
		Warning(LOCATION, "Container %s is marked for strictly typed keys, which are not yet supported.", name.c_str());
		type |= ContainerType::STRICTLY_TYPED_KEYS;
	}

	if (optional_string("+Strictly Typed Data")) {
		Warning(LOCATION, "Container %s is marked for strictly typed data, which are not yet supported.", name.c_str());
		type |= ContainerType::STRICTLY_TYPED_DATA;
	}

	required_string("$Data:");
	stuff_string_list(data);

	if (optional_string("+Network Container")) {
		Warning(LOCATION, "Container %s is marked as a network container, which is not yet supported.", name.c_str());
		type |= ContainerType::NETWORK;
	}

	if (optional_string("+Eternal")) {
		type |= ContainerType::SAVE_TO_PLAYER_FILE;
	}

	// campaign-persistent
	if (optional_string("+Save On Mission Progress")) {
		type |= ContainerType::SAVE_ON_MISSION_PROGRESS;
	}

	// player-persistent
	if (optional_string("+Save On Mission Close")) {
		Assert(none(type & ContainerType::SAVE_ON_MISSION_PROGRESS));
		type |= ContainerType::SAVE_ON_MISSION_CLOSE;
	}

	Assert(none(type & ContainerType::SAVE_TO_PLAYER_FILE) ||
		   (any(type & ContainerType::SAVE_ON_MISSION_PROGRESS) ^ any(type & ContainerType::SAVE_ON_MISSION_CLOSE)));

	return valid;
}

void stuff_sexp_list_containers()
{
	SCP_vector<SCP_string>	parsed_data;

	while (required_string_either("$End Lists", "$Name:")) {
		Sexp_containers.emplace_back();
		auto &new_list = Sexp_containers.back();

		new_list.type = ContainerType::LIST;
		if (stuff_one_generic_sexp_container(new_list.container_name, new_list.type, new_list.opf_type, parsed_data)) {
			std::copy(parsed_data.begin(), parsed_data.end(), back_inserter(new_list.list_data));
			Containers_by_name_map.emplace(new_list.container_name, &new_list);
		} else {
			Sexp_containers.pop_back();
		}
	}
}

void stuff_sexp_map_containers()
{
	SCP_vector<SCP_string>	parsed_data;

	while (required_string_either("$End Maps", "$Name:")) {
		Sexp_containers.emplace_back();
		auto& new_map = Sexp_containers.back();

		new_map.type = ContainerType::MAP;
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
				Containers_by_name_map.emplace(new_map.container_name, &new_map);
			}
		} else {
			Sexp_containers.pop_back();
		}
	}
}

sexp_container *get_sexp_container(const char* name) {
	auto container_it = Containers_by_name_map.find(name);
	if (container_it != Containers_by_name_map.end()) {
		return container_it->second;
	} else {
		return nullptr;
	}
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

	for (auto &container : Sexp_containers) {
		if (str_prefix(container.container_name.c_str(), text.c_str())) {
			return &container;
		}
	}

	// not found
	return nullptr;
}

/**
* Helper function for sexp_container_replace_refs_with_values(). Given a SEXP Container it works out what modifer was used and what the replacement string should be.
**/
bool get_replace_text_for_modifier(const SCP_string &text,
	sexp_container &container,
	const size_t lookHere,
	SCP_string &replacement_text,
	size_t &num_chars_to_replace)
{
	// for map containers, check if this matches a map key
	if (container.is_map()) {
		if (container.map_data.empty()) {
			Warning(LOCATION, "Attempt to replace text using empty map container %s", container.container_name.c_str());
			return false;
		}

		const size_t key_ends_here = text.find(sexp_container::DELIM, lookHere);
		if (key_ends_here == SCP_string::npos) {
			Warning(LOCATION,
				"sexp_container_replace_refs_with_values() found a map container called %s to replace but the key is "
				"missing a final '&'",
				container.container_name.c_str());
			return false;
		}

		const SCP_string key = text.substr(lookHere, key_ends_here - lookHere);
		const auto iter = container.map_data.find(key);

		if (iter == container.map_data.end()) {
			Warning(LOCATION,
				"sexp_container_replace_refs_with_values() found a map container called %s but the key %s is not found",
				container.container_name.c_str(),
				key.c_str());
			return false;
		}

		replacement_text = iter->second;

		num_chars_to_replace += key.length();
	} else {
		// for list containers, check if this matches a list modifier
		Assert(container.is_list());

		if (container.list_data.empty()) {
			Warning(LOCATION, "Attempt to replace text using empty list container %s", container.container_name.c_str());
			return false;
		}

		const list_modifier *modifier_to_use = nullptr;

		for (const auto &modifier : List_modifiers) {
			if (str_prefix(modifier.name, text.c_str() + lookHere)) {
				modifier_to_use = &modifier;
				break;
			}
		}

		if (!modifier_to_use) {
			Warning(LOCATION,
				"sexp_container_replace_refs_with_values() found a list container called %s to replace but the modifer "
				"is not recognised",
				container.container_name.c_str());
			return false;
		}

		int data_index = 0;
		size_t number_length = 0;
		SCP_string number_string;
		auto &list_data = container.list_data;
		auto list_it = list_data.begin();

		switch (modifier_to_use->modifier) {
		case ListModifier::GET_FIRST:
			replacement_text = list_data.front();
			break;

		case ListModifier::GET_LAST:
			replacement_text = list_data.back();
			break;

		case ListModifier::REMOVE_FIRST:
			replacement_text = list_data.front();
			list_data.pop_front();
			break;

		case ListModifier::REMOVE_LAST:
			replacement_text = list_data.back();
			list_data.pop_back();
			break;

		case ListModifier::GET_RANDOM:
			data_index = util::Random::next((int)list_data.size());
			replacement_text = *std::next(list_it, data_index);
			break;

		case ListModifier::REMOVE_RANDOM:
			data_index = util::Random::next((int)list_data.size());
			std::advance(list_it, data_index);
			replacement_text = *list_it;
			list_data.erase(list_it);
			break;

		case ListModifier::AT_INDEX:
			number_string = text.substr(lookHere + strlen(modifier_to_use->name));
			Assert(!number_string.empty());
			data_index = atoi(number_string.c_str());
			if (data_index < 0 || data_index >= (int)list_data.size()) {
				Warning(LOCATION,
					"sexp_container_replace_refs_with_values() found invalid index '%s' into list container %s",
					number_string.c_str(),
					container.container_name.c_str());
				return false;
			}
			replacement_text = *std::next(list_it, data_index);

			// include spaces, in case the FREDder put one between "At" and the index
			number_length = strspn(number_string.c_str(), " 0123456789");
			break;

		default:
			Warning(LOCATION, "sexp_container_replace_refs_with_values() found a container called %s to replace but the modifer is not recognised", container.container_name.c_str());
			return false;
		}

		num_chars_to_replace += strlen(modifier_to_use->name);

		// for the "At" modifier, we need to include the chars used by the index
		if (modifier_to_use->modifier == ListModifier::AT_INDEX) {
			num_chars_to_replace += number_length;
		}
	}

	// account for the final '&'
	++num_chars_to_replace;

	return true;
}

// inspired by sexp_replace_variable_names_with_values()
/**
* Replace container references in a string with their values
**/
bool sexp_container_replace_refs_with_values(SCP_string &text)
{
	bool replaced_anything = false;
	size_t lookHere = 0;
	int num_replacements = 0;

	while (lookHere < text.length()) {
		// look for the meta-character
		size_t foundHere = text.find(sexp_container::DELIM, lookHere);

		// found?
		if (foundHere != SCP_string::npos && foundHere != text.length() - 1) {
			// see if a container starts at the next char
			auto* p_container = get_sexp_container_special(text, foundHere + 1);
			if (p_container != nullptr) {
				// we want to replace either &list_container&list_modifier& or &map_container&map_key&

				// + 2 for the enclosing '&' chars around the container name
				size_t num_chars_to_replace = p_container->container_name.length() + 2;

				// make sure the enclosing '&' appears after the container name
				size_t expected_amp_pos = foundHere + num_chars_to_replace - 1;
				if (expected_amp_pos >= text.length() || text[expected_amp_pos] != sexp_container::DELIM) {
					// no modifier/key specified - ignore this matching container name, as the replacement is malformed
					lookHere = foundHere + 1;
					Warning(LOCATION,
						"sexp_container_replace_refs_with_values() found a container called %s to replace but there is "
						"no modifer in the string. This format is expected for lists &Container_Name&Modifier&. This "
						"format is expected for maps &Container_Name&Key&",
						p_container->container_name.c_str());
					continue;
				}

				size_t modifierLookHere = foundHere + num_chars_to_replace;

				SCP_string replacement_text;

				if (!get_replace_text_for_modifier(text, *p_container, modifierLookHere, replacement_text, num_chars_to_replace)) {
					// list modifier/map key was invalid
					// skip past the text we were unable to replace
					lookHere = foundHere + num_chars_to_replace;
					continue;
				}

				// perform the replacement
				text.replace(foundHere, num_chars_to_replace, replacement_text);
				replaced_anything = true;

				++num_replacements;
				// avoid potential infinite loops
				if (num_replacements >= 50) {
					Warning(LOCATION,
						"sexp_container_replace_refs_with_values() found potential multidimensionality infinite loop "
						"in '%s' at position %d.",
						text.c_str(),
						(int)lookHere);
					break;
				}

				// don't go past the replacement point, because if container multidimensionality
				// is being used, then the resulting text will contain another container ref
				lookHere = foundHere;
			} else { // no container name found 
				lookHere = foundHere + 1;
			}
		} else {
			// no potential metacharacter found, so we're done replacing
			break;
		}
	}

	return replaced_anything;
}

bool sexp_container_replace_refs_with_values(char* text, size_t max_len)
{
	Assert(text != nullptr);

	SCP_string text_str = text;

	bool replaced_anything = sexp_container_replace_refs_with_values(text_str);

	if (replaced_anything) {
		Assert(max_len > 0);
		strncpy(text, text_str.c_str(), max_len - 1);
		text[max_len - 1] = 0;
	}

	return replaced_anything;
}

// inspired by sexp_campaign_file_variable_count()
bool sexp_container_has_persistent_non_eternal_containers()
{
	for (const auto &container : Sexp_containers) {
		if (container.is_persistent() && !container.is_eternal()) {
			return true;
		}
	}

	return false;
}
