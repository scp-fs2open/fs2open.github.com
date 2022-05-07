/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <algorithm>
#include <array>
#include <iterator>

#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"
#include "globalincs/toolchain.h"
#include "mission/missiongoals.h"
#include "parse/generic_log.h"
#include "parse/parselo.h"
#include "parse/sexp_container.h"
#include "utils/Random.h"

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

// CTEXT()-related data
namespace {
	const char *Empty_str = "";
} // namespace

// map_container_hash impl

uint32_t map_container_hash::operator()(const SCP_string &str) const
{
	// hash function recipe from Effective Java, Item 9
	// occasional overflow is expected and not a problem
	// map key matching is case-sensitive
	uint32_t result = 42u; // arbitrary non-zero number
	for (char c : str) {
		result = result * 31 + c;
	}

	return result;
}

// sexp_container data and functions
const SCP_string sexp_container::DELIM_STR(1, sexp_container::DELIM);


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

// ListModifier-related functions

ListModifier get_list_modifier(const char *text, bool accept_prefix)
{
	Assertion(text != nullptr, "Attempt to get list modifier from null name. Please report!");

	for (const auto &modifier_obj : List_modifiers) {
		if (accept_prefix && !strnicmp(modifier_obj.name, text, strlen(modifier_obj.name))) {
			return modifier_obj.modifier;
		} else if (!accept_prefix && !stricmp(modifier_obj.name, text)) {
			return modifier_obj.modifier;
		}
	}

	// not found
	return ListModifier::INVALID;;
}

const char *get_list_modifier_name(const ListModifier modifier)
{
	Assertion(modifier != ListModifier::INVALID, "Attempt to get name of invalid list modifier. Please report!");

	for (const auto &modifier_obj : List_modifiers) {
		if (modifier_obj.modifier == modifier) {
			return modifier_obj.name;
		}
	}

	UNREACHABLE("get_list_modifier_name() given unknown modifier %d", (int)modifier);
	return Empty_str;
}

// sexp_container.h functions

/**
 * Clear the SEXP Containers and related data
 */
void init_sexp_containers()
{
	Sexp_containers.clear();
}

void update_sexp_containers(SCP_vector<sexp_container> &containers,
	const SCP_unordered_map<SCP_string, SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to>
		&renamed_containers)
{
	Sexp_containers = std::move(containers);

	if (!renamed_containers.empty()) {
		for (int i = 0; i < Num_sexp_nodes; i++) {
			auto &node = Sexp_nodes[i];
			if (node.type == SEXP_ATOM && node.subtype == SEXP_ATOM_CONTAINER) {
				const auto new_name_it = renamed_containers.find(node.text);
				if (new_name_it != renamed_containers.cend()) {
					strcpy_s(node.text, new_name_it->second.c_str());
				}
			}
		}
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
		opf_type = OPF_STRING;
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
		sexp_container new_list;

		new_list.type = ContainerType::LIST;
		if (stuff_one_generic_sexp_container(new_list.container_name, new_list.type, new_list.opf_type, parsed_data)) {
			std::copy(parsed_data.begin(), parsed_data.end(), back_inserter(new_list.list_data));
			Sexp_containers.emplace_back(std::move(new_list));
		}
	}
}

void stuff_sexp_map_containers()
{
	SCP_vector<SCP_string>	parsed_data;

	while (required_string_either("$End Maps", "$Name:")) {
		sexp_container new_map;

		new_map.type = ContainerType::MAP;
		if (stuff_one_generic_sexp_container(new_map.container_name, new_map.type, new_map.opf_type, parsed_data)) {
			if (parsed_data.size() % 2 != 0) {
				Warning(LOCATION,
					"Data in the SEXP Map container is corrupt. Must be an even number of entries. Instead have %d",
					(int)parsed_data.size());
				log_printf(LOGFILE_EVENT_LOG,
					"Data in the SEXP Map container is corrupt. Must be an even number of entries. Instead have %d",
					(int)parsed_data.size());
			} else {
				for (int i = 0; i < (int)parsed_data.size(); i += 2) {
					new_map.map_data.emplace(parsed_data[i], parsed_data[i + 1]);
				}
				Sexp_containers.emplace_back(std::move(new_map));
			}
		}
	}
}

sexp_container *get_sexp_container(const char *name) {
	Assertion(name != nullptr, "Attempt to look up container from null name. Please report!");
	// unlike get_sexp_container_special(), empty name is ok
	// even if the result will always be nullptr
	// it can happen if CTEXT() returns empty str

	for (auto &container : Sexp_containers) {
		if (!stricmp(name, container.container_name.c_str())) {
			return &container;
		}
	}

	return nullptr;
}

/**
* Tests whether this position in a character array is the start of a container name
* Returns a pointer to the container or nullptr if not found
**/
sexp_container *get_sexp_container_special(const SCP_string &text, size_t start_pos)
{
	Assert(!text.empty());
	Assert(start_pos < text.length());

	for (auto &container : Sexp_containers) {
		// in case one container's name is a prefix of another's, include terminating DELIM
		const SCP_string name_to_find = container.container_name + sexp_container::DELIM_STR;
		if (!strnicmp(text.c_str() + start_pos, name_to_find.c_str(), name_to_find.length())) {
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
				"Attempt to replace text from map container %s but the key is "
				"missing a final '&'",
				container.container_name.c_str());
			return false;
		}

		const SCP_string key = text.substr(lookHere, key_ends_here - lookHere);
		const auto iter = container.map_data.find(key);

		if (iter == container.map_data.end()) {
			Warning(LOCATION,
				"Attempt to replace text from map container %s with nonexistent key %s",
				container.container_name.c_str(),
				key.c_str());
			return false;
		}

		replacement_text = iter->second;

		num_chars_to_replace += key.length();
	} else {
		// for list containers, check if this matches a list modifier
		Assertion(container.is_list(),
			"Attempt to replace text using container %s with unknwon type %d. Please report!",
			container.container_name.c_str(),
			(int)container.type);

		if (container.list_data.empty()) {
			Warning(LOCATION, "Attempt to replace text using empty list container %s", container.container_name.c_str());
			return false;
		}

		const auto modifier = get_list_modifier(text.c_str() + lookHere, true);

		if (modifier == ListModifier::INVALID) {
			Warning(LOCATION,
				"Attempt to reaplce text from list container %s using unrecognized list modifer",
				container.container_name.c_str());
			return false;
		}

		const size_t modifier_length = strlen(get_list_modifier_name(modifier));
		int data_index = 0;
		size_t number_length = 0;
		SCP_string number_string;
		auto &list_data = container.list_data;
		auto list_it = list_data.begin();

		switch (modifier) {
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
			number_string = text.substr(lookHere + modifier_length);
			if (number_string.empty()) {
				Warning(LOCATION,
					"Container text replacement found empty index into list container %s",
					container.container_name.c_str());
				return false;
			}
			data_index = atoi(number_string.c_str());
			if (data_index < 0 || data_index >= (int)list_data.size()) {
				Warning(LOCATION,
					"Container text replacement found invalid index '%s' into list container %s",
					number_string.c_str(),
					container.container_name.c_str());
				return false;
			}
			replacement_text = *std::next(list_it, data_index);

			// include spaces, in case the FREDder put one between "At" and the index
			number_length = strspn(number_string.c_str(), " 0123456789");
			break;

		default:
			UNREACHABLE("Unhandled list modifier %d. Please report!", (int)modifier);
			return false;
		}

		num_chars_to_replace += modifier_length;

		// for the "At" modifier, we need to include the chars used by the index
		if (modifier == ListModifier::AT_INDEX) {
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
			auto *p_container = get_sexp_container_special(text, foundHere + 1);
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
						"Attempt to replace text from container %s with no modifier. "
						"This format is expected for lists &Container_Name&Modifier&. This "
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
						"Container text replacement found potential infinite loop in multidimensionality "
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

bool sexp_container_replace_refs_with_values(char *text, size_t max_size)
{
	Assertion(text != nullptr, "Attempt to perform container text replacement with null text. Please report!");

	SCP_string text_str = text;

	bool replaced_anything = sexp_container_replace_refs_with_values(text_str);

	if (replaced_anything) {
		Assertion(max_size > 0, "Attempt to perform container text replacement with zero-sized buffer. Please report!");
		strncpy(text, text_str.c_str(), max_size - 1);
		text[max_size - 1] = 0;
	}

	return replaced_anything;
}

// Containers should not be modified if the game is simply checking the syntax. 
bool are_containers_modifiable()
{
	// can always modify containers during the mission itself
	if (Game_mode & GM_IN_MISSION) {
		return true;
	}

	// can also modify if we are calling the sexp from a script or if we are briefing / debriefing, etc.
	switch (gameseq_get_state()) {
	case GS_STATE_BRIEFING:
	case GS_STATE_DEBRIEF:
	case GS_STATE_CMD_BRIEF:
	case GS_STATE_FICTION_VIEWER:
	case GS_STATE_SCRIPTING:
		return true;

	default:
		return false;
	}
}

bool sexp_container_CTEXT_helper(int &node, sexp_container &container, SCP_string &result)
{
	if (container.is_map()) {
		if (container.map_data.empty()) {
			Warning(LOCATION, "Attempt to retrieve data from empty map container %s", container.container_name.c_str());
			return false;
		}

		const char *key = CTEXT(node);
		const auto value_it = container.map_data.find(SCP_string(key));

		if (value_it != container.map_data.end()) {
			result = value_it->second;
			return true;
		} else {
			if (Log_event) {
				SCP_string log_string = container.container_name;
				log_string.append(" map container returned nothing when searched for key ");
				log_string.append(key);
				Current_event_log_container_buffer->emplace_back(log_string);
			}
			return false;
		}
	} else if (container.is_list()) {
		if (container.list_data.empty()) {
			Warning(LOCATION, "Attempt to retrieve data from empty list container %s", container.container_name.c_str());
			return false;
		}

		const char *modifier_name = CTEXT(node);
		const auto modifier = get_list_modifier(modifier_name);


		if (modifier == ListModifier::INVALID) {
			const SCP_string &container_name = container.container_name;
			Warning(LOCATION,
				"Illegal operation attempted on %s container. There is no modifier called %s.",
				container_name.c_str(),
				modifier_name);
			log_printf(LOGFILE_EVENT_LOG,
				"Illegal operation attempted on %s container. There is no modifier called %s.",
				container_name.c_str(),
				modifier_name);
			return false;
		}

		int data_index = -1;
		auto &list_data = container.list_data;
		auto list_it = list_data.begin();

		switch (modifier) {
		case ListModifier::GET_FIRST:
		case ListModifier::REMOVE_FIRST:
			result = list_data.front();
			if (modifier == ListModifier::REMOVE_FIRST && are_containers_modifiable()) {
				list_data.pop_front();
			}
			return true;

		case ListModifier::GET_LAST:
		case ListModifier::REMOVE_LAST:
			result = list_data.back();
			if (modifier == ListModifier::REMOVE_LAST && are_containers_modifiable()) {
				list_data.pop_back();
			}
			return true;

		case ListModifier::GET_RANDOM:
		case ListModifier::REMOVE_RANDOM:
			data_index = util::Random::next((int)list_data.size());
			std::advance(list_it, data_index);
			result = *list_it;
			if (modifier == ListModifier::REMOVE_RANDOM && are_containers_modifiable()) {
				list_data.erase(list_it);
			}
			return true;

		case ListModifier::AT_INDEX:
			node = CDR(node);
			if (node == -1) {
				return false;
			}
			bool is_nan, is_nan_forever;
			data_index = eval_num(node, is_nan, is_nan_forever);
			if (is_nan || is_nan_forever) {
				return false;
			}
			if (data_index < 0 || data_index >= (int)list_data.size()) {
				return false;
			}
			result = *std::next(list_it, data_index);
			return true;

		default:
			UNREACHABLE("Unhandled list modifier %d found for container %s",
				(int)modifier,
				container.container_name.c_str());
			break;
		}
	}
	return false;
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

const char *sexp_container_CTEXT(int node)
{
	auto *p_container = get_sexp_container(Sexp_nodes[node].text);

	if (!p_container) {
		Warning(LOCATION, "sexp_container_CTEXT() called for %s, a container which does not exist!", Sexp_nodes[node].text);
		log_printf(LOGFILE_EVENT_LOG, "sexp_container_CTEXT() called for %s, a container which does not exist!", Sexp_nodes[node].text);
		return Empty_str;
	}

	node = CAR(node);
	Assert(node != -1);

	SCP_string result;

	for (int sanity = 0; sanity < 20; ++sanity) {
		auto &container = *p_container;
		bool success = sexp_container_CTEXT_helper(node, container, result);

		if (Log_event) {
			SCP_string log_string = container.container_name;
			if (success) {
				log_string += " container returned ";
				log_string += result;
			} else {
				log_string += " container lookup failed";
			}
			Current_event_log_container_buffer->emplace_back(log_string);
		}

		if (!success || result.empty()) {
			return Empty_str;
		}

		if (result.front() != sexp_container::DELIM) {
			if (!Sexp_nodes[node].cache) {
				Sexp_nodes[node].cache = new sexp_cached_data(OPF_CONTAINER_NAME, result);
			} else {
				Sexp_nodes[node].cache->update_container_CTEXT_result(result);
			}
			return Sexp_nodes[node].cache->container_CTEXT_result;
		} else {
			// we're dealing with a multidimentional container
			node = CDR(node);

			if (node == -1) {
				Warning(LOCATION,
					"Multidimensional container argument missing for '%s' from container %s!",
					result.c_str(),
					container.container_name.c_str());
				log_printf(LOGFILE_EVENT_LOG,
					"Multidimensional container argument missing for '%s' from container %s!",
					result.c_str(),
					container.container_name.c_str());
				return Empty_str;
			}

			// "- 2" because we're skipping the first and last chars, which are/should both be &
			auto *p_next_container = get_sexp_container(result.substr(1, (result.length() - 2)).c_str());

			if (p_next_container != nullptr) {
				p_container = p_next_container;
				result.clear();
			} else {
				Warning(LOCATION, "There is no container called %s in this mission.", result.c_str());
				return Empty_str;
			}
		}
	}

	// we've hit sanity limit; give up
	return Empty_str;
}

// status SEXPs
int sexp_is_container_empty(int node)
{
	const char *container_name = CTEXT(node);
	const auto *p_container = get_sexp_container(container_name);

	if (!p_container) {
		Warning(LOCATION, "Is-container-empty called on nonexistent container %s.", container_name);
		return SEXP_FALSE;
	}

	const auto &container = *p_container;

	if (container.is_map()) {
		if (container.map_data.empty()) {
			return SEXP_TRUE;
		}
	} else if (container.is_list()) {
		if (container.list_data.empty()) {
			return SEXP_TRUE;
		}
	} else {
		UNREACHABLE("Container %s has invalid type (%d). Please report!", container_name, (int)container.type);
		return SEXP_FALSE;
	}
}

int sexp_get_container_size(int node)
{
	const char *container_name = CTEXT(node);
	const auto *p_container = get_sexp_container(container_name);

	if (!p_container) {
		Warning(LOCATION, "Get-container-size called on nonexistent container %s.", container_name);
		return 0;
	}

	const auto &container = *p_container;

	if (container.is_map()) {
		return (int)container.map_data.size();
	} else if (container.is_list()) {
		return (int)container.list_data.size();
	} else {
		UNREACHABLE("Container %s has invalid type (%d). Please report!", container_name, (int)container.type);
		return 0;
	}
}

int sexp_list_has_data(int node)
{
	const char *container_name = CTEXT(node);
	const auto *p_container = get_sexp_container(container_name);

	if (!p_container) {
		Warning(LOCATION, "List-has-data called on nonexistent container %s.", container_name);
		return SEXP_FALSE;
	}

	const auto &container = *p_container;

	if (!container.is_list()) {
		Warning(LOCATION, "List-has-data called on non-list container %s.", container_name);
		return SEXP_FALSE;
	}

	const auto &list_data = container.list_data;

	node = CDR(node);
	// there should be at least one string to search for
	Assertion(node != -1, "List-has-data wasn't given data to look for. Please report!");

	SCP_string possible_data;
	while (node != -1) {
		// TODO: revisit if traversing the list for every item proves unacceptably slow
		possible_data = CTEXT(node);

		if (std::find(list_data.begin(), list_data.end(), possible_data) == list_data.end()) {
			return SEXP_FALSE;
		}

		node = CDR(node);
	}

	return SEXP_TRUE;
}

int sexp_list_data_index(int node)
{
	const char *container_name = CTEXT(node);
	const auto *p_container = get_sexp_container(container_name);

	if (!p_container) {
		Warning(LOCATION, "List-data-index called on nonexistent container %s.", container_name);
		return -1;
	}

	const auto &container = *p_container;

	if (!container.is_list()) {
		Warning(LOCATION, "List-data-index called on non-list container %s.", container_name);
		return -1;
	}

	node = CDR(node);
	Assertion(node != -1, "List-data-index wasn't given data to look for. Please report!");

	const SCP_string possible_data = CTEXT(node);

	int data_index = 0;
	for (const SCP_string &data : container.list_data) {
		if (possible_data == data) {
			return data_index;
		}
		++data_index;
	}

	return -1;
}

int sexp_map_has_key(int node)
{
	const char *container_name = CTEXT(node);
	const auto *p_container = get_sexp_container(container_name);

	if (!p_container) {
		Warning(LOCATION, "Map-has-key called on nonexistent container %s.", container_name);
		return SEXP_FALSE;
	}

	const auto &container = *p_container;

	if (!container.is_map()) {
		Warning(LOCATION, "Map-has-key called on non-map container %s.", container_name);
		return SEXP_FALSE;
	}

	const auto &map_data = container.map_data;

	node = CDR(node);
	// there should be at least one key to search for
	Assertion(node != -1, "Map-has-key wasn't given keys to look for. Please report!");

	SCP_string possible_key;
	while (node != -1) {
		possible_key = CTEXT(node);
		if (map_data.find(possible_key) == map_data.end()) {
			return SEXP_FALSE;
		}

		node = CDR(node);
	}

	return SEXP_TRUE;
}

/**
* Check if a SEXP map container has a key whose data matches a specific string.
* If a third argument (string variable) is provided, store the data's associated key, if a match is found.
* Returns -1 if the element is not in the container
*/
int sexp_map_has_data_item(int node)
{
	const char *container_name = CTEXT(node);
	const auto *p_container = get_sexp_container(container_name);

	if (!p_container) {
		Warning(LOCATION, "Map-has-data-item called on nonexistent container %s.", container_name);
		return SEXP_FALSE;
	}

	const auto &container = *p_container;

	if (!container.is_map()) {
		Warning(LOCATION, "Map-has-data-item called on non-map container %s.", container_name);
		return SEXP_FALSE;
	}

	node = CDR(node);
	Assertion(node != -1, "Map-has-data-item wasn't given data to look for. Please report!");

	const SCP_string possible_data = CTEXT(node);

	for (const auto &kv_pair : container.map_data) {
		if (possible_data == kv_pair.second) {
			// check for optional variable to store the key
			node = CDR(node);
			if (node != -1) {
				int var_index = -1;
				if (Sexp_nodes[node].type & SEXP_FLAG_VARIABLE) {
					var_index = sexp_get_variable_index(node);
				} else {
					// perhaps the node text is the variable name as data
					var_index = get_index_sexp_variable_name(Sexp_nodes[node].text);
				}

				if (var_index >= 0) {
					const int var_type = Sexp_variables[var_index].type;
					if (((var_type & SEXP_VARIABLE_STRING) && any(container.type & ContainerType::STRING_KEYS)) ||
						((var_type & SEXP_VARIABLE_NUMBER) && any(container.type & ContainerType::NUMBER_KEYS))) {
						// assign key to variable
						sexp_modify_variable(kv_pair.first.c_str(), var_index);
					} else {
						Warning(LOCATION,
							"Map-has-data-item given optional variable %s whose type doesn't match keys of map "
							"container %s",
							Sexp_variables[var_index].variable_name,
							container.container_name.c_str());
					}
				} else {
					Warning(LOCATION, "Map-has-data-item given invalid optional variable %s", Sexp_nodes[node].text);
				}
			}

			return SEXP_TRUE;
		}
	}

	return SEXP_FALSE;
}

int sexp_container_eval_status_sexp(int op_num, int node)
{
	switch (op_num) {
		case OP_IS_CONTAINER_EMPTY:
			return sexp_is_container_empty(node);
			break;

		case OP_GET_CONTAINER_SIZE:
			return sexp_get_container_size(node);
			break;

		case OP_LIST_HAS_DATA:
			return sexp_list_has_data(node);
			break;

		case OP_LIST_DATA_INDEX:
			return sexp_list_data_index(node);
			break;

		case OP_MAP_HAS_KEY:
			return sexp_map_has_key(node);
			break;

		case OP_MAP_HAS_DATA_ITEM:
			return sexp_map_has_data_item(node);
			break;

		default:
			UNREACHABLE("Unknown container status SEXP operator %d. Please report!", op_num);
			return SEXP_FALSE;
	}
}
