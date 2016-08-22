
#include "pilotfile/JSONFileHandler.h"

namespace {
int json_write_callback(const char *buffer, size_t size, void *data) {
	CFILE* cfp = (CFILE*)data;

	if (cfwrite(buffer, 1, (int)size, cfp) != size) {
		return -1; // Error
	} else {
		return 0; // Success
	}
}

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
	std::pair<Section, const char*>(Section::LastMissions, "last_mission")
};

const char* lookupSectionName(Section s) {
	for (auto& pair : SectionMapping) {
		if (pair.first == s) {
			return pair.second;
		}
	}
	Assertion(false, "Unknown section!");
	return nullptr;
}

Section lookupSectionValue(const char* name) {
	for (auto& pair : SectionMapping) {
		if (!strcmp(pair.second, name)) {
			return pair.first;
		}
	}
	Assertion(false, "Unknown section name!");
	return Section::Unnamed;
}
}

pilot::JSONFileHandler::JSONFileHandler(CFILE* cfp) : _cfp(cfp) {
	Assertion(cfp != nullptr, "File pointer must be valid!");

	_rootObj = json_object();
	_currentEl = _rootObj;
	_elementStack.push_back(_currentEl);
}

pilot::JSONFileHandler::~JSONFileHandler() {
	json_decref(_rootObj);

	cfclose(_cfp);
	_cfp = nullptr;
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
	json_object_set_new(_currentEl, name, json_string(str));
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
	_currentEl = obj;
	_elementStack.push_back(_currentEl);
}
void pilot::JSONFileHandler::endSectionWrite() {
	Assertion(json_is_object(_currentEl), "Section ended while not in a section!");

	_elementStack.pop_back();
	_currentEl = _elementStack.back();
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
	_currentEl = array;
	_elementStack.push_back(_currentEl);
}
void pilot::JSONFileHandler::endArrayWrite() {
	Assertion(json_is_array(_currentEl), "Array ended while not in an array!");

	_elementStack.pop_back();
	_currentEl = _elementStack.back();
}
void pilot::JSONFileHandler::flush() {
	Assertion(_elementStack.size() == 1, "Not all sections or arrays have been ended!");

	json_dump_callback(_rootObj, json_write_callback, _cfp, JSON_INDENT(4));
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

