/*
 * Def_Files.cpp
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/globalincs/def_files.cpp $
 * $Revision: 2.22 $
 * $Date: 2007-07-15 02:45:17 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.21  2007/02/27 01:44:48  Goober5000
 * add two features for WCS: specifyable shield/weapon recharge rates, and removal of linked fire penalty
 *
 * Revision 2.20  2006/12/28 00:59:26  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.19  2006/07/06 21:23:20  Goober5000
 * add CVS headers
 * --Goober5000
 *
 */


#include <string.h>
#include "globalincs/pstypes.h"

//Struct used to hold data about file defaults
typedef struct def_file
{
	char* filename;
	char *contents;
}def_file;

//:PART 1:
//**********
extern char *Default_species_table;
extern char *Default_iff_table;
extern char *Default_shiptypes_table;
extern char *Default_ai_profiles_table;
//**********

//:PART 2:
//**********
def_file Default_files[] =
{
	{ "species_defs.tbl",		Default_species_table },
	{ "iff_defs.tbl",			Default_iff_table },
	{ "objecttypes.tbl",		Default_shiptypes_table },
	{ "ai_profiles.tbl",		Default_ai_profiles_table },
};

static int Num_default_files = sizeof(Default_files) / sizeof(def_file);
//**********

char *defaults_get_file(char *filename)
{
	for(int i = 0; i < Num_default_files; i++)
	{
		if(!stricmp(Default_files[i].filename, filename))
		{
			return Default_files[i].contents;
		}
	}

	//WMC - This is really bad, because it means we have a default table missing.
	Error(LOCATION, "Default table '%s' missing from executable - contact a coder.", filename);
	return NULL;
}

//:PART 3:
//**********
//=========================================================================

// This is the default table.
// Please note that the {\n\}s should be removed from the end of each line
// if you intend to use this to format your own table.

char *Default_species_table = "\
																		\n\
#SPECIES DEFS															\n\
																		\n\
;------------------------												\n\
; Terran																\n\
;------------------------												\n\
$Species_Name: Terran													\n\
$Default IFF: Friendly													\n\
$FRED Color: ( 0, 0, 192 )												\n\
$MiscAnims:																\n\
	+Debris_Texture: debris01a											\n\
	+Shield_Hit_ani: shieldhit01a										\n\
$ThrustAnims:															\n\
	+Normal:	thruster01												\n\
	+Afterburn:	thruster01a												\n\
$ThrustGlows:															\n\
	+Normal:	thrusterglow01											\n\
	+Afterburn:	thrusterglow01a											\n\
$AwacsMultiplier: 1.00													\n\
																		\n\
;------------------------												\n\
; Vasudan																\n\
;------------------------												\n\
$Species_Name: Vasudan													\n\
$Default IFF: Friendly													\n\
$FRED Color: ( 0, 128, 0 )												\n\
$MiscAnims:																\n\
	+Debris_Texture: debris01b											\n\
	+Shield_Hit_ani: shieldhit01a										\n\
$ThrustAnims:															\n\
	+Normal:	thruster02												\n\
	+Afterburn:	thruster02a												\n\
$ThrustGlows:															\n\
	+Normal:	thrusterglow02											\n\
	+Afterburn:	thrusterglow02a											\n\
$AwacsMultiplier: 1.25													\n\
																		\n\
;------------------------												\n\
; Shivan																\n\
;------------------------												\n\
$Species_Name: Shivan													\n\
$Default IFF: Hostile													\n\
$FRED Color: ( 192, 0, 0 )												\n\
$MiscAnims:																\n\
	+Debris_Texture: debris01c											\n\
	+Shield_Hit_ani: shieldhit01a										\n\
$ThrustAnims:															\n\
	+Normal:	thruster03												\n\
	+Afterburn:	thruster03a												\n\
$ThrustGlows:															\n\
	+Normal:	thrusterglow03											\n\
	+Afterburn:	thrusterglow03a											\n\
$AwacsMultiplier: 1.50													\n\
																		\n\
#END																	\n\
";

//=========================================================================

// This is the default table.
// Please note that the {\n\}s should be removed from the end of each line
// and the {\"}s  should be replaced with {"}s if you intend to use this to
// format your own table.

char *Default_iff_table = "\
																		\n\
#IFFs																	\n\
																		\n\
;; Every iff_defs.tbl must contain a Traitor entry.  Traitors attack	\n\
;; one another (required by the dogfighting code) but it is up to you	\n\
;; to decide who attacks the traitor or whom else the traitor attacks.	\n\
$Traitor IFF: Traitor													\n\
																		\n\
;------------------------												\n\
; Friendly																\n\
;------------------------												\n\
$IFF Name: Friendly														\n\
$Color: ( 0, 255, 0 )													\n\
$Attacks: ( \"Hostile\" \"Neutral\" \"Traitor\" )						\n\
$Flags: ( \"support allowed\" )											\n\
$Default Ship Flags: ( \"cargo-known\" )								\n\
																		\n\
;------------------------												\n\
; Hostile																\n\
;------------------------												\n\
$IFF Name: Hostile														\n\
$Color: ( 255, 0, 0 )													\n\
$Attacks: ( \"Friendly\" \"Neutral\" \"Traitor\" )						\n\
+Sees Friendly As: ( 255, 0, 0 )										\n\
+Sees Hostile As: ( 0, 255, 0 )											\n\
																		\n\
;------------------------												\n\
; Neutral																\n\
;------------------------												\n\
$IFF Name: Neutral														\n\
$Color: ( 255, 0, 0 )													\n\
$Attacks: ( \"Friendly\" \"Traitor\" )									\n\
+Sees Friendly As: ( 255, 0, 0 )										\n\
+Sees Hostile As: ( 0, 255, 0 )											\n\
+Sees Neutral As: ( 0, 255, 0 )											\n\
																		\n\
;------------------------												\n\
; Unknown																\n\
;------------------------												\n\
$IFF Name: Unknown														\n\
$Color: ( 255, 0, 255 )													\n\
$Attacks: ( \"Hostile\" )												\n\
+Sees Neutral As: ( 0, 255, 0 )											\n\
+Sees Traitor As: ( 0, 255, 0 )											\n\
$Flags: ( \"exempt from all teams at war\" )							\n\
																		\n\
;------------------------												\n\
; Traitor																\n\
;------------------------												\n\
$IFF Name: Traitor														\n\
$Color: ( 255, 0, 0 )													\n\
$Attacks: ( \"Friendly\" \"Hostile\" \"Neutral\" \"Traitor\" )			\n\
+Sees Friendly As: ( 255, 0, 0 )										\n\
																		\n\
#End																	\n\
";

//=========================================================================

// This is the default table.
// Please note that the {\n\}s and {""\}s should be removed from the end of
// each line and the {\"}s  should be replaced with {"}s if you intend to
// use this to format your own table.

char *Default_shiptypes_table = "\
																		\n\
#Ship types																\n\
""\
$Name:					Navbuoy											\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		10.0											\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Turrets attack this:	YES											\n\
""\
$Name:					Sentry Gun										\n\
$Counts for Alone:		YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			0.10											\n\
$EMP Multiplier:		10.0											\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Accept Player Orders:	NO											\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
""\
$Name:					Escape Pod										\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Warp Pushable:			YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		10.0											\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Turrets attack this:	YES											\n\
""\
$Name:					Cargo											\n\
$Scannable:				YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			0.10											\n\
$EMP Multiplier:		10.0											\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Passive docks:			( \"cargo\" )								\n\
""\
$Name:					Support											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		3.5												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"dock\" \"undock\" \"waypoints\" \"waypoints once\" \"stay near ship\" \"keep safe dist\" \"stay still\" \"play dead\" )							\n\
	+Accept Player Orders:	YES											\n\
	+Player orders:			( \"rearm me\" \"abort rearm\" \"stay near me\" \"stay near ship\" \"keep safe dist\" \"depart\" )																	\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Active docks:			( \"support\" )								\n\
""\
;;WMC - Stealth ships always have another type, so this isn't used		\n\
$Name:					Stealth											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Fighter											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Bomber											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
;;WMC - This fighter/bomber type doesn't seem to be used anywhere, because no ship is set as both fighter and bomber																																																								\n\
$Name:					Fighter/bomber									\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
;;$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		1.5											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Transport										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		2.0												\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
	+Disappear factor:		2.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"dock\" \"depart\" )		\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Freighter										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Scannable:				YES												\n\
;;$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		1.75											\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
	+Disappear factor:		2.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"dock\" \"depart\" )		\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Active docks:			( \"cargo\" )								\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					AWACS											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.8												\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
	+Disappear factor:		2.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"depart\" )				\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Gas Miner										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		1.0												\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
	+Disappear factor:		2.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"depart\" )				\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Cruiser											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.9												\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
	+Disappear factor:		2.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"depart\" )				\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Corvette										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.3333											\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
	+Disappear factor:		2.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"depart\" )				\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Capital											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushes:			YES												\n\
;;$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.2												\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			750.0										\n\
	+Disappear factor:		3.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"waypoints\" \"waypoints once\" \"depart\" \"stay still\" \"play dead\" \"stay near ship\" )						\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"depart\" )								\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Super Cap										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushes:			YES												\n\
;;$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.075											\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			1000.0										\n\
	+Disappear factor:		3.0											\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"waypoints\" \"waypoints once\" \"depart\" \"stay still\" \"play dead\" \"stay near ship\" )						\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Drydock											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.5												\n\
$Beams Easily Hit:		YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			750.0										\n\
	+Disappear factor:		3.0											\n\
$AI:																	\n\
	+Accept Player Orders:	YES											\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
$Name:					Knossos Device									\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
;;$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.10											\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			1000.0										\n\
	+Disappear factor:		3.0											\n\
$AI:																	\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Passive docks:			( \"support\" )								\n\
""\
#End																	\n\
";

//=========================================================================

// This is the default table.
// Please note that the {\n\}s and {\n""\}s should be removed from the end
// of each line if you intend to use this to format your own table.

char *Default_ai_profiles_table = "\
																		\n\
;; AI Profiles table.  Incorporates stuff from the old difficulty.tbl	\n\
;; plus additional flags previously covered under the blanket New AI	\n\
;; flag.																\n\
;;																		\n\
;; This is what the retail table would look like, but you don't have to	\n\
;; specify it as it's stored internally by the game.  This leaves you	\n\
;; with four other slots to specify four other profiles.  Every setting	\n\
;; is optional, so if you don't specify something it will inherit from	\n\
;; the FS2 retail setting.  If you don't specify a default profile, it	\n\
;; will set the default to retail as well.								\n\
;;																		\n\
																		\n\
#AI Profiles															\n\
																		\n\
$Default Profile: FS2 RETAIL											\n\
																		\n\
																		\n\
$Profile Name: FS2 RETAIL												\n\
																		\n\
																		\n\
;; Difficulty-related values; much of this was originally in			\n\
;; difficulty.tbl.  Each option specifies a list corresponding to the	\n\
;; five skill values (Very Easy, Easy, Medium, Hard, Insane).			\n\
																		\n\
																		\n\
;; speed of afterburner recharge										\n\
$Player Afterburner Recharge Scale: 5, 3, 2, 1.5, 1						\n\
																		\n\
;; maximum damage inflicted by friendly beam fire						\n\
$Max Beam Friendly Fire Damage: 0, 5, 10, 20, 30						\n\
																		\n\
;; factor applied to player countermeasure lifetime						\n\
$Player Countermeasure Life Scale: 3, 2, 1.5, 1.25, 1					\n\
																		\n\
;; chance a countermeasure will be fired by an AI-controlled ship		\n\
;; (this is scaled by ai_class)											\n\
$AI Countermeasure Firing Chance: 0.2, 0.3, 0.5, 0.9, 1.1				\n\
																		\n\
;; seconds to add to the time it takes for an enemy to come in range of	\n\
;; (i.e. target) a friendly ship										\n\
$AI In Range Time: 2, 1.4, 0.75, 0, -1									\n\
																		\n""\
;; AI ships will link ballistic primaries if ammo levels are greater	\n\
;; than these percents													\n\
$AI Always Links Ammo Weapons: 95, 80, 60, 40, 20						\n\
$AI Maybe Links Ammo Weapons: 90, 60, 40, 20, 10						\n\
																		\n\
;; AI ships will link laser primaries if energy levels are greater		\n\
;; than these percents													\n\
$AI Always Links Energy Weapons: 100, 80, 60, 40, 20					\n\
$AI Maybe Links Energy Weapons: 90, 60, 40, 20, 10						\n\
																		\n\
;; maximum number of missiles allowed to be homing in on a player at a	\n\
;; given time (single-player only; no restriction in multiplayer)		\n\
$Max Missles Locked on Player: 2, 3, 4, 7, 99							\n\
																		\n\
;; maximum number of ships allowed to be attacking the player at a		\n\
;; given time (single-player only; no restriction in multiplayer)		\n\
$Max Player Attackers: 2, 3, 4, 5, 99									\n\
																		\n\
;; maximum number of active (i.e. 'thrown') asteroids that can be		\n\
;; heading toward a friendly ship at any given time						\n\
$Max Incoming Asteroids: 3, 4, 5, 7, 10									\n\
																		\n\
;; factor applied to damage suffered by the player						\n\
$Player Damage Factor: 0.25, 0.5, 0.65, 0.85, 1							\n\
																		\n\
;; factor applied to subsystem damage suffered by the player			\n\
;; (in addition to Player Damage Factor)								\n\
$Player Subsys Damage Factor: 0.2, 0.4, 0.6, 0.8, 1						\n\
																		\n\
;; measure of time (in F1_0 units) after which the AI will recalculate	\n\
;; the position of its target											\n\
$Predict Position Delay: 2, 1.5, 1.333, 0.5, 0							\n\
																		\n\
;; seconds between each instance of an AI ship managing its shields		\n\
$AI Shield Manage Delay: 5, 4, 2.5, 1.2, 0.1							\n\
																		\n""\
;; factor applied to 'fire wait' for friendly ships						\n\
$Friendly AI Fire Delay Scale: 2, 1.4, 1.25, 1.1, 1						\n\
																		\n\
;; factor applied to 'fire wait' for hostile ships						\n\
$Hostile AI Fire Delay Scale: 4, 2.5, 1.75, 1.25, 1						\n\
																		\n\
;; factor applied to time it takes for enemy ships to turn				\n\
$AI Turn Time Scale: 3, 2.2, 1.6, 1.3, 1								\n\
																		\n\
;; factor applied to the speed at which the player's shields recharge	\n\
$Player Shield Recharge Scale: 4, 2, 1.5, 1.25, 1						\n\
																		\n\
;; factor applied to the speed at which the player's weapons recharge	\n\
$Player Weapon Recharge Scale: 10, 4, 2.5, 2, 1.5						\n\
																		\n\
																		\n\
;; General AI-related flags.  These were previously all lumped together	\n\
;; under the New AI mission flag.										\n\
																		\n\
																		\n\
;; if set, big ships can attack a beam turret that's firing on them		\n\
;; from a ship that they don't currently have targeted.					\n\
$big ships can attack beam turrets on untargeted ships: NO				\n\
																		\n\
;; if set, enables the new primary weapon selection method				\n\
$smart primary weapon selection: NO										\n\
																		\n\
;; if set, enables the new secondary weapon selection method (including	\n\
;; proper use of bomber+ missiles)										\n\
$smart secondary weapon selection: NO									\n\
																		\n\
;; if set, enables smart shield management (previously was				\n\
;; -smart_shields on the command line)									\n\
$smart shield management: NO											\n\
																		\n""\
;; if set, allows an AI ship to switch to rapid fire for dumbfire		\n\
;; missiles																\n\
$allow rapid secondary dumbfire: NO										\n\
																		\n\
;; if set, causes huge turret weapons (including anti-capship beams) to	\n\
;; not target bombs														\n\
$huge turret weapons ignore bombs: NO									\n\
																		\n\
;; if set, removes the random turret fire delay (from .1 to .9 seconds)	\n\
;; inserted in addition to AI Fire Delay Scale							\n\
$don't insert random turret fire delay: NO								\n\
																		\n\
;; if set, triggers a hack to improves the accuracy of non-homing swarm	\n\
;; missiles by firing them along the turret's last fire direction		\n\
;; rather than the direction it currently faces							\n\
$hack improve non-homing swarm turret fire accuracy: NO					\n\
																		\n\
;; if set, shockwaves will cause damage to small ship subsystems		\n\
;; (like in FS1)														\n\
$shockwaves damage small ship subsystems: NO							\n\
																		\n\
;; if set, ships will not be able to engage their jump drive if their	\n\
;; navigation subsystem is damaged or destroyed							\n\
$navigation subsystem governs warpout capability: NO					\n\
																		\n\
;; if set, will not use a minimum speed limit for docked ships			\n\
;; (like in FS1)														\n\
$ignore lower bound for minimum speed of docked ship: NO				\n\
																		\n\
;; if set, will remove the increased delay when weapons are linked		\n\
$disable linked fire penalty: NO										\n\
																		\n\
;; if set, will not scale weapon damage according to capital/supercap	\n\
;; (like in FS1)														\n\
$disable weapon damage scaling: NO										\n\
																		\n\
;; if set, will add the weapon velocity to the firing ship's velocity	\n\
$use additive weapon velocity: NO										\n\
																		\n\
																		\n\
#End																	\n\
";
