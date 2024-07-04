
#include <jansson.h>
#include "pilotfile/JSONFileHandler.h"
#include "libs/jansson.h"
#include "parse/parselo.h"

namespace {
const SCP_vector<std::pair<Section, const char*>> SectionMapping {
	std::pair<Section, const char*>(Section::Unnamed, nullptr),
	std::pair<Section, const char*>(Section::Flags, "flags"),
	std::pair<Section, const char*>(Section::Info, "info"),
	std::pair<Section, const char*>(Section::Loadout, "loadout"),
	std::pair<Section, const char*>(Section::Controls, "controls"),
	std::pair<Section, const char*>(Section::Multiplayer, "multiplayer"),
	std::pair<Section, const char*>(Section::Scoring, "scoring"),
	std::pair<Section, const char*>(Section::ScoringMulti, "scoring_multi"),
	std::pair<Section, const char*>(Section::Techroom, "techroom"),
	std::pair<Section, const char*>(Section::HUD, "hud"),
	std::pair<Section, const char*>(Section::Settings, "settings"),
	std::pair<Section, const char*>(Section::RedAlert, "red_alert"),
	std::pair<Section, const char*>(Section::Variables, "variables"),
	std::pair<Section, const char*>(Section::Missions, "missions"),
	std::pair<Section, const char*>(Section::Cutscenes, "cutscenes"),
	std::pair<Section, const char*>(Section::LastMissions, "last_mission"),
	std::pair<Section, const char*>(Section::Containers, "containers")
};

const char* lookupSectionName(Section s) {
	for (auto& pair : SectionMapping) {
		if (pair.first == s) {
			return pair.second;
		}
	}
	return nullptr;
}

Section lookupSectionValue(const char* name) {
	Assertion(name != nullptr, "Key name must be a valid pointer!");

	for (auto& pair : SectionMapping) {
		if (pair.second == nullptr) {
			// Skip the unnamed section
			continue;
		}
		if (!strcmp(pair.second, name)) {
			return pair.first;
		}
	}
	return Section::Invalid;
}
}

pilot::JSONFileHandler::JSONFileHandler(CFILE* cfp, bool reading) : _cfp(cfp) {
	Assertion(cfp != nullptr, "File pointer must be valid!");

	if (reading) {
		json_error_t error;
		_rootObj = json_load_cfile(cfp, 0, &error);

		if (!_rootObj) {
			SCP_string errorStr;
			sprintf(errorStr, "Error while reading pilot file! %d: %s", error.line, error.text);
			throw std::runtime_error(errorStr);
		}
	} else {
		_rootObj = json_object();
	}
	_currentEl = _rootObj;
	_elementStack.push_back(_currentEl);
}

pilot::JSONFileHandler::~JSONFileHandler() {
	json_decref(_rootObj);

	cfclose(_cfp);
	_cfp = nullptr;
}
void pilot::JSONFileHandler::pushElement(json_t* t) {
	Assertion(t != nullptr, "Invalid JSON element pointer passed!");

	_currentEl = t;
	_elementStack.push_back(_currentEl);
}
void pilot::JSONFileHandler::popElement() {
	Assertion(_elementStack.size() > 1, "Element stack may not get smaller than one element!");

	_elementStack.pop_back();
	_currentEl = _elementStack.back();
}
void pilot::JSONFileHandler::writeByte(const char* name, std::int8_t value) {
	writeInteger(name, value);
}
void pilot::JSONFileHandler::writeUByte(const char* name, std::uint8_t value) {
	writeInteger(name, value);
}
void pilot::JSONFileHandler::writeShort(const char* name, std::int16_t value) {
	writeInteger(name, value);
}
void pilot::JSONFileHandler::writeInt(const char* name, std::int32_t value) {
	writeInteger(name, value);
}
void pilot::JSONFileHandler::writeUInt(const char* name, std::uint32_t value) {
	writeInteger(name, value);
}
void pilot::JSONFileHandler::writeFloat(const char* name, float value) {
	ensureNotExists(name);
	json_object_set_new(_currentEl, name, json_real(value));
}
void pilot::JSONFileHandler::writeString(const char* name, const char* str) {
	ensureNotExists(name);
	json_t *jstr = nullptr;

	if (str) {
		// if this string isn't proper UTF-8, try to convert it
		auto len = strlen(str);
		if (utf8::find_invalid(str, str + len) != str + len) {
			SCP_string buffer;
			coerce_to_utf8(buffer, str);
			jstr = json_string(buffer.c_str());
		} else {
			jstr = json_string(str);
		}
	}

	json_object_set_new(_currentEl, name, jstr);
}
void pilot::JSONFileHandler::beginWritingSections() {
	ensureNotExists("sections");

	json_t* obj = json_object();

	Assertion(json_typeof(_currentEl) == JSON_OBJECT, "Sections can only be written into a JSON object!");

	json_object_set_new(_currentEl, "sections", obj);

	pushElement(obj);
}
void pilot::JSONFileHandler::startSectionWrite(Section id) {
	auto key_name = lookupSectionName(id);

	json_t* obj = json_object();

	if (json_is_array(_currentEl)) {
		// We are in an array, section must be unnamed
		Assertion(key_name == nullptr, "Inside an array there can be no named section!");
		json_array_append_new(_currentEl, obj);
	} else {
		Assertion(key_name != nullptr, "Section outside of arrays must be named!");
		json_object_set_new(_currentEl, key_name, obj);
	}
	pushElement(obj);
}
void pilot::JSONFileHandler::endSectionWrite() {
	Assertion(json_is_object(_currentEl), "Section ended while not in a section!");

	popElement();
}
void pilot::JSONFileHandler::endWritingSections() {
	Assertion(json_is_object(_currentEl), "Section writing ended while not in object!");

	popElement();
}
void pilot::JSONFileHandler::startArrayWrite(const char* name, size_t, bool) {
	auto array = json_array();

	if (json_is_array(_currentEl)) {
		// We are in an array, section must be unnamed
		Assertion(name == nullptr, "Inside an array there can be no named section!");
		json_array_append_new(_currentEl, array);
	} else {
		Assertion(name != nullptr, "Section outside of arrays must be named!");
		json_object_set_new(_currentEl, name, array);
	}
	pushElement(array);
}
void pilot::JSONFileHandler::endArrayWrite() {
	Assertion(json_is_array(_currentEl), "Array ended while not in an array!");

	popElement();
}
void pilot::JSONFileHandler::flush() {
	Assertion(_elementStack.size() == 1, "Not all sections or arrays have been ended!");

	json_dump_cfile(_rootObj, _cfp, JSON_INDENT(4));
}
void pilot::JSONFileHandler::writeInteger(const char* name, json_int_t val) {
	ensureNotExists(name);

	json_object_set_new(_currentEl, name, json_integer(val));
}
void pilot::JSONFileHandler::ensureNotExists(const char* name) {
	Assertion(json_is_object(_currentEl), "Currently not in an element that supports keys!");
	// Make sure we don't overwrite previous values
	Assertion(json_object_get(_currentEl, name) == nullptr, "Entry with name %s already exists!", name);
}

json_int_t pilot::JSONFileHandler::readInteger(const char* name) {
	auto el = json_object_get(_currentEl, name);

	if (el == nullptr || json_typeof(el) != JSON_INTEGER) {
		Error(LOCATION, "JSON element %s must be an integer but it is not valid!", name);
		return 0;
	}

	return json_integer_value(el);
}
void pilot::JSONFileHandler::ensureExists(const char* name) {
	if (json_typeof(_currentEl) != JSON_OBJECT) {
		Error(LOCATION, "JSON reading requires a value with name '%s' but the current element is not an object!", name);
	}
	if (json_object_get(_currentEl, name) == nullptr) {
		Error(LOCATION, "JSON reading requires a value with name '%s' but there is no such value!", name);
	}
}
std::int8_t pilot::JSONFileHandler::readByte(const char* name) {
	return (std::int8_t)readInteger(name);
}
std::uint8_t pilot::JSONFileHandler::readUByte(const char* name) {
	return (std::uint8_t)readInteger(name);
}
std::int16_t pilot::JSONFileHandler::readShort(const char* name) {
	return (std::int16_t)readInteger(name);
}
std::int32_t pilot::JSONFileHandler::readInt(const char* name) {
	return (std::int32_t)readInteger(name);
}
std::uint32_t pilot::JSONFileHandler::readUInt(const char* name) {
	return (std::uint32_t)readInteger(name);
}
float pilot::JSONFileHandler::readFloat(const char* name) {
	auto el = json_object_get(_currentEl, name);

	if (el == nullptr || json_typeof(el) != JSON_REAL) {
		Error(LOCATION, "JSON element %s must be a float but it is not valid!", name);
		return 0.0f;
	}

	return (float)json_real_value(el);
}
SCP_string pilot::JSONFileHandler::readString(const char* name) {
	auto el = json_object_get(_currentEl, name);

	if (el == nullptr || json_typeof(el) != JSON_STRING) {
		Error(LOCATION, "JSON element %s must be a string but it is not valid!", name);
		return SCP_string();
	}
	auto json_str = json_string_value(el);
	SCP_string val;
	val.assign(json_str, json_str + json_string_length(el));

	return val;
}
void pilot::JSONFileHandler::beginSectionRead() {
	ensureExists("sections");

	auto sections = json_object_get(_currentEl, "sections");
	if (json_typeof(sections) != JSON_OBJECT) {
		Error(LOCATION, "Sections must be a JSON object!");
	}

	pushElement(sections);

	Assertion(_sectionIterator == nullptr, "Section nesting is not yet supported!");
	_sectionIterator = json_object_iter(_currentEl);
	// Signals to nextSection that we just started iterating though the sections since it needs to do something special in that case
	_startingSectionIteration = true;
}
bool pilot::JSONFileHandler::hasMoreSections() {
	return _sectionIterator != nullptr;
}
Section pilot::JSONFileHandler::nextSection() {
	Assertion(_sectionIterator != nullptr, "Section iterator must be valid for this function!");

	// If we just started our iteration then there is no previous element we need to pop from the stack
	if (!_startingSectionIteration) {
		// We have to pop the previous section first
		popElement();

		Assertion(json_typeof(_currentEl) == JSON_OBJECT, "The previous element should have been an object!");
	} else {
		// We are now in normal operations so we can reset this flag
		_startingSectionIteration = false;
	}

	auto key = json_object_iter_key(_sectionIterator);
	auto section = lookupSectionValue(key);

	auto el = json_object_iter_value(_sectionIterator);
	if (json_typeof(el) != JSON_OBJECT) {
		Error(LOCATION, "The section element of '%s' must be an object but it's a different type!", key);
		return Section::Invalid;
	}

	// Move the iterator to the next object for the next iteration
	_sectionIterator = json_object_iter_next(_currentEl, _sectionIterator);

	// Move into the current section element
	pushElement(el);

	return section;
}
void pilot::JSONFileHandler::endSectionRead() {
	// First, remove the element we are currently in, if there is one
	if (json_typeof(_currentEl) != JSON_ARRAY) {
		popElement();
	}

	Assertion(json_typeof(_currentEl) == JSON_OBJECT, "Current element for section reading is not an object!");

	popElement();
	_sectionIterator = nullptr;
}
size_t pilot::JSONFileHandler::startArrayRead(const char* name, bool /*short_index*/) {
	Assertion(_arrayIndex == INVALID_SIZE, "Array nesting is not supported yet!");

	ensureExists(name);

	auto array = json_object_get(_currentEl, name);
	if (json_typeof(array) != JSON_ARRAY) {
		Error(LOCATION, "Expected an array for '%s' but it was a different type!", name);
		return 0;
	}

	pushElement(array);
	auto size = json_array_size(array);

	if (size == 0) {
		// Nothing to do here, avoid calling nextArraySection since that assumes that there is at least one element
		return size;
	}
	_arrayIndex = 0;
	nextArraySection(false);

	return size;
}
void pilot::JSONFileHandler::nextArraySection(bool in_section) {
	Assertion(_arrayIndex != INVALID_SIZE, "Array index must be valid for this function!");

	if (in_section) {
		// We have to pop the previous section first
		popElement();

		Assertion(json_typeof(_currentEl) == JSON_ARRAY, "The previous element should have been an array!");
	}

	auto max = json_array_size(_currentEl);
	Assertion(_arrayIndex <= max, "Invalid array index detected!");

	// Silently ignore if we are one past the last element since this function is used in a for loop where this function
	// is executed one time after the index is incremented past the last element
	if (_arrayIndex == max) {
		// Increment the index so we catch usage errors the next time someone tries to call this function
		++_arrayIndex;
		return;
	}

	// We use the current array index and then increment it to avoid skipping the first element
	auto el = json_array_get(_currentEl, _arrayIndex);
	++_arrayIndex;

	pushElement(el);
}
void pilot::JSONFileHandler::nextArraySection() {
	nextArraySection(true);
}
void pilot::JSONFileHandler::endArrayRead() {
	// First, remove the element we are currently in if it exists
	if (json_typeof(_currentEl) != JSON_ARRAY) {
		popElement();
	}

	Assertion(json_typeof(_currentEl) == JSON_ARRAY, "Current element must be an array!");

	popElement();
	_arrayIndex = INVALID_SIZE;
}

