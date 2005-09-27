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
 * $Revision: 1.22 $
 * $Date: 2005-09-27 05:25:19 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
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
 * for #pragma warning disable to indicate the message being disabled
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


#pragma warning(disable:4710)	// function not inlined

#include "species_defs/species_defs.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"
#include "iff_defs/iff_defs.h"


int Num_species;
species_info Species_info[MAX_SPECIES];


//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This is the default table
// Please note that the {\n\}s should be removed from the end of each line
// if you intend to use this to format your own species_defs.tbl.

char *default_species_table = "\
												\n\
#SPECIES DEFS									\n\
												\n\
;------------------------						\n\
; Terran										\n\
;------------------------						\n\
$Species_Name: Terran							\n\
$Default IFF: Friendly							\n\
$FRED Color: ( 0, 0, 192 )						\n\
$MiscAnims:										\n\
	+Debris_Texture: debris01a					\n\
	+Shield_Hit_ani: shieldhit01a				\n\
$ThrustAnims:									\n\
	+Normal:	thruster01						\n\
	+Afterburn:	thruster01a						\n\
$ThrustGlows:									\n\
	+Normal:	thrusterglow01					\n\
	+Afterburn:	thrusterglow01a					\n\
$AwacsMultiplier: 1.00							\n\
												\n\
;------------------------						\n\
; Vasudan										\n\
;------------------------						\n\
$Species_Name: Vasudan							\n\
$Default IFF: Friendly							\n\
$FRED Color: ( 0, 128, 0 )						\n\
$MiscAnims:										\n\
	+Debris_Texture: debris01b					\n\
	+Shield_Hit_ani: shieldhit01a				\n\
$ThrustAnims:									\n\
	+Normal:	thruster02						\n\
	+Afterburn:	thruster02a						\n\
$ThrustGlows:									\n\
	+Normal:	thrusterglow02					\n\
	+Afterburn:	thrusterglow02a					\n\
$AwacsMultiplier: 1.25							\n\
												\n\
;------------------------						\n\
; Shivan										\n\
;------------------------						\n\
$Species_Name: Shivan							\n\
$Default IFF: Hostile							\n\
$FRED Color: ( 192, 0, 0 )						\n\
$MiscAnims:										\n\
	+Debris_Texture: debris01c					\n\
	+Shield_Hit_ani: shieldhit01a				\n\
$ThrustAnims:									\n\
	+Normal:	thruster03						\n\
	+Afterburn:	thruster03a						\n\
$ThrustGlows:									\n\
	+Normal:	thrusterglow03					\n\
	+Afterburn:	thrusterglow03a					\n\
$AwacsMultiplier: 1.50							\n\
												\n\
#END											\n\
";

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This function parses the data from the species_defs.tbl
// Names only - actual loading is done elsewhere

void species_init()
{
	// Goober5000 - condensed check for table file
	CFILE *sdt = cfopen("species_defs.tbl", "rb");
	int table_exists = (sdt != NULL);
	if (table_exists)
		cfclose(sdt);

	// Goober5000 - if table doesn't exist, use the default table (see above)
	if (table_exists)
		read_file_text("species_defs.tbl");
	else
		read_file_text_from_array(default_species_table);

	reset_parse();	

	required_string("#SPECIES DEFS");

	// no longer required: counted automatically
	if (optional_string("$NumSpecies:"))
	{
		int temp;
		stuff_int(&temp);
	}

	// begin reading data
	Num_species = 0;
	while (required_string_either("#END","$Species_Name:"))
	{
		species_info *species;
		
		// make sure we're under the limit
		if (Num_species >= MAX_SPECIES)
		{
			Warning(LOCATION, "Too many species in species_defs.tbl!  Max is %d.\n", MAX_SPECIES);
			skip_to_string("#END", NULL);
			break;
		}

		species = &Species_info[Num_species];
		Num_species++;

		// Start Species - Get its name
		required_string("$Species_Name:");
		stuff_string(species->species_name, F_NAME, NULL, NAME_LENGTH);

		// Goober5000 - IFF
		if (optional_string("$Default IFF:"))
		{
			bool iff_found = false;
			char temp_name[NAME_LENGTH];
			stuff_string(temp_name, F_NAME, NULL, NAME_LENGTH);

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
		else
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
		else
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
		required_string("+Debris_Texture:");
		stuff_string(species->debris_texture.filename, F_NAME, NULL, MAX_FILENAME_LEN);

		// Shield Hit Animation
		required_string("+Shield_Hit_ani:");
		stuff_string(species->shield_anim.filename, F_NAME, NULL, MAX_FILENAME_LEN);


		// Thruster Anims
		required_string("$ThrustAnims:");

		// favor new style
		if (!optional_string("+Pri_Normal:"))
			required_string("+Normal:");
		stuff_string(species->thruster_info.flames.normal.filename, F_NAME, NULL, MAX_FILENAME_LEN);

		// and again
		if (!optional_string("+Pri_Afterburn:"))
			required_string("+Afterburn:");
		stuff_string(species->thruster_info.flames.afterburn.filename, F_NAME, NULL, MAX_FILENAME_LEN);


		// old stuff for compatibility
		char dummy[MAX_FILENAME_LEN];
		if (optional_string("+Sec_Normal:")) stuff_string(dummy, F_NAME, NULL, MAX_FILENAME_LEN);
		if (optional_string("+Sec_Afterburn:")) stuff_string(dummy, F_NAME, NULL, MAX_FILENAME_LEN);
		if (optional_string("+Ter_Normal:")) stuff_string(dummy, F_NAME, NULL, MAX_FILENAME_LEN);
		if (optional_string("+Ter_Afterburn:")) stuff_string(dummy, F_NAME, NULL, MAX_FILENAME_LEN);


		// Thruster Glow Anims
		required_string("$ThrustGlows:");

		required_string("+Normal:");
		stuff_string(species->thruster_info.glow.normal.filename, F_NAME, NULL, MAX_FILENAME_LEN);
		
		required_string("+Afterburn:");
		stuff_string(species->thruster_info.glow.afterburn.filename, F_NAME, NULL, MAX_FILENAME_LEN);


		// Goober5000 - AWACS multiplier
		if (optional_string("$AwacsMultiplier:"))
		{
			stuff_float(&species->awacs_multiplier);
		}
		else
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
	}
	
	required_string("#END");
}
