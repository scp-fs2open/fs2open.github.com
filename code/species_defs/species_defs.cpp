/*
 * Species_Defs.CPP
 * Extended Species Support for FS2 Open
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

/*
 * $Logfile: /Freespace2/code/species_defs/species_defs.cpp $
 * $Revision: 1.32.2.6 $
 * $Date: 2007-10-17 20:58:25 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.32.2.5  2007/10/15 06:43:22  taylor
 * FS2NetD v.2  (still a work in progress, but is ~98% complete)
 *
 * Revision 1.32.2.4  2007/09/02 02:07:47  Goober5000
 * added fixes for #1415 and #1483, made sure every read_file_text had a corresponding setjmp, and sync'd the parse error messages between HEAD and stable
 *
 * Revision 1.32.2.3  2007/07/28 04:43:43  Goober5000
 * a couple of tweaks
 *
 * Revision 1.32.2.2  2006/09/11 01:17:07  taylor
 * fixes for stuff_string() bounds checking
 *
 * Revision 1.32.2.1  2006/08/27 18:12:42  taylor
 * make Species_info[] and Asteroid_info[] dynamic
 *
 * Revision 1.32  2006/02/13 03:11:19  Goober5000
 * nitpick
 *
 * Revision 1.31  2006/02/13 00:20:46  Goober5000
 * more tweaks, plus clarification of checks for the existence of files
 * --Goober5000
 *
 * Revision 1.30  2006/02/06 02:29:17  wmcoolmon
 * WMC-Since we're abusing species_defs.tbl anyways, remove $NumSpecies
 *
 * Revision 1.29  2006/01/26 03:23:30  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
 * Revision 1.28  2006/01/05 05:12:11  taylor
 * allow both +Pri style and original style (+Normal, etc) for species_defs TBMs
 * allow for "<none>" as a bitmap/anim name, to have no effect
 * fix ship/weapon thruster rendering to handle missing primary animation and/or glow graphics
 *
 * Revision 1.27  2005/12/29 08:08:42  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 1.26  2005/12/29 00:01:29  taylor
 * add support for a species_defs modular table with XMT support (*-sdf.tbm)
 *
 * Revision 1.25  2005/11/21 23:57:26  taylor
 * some minor thruster cleanup, if you could actually use the term "clean"
 *
 * Revision 1.24  2005/10/24 12:42:14  taylor
 * init thruster stuff properly so that bmpman doesn't have a fit
 *
 * Revision 1.23  2005/10/24 07:13:05  Goober5000
 * merge Bobboau's thruster code back in; hopefully this covers everything
 * --Goober5000
 *
 * Revision 1.22  2005/09/27 05:25:19  Goober5000
 * initial commit of basic IFF code
 * --Goober5000
 *
 * Revision 1.21  2005/09/27 05:01:52  Goober5000
 * betterizing
 * --Goober5000
 *
 * Revision 1.20  2005/09/25 08:25:16  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 1.19  2005/09/25 05:13:07  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 1.18  2005/09/24 07:18:15  Goober5000
 * fixage
 * --Goober5000
 *
 * Revision 1.17  2005/09/24 07:07:17  Goober5000
 * another species overhaul
 * --Goober5000
 *
 * Revision 1.16  2005/09/18 02:28:18  Goober5000
 * a small fix
 * --Goober5000
 *
 * Revision 1.15  2005/08/20 18:23:02  Goober5000
 * made AwacsMultiplier more user-friendly
 * --Goober5000
 *
 * Revision 1.14  2005/07/13 02:01:30  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 1.13  2005/07/13 00:44:24  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 1.12  2005/01/25 23:32:46  Goober5000
 * species will now load from a default table instead of being initialized via code
 * --Goober5000
 *
 * Revision 1.11  2004/11/22 06:17:57  taylor
 * make sure species_defs.tbl can be found outside of root[0] and in VPs
 *
 * Revision 1.10  2004/07/26 20:47:53  Kazan
 * remove MCD complete
 *
 * Revision 1.9  2004/07/12 16:33:07  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.8  2004/03/31 05:42:29  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
 * --Goober5000
 *
 * Revision 1.7  2004/03/05 09:02:13  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 1.6  2004/02/20 04:29:56  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 1.5  2003/11/11 02:15:46  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 1.4  2003/11/06 20:22:18  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 1.3  2003/10/16 16:38:17  Kazan
 * couple more types in species_defs.cpp, also finished up "Da Species Upgrade"
 *
 * Revision 1.2  2003/10/16 01:55:26  Kazan
 * fixed typo in fallback code array index
 *
 * Revision 1.1  2003/10/15 22:03:27  Kazan
 * Da Species Update :D
 *
 */


#include <vector>

#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"
#include "species_defs/species_defs.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"
#include "iff_defs/iff_defs.h"
#include "graphics/generic.h"
#include "localization/localize.h"


std::vector<species_info> Species_info;

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
	if (!stricmp(NOX("<none>"), species->thruster_info.flames.normal.filename))
		species->thruster_info.flames.normal.filename[0] = '\0';

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
	if (!stricmp(NOX("<none>"), species->thruster_info.flames.afterburn.filename))
		species->thruster_info.flames.afterburn.filename[0] = '\0';

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
	if (!stricmp(NOX("<none>"), species->thruster_info.glow.normal.filename))
		species->thruster_info.glow.normal.filename[0] = '\0';

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
	if (!stricmp(NOX("<none>"), species->thruster_info.glow.afterburn.filename))
		species->thruster_info.glow.afterburn.filename[0] = '\0';
}

void parse_species_tbl(char *filename)
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
		read_file_text(filename);

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
			strcpy(species->species_name, species_name);
		}

		// Goober5000 - IFF
		if (optional_string("$Default IFF:"))
		{
			bool iff_found = false;
			char temp_name[NAME_LENGTH];
			stuff_string(temp_name, F_NAME, NAME_LENGTH);

			// search for it in iffs
			for (int i = 0; i < Num_iffs; i++)
			{
				if (!stricmp(Iff_info[i].iff_name, temp_name))
				{
					species->default_iff = i;
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
		if ((!no_create && required_string("+Shield_Hit_ani:")) || optional_string("+Shield_Hit_ani:"))
		{
			generic_anim_init(&species->shield_anim, NULL);
			stuff_string(species->shield_anim.filename, F_NAME, MAX_FILENAME_LEN);
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

		// don't add new entry if this is just a modified one
		if ( !no_create )
			Species_info.push_back( new_species );
	}
	
	required_string("#END");

	// add tbl/tbm to multiplayer validation list
	extern void fs2netd_add_table_validation(char *tblname);
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
