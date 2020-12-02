#include "controlconfig/controlsconfig.h"
#include "controlconfig/presets.h"
#include "libs/jansson.h"
#include "parse/parselo.h"

#include <jansson.h>

using namespace io::presets;

const char * lookupSectionName(Section id) {
	switch (id) {
	case Section::Unammed:
		return "";

	case Section::Actions:
		return "actions";

	case Section::Primary:
		return "primary";

	case Section::Secondary:
		return "secondary";

	case Section::Invalid:
	default:
		return nullptr;
	}
}

bool test_write() {
	SCP_string filename = "testPreset.json";
	std::unique_ptr<PresetFileHandler> handler = nullptr;

	// " why is the .plr file opening this in wb?
	CFILE* fp = cfopen(filename.c_str(), "w", CFILE_NORMAL, CF_TYPE_PLAYERS, false,
	                  CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	
	if (!fp) {
		mprintf(("PLR => Unable to open '%s' for saving!\n", filename.c_str()));
		return false;
	}

	try {
		handler.reset(new PresetFileHandler(fp, false));
	} catch (const std::exception& e) {
		mprintf(("PLR => Failed to open JSON: %s\n", e.what()));
		return false;
	}

	// Write header and version
	handler->writeInt("signature", PST_FILE_ID);
	handler->writeUByte("version", PST_VERSION);

	mprintf(("PST => Saving %s with version %d...\n", filename.c_str(), (int)PST_VERSION));

	handler->beginWritingSections();
	
	handler->startSectionWrite(Section::Actions);
	for (auto i = 0; i < Control_config.size(); ++i) {
		const auto& item = Control_config[i];
		const auto& first = item.first;
		const auto& second = item.second;

		handler->startSectionWrite(i);

		handler->startSectionWrite(Section::Primary);
		handler->writeString("cid",   mValToCID(first.cid));
		handler->writeString("flags", mValToCCF(first.flags).c_str());
		handler->writeString("input", mValToInput(first).c_str());
		handler->endSectionWrite(); // Primary

		handler->startSectionWrite(Section::Secondary);
		handler->writeString("cid", mValToCID(second.cid));
		handler->writeString("flags", mValToCCF(second.flags).c_str());
		handler->writeString("input", mValToInput(second).c_str());
		handler->endSectionWrite(); // Secondary

		handler->endSectionWrite(); // Control_config[i]
	}
	handler->endSectionWrite(); // Actions

	handler->endWritingSections();
	handler->flush();

	return true;
}

void PresetFileHandler::startSectionWrite(Section id) {
	auto key_name = lookupSectionName(id);
	startSectionWriteCommon(key_name);
}


void PresetFileHandler::startSectionWrite(int IoActionId) {
	auto key_name = mValToAction(IoActionId);
	startSectionWriteCommon(key_name);
}

void PresetFileHandler::startSectionWriteCommon(const char *key_name) {
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

