#include "ship_info.h"

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

table_manager Ship_info_parse_items;

// Ship_info table definitions
void init_ship_info_parse_items() 
{
	auto factory = new parse_item_builder;

	parse_item_builder::set_table_type(&Ship_info_parse_items, Table_Types::SHIPS);

	// add the items one at a time, in the exact order the table needs them.
	// for every item, the contents are reset
	factory->new_parse_item(SCP_string("$Name:"))
		->set_description(
			SCP_string("The name of the ship class.  This will overwrite a previous entry of the same name, unless +no-create is used."))
		->set_type(Parse_Input_Types::STRING)
		->set_required(true)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+nocreate"))
		->set_description(SCP_string("In modular tables, allows you to add and replace info to entries from previous tables, without replacing all of the previous info. If the ship does not already exist, it will not be created. Should not be used with +Use Template:"))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	// not on wiki
	factory->new_parse_item(SCP_string("+remove"))
		->set_description(SCP_string("This will keep FSO from using all table entries of this ship class."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 20, 2)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Use Template:"))
		->set_description(SCP_string("Sets this table entry to use an existing, already-parsed ship entry as a template instead of creating totally new one. Should not be used with +nocreate."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Alt name:"))
		->set_description(SCP_string("Alternative $Name: for the ship.")) // TODO: Where is this used?
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Display Name:"))
		->set_description(SCP_string("Alternative $Name: for the ship."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Short name:"))
		->set_description(SCP_string("A name used in FRED to better describe the purpose of the ship."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Species:"))
		->set_description(SCP_string("The species entry for this ship. Need to match an entry in the Species.tbl"))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Type:"))
		->set_description(SCP_string("Descriptive type name for techroom and ship selection screens, like \"Heavy Assault Fighter\" or \"Escort Cruiser\"."))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Maneuverability:"))
		->set_description(SCP_string("How maneuverable this ship is, for techroom and ship selection screens, usually things like \"High\" \"Poor\" \"Very Poor\"/"))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Armor:"))
		->set_description(SCP_string("How much armor this ship has, for techroom and ship selection screens, usually things like \"High\" \"Poor\" \"Very Poor\"/"))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Manufacturer:"))
		->set_description(SCP_string("What company made this ship, for techroom and ship selection screens, usually things like \"Subach-Ines\" or \"Akheton Corporation\"."))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Description:"))
		->set_description(SCP_string("")) // TODO: what is this for?
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Tech Title:"))
		->set_description(SCP_string("Defines name of the ship as displayed in the tech room."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Tech Description:"))
		->set_description(SCP_string("The description displayed in the tech room."))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Length:"))
		->set_description(SCP_string("How long is this ship, for techroom and ship selection screens"))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Length:"))
		->set_description(SCP_string("How long is this ship, for techroom and ship selection screens"))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Gun Mounts:"))
		->set_description(SCP_string("How many primary gun mounts does this ship have, for techroom and ship selection screens"))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Missile Banks:"))
		->set_description(SCP_string("How many missile banks does this ship have, for techroom and ship selection screens"))
		->set_type(Parse_Input_Types::XSTR)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Selection Effect:"))
		->set_description(SCP_string("In the ship selection screen, which effect should be used, FS2, FS1, or OFF."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Cockpit POF file:"))
		->set_description(SCP_string("Filename for a cockpit model."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Cockpit Offset:"))
		->set_description(SCP_string("Defines the offset of the cockpit model."))
		->set_type(Parse_Input_Types::VEC3)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Cockpit Display:"))
		->set_description(SCP_string("Defines a rectangle surface on a cockpit texture that can be updated dynamically. Multiple displays can be defined, but only one needs to be defined for each texture on the cockpit model."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Texture:"))
		->set_description(SCP_string("The texture that is going to be used for this cockpit display."))
		->set_type(Parse_Input_Types::TEXTURE_FILENAME)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Offsets:"))
		->set_description(SCP_string("The offset for this texture onto the area on the cockpit model."))
		->set_type(Parse_Input_Types::VEC2)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Size:"))
		->set_description(SCP_string("The vertical and horizontal size of the display area."))
		->set_type(Parse_Input_Types::VEC2)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Background:"))
		->set_description(SCP_string("(Optional) Filename of the bitmap to be used as the background of this display."))
		->set_type(Parse_Input_Types::TEXTURE_FILENAME)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Foreground:"))
		->set_description(SCP_string("(Optional) Filename of the bitmap to be used as the foreground of this display."))
		->set_type(Parse_Input_Types::TEXTURE_FILENAME)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Display Name:"))
		->set_description(SCP_string("The name to reference this cockpit by in other tables."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("POF file:"))
		->set_description(SCP_string("The main pof model file for this ship."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("POF file Techroom:"))
		->set_description(SCP_string("A model file that is only used in the weapon loadout screen."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Texture Replace:"))
		->set_description(SCP_string("An entry used to denote a list of textures to replace."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+old:"))
		->set_description(SCP_string("The texture to be replaced."))
		->set_type(Parse_Input_Types::TEXTURE_FILENAME)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+new:"))
		->set_description(SCP_string("The replacing texture."))
		->set_type(Parse_Input_Types::TEXTURE_FILENAME)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$POF Target File:"))
		->set_description(SCP_string("Optional hud targeting model."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$POF Target LOD:"))
		->set_description(SCP_string("Defines the LOD (Level-Of-Detail) of target model used in the HUD targetbox."))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Detail Distance:"))
		->set_description(SCP_string("Defines the distance where the change between different Levels-Of-Details (LODs) occurs.\n"
			"Take notice that these are base values. Model detail in Detail options, (within game press F2), applies a multiplier to these values. These multipliers default to (from left to right): 1/8, 1/4, 1, 4, and 8, and may be overridden by the $Detail Distance Multiplier.\n"))
		->set_type(Parse_Input_Types::INT_LIST)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Collision LOD:"))
		->set_description(SCP_string("Defines an alternative LOD (Level-Of-Detail) to use for collision detection. Using this can increase performance without visible drawbacks if the LOD chosen has much simpler geometry but the shape still closely matches the base model."))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(3, 7, 4)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$ND:"))
		->set_description(SCP_string("Values for palettized rendering."))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(2, 0, 0)
		->set_deprecation_version(3, 6, 12)
		->set_deprecation_message(SCP_string("This is an approximate deprecation version since this was deprecated \"Before 2008\".  We simply do not use the palette code anymore."))
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Enable Team Colors:"))
		->set_description(SCP_string("Enables Team Colors for a ship without setting a default color setting."))
		->set_type(Parse_Input_Types::BOOL)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Default Team:"))
		->set_description(SCP_string("Specifies the default team name to be used for coloring a ship. None will set the default to 'no coloring'.\nNeeds to match an entry in colors.tbl!"))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 16)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Show Damage:"))
		->set_description(SCP_string("Never Implemented."))
		->set_type(Parse_Input_Types::BOOL)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Damage Lighting Type:"))
		->set_description(SCP_string("Changes the damage lightning effect shown on highly damaged ships. Only current options are 'None' and 'Default'"))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Impact:"))
		->set_description(SCP_string("For introducing options for impacts."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Damage Type:"))
		->set_description(SCP_string("Changes the damage type of collisions with this ship."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Impact Spew:"))
		->set_description(SCP_string("Tells the ship parser to look for options involving particle spew."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max particles:"))
		->set_description(SCP_string("Max number of particles for an impact spew. Can be set to zero"))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min particles:"))
		->set_description(SCP_string("Min number of particles for an impact spew."))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Radius:"))
		->set_description(SCP_string("The maximum radius for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Radius:"))
		->set_description(SCP_string("The minimum radius for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Man Lifetime:"))
		->set_description(SCP_string("The maximum lifetime for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Lifetime:"))
		->set_description(SCP_string("The minimum lifetime for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Velocity:"))
		->set_description(SCP_string("The maximum velocity for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Velocity:"))
		->set_description(SCP_string("The minimum velocity for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Normal Variance:"))
		->set_description(SCP_string("How much to vary the angle of the spew.  0 is no difference from the normal, 1 is 180 degrees, 2 is 360 degrees. Default is 1."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Radius:"))
		->set_description(SCP_string("The minimum radius for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Damage Spew:"))
		->set_description(SCP_string("Tells the ship parser to look for options involving \"smoke\" spew."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max particles:"))
		->set_description(SCP_string("Max number of particles for a damage spew. Can be set to zero"))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min particles:"))
		->set_description(SCP_string("Min number of particles for a damage spew."))
		->set_type(Parse_Input_Types::INT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Radius:"))
		->set_description(SCP_string("The maximum radius for particles for a damage spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Radius:"))
		->set_description(SCP_string("The minimum radius for particles for a damage spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Man Lifetime:"))
		->set_description(SCP_string("The maximum lifetime for particles for a damage spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Lifetime:"))
		->set_description(SCP_string("The minimum lifetime for particles for a damage spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Velocity:"))
		->set_description(SCP_string("The maximum velocity for particles for a damage spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Velocity:"))
		->set_description(SCP_string("The minimum velocity for particles for a damage spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Normal Variance:"))
		->set_description(SCP_string("How much to vary the angle of the spew.  0 is no difference from the normal, 1 is 180 degrees, 2 is 360 degrees. Default is 1."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Radius:"))
		->set_description(SCP_string("The minimum radius for particles for an impact spew."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Collision Physics:"))
		->set_description(SCP_string("A marker that tells FSO to start looking for collision phyiscs entries. These can be configured for a landing, meaning the ship will not take damage or shake when it touches down. Instead, the ship will reorient towards a neutral resting orientation. \"Reorient\" thresholds below mean that the ship will still take damage (counts as a crash) but will move towards the correct landing orientation."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Bounce:"))
		->set_description(SCP_string("When this ship collides with a large ship, how many meters to instantly move it away. Default is 5.0."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Both Small Bounce:"))
		->set_description(SCP_string("When this ship collides with a small ship, how many meters to instantly move it away. Default is 5.0."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Friction:"))
		->set_description(SCP_string("During collisions, how much friction to apply (slowing lateral movement). Default is 0.0."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Rotation Factor:"))
		->set_description(SCP_string("Affects the rotational energy of collisions. Default is 0.2."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Max Forward Vel:"))
		->set_description(SCP_string("Maximum velocity for which landing physics are used."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Min Forward Vel:"))
		->set_description(SCP_string("Minimum velocity for which landing physics are used."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Max Descent Vel:"))
		->set_description(SCP_string("Maximum velocity at which the ship can \"hit the deck\" and still be considered a landing. "))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Max Horizontal Vel:"))
		->set_description(SCP_string("Maximum sideways velocity the ship can land at."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Max Angle:"))
		->set_description(SCP_string("Maximum angle of attack the ship can land at (in degrees)."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Min Angle:"))
		->set_description(SCP_string("Maximum angle of attack the ship can land at (in degrees)."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Max Rotate Angle:"))
		->set_description(SCP_string("How many degrees the ship can be rotated relative to the landing surface in order to count as a landing."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Max Forward Vel:"))
		->set_description(SCP_string("Maximum velocity for which the ship will be adjusted to the correct landing orientation."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Min Forward Vel:"))
		->set_description(SCP_string("Minimum velocity for which the ship will be adjusted to the correct landing orientation."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Max Descent Vel:"))
		->set_description(SCP_string("Maximum velocity at which the ship can \"hit the deck\" and still reorient."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Max Horizontal Vel:"))
		->set_description(SCP_string("Maximum sideways velocity for reorienting."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Max Angle:"))
		->set_description(SCP_string("Maximum angle of attack for reorienting to kick in (in degrees)."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Min Angle:"))
		->set_description(SCP_string("Minimum angle of attack for reorienting to kick in (in degrees)."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Min Angle:"))
		->set_description(SCP_string("Minimum angle of attack for reorienting to kick in (in degrees)."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Max Rotate Angle:"))
		->set_description(SCP_string("How many degrees the ship can be rotated relative to the landing surface in order to be reoriented on impact."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Reorient Speed Mult:"))
		->set_description(SCP_string("How quickly the reorientation takes place (when applicable)"))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Rest Angle:"))
		->set_description(SCP_string("Angle of the ship's nose relative to the plane of the landing surface when the ship is at rest. Reorient will move the ship towards this angle."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Landing Sound:"))
		->set_description(SCP_string("Sound to play when landing (if it's not a landing, normal collision sound is used)."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(3, 6, 14)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Collision Sound Light:"))
		->set_description(SCP_string("Sound when this ship collides normally."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Collision Sound Heavy:"))
		->set_description(SCP_string("Sound when this ship collides at high speed."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Collision Sound Shielded:"))
		->set_description(SCP_string("Sound when this ship collides with shields active."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Debris:"))
		->set_description(SCP_string("An entry that tells fso to look for Debris options."))
		->set_type(Parse_Input_Types::MARKER)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Lifetime:"))
		->set_description(SCP_string("Defines the minimum lifetime of the debris in seconds (Default value is a random number) "))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Lifetime:"))
		->set_description(SCP_string("Defines the maximum lifetime of the debris in seconds"))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Speed:"))
		->set_description(SCP_string("Defines the minimum speed of the debris, in meters per second."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Speed:"))
		->set_description(SCP_string("Defines the maximum speed of the debris, in meters per second."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Rotation speed:"))
		->set_description(SCP_string("Defines the minimum rotational speed of the debris, in radians per second."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Rotation speed:"))
		->set_description(SCP_string("Defines the maximum rotational speed of the debris, in radians per second."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Damage Type:"))
		->set_description(SCP_string("Defines the damage type of the debris by referencing an armor table entry."))
		->set_type(Parse_Input_Types::STRING)
		->set_implementation_version(3, 6, 10)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Min Hitpoints:"))
		->set_description(SCP_string("Defines the minimum hitpoints assigned for generated debris pieces."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 12)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Max Hitpoints:"))
		->set_description(SCP_string("Defines the maximum hitpoints assigned for generated debris pieces."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 12)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Damage Multiplier:"))
		->set_description(SCP_string("Defines the collision damage multiplier for collisions against generated debris pieces."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 12)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Lightning Arc Percent:"))
		->set_description(SCP_string("Controls what percent of debris pieces will have the damage lightning effect applied to them. Valid values are from 0-100. Defaults to 50."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(3, 6, 12)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Ambient Sound:"))
		->set_description(SCP_string("The sound played in the vicinity of debris."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Collision Sound Light:"))
		->set_description(SCP_string("The sound played in the case of a 'light' collision."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Collision Sound Heavy:"))
		->set_description(SCP_string("The sound played in the case of a 'heavy' collision."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("+Explosion Sound:"))
		->set_description(SCP_string("The sound played when a debris piece explodes."))
		->set_type(Parse_Input_Types::SOUND_FILENAME)
		->set_implementation_version(20, 2, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Density:"))
		->set_description(SCP_string("Multiplies the mass and moment of inertia of the relevant ship. High values reduce the spin and displacement from collisions."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Damp:"))
		->set_description(SCP_string("Affects how quickly you will accel/decel to your target velocity. High values increase the time required. Low values decrease it, with 0.0 being no effect."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Rotdamp:"))
		->set_description(SCP_string("Affects how quickly you will accel/decel to your target rotational velocity. High values increase the time required. Low values decrease it, with 0.0 being no effect."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);



	factory->new_parse_item(SCP_string("$Banking Constant:"))
		->set_description(SCP_string("Defines a factor for how much roll is added during a yaw. Set as 1 for full roll and 0 is no roll. Default is 0.5."))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Ship_info_parse_items);
}

