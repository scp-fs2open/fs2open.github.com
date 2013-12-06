/*
 * Species_Defs.CPP
 * Extended Species Support for FS2 Open
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */


#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"
#include "species_defs/species_defs.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"
#include "iff_defs/iff_defs.h"
#include "graphics/generic.h"
#include "localization/localize.h"


SCP_vector<species_info> Species_info;

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This function parses the data from the species_defs.tbl
// Names only - actual loading is done elsewhere

void parse_thrust_anims(species_info *species, bool no_create)
{
	if (!no_create)
	{
		required_string("$ThrustAnims:");

		generic_anim_init(&species->thruster_info.flames.normal, NULL);
		generic_anim_init(&species->thruster_info.flames.afterburn, NULL);
		generic_bitmap_init(&species->thruster_secondary_glow_info.normal, NULL);
		generic_bitmap_init(&species->thruster_secondary_glow_info.afterburn, NULL);
		generic_bitmap_init(&species->thruster_tertiary_glow_info.normal, NULL);
		generic_bitmap_init(&species->thruster_tertiary_glow_info.afterburn, NULL);
		generic_bitmap_init(&species->thruster_distortion_info.normal, NULL);
		generic_bitmap_init(&species->thruster_distortion_info.afterburn, NULL);
	}
	else if (!optional_string("$ThrustAnims:"))
	{
		return;
	}

	// favor new style
	if (no_create)
	{
		if (optional_string("+Pri_Normal:") || optional_string("+Normal:"))
			stuff_string(species->thruster_info.flames.normal.filename, F_NAME, MAX_FILENAME_LEN);
	}
	else
	{
		if (!optional_string("+Pri_Normal:"))
			required_string("+Normal:");

		stuff_string(species->thruster_info.flames.normal.filename, F_NAME, MAX_FILENAME_LEN);
	}

	// if no primary thruster anim is wanted then clear it
	if ( !VALID_FNAME(species->thruster_info.flames.normal.filename) )
		generic_anim_init(&species->thruster_info.flames.normal, NULL);

	// and again
	if (no_create)
	{
		if (optional_string("+Pri_Afterburn:") || optional_string("+Afterburn:"))
			stuff_string(species->thruster_info.flames.afterburn.filename, F_NAME, MAX_FILENAME_LEN);
	}
	else
	{
		if (!optional_string("+Pri_Afterburn:"))
			required_string("+Afterburn:");

		stuff_string(species->thruster_info.flames.afterburn.filename, F_NAME, MAX_FILENAME_LEN);
	}

	// if no primary thruster anim is wanted then clear it
	if ( !VALID_FNAME(species->thruster_info.flames.afterburn.filename) )
		generic_anim_init(&species->thruster_info.flames.afterburn, NULL);

	// extra thruster stuff, bah
	if (optional_string("+Sec_Normal:"))
		stuff_string(species->thruster_secondary_glow_info.normal.filename, F_NAME, MAX_FILENAME_LEN);

	// etc.
	if (optional_string("+Sec_Afterburn:"))
		stuff_string(species->thruster_secondary_glow_info.afterburn.filename, F_NAME, MAX_FILENAME_LEN);

	// etc.
	if (optional_string("+Ter_Normal:"))
		stuff_string(species->thruster_tertiary_glow_info.normal.filename, F_NAME, MAX_FILENAME_LEN);

	// etc.
	if (optional_string("+Ter_Afterburn:"))
		stuff_string(species->thruster_tertiary_glow_info.afterburn.filename, F_NAME, MAX_FILENAME_LEN);
	
	// etc.
	if (optional_string("+Dist_Normal:"))
		stuff_string(species->thruster_distortion_info.normal.filename, F_NAME, MAX_FILENAME_LEN);

	// etc.
	if (optional_string("+Dist_Afterburn:"))
		stuff_string(species->thruster_distortion_info.afterburn.filename, F_NAME, MAX_FILENAME_LEN);
}

void parse_thrust_glows(species_info *species, bool no_create)
{
	if (!no_create)
	{
		required_string("$ThrustGlows:");

		generic_anim_init(&species->thruster_info.glow.normal, NULL);
		generic_anim_init(&species->thruster_info.glow.afterburn, NULL);
	}
	else if (!optional_string("$ThrustGlows:"))
	{
		return;
	}

	if (no_create)
	{
		if (optional_string("+Normal:"))
			stuff_string(species->thruster_info.glow.normal.filename, F_NAME, MAX_FILENAME_LEN);
	}
	else
	{
		required_string("+Normal:");
		stuff_string(species->thruster_info.glow.normal.filename, F_NAME, MAX_FILENAME_LEN);
	}

	// if no glow is wanted then clear it
	if ( !VALID_FNAME(species->thruster_info.glow.normal.filename) )
		generic_anim_init(&species->thruster_info.glow.normal, NULL);

	if (no_create)
	{
		if (optional_string("+Afterburn:"))
			stuff_string(species->thruster_info.glow.afterburn.filename, F_NAME, MAX_FILENAME_LEN);
	}
	else
	{
		required_string("+Afterburn:");
		stuff_string(species->thruster_info.glow.afterburn.filename, F_NAME, MAX_FILENAME_LEN);
	}

	// if no glow is wanted then clear it
	if ( !VALID_FNAME(species->thruster_info.glow.afterburn.filename) )
		generic_anim_init(&species->thruster_info.glow.afterburn, NULL);
}

void parse_species_tbl(const char *filename)
{
	int i, rval;
	char species_name[NAME_LENGTH];

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", (filename) ? filename : NOX("<default species_defs.tbl>"), rval));
		lcl_ext_close();
		return;
	}

	if (filename == NULL)
		read_file_text_from_array(defaults_get_file("species_defs.tbl"));
	else
		read_file_text(filename, CF_TYPE_TABLES);

	reset_parse();		


	// start parsing
	required_string("#SPECIES DEFS");

	// no longer required: counted automatically
	if (optional_string("$NumSpecies:"))
	{
		int temp;
		stuff_int(&temp);
	}

	// begin reading data
	while (required_string_either("#END","$Species_Name:"))
	{
		bool no_create = false;
		species_info *species, new_species;

		species = &new_species;

		// Start Species - Get its name
		required_string("$Species_Name:");
		stuff_string(species_name, F_NAME, NAME_LENGTH);

		if (optional_string("+nocreate"))
		{
			no_create = true;

			for (i = 0; i < (int)Species_info.size(); i++)
			{
				if (!stricmp(Species_info[i].species_name, species_name))
				{
					species = &Species_info[i];
					break;
				}
			}
		}
		else
		{
			strcpy_s(species->species_name, species_name);
		}

		// Goober5000 - IFF
		if (optional_string("$Default IFF:"))
		{
			bool iff_found = false;
			char temp_name[NAME_LENGTH];
			stuff_string(temp_name, F_NAME, NAME_LENGTH);

			// search for it in iffs
			for (int iLoop = 0; iLoop < Num_iffs; iLoop++)
			{
				if (!stricmp(Iff_info[iLoop].iff_name, temp_name))
				{
					species->default_iff = iLoop;
					iff_found = true;
				}
			}

			if (!iff_found)
			{
				species->default_iff = 0;
				Warning(LOCATION, "Species %s default IFF %s not found in iff_defs.tbl!  Defaulting to %s.\n", species->species_name, temp_name, Iff_info[species->default_iff].iff_name);
			}
		}
		else if (!no_create)
		{
			// we have no idea which it could be, so default to 0
			species->default_iff = 0;

			// let them know
			Warning(LOCATION, "$Default IFF not specified for species %s in species_defs.tbl!  Defaulting to %s.\n", species->species_name, Iff_info[species->default_iff].iff_name);
		}

		// Goober5000 - FRED color
		if (optional_string("$FRED Color:") || optional_string("$FRED Colour:"))
		{
			stuff_int_list(species->fred_color.a1d, 3, RAW_INTEGER_TYPE);
		}
		else if (!no_create)
		{
			// set defaults to Volition's originals
			if (!stricmp(species->species_name, "Terran"))
			{
				species->fred_color.rgb.r = 0;
				species->fred_color.rgb.g = 0;
				species->fred_color.rgb.b = 192;
			}
			else if (!stricmp(species->species_name, "Vasudan"))
			{
				species->fred_color.rgb.r = 0;
				species->fred_color.rgb.g = 128;
				species->fred_color.rgb.b = 0;
			}
			else if (!stricmp(species->species_name, "Shivan"))
			{
				species->fred_color.rgb.r = 192;
				species->fred_color.rgb.g = 0;
				species->fred_color.rgb.b = 0;
			}
			else if (!stricmp(species->species_name, "Ancients") || !stricmp(species->species_name, "Ancient"))
			{
				species->fred_color.rgb.r = 192;
				species->fred_color.rgb.g = 0;
				species->fred_color.rgb.b = 192;
			}
			else
			{
				species->fred_color.rgb.r = 0;
				species->fred_color.rgb.g = 0;
				species->fred_color.rgb.b = 0;
			}

			// let them know
			Warning(LOCATION, "$FRED Color not specified for species %s in species_defs.tbl!  Defaulting to (%d, %d, %d).\n", species->species_name, species->fred_color.rgb.r, species->fred_color.rgb.g, species->fred_color.rgb.b);
		}

		// stuff
		optional_string("$MiscAnims:");

		// Get its Debris Texture
		if ((!no_create && required_string("+Debris_Texture:")) || optional_string("+Debris_Texture:"))
		{
			generic_bitmap_init(&species->debris_texture, NULL);
			stuff_string(species->debris_texture.filename, F_NAME, MAX_FILENAME_LEN);
		}


		// Shield Hit Animation
        if (optional_string("+Shield_Hit_ani:")) // Shouldn't be required -- LPine
		{
			generic_anim_init(&species->shield_anim, NULL);
			stuff_string(species->shield_anim.filename, F_NAME, MAX_FILENAME_LEN);
		}
        else if (!no_create)
        {
            species->shield_anim.filename[0] = '\0';
            species->shield_anim.first_frame = -1; // Landmine to trip up anyone who does end up using this
        }


		// Thruster Anims
		parse_thrust_anims(species, no_create);

		// Thruster Glow Anims
		parse_thrust_glows(species, no_create);


		// Goober5000 - AWACS multiplier
		if (optional_string("$AwacsMultiplier:"))
		{
			stuff_float(&species->awacs_multiplier);
		}
		else if (!no_create)
		{
			// set defaults to Volition's originals
			if (!stricmp(species->species_name, "Vasudan"))
				species->awacs_multiplier = 1.25f;
			else if (!stricmp(species->species_name, "Shivan"))
				species->awacs_multiplier = 1.50f;
			else
				species->awacs_multiplier = 1.0f;

			// let them know
			Warning(LOCATION, "$AwacsMultiplier not specified for species %s in species_defs.tbl!  Defaulting to %.2d.\n", species->species_name, species->awacs_multiplier);
		}

		// Goober5000 - countermeasure type
		// (we won't be able to resolve it until after we've parsed the weapons table)
		if (optional_string("$Countermeasure type:"))
			stuff_string(species->cmeasure_name, F_NAME, NAME_LENGTH);

		// don't add new entry if this is just a modified one
		if ( !no_create )
			Species_info.push_back( new_species );
	}
	
	required_string("#END");

	// add tbl/tbm to multiplayer validation list
	extern void fs2netd_add_table_validation(const char *tblname);
	fs2netd_add_table_validation(filename);

	// close localization
	lcl_ext_close();
}

int Species_initted = 0;

void species_init()
{
	if (Species_initted)
		return;

	Species_info.clear();


	if (cf_exists_full("species_defs.tbl", CF_TYPE_TABLES))
		parse_species_tbl("species_defs.tbl");
	else
		parse_species_tbl(NULL);

	parse_modular_table("*-sdf.tbm", parse_species_tbl);


	Species_initted = 1;
}
