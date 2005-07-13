// Species_Defs.h
// Extended Species Support for FS2 Open
// Derek Meek
// 10-14-2003

/*
 * $Logfile: /Freespace2/code/species_defs/species_defs.h $
 * $Revision: 1.13 $
 * $Date: 2005-07-13 00:44:24 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
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
#include "mission/missionparse.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "cfile/cfile.h"
//#include <memory.h>


// from shield.cpp
int True_NumSpecies = 3;

// manually extern everything here - because it's not all needed throughout the entire system
extern shield_ani Shield_ani[MAX_SHIELD_ANIMS];
extern char Species_names[MAX_SPECIES][SPECIES_NAME_MAXLEN+1];
extern char Debris_texture_files[MAX_SPECIES][MAX_DEBRIS_TNAME_LEN+1];
extern char	Thrust_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
extern char	Thrust_secondary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
extern char	Thrust_tertiary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
extern char	Thrust_glow_anim_names[NUM_THRUST_GLOW_ANIMS][MAX_FILENAME_LEN];
extern float AwacsMultiplier[MAX_SPECIES];

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This is the default table
char *default_species_table = "\
; ----------------------------------------------\n\
; Species_defs.tbl								\n\
; Derek 'Kazan' Meek							\n\
; FS2 Open Species table						\n\
;												\n\
; ----------------------------------------------\n\
												\n\
#SPECIES DEFS									\n\
												\n\
$NumSpecies: 3									\n\
												\n\
;------------------------						\n\
; Terran										\n\
;------------------------						\n\
$Species_Name: Terran							\n\
	+Debris_Texture: debris01a					\n\
	+Shield_Hit_ani: shieldhit01a				\n\
$ThrustAnims:									\n\
	+Pri_Normal:	thruster01					\n\
	+Pri_Afterburn:	thruster01a					\n\
	+Sec_Normal:	thruster02-01				\n\
	+Sec_Afterburn:	thruster02-01a				\n\
	+Ter_Normal:	thruster03-01				\n\
	+Ter_Afterburn:	thruster03-01a				\n\
$ThrustGlows:									\n\
	+Normal:	thrusterglow01					\n\
	+Afterburn:	thrusterglow01a					\n\
$AwacsMultiplier: 1.00							\n\
												\n\
;------------------------						\n\
; Vasudan										\n\
;------------------------						\n\
$Species_Name: Vasudan							\n\
	+Debris_Texture: debris01b					\n\
	+Shield_Hit_ani: shieldhit01a				\n\
$ThrustAnims:									\n\
	+Pri_Normal:	thruster02					\n\
	+Pri_Afterburn:	thruster02a					\n\
	+Sec_Normal:	thruster02-02				\n\
	+Sec_Afterburn:	thruster02-02a				\n\
	+Ter_Normal:	thruster03-02				\n\
	+Ter_Afterburn:	thruster03-02a				\n\
$ThrustGlows:									\n\
	+Normal:	thrusterglow02					\n\
	+Afterburn:	thrusterglow02a					\n\
$AwacsMultiplier: 1.25							\n\
												\n\
;------------------------						\n\
; Shivan										\n\
;------------------------						\n\
$Species_Name: Shivan							\n\
	+Debris_Texture: debris01c					\n\
	+Shield_Hit_ani: shieldhit01a				\n\
$ThrustAnims:									\n\
	+Pri_Normal:	thruster03					\n\
	+Pri_Afterburn:	thruster03a					\n\
	+Sec_Normal:	thruster02-03				\n\
	+Sec_Afterburn:	thruster02-03a				\n\
	+Ter_Normal:	thruster03-03				\n\
	+Ter_Afterburn:	thruster03-03a				\n\
$ThrustGlows:									\n\
	+Normal:	thrusterglow03					\n\
	+Afterburn:	thrusterglow03a					\n\
$AwacsMultiplier: 1.50							\n\
												\n\
#END											\n\
";

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This function loads the data from the species_defs.tbl
// It setups up Species_names, Debris Texture Files, Thruster Animations and Thruster Glow Animations
// Names only - actual loading is done elsewhere

void Init_Species_Definitions()
{
	memset(Shield_ani,					0, MAX_SHIELD_ANIMS * sizeof(shield_ani));
	memset(Species_names,				0, MAX_SHIELD_ANIMS * (SPECIES_NAME_MAXLEN+1));
	memset(Debris_texture_files,		0, MAX_SHIELD_ANIMS * (MAX_DEBRIS_TNAME_LEN+1));
	memset(Thrust_anim_names,			0, NUM_THRUST_ANIMS * MAX_FILENAME_LEN);
	memset(Thrust_secondary_anim_names, 0, NUM_THRUST_ANIMS * MAX_FILENAME_LEN);
	memset(Thrust_tertiary_anim_names,	0, NUM_THRUST_ANIMS * MAX_FILENAME_LEN);
	memset(Thrust_glow_anim_names,		0, NUM_THRUST_ANIMS * MAX_FILENAME_LEN);

	char cstrtemp[MAX_SHIELD_ANIMNAME_LEN+1];

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

	required_string("$NumSpecies:");
	stuff_int(&True_NumSpecies);

	ASSERT(True_NumSpecies < MAX_SPECIES);

	int thrust_index;

	for (int i = 0; i < True_NumSpecies; i++)
	{
		// Start Species - Get its name
		required_string("$Species_Name:");
		stuff_string(Species_names[i],								F_NAME, NULL, SPECIES_NAME_MAXLEN);

		// Get its Debris Texture
		required_string("+Debris_Texture:");
		stuff_string(Debris_texture_files[i],						F_NAME, NULL, MAX_DEBRIS_TNAME_LEN);

		// Shield Hit Animation
		memset(cstrtemp, 0, MAX_SHIELD_ANIMNAME_LEN+1);
		required_string("+Shield_Hit_ani:");
		stuff_string(cstrtemp,										F_NAME, NULL, MAX_SHIELD_ANIMNAME_LEN);
		strcpy(Shield_ani[i].filename, cstrtemp);

		// Thruster Anims
		thrust_index = i*2;

		required_string("$ThrustAnims:");

		required_string("+Pri_Normal:");
		stuff_string(Thrust_anim_names[thrust_index],				F_NAME, NULL, MAX_FILENAME_LEN);

		required_string("+Pri_Afterburn:");
		stuff_string(Thrust_anim_names[thrust_index+1],				F_NAME, NULL, MAX_FILENAME_LEN);

		required_string("+Sec_Normal:");
		stuff_string(Thrust_secondary_anim_names[thrust_index],		F_NAME, NULL, MAX_FILENAME_LEN);

		required_string("+Sec_Afterburn:");
		stuff_string(Thrust_secondary_anim_names[thrust_index+1],	F_NAME, NULL, MAX_FILENAME_LEN);

		required_string("+Ter_Normal:");
		stuff_string(Thrust_tertiary_anim_names[thrust_index],		F_NAME, NULL, MAX_FILENAME_LEN);
		
		required_string("+Ter_Afterburn:");
		stuff_string(Thrust_tertiary_anim_names[thrust_index+1],	F_NAME, NULL, MAX_FILENAME_LEN);



		// Thruster Glow Anims
		required_string("$ThrustGlows:");

		required_string("+Normal:");
		stuff_string(Thrust_glow_anim_names[thrust_index],			F_NAME, NULL, MAX_FILENAME_LEN);
		
		required_string("+Afterburn:");
		stuff_string(Thrust_glow_anim_names[thrust_index+1],		F_NAME, NULL, MAX_FILENAME_LEN);


		// Goober5000 - AWACS multiplier (which Kazan forgot or missed)
		required_string("$AwacsMultiplier:");
		stuff_float(&AwacsMultiplier[i]);
	}
	
	required_string("#END");
	
}
