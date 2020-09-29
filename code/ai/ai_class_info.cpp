#include "ai_class_info.h"

/*	quick reference!!

_table_string   = "Name"				// string, name of the itme.
_description    = "Description"			// string, the description that modders will use to figure out how to use this item.
_type   = Parse_Input_Types::			// which type of input this is, uses the Parse_Input_Types enum class
_required								// bool, is this a required field?
_enforced_count 						// int, how many inputs are *required* for this field
_major_version_number_implemented   	// int, retail is 2
_minor_version_number_implemented		// int, retail is 0 
_revision_number_implemented	   		// int, retail is 0
_major_version_number_deprecated		// int, usually not defined unless this option was actually deprecated.
_minor_version_number_deprecated		// int, usually not defined unless this option was actually deprecated.
_revision_number_deprecated				// int, usually not defined unless this option was actually deprecated.
_deprecation_message					// string why was this deprecated??
*/

// ai (ai_class info) table definitions
ai_class_info_parse_items::ai_class_info_parse_items()
{
	auto factory = new parse_item_builder;

	// add the items one at a time, in the exact order the table needs them.
	// NOTE! the _enforced_count and deprecation fields were ignored.
	_all_items.push_back(*factory->new_parse_item(SCP_string("$Name:"))
		.set_description(SCP_string("The name of the AI class. Does not support +nocreate."))
		.set_type(Parse_Input_Types::STRING)
		.set_required(true)
		.set_implementation_version(2, 0, 0)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Accuracy:"))
		.set_description(SCP_string("How accurately the ship fires its weapons. Value is used to scale the error in the AI aim."
									" With repeated shots, AI aim will improve. "
									"Note that the AI is always 100% accurate when aiming for subsystems, according to Retail code.\n\n"
									"Valid values are five entries from 0.0 to 1.0 and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(true)
		.set_implementation_version(2, 0, 0)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Evasion:"))
		.set_description(SCP_string("How good the ship is at evading. Value defines the frequency of AI course changes while it is evading.\n\n"
			"Valid values are five entries from 0 to 100 and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(true)
		.set_implementation_version(2, 0, 0)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Courage:"))
		.set_description(SCP_string("How likely the ship is to chance danger to accomplish goals. Basically the lower the value sooner the AI will start evading when attacked and its less likely to target turrets.\n\n"
			"Valid values are five entries from 0 to 100 and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(true)
		.set_implementation_version(2, 0, 0)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Patience:"))
		.set_description(SCP_string("This value affects how quickly the AI will attempt to \"break\" a stalemate situation if one is detected."
			"Only applies if $Stalemate Time Threshold and $Stalemate Distance Threshold are used. The higher the patience, the longer the ship will wait in \"stalemate\" before breaking. "
			"If patience is 100, the AI will never break.\n\n"
			"This entry is always required, even before its 3.6.12 implementation version.\n\n"
			"Valid values are five entries from 0 to 100 and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(true)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("Usage Note"))
		.set_description(SCP_string("If the following four attributes are *not* specified, the code uses the value referenced by the index of the AI class instead. \n\n"
									"Use these if you want finer-grained control over the AI classes and/or you don't want the order of the AI classes to affect their behavior. "
									"If these are not specified, the behaviors described will be determined by the order of the AI class relative to the others in the file. "
									"Furthermore, having a different number of AI classes than retail can result in strange behavior if these are *not* set.\n\n"
									"It is recommended that if you make heavy use of custom AI classes, you specify all of these *and* set $Autoscale by AI Class Index: to \"No\"."))
		.set_type(Parse_Input_Types::USAGE_NOTE)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(true)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Afterburner Use Factor:"))
		.set_description(SCP_string("Affects how probably it is that the AI will use afterburners in \"maybe\" situations. "
									"A value of 1 means always, 2 means half the time, 3 a third of the time, etc."
									"\n\nValid values are five entries above 0 and correspond to the difficulty levels from lowest to highest."
									"\n\nSee usage note when deciding whether to use this entry."))
		.set_type(Parse_Input_Types::INT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Shockwave Evade Chances Per Second:"))
		.set_description(SCP_string("Controls how likely it is for a ship to try to start evading a shockwave. The higher the number, the more \"chances per second\" the ship has to evade."
									"\n\nValid values are five entries from 0.0 and 1.0 and correspond to the difficulty levels from lowest to highest."
									"\n\nSee usage note when deciding whether to use this entry."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Get Away Chance:"))
		.set_description(SCP_string("How likely the AI is to use a \"get away\" maneuver instead of simply making evasive turns. \"Get away\" usually involves the AI flying straight away," 
									"usually on afterburner, and making small jinking motions (instead of large evasive turns). Higher values result in more \"jousting\" fights."
									"\n\nValid values are five entries from 0.0 and 1.0 and correspond to the difficulty levels from lowest to highest."
									"\n\nSee usage note when deciding whether to use this entry."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Secondary Range Multiplier:"))
		.set_description(SCP_string("Multiplier which affects from how far away the AI will begin firing secondary weapons. Capped by the actual maximum range of the weapon. "
									"Penalty for firing in nebula still applies above and beyond this."
									"\n\nValid values are five entries from 0.0 and 1.0 and correspond to the difficulty levels from lowest to highest."
									"\n\nSee usage note when deciding whether to use this entry."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$Autoscale by AI Class Index:"))
		.set_description(SCP_string("Keeps FSO from using default ordering behavior for AI Class entries, if the previous 4 entries are defined for this AI class.\n\n"
									"\n\nValid values are five entries from 0.0 and 1.0 and correspond to the difficulty levels from lowest to highest."
									"\n\nSee usage note when deciding whether to use this entry."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$AI Countermeasure Firing Chance:"))
		.set_description(SCP_string("The percent chance for a countermeasure to be fired. If not specified, values from ai_profiles.tbl will be used.\n\n"
									"Valid values are five entries from 0.0 and 1.0 and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("$AI In Range Time:"))
		.set_description(SCP_string("The delay (in seconds) for the AI to fire its weapons after getting in range. If not specified, values from ai_profiles.tbl will be used.\n\n"
			"Valid values are five entries, from -1 up, and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("AI Always Links Ammo Weapons:"))
		.set_description(SCP_string("AI ships will link ballistic primaries if ammo levels are greater than these percents. If not specified, values from ai_profiles.tbl will be used.\n\n"
			"Valid values are five entries 0.0 and up and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());

	_all_items.push_back(*factory->new_parse_item(SCP_string("AI Maybe Links Ammo Weapons:"))
		.set_description(SCP_string("Defines the same as $AI Always Links Ammo Weapons: but in situations when its hull is below 33 % of maximum hitpoints. If not specified, values from ai_profiles.tbl will be used.\n\n"
			"Valid values are five entries 0.0 and up and correspond to the difficulty levels from lowest to highest."))
		.set_type(Parse_Input_Types::FLOAT_LIST)
		.set_enforced_count(NUM_SKILL_LEVELS)
		.set_required(false)
		.set_implementation_version(3, 6, 12)
		.finished_product());
}