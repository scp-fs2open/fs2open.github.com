#pragma once

#include "cfile/cfile.h"
#include "controlconfig/controlsconfig.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

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
	Unnamed = 0,
	
	// arrays
	Actions,

	// subsections
	Primary,
	Secondary
};

class PresetFileHandler {
public:
	PresetFileHandler(CFILE* cfp, bool reading);
	~PresetFileHandler();

	/**
	 * begins writing to a section using the presets::Section as its name
	 * @param[in] s     id of the section
	 */
	void beginSectionWrite(presets::Section s);

	/**
	 * begins reading a section, if it exists
	 * @param[in] s    id of the section
	 */
	bool beginSectionRead(presets::Section s);

	/**
	 * Begins writing to an array using the presets::Section as its name
	 *
	 * @param[in] s     id of the array
	 */
	void beginArrayWrite(presets::Section s);

	/**
	 * Begins reading from an array
	 * @param[int] s    id of the array
	 * @param[out] size the size of the array
	 */
	bool beginArrayRead(presets::Section s, size_t &size);

	/**
	 * ends writing to a section
	 */
	void endSectionWrite();

	/**
	 * ends reading a section
	 */
	void endSectionRead();

	/**
	 * ends writing to an array.
	 */
	void endArrayWrite();

	/**
	 * ends reading from an array
	 */
	void endArrayRead();

	/**
	 * Writes a string with the given key name
	 */
	void writeString(const char* key, SCP_string s);

	/**
	 * Reads a string with the given key name
	 */
	SCP_string readString(const char* key);

	/**
	 * Writes an int with the given key name
	 */
	void writeInt(const char* key, int val);

	/**
	 * Reads an int with the given key name
	 */
	int readInt(const char* key);

	/**
	 * For writing, Flushes the buffers and dumps to file
	 */
	void flush();

	/**
	 * Advances the array parser to the next element
	 */
	void nextArrayElement();

private:
	CFILE* _cfp = nullptr;

	json_t* _rootObj = nullptr;

	json_t* _currentEl = nullptr;

	/**
	 * BufferStack of all json elements being read/written
	 */
	SCP_vector<json_t*> _elementStack;

	/**
	 * Stack to keep track of the heirachy/nesting. Back is immediate parent, Front is root
	 */
	SCP_vector<void*> _parentStack;

	size_t _arrayIndex = INVALID_SIZE;

	//void* _sectionIterator = nullptr;	//!< Pointer to the current section/array we're in

private:
	/**
	 * Pushes an element onto the elementStack
	 */
	void pushElement(json_t* el);
	
	/**
	 * Pops an element off the elementStack
	 */
	json_t * popElement();

	/**
	 * Used for optionals, checks if an element exists with the given name
	 * @return True     if the element exists
	 * @return False    otherwise
	 */
	bool exists(const char* name);

	/**
	 * Throws an error if an element with the given name does not exist
	 */
	void ensureExists(const char* name);

	/**
	 * Throws an error if an element with the given name exists
	 */
	void ensureNotExists(const char* name);
};

} // namespace presets
} // namespace io

/**
 * @brief Saves the given preset to file.
 * @param[in] preset    The preset to save
 * @param[in] overwrite If true, overwrite existing preset file which have the same name as the given preset
 *
 * @returns True    if successful, or
 * @returns False   Preset does not have a name, or
 * @returns False   Preset file exists and overwrite == false, or
 * @returns False   Preset file .JSON could not be written
 */
bool save_preset_file(CC_preset preset, bool overwrite);

/**
 * @brief Loads in all preset .json files from 'players/presets'
 * @details If a preset file is unique, it is loaded into the game and available for use by the player.  However, if
 * a preset is a duplicate of another preset, it is ignored, and the player is warned of it
 */
void load_preset_files();

/**
 * @brief Checks if the given preset is a duplicate within Control_config_presets vector
 * @returns tterator to the duplicate if found, or
 * @returns iterator to Control_config_presets.end() otherwise
 */
SCP_vector<CC_preset>::iterator preset_find_duplicate(const CC_preset& new_preset);


/**
 * @brief Returns true if a preset file with the given name exists.
 */
bool preset_file_exists(SCP_string name);
