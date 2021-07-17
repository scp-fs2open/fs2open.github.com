#include "asteroid_info.h"

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

table_manager Asteroid_parse_items;

// asteroid table definitions
void init_asteroid_parse_items()
{
	auto factory = new parse_item_builder;

	parse_item_builder::set_table_type(&Asteroid_parse_items, Table_Types::ASTEROID);

	factory->new_parse_item(SCP_string("$Name:"))
		->set_description(SCP_string("Defines the name of asteroid or debris object"))
		->set_type(Parse_Input_Types::STRING)
		->set_required(true)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Asteroid_parse_items);


	factory->new_parse_item(SCP_string("$POF file1:"))
		->set_description(SCP_string("Defines the name of the model file (.pof) used for asteroid or debris model.  Debris models will only use POF file1."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Asteroid_parse_items);


	factory->new_parse_item(SCP_string("$POF file2:"))
		->set_description(SCP_string("Defines the name of the alternate asteroid or debris model of the set size. "
			"This does not work for debris chunks. The game will always only use POF file1."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Asteroid_parse_items);


	factory->new_parse_item(SCP_string("$POF file3:"))
		->set_description(SCP_string("Defines the name of the alternate asteroid model. "
			"This does not work for debris chunks. The game will always only use POF file1. "
			"If the object in question is has $POF file2: set as 'none' then this entry is not needed."))
		->set_type(Parse_Input_Types::MODEL_FILENAME)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Detail distance:"))
		->set_description(SCP_string("Definies the distance where the change between different Levels-Of-Details (LODs) occurs, needs one for each LOD."))
		->set_type(Parse_Input_Types::INT_LIST)
		->set_required(true)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Max Speed:"))
		->set_description(SCP_string("Defines the maximum velocity of the asteroid"))
		->set_type(Parse_Input_Types::FLOAT)
		->set_implementation_version(2, 0, 0)
		->finished_product(&Asteroid_parse_items);


	factory->new_parse_item(SCP_string("$Damage Type:"))
		->set_description(SCP_string("*Defines the damage type of the asteroid explosion. Requires armor.tbl"))
		->set_implementation_version(3, 6, 10)
		->set_type(Parse_Input_Types::STRING)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Explosion Animations:"))
		->set_description(SCP_string("Defines the explosion animations used for the asteroid, using the index from fireball.tbl."))
		->set_implementation_version(3, 6, 15)
		->set_type(Parse_Input_Types::INT_LIST)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Explosion Radius Mult:"))
		->set_description(SCP_string("Defines the radius multiplier for the explosion fireballs. "
			"Setting to 1.0 causes the radius of the fireball to be equal to the radius of the asteroid. "
			"Defaults to 1.5 for large asteroids and 1.0 for others."))
		->set_implementation_version(3, 6, 15)
		->set_type(Parse_Input_Types::FLOAT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Expl inner rad:"))
		->set_description(SCP_string("Radius at which the full explosion damage is done, in meters."))
		->set_implementation_version(2, 0, 0)
		->set_type(Parse_Input_Types::FLOAT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Expl outer rad:"))
		->set_description(SCP_string("Maximum radius at which any damage is done, in meters."))
		->set_implementation_version(2, 0, 0)
		->set_type(Parse_Input_Types::FLOAT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Expl damage:"))
		->set_description(SCP_string("Amount of damage done inside the inner radius."))
		->set_implementation_version(2, 0, 0)
		->set_type(Parse_Input_Types::FLOAT)
		->finished_product(&Asteroid_parse_items);


	factory->new_parse_item(SCP_string("$Expl blast:"))
		->set_description(SCP_string("The intensity of the blast effect when you're within the outer radius."))
		->set_implementation_version(2, 0, 0)
		->set_type(Parse_Input_Types::FLOAT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Hitpoints:"))
		->set_description(SCP_string("Defines the hitpoints of the asteroid or debris piece. "
			"Hitpoints are modified downward by skill level."))
		->set_implementation_version(2, 0, 0)
		->set_type(Parse_Input_Types::FLOAT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("$Split:"))
		->set_description(SCP_string("Defines what kind of asteroids this type of asteroid should split into when destroyed. "
			"You may define any number of $Split rules. The int references an asteroid defined by this table. "
			"The first asteroid defined by the table has index 0, the second one has index 1, etc."))
		->set_implementation_version(3, 6, 15)
		->set_type(Parse_Input_Types::INT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("+Min:"))
		->set_description(SCP_string("The minimum number of spawned asteroids. Default 0."))
		->set_implementation_version(3, 6, 15)
		->set_type(Parse_Input_Types::INT)
		->finished_product(&Asteroid_parse_items);

	factory->new_parse_item(SCP_string("+Max:"))
		->set_description(SCP_string("The maximum number of spawned asteroids. Default 0."))
		->set_implementation_version(3, 6, 15)
		->set_type(Parse_Input_Types::INT)
		->finished_product(&Asteroid_parse_items);

	Asteroid_parse_items.dump_to_file();

}
