// Species_Defs.h
// Extended Species Support for FS2 Open
// Derek Meek
// 10-14-2003

/*
 * $Logfile: /Freespace2/code/species_defs/species_defs.h $
 * $Revision: 1.8 $
 * $Date: 2004-03-31 05:42:29 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
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
 *
 *
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

// this isn't globalized because it shouldn't be!
void Init_Species_LoadDefault();

// manually extern everything here - because it's not all needed throughout the entire system
extern shield_ani Shield_ani[MAX_SHIELD_ANIMS];
extern char Species_names[MAX_SPECIES_NAMES][SPECIES_NAME_MAXLEN+1];
extern char Debris_texture_files[MAX_SPECIES_NAMES][MAX_DEBRIS_TNAME_LEN+1];
extern char	Thrust_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
extern char	Thrust_secondary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
extern char	Thrust_tertiary_anim_names[NUM_THRUST_ANIMS][MAX_FILENAME_LEN];
extern char	Thrust_glow_anim_names[NUM_THRUST_GLOW_ANIMS][MAX_FILENAME_LEN];

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This function Loads the data from the species_defs.tbl
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
	
	if (!cf_exist( "species_defs.tbl", CF_TYPE_TABLES ))
	{
		Init_Species_LoadDefault();
		return;
	}


	read_file_text("species_defs.tbl");
	reset_parse();	

	required_string("#SPECIES DEFS");

	required_string("$NumSpecies:");
	stuff_int(&True_NumSpecies);

	//ASSERT(True_NumSpecies < MAX_SPECIES_NAMES);
	int thrust_index;

	for (int i = 0; i < True_NumSpecies; i++)
	{
		// Start Species - Get it's name
		required_string("$Species_Name:");
		stuff_string(Species_names[i],								F_NAME, NULL, SPECIES_NAME_MAXLEN);

		// Get it's Debris Texture
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

	}
	
	required_string("#END");
	
}


//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// This function initialized the data to the default [V] hardcoded values
// See Below this function for what species_defs.tbl this is equivilent to
void Init_Species_LoadDefault()
{

	True_NumSpecies = 3;

	// -------------- Terran -------------- 

	strncpy(Species_names[0],					"Terran",			SPECIES_NAME_MAXLEN);
	strncpy(Debris_texture_files[0],			"debris01a",		MAX_DEBRIS_TNAME_LEN);
	strcpy(Shield_ani[0].filename, "shieldhit01a");

	// species*2 ? afterburning?1:0
	strncpy(Thrust_anim_names[0],				"thruster01",		MAX_FILENAME_LEN);
	strncpy(Thrust_anim_names[1],				"thruster01a",		MAX_FILENAME_LEN);
	strncpy(Thrust_secondary_anim_names[0],		"thruster02-01",	MAX_FILENAME_LEN);
	strncpy(Thrust_secondary_anim_names[1],		"thruster02-01a",	MAX_FILENAME_LEN);
	strncpy(Thrust_tertiary_anim_names[0],		"thruster03-01",	MAX_FILENAME_LEN);
	strncpy(Thrust_tertiary_anim_names[1],		"thruster03-01",	MAX_FILENAME_LEN);
	strncpy(Thrust_glow_anim_names[0],			"thrusterglow01",	MAX_FILENAME_LEN);
	strncpy(Thrust_glow_anim_names[1],			"thrusterglow01a",	MAX_FILENAME_LEN);


	// -------------- Vasudan -------------- 

	strncpy(Species_names[1],					"Vasudan",			SPECIES_NAME_MAXLEN);
	strncpy(Debris_texture_files[1],			"debris01b",		MAX_DEBRIS_TNAME_LEN);
	strcpy(Shield_ani[1].filename, "shieldhit01a");

	// species*2 ? afterburning?1:0
	strncpy(Thrust_anim_names[2],				"thruster02",		MAX_FILENAME_LEN);
	strncpy(Thrust_anim_names[3],				"thruster02a",		MAX_FILENAME_LEN);
	strncpy(Thrust_secondary_anim_names[2],		"thruster02-02",	MAX_FILENAME_LEN);
	strncpy(Thrust_secondary_anim_names[3],		"thruster02-02a",	MAX_FILENAME_LEN);
	strncpy(Thrust_tertiary_anim_names[2],		"thruster03-02",	MAX_FILENAME_LEN);
	strncpy(Thrust_tertiary_anim_names[3],		"thruster03-02",	MAX_FILENAME_LEN);
	strncpy(Thrust_glow_anim_names[2],			"thrusterglow02",	MAX_FILENAME_LEN);
	strncpy(Thrust_glow_anim_names[3],			"thrusterglow02a",	MAX_FILENAME_LEN);



	// -------------- Shivan -------------- 

	strncpy(Species_names[2],					"Shivan",			SPECIES_NAME_MAXLEN);
	strncpy(Debris_texture_files[2],			"debris01c",		MAX_DEBRIS_TNAME_LEN);
	strcpy(Shield_ani[2].filename, "shieldhit01a");

	// species*2 ? afterburning?1:0
	strncpy(Thrust_anim_names[4],				"thruster03",		MAX_FILENAME_LEN);
	strncpy(Thrust_anim_names[5],				"thruster03a",		MAX_FILENAME_LEN);
	strncpy(Thrust_secondary_anim_names[4],		"thruster02-03",	MAX_FILENAME_LEN);
	strncpy(Thrust_secondary_anim_names[5],		"thruster02-03a",	MAX_FILENAME_LEN);
	strncpy(Thrust_tertiary_anim_names[4],		"thruster03-03",	MAX_FILENAME_LEN);
	strncpy(Thrust_tertiary_anim_names[5],		"thruster03-03",	MAX_FILENAME_LEN);
	strncpy(Thrust_glow_anim_names[4],			"thrusterglow03",	MAX_FILENAME_LEN);
	strncpy(Thrust_glow_anim_names[5],			"thrusterglow03a",	MAX_FILENAME_LEN);


}

// This is equivlient to the table
/*
; -----------------------------------------------
; Species_defs.tbl
; Derek "Kazan" Meek
; FS2 Open Species table
;
; -----------------------------------------------

#SPECIES DEFS

$NumSpecies: 3

;------------------------
; Terran
;------------------------
$Species_Name: Terran
	+Debris_Texture: debris01a
	+Shield_Hit_ani: shieldhit01a
$ThrustAnims:
	+Pri_Normal:	thruster01
	+Pri_Afterburn:	thruster01a
	+Sec_Normal:	thruster02-01
	+Sec_Afterburn:	thruster02-01a
	+Ter_Normal:	thruster03-01
	+Ter_Afterburn:	thruster03-01a
$ThrustGlows:
	+Normal:	thrusterglow01
	+Afterburn:	thrusterglow01a

;------------------------
; Vasudan
;------------------------
$Species_Name: Vasudan
	+Debris_Texture: debris01b
	+Shield_Hit_ani: shieldhit01a
$ThrustAnims:
	+Pri_Normal:	thruster02
	+Pri_Afterburn:	thruster02a
	+Sec_Normal:	thruster02-02
	+Sec_Afterburn:	thruster02-02a
	+Ter_Normal:	thruster03-02
	+Ter_Afterburn:	thruster03-02a
$ThrustGlows:
	+Normal:	thrusterglow02
	+Afterburn:	thrusterglow02a

;------------------------
; Shivan
;------------------------
$Species_Name: Shivan
	+Debris_Texture: debris01c
	+Shield_Hit_ani: shieldhit01a
$ThrustAnims:
	+Pri_Normal:	thruster03
	+Pri_Afterburn:	thruster03a
	+Sec_Normal:	thruster02-03
	+Sec_Afterburn:	thruster02-03a
	+Ter_Normal:	thruster03-03
	+Ter_Afterburn:	thruster03-03a
$ThrustGlows:
	+Normal:	thrusterglow03
	+Afterburn:	thrusterglow03a
#END
*/