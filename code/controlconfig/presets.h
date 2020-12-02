#pragma once

#include "cfile/cfile.h"
#include "controlconfig/controlsconfig.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "pilotfile/JSONFileHandler.h"

#include <jansson.h>

/** API for saving and loading player custom bindings, known as Presets

 Init.  If the player/preset dir doesn't exist. Create it, and print a help.txt with all enums (as strings) to label
 the button names and action names.

 Example output:
 {
	"actions": {
		"TARGET_NEXT": {
			"first": {
				"cid": 0,
				"flags": 0,
				"btn": 0
			},
			"second": {
				"cid": 0,
				"flags": 0,
				"btn": 0
			}
		},
		"TARGET_PREV" : {
			"first": {
				"cid": 0,
				"flags": 0,
				"btn": 0
			},
			"second": {
				"cid": 0,
				"flags": 0,
				"btn": 0
			}
		},
	},
	"version": 0
 }


*/
namespace io {
namespace presets {

static const unsigned int PST_VERSION = 0;
static const unsigned int PST_FILE_ID = 0x5f545350;  // "PST_" in file

enum class Section {
	Invalid = -1,
	Unammed = 0,
	Actions,

	// subsections
	Primary,
	Secondary
};

class PresetFileHandler : public pilot::JSONFileHandler {
public:
	PresetFileHandler(CFILE* cfp, bool reading) : JSONFileHandler(cfp, reading) {};

	/**
	 * Begins writing to a section using the presets::Section as its name
	 */
	void startSectionWrite(presets::Section id);
	
	/**
	 * Begins writing to a section using IoActionId as its name
	 */
	void startSectionWrite(int IoActionId);

protected:
	void startSectionWriteCommon(const char* key_name);

private:
/*
	CFILE* _cfp = nullptr;

	json_t* _rootObj = nullptr;

	json_t* _currentEl = nullptr;

	SCP_vector<json_t*> _elementStack;
	void pushElement(json_t* el);
	void popElement();

	void* _sectionIterator = nullptr;
	bool _startingSectionIteration = false;
	size_t _arrayIndex = INVALID_SIZE;
*/
};

class Preset {
public:
	/**
	 * @brief Saves the given preset to its own file
	 *
	 * @param[in] preset  Preset to save.  Uses the CC_preset::name to determine filename
	 */
	void save(CC_preset &preset);

	/**
	 * @brief Loads the given file into the preset
	 *
	 * @param[in]  file     filepath to the prest file
	 * @param[out] preset   preset to load into.  Will be named after the filename
	 */
	bool load(SCP_string file, CC_preset &preset);

protected:

private:

};


} // namespace presets
} // namespace io

bool test_write();
