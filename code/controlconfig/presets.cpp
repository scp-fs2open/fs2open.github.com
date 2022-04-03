#include "controlconfig/controlsconfig.h"
#include "controlconfig/presets.h"
#include "libs/jansson.h"
#include "parse/parselo.h"

#include <jansson.h>

using namespace io::presets;

const SCP_vector<std::pair<Section, const char*>> SectionMapping {
	std::pair<Section, const char*>(Section::Unnamed, nullptr),
	std::pair<Section, const char*>(Section::Actions, "actions"),
	std::pair<Section, const char*>(Section::Primary, "primary"),
	std::pair<Section, const char*>(Section::Secondary, "secondary"),
};

const char * lookupSectionName(Section id) {
	for (const auto& pair: SectionMapping) {
		if (pair.first == id) {
			return pair.second;
		}
	}

	return nullptr;
}

Section lookupSectionValue(const char* name) {
	Assertion(name != nullptr, "Key name must be a valid pointer!");

	for (auto& pair : SectionMapping) {
		if (!strcmp(pair.second, name)) {
			return pair.first;
		}
	}
	return Section::Invalid;
}

void load_preset_files() {
	SCP_vector<SCP_string> filelist;
	cf_get_file_list(filelist, CF_TYPE_PLAYER_BINDS, NOX("*.json"), CF_SORT_NAME, nullptr,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	std::unique_ptr<PresetFileHandler> handler = nullptr;

	for (const auto &file : filelist) {
		CFILE* fp = cfopen((file + ".json").c_str(), "r", CFILE_NORMAL, CF_TYPE_PLAYER_BINDS, false,
						   CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

		if (!fp) {
			mprintf(("PST => Unable to open '%s' for loading!\n", file.c_str()));
			// try next
			continue;
		}

		try {
			handler.reset(new PresetFileHandler(fp, true));
		} catch (const std::exception& e) {
			mprintf(("PST => Failed to open JSON: `%s`\n", e.what()));
			continue;
		}

		// Check the signature
		int pst_id = handler->readInt("signature");
		if (pst_id != PST_FILE_ID) {
			mprintf(("PST => Invalid header id for `%s`, skipping...\n", file.c_str()));
			continue;
		}

		// Version
		int version = handler->readInt("version");
		mprintf(("PST => Loading `%s` with version %i\n", file.c_str(), version));


		// Start reading in data
		CC_preset preset;
		preset.name = file;
		preset.type = Preset_t::pst;
		preset.bindings.resize(Control_config.size());

		size_t size;
		handler->beginArrayRead(Section::Actions, size);
		for (size_t i = 0; i < size; ++i, handler->nextArrayElement()) {
			SCP_string str;
			int id;

			//handler->beginSectionRead(Section::Unnamed);
			str = handler->readString("bind");
			id = ActionToVal(str.c_str());

			if (id < 0) {
				// Unknown bind, possibly custom.  Ignore for now
				handler->endSectionRead();	// Unnamed
				continue;
			}

			auto &item = preset.bindings[id];

			handler->beginSectionRead(Section::Primary);
			str = handler->readString("cid");
			auto cid = CIDToVal(str.c_str());

			str = handler->readString("flags");
			auto flags = CCFToVal(str.c_str());

			str = handler->readString("input");
			auto btn = InputToVal(cid, str.c_str());
			item.first.take(cid, btn, flags);

			handler->endSectionRead(); // Primary


			handler->beginSectionRead(Section::Secondary);
			str = handler->readString("cid");
			cid = CIDToVal(str.c_str());

			str = handler->readString("flags");
			flags = CCFToVal(str.c_str());

			str = handler->readString("input");
			btn = InputToVal(cid, str.c_str());
			item.second.take(cid, btn, flags);
			handler->endSectionRead(); // Secondary

			//handler->endSectionRead(); // Unnamed
		}
		handler->endArrayRead(); // Actions

		// Done with the file
		auto it = preset_find_duplicate(preset);

		if (it == Control_config_presets.end()) {
			Control_config_presets.push_back(preset);

		} else if ((it->name != preset.name) || (it->type != Preset_t::pst)) {
			// Complain and ignore if the preset names or the type differs
			Warning(LOCATION, "PST => Preset '%s' is a duplicate of an existing preset, ignoring", preset.name.c_str());
		
		} // else, silent ignore
	}
}

bool preset_file_exists(SCP_string name) {
	if (name.find(".json") == std::string::npos) {
		name.append(".json");
	}

	return cf_exists(name.c_str(), CF_TYPE_PLAYER_BINDS) != 0;
}

bool save_preset_file(CC_preset preset, bool overwrite) {
	// Must have a name
	if (preset.name.empty()) {
		mprintf(("PST => Unable to save preset, missing name!"));
		return false;
	}
	
	SCP_string filename = preset.name + ".json";
	std::unique_ptr<PresetFileHandler> handler = nullptr;

	// Check if there's a file already
	CFILE* fp = cfopen(filename.c_str(), "r", CFILE_NORMAL, CF_TYPE_PLAYER_BINDS, false,
	                  CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	
	if ((fp != nullptr) && !overwrite) {
		mprintf(("PST => Unable to save '%s', file already exists!\n", filename.c_str()));
		return false;
	}

	// Try opening file for write
	fp = cfopen(filename.c_str(), "w", CFILE_NORMAL, CF_TYPE_PLAYER_BINDS, false,
					   CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if (!fp) {
		mprintf(("PST => Unable to save '%s', unknown error!\n", filename.c_str()));
	}

	try {
		handler.reset(new PresetFileHandler(fp, false));
	} catch (const std::exception& e) {
		mprintf(("PST => Failed to open JSON: %s\n", e.what()));
		return false;
	}

	// Write header and version
	handler->writeInt("signature", PST_FILE_ID);
	handler->writeInt("version", PST_VERSION);

	mprintf(("PST => Saving %s with version %d...\n", filename.c_str(), (int)PST_VERSION));

	handler->beginArrayWrite(Section::Actions);
	for (int i = 0; static_cast<size_t>(i) < preset.bindings.size(); ++i) {
		const auto& item = preset.bindings[i];
		const auto& first = item.first;
		const auto& second = item.second;

		handler->beginSectionWrite(Section::Unnamed);
		handler->writeString("bind", ValToAction(i));

		handler->beginSectionWrite(Section::Primary);
		handler->writeString("cid",   ValToCID(first.get_cid()));
		handler->writeString("flags", ValToCCF(first.get_flags()));
		handler->writeString("input", ValToInput(first));
		handler->endSectionWrite(); // Primary

		handler->beginSectionWrite(Section::Secondary);
		handler->writeString("cid", ValToCID(second.get_cid()));
		handler->writeString("flags", ValToCCF(second.get_flags()));
		handler->writeString("input", ValToInput(second));
		handler->endSectionWrite(); // Secondary

		handler->endSectionWrite();	// Control_config[i]
	}
	handler->endArrayWrite(); // Actions

	handler->flush();

	return true;
}

SCP_vector<CC_preset>::iterator preset_find_duplicate(const CC_preset& new_preset) {
	SCP_vector<CC_preset>::iterator it = Control_config_presets.begin();

	for (; it != Control_config_presets.end(); ++it) {
		size_t i;
		for (i = 0; i < it->bindings.size(); ++i) {
			if (new_preset.bindings[i] != it->bindings[i]) {
				// These two differ, check the next in the vector
				break;
			}
		}

		if (i == it->bindings.size()) {
			// These two are equal
			break;
		}
	}

	return it;
}

PresetFileHandler::PresetFileHandler(CFILE* cfp, bool reading) : _cfp(cfp) {
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
};

PresetFileHandler::~PresetFileHandler() {
	json_decref(_rootObj);

	cfclose(_cfp);
	_cfp = nullptr;
};

bool PresetFileHandler::beginSectionRead(Section s) {
	const char * sectionName = lookupSectionName(s);
	ensureExists(sectionName);

	auto section = json_object_get(_currentEl, sectionName);
	if (json_typeof(section) != JSON_OBJECT) {
		Error(LOCATION, "Section must be a JSON object!");
		return false;
	}

	pushElement(section);

	return true;
};

void PresetFileHandler::beginSectionWrite(Section s) {
	auto key_name = lookupSectionName(s);

	json_t* obj = json_object();

	if (json_is_array(_currentEl)) {
		// Currently in an array, section must be unnamed
		Assertion(key_name == nullptr, "Elements of array may not be named sections!");
		json_array_append_new(_currentEl, obj);
	} else {
		Assertion(key_name != nullptr, "Section outside of arrays must be named!");
		json_object_set_new(_currentEl, key_name, obj);
	}
	pushElement(obj);
};

bool PresetFileHandler::beginArrayRead(Section s, size_t &size) {
	Assertion(_arrayIndex == INVALID_SIZE, "Array nesting is not supported yet!");

	const char * name = lookupSectionName(s);
	ensureExists(name);

	auto array = json_object_get(_currentEl, name);
	if (json_typeof(array) != JSON_ARRAY) {
		Error(LOCATION, "Expected an array for '%s' but it was a different type!", name);
		return false;
	}

	pushElement(array);
	size = json_array_size(array);

	if (size == 0) {
		// Nothing to do here, avoid calling nextArraySection since that assumes that there is at least one element
		return true;
	} // Else, advance to the first element in the array

	_arrayIndex = 0;
	nextArrayElement();

	return true;
}

void PresetFileHandler::beginArrayWrite(Section s) {
	const char * name = lookupSectionName(s);
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

void PresetFileHandler::endSectionRead() {
	Assertion(json_typeof(_currentEl) == JSON_OBJECT, "Current element for section reading is not an object!");

	popElement();
};

void PresetFileHandler::endSectionWrite() {
	Assertion(json_is_object(_currentEl), "Section ended while not in a section!");

	popElement();
};

void PresetFileHandler::endArrayRead() {
	// First, remove the element we are currently in if it exists
	if (json_typeof(_currentEl) != JSON_ARRAY) {
		popElement();
	}

	Assertion(json_typeof(_currentEl) == JSON_ARRAY, "Current element must be an array!");

	// TODO Sections are straightforward, but Arrays may need special consideration re: index.
	popElement();
	_arrayIndex = INVALID_SIZE;
};

void PresetFileHandler::endArrayWrite() {
	Assertion(json_is_array(_currentEl), "Array ended while not in an array!");

	popElement();
};

void PresetFileHandler::ensureExists(const char* name) {
	if (json_typeof(_currentEl) != JSON_OBJECT) {
		Error(LOCATION, "JSON reading requires a value with name '%s' but the current element is not an object!", name);
	}
	if (json_object_get(_currentEl, name) == nullptr) {
		Error(LOCATION, "JSON reading requires a value with name '%s' but there is no such value!", name);
	}
}

void PresetFileHandler::ensureNotExists(const char* name) {
	// Stuff debug checks into booleans, otherwise clang will trigger a false positive for static method with just the asserts
	// Make sure we're in an element that can support keys
	bool supports_keys = json_is_object(_currentEl);

	// Make sure we don't overwrite previous values
	bool element_is_unique = json_object_get(_currentEl, name) == nullptr;

	Assertion(supports_keys, "Currently not in an element that supports keys!");
	Assertion(element_is_unique, "Entry with name %s already exists!", name);
}

bool PresetFileHandler::exists(const char* name) {
	if (json_typeof(_currentEl) != JSON_OBJECT) {
		return false;
	}
	if (json_object_get(_currentEl, name) == nullptr) {
		return false;
	}

	return true;
}

void PresetFileHandler::flush() {
	Assertion(_elementStack.size() == 1, "Not all sections or arrays have been ended!");

	json_dump_cfile(_rootObj, _cfp, JSON_INDENT(4) | JSON_PRESERVE_ORDER);
}

void PresetFileHandler::nextArrayElement() {
	Assertion(_arrayIndex != INVALID_SIZE, "Array index must be valid for this function!");
	bool in_section = json_is_object(_currentEl);

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

json_t * PresetFileHandler::popElement() {
	Assertion(_elementStack.size() > 1, "Element stack may not get smaller than one element!");

	_elementStack.pop_back();
	_currentEl = _elementStack.back();
	return _currentEl;
}

void PresetFileHandler::pushElement(json_t* el) {
	Assertion(el != nullptr, "Invalid JSON element pointer passed!");

	_currentEl = el;
	_elementStack.push_back(_currentEl);
}

SCP_string PresetFileHandler::readString(const char* key) {
	auto el = json_object_get(_currentEl, key);

	if (el == nullptr || json_typeof(el) != JSON_STRING) {
		Error(LOCATION, "JSON element %s must be a string but it is not valid!", key);
		return SCP_string();
	}
	auto json_str = json_string_value(el);
	SCP_string val;
	val.assign(json_str, json_str + json_string_length(el));

	return val;
}

int PresetFileHandler::readInt(const char* key) {
	auto el = json_object_get(_currentEl, key);

	if (el == nullptr || json_typeof(el) != JSON_INTEGER) {
		Error(LOCATION, "JSON element %s must be an integer but it is not valid!", key);
		return 0;
	}

	return static_cast<int>(json_integer_value(el));
}

void PresetFileHandler::writeString(const char* key, SCP_string s) {
	ensureNotExists(key);
	json_t *jstr = nullptr;

	if (!s.empty()) {
		// if this string isn't proper UTF-8, try to convert it
		if (utf8::find_invalid(s.begin(), s.end()) != s.end()) {
			SCP_string buffer;
			coerce_to_utf8(buffer, s.c_str());
			jstr = json_string(buffer.c_str());
		} else {
			jstr = json_string(s.c_str());
		}
	}

	json_object_set_new(_currentEl, key, jstr);
};

void PresetFileHandler::writeInt(const char* key, int val) {
	ensureNotExists(key);

	json_object_set_new(_currentEl, key, json_integer(val));
}
