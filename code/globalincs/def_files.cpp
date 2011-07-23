/*
 * Def_Files.cpp
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */




#include <string.h>
#include "globalincs/pstypes.h"

//Struct used to hold data about file defaults
typedef struct def_file
{
	char* filename;
	char *contents;
} def_file;

//:PART 1:
//**********
extern char *Default_species_table;
extern char *Default_iff_table;
extern char *Default_shiptypes_table;
extern char *Default_ai_profiles_table;
extern char *Default_autopilot_table;
extern char *Default_fonts_table;
extern char *Default_post_processing_table;
extern char* Default_main_vertex_shader;
extern char* Default_main_fragment_shader;
extern char* Default_fxaa_vertex_shader;
extern char* Default_fxaa_fragment_shader;
extern char* Default_blur_fragment_shader;
extern char* Default_brightpass_fragment_shader;
extern char* Default_post_fragment_shader;
extern char* Default_post_vertex_shader;
extern char* Default_fxaa_prepass_shader;
//**********

//:PART 2:
//**********
def_file Default_files[] =
{
	{ "species_defs.tbl",		Default_species_table },
	{ "iff_defs.tbl",			Default_iff_table },
	{ "objecttypes.tbl",		Default_shiptypes_table },
	{ "ai_profiles.tbl",		Default_ai_profiles_table },
	{ "autopilot.tbl",			Default_autopilot_table },
	{ "fonts.tbl",				Default_fonts_table },
	{ "post_processing.tbl",	Default_post_processing_table},
	{ "main-f.sdr",				Default_main_fragment_shader},
	{ "main-v.sdr",				Default_main_vertex_shader},
	{ "fxaa-v.sdr",				Default_fxaa_vertex_shader},
	{ "fxaa-f.sdr",				Default_fxaa_fragment_shader},
	{ "blur-f.sdr",				Default_blur_fragment_shader},
	{ "brightpass-f.sdr",		Default_brightpass_fragment_shader},
	{ "post-f.sdr",				Default_post_fragment_shader},
	{ "post-v.sdr",				Default_post_vertex_shader},
	{ "fxaapre-f.sdr",			Default_fxaa_prepass_shader}
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
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		10.0											\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Turrets attack this:	YES											\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Sentry Gun										\n\
$Counts for Alone:		YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			0.10											\n\
$EMP Multiplier:		10.0											\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Accept Player Orders:	NO											\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Escape Pod										\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Warp Pushable:			YES												\n\
$Turrets prioritize ship target: YES									\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		10.0											\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\" )	\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Turrets attack this:	YES											\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Cargo											\n\
$Scannable:				YES												\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			0.10											\n\
$EMP Multiplier:		10.0											\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Passive docks:			( \"cargo\" )								\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Support											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
$Turrets prioritize ship target: YES									\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		3.5												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"dock\" \"undock\" \"waypoints\" \"waypoints once\" \"stay near ship\" \"keep safe dist\" \"stay still\" \"play dead\" )							\n\
	+Accept Player Orders:	YES											\n\
	+Player orders:			( \"rearm me\" \"abort rearm\" \"depart\" )																	\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Active docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
;;WMC - Stealth ships always have another type, so this isn't used		\n\
$Name:					Stealth											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Fighter											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
$Turrets prioritize ship target: YES									\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"form on wing\")	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Bomber											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
$Turrets prioritize ship target: YES									\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"form on wing\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
;;WMC - This fighter/bomber type doesn't seem to be used anywhere, because no ship is set as both fighter and bomber																																																								\n\
$Name:					Fighter/bomber									\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushable:			YES												\n\
$Max Debris Speed:		200												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		4.0												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"ignore ship (new)\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"form on wing\" )	\n\
	+Accept Player Orders:	YES											\n\
	+Player Orders:			( \"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"ignore ship (new)\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\" )		\n\
	+Auto attacks:			YES											\n\
	+Actively Pursues:		( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )																\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Transport										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		2.0												\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			500.0										\n\
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
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Freighter										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Scannable:				YES												\n\
$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		1.75											\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
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
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					AWACS											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.8												\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
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
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Gas Miner										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		1.0												\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
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
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Cruiser											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.9												\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
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
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Corvette										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		150												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.3333											\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	YES												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			600.0										\n\
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
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Capital											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushes:			YES												\n\
$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.2												\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	NO												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			750.0										\n\
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
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Super Cap										\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Warp Pushes:			YES												\n\
$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.075											\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	NO												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			1000.0										\n\
$AI:																	\n\
	+Valid goals:			( \"fly to ship\" \"attack ship\" \"attack wing\" \"waypoints\" \"waypoints once\" \"depart\" \"stay still\" \"play dead\" \"stay near ship\" )						\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Can Form Wing:			YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Drydock											\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.5												\n\
$Beams Easily Hit:		YES												\n\
$Protected on cripple:	NO												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			750.0										\n\
$AI:																	\n\
	+Accept Player Orders:	YES											\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
""\
$Name:					Knossos Device									\n\
$Counts for Alone:		YES												\n\
$Praise Destruction:	YES												\n\
$On Hotkey List:		YES												\n\
$Target as Threat:		YES												\n\
$Show Attack Direction:	YES												\n\
$Max Debris Speed:		100												\n\
$FF Multiplier:			1.0												\n\
$EMP Multiplier:		0.10											\n\
$Protected on cripple:	NO												\n\
$Fog:																	\n\
	+Start dist:			10.0										\n\
	+Compl dist:			1000.0										\n\
$AI:																	\n\
	+Auto attacks:			YES											\n\
	+Attempt Broadside:		YES											\n\
	+Actively Pursues:		( \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )										\n\
	+Guards attack this:	YES											\n\
	+Turrets attack this:	YES											\n\
	+Passive docks:			( \"support\" )								\n\
	+Ignored on cripple by:	( \"navbuoy\" \"sentry gun\" \"escape pod\" \"cargo\" \"support\" \"stealth\" \"fighter\" \"bomber\" \"fighter/bomber\" \"transport\" \"freighter\" \"awacs\" \"gas miner\" \"cruiser\" \"corvette\" \"capital\" \"super cap\" \"drydock\" \"knossos device\" )	\n\
$Vaporize Percent Chance: 0.0											\n\
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
;; Multiplier that modifies the length and frequency of bursts used		\n\
;; by the AI for ballistic primary weapons								\n\
$Primary Ammo Burst Multiplier: 0, 0, 0, 0, 0							\n\
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
;; factor applied to 'fire wait' for secondaries of friendly ships		\n\
$Friendly AI Secondary Fire Delay Scale: 0.4, 0.6, 0.8, 1.0, 1.2		\n\
																		\n\
;; factor applied to 'fire wait' for secondaries of hostile ships		\n\
$Hostile AI Secondary Fire Delay Scale: 1.4, 1.2, 1.0, 0.8, 0.6			\n\
																		\n\
;; factor applied to time it takes for enemy ships to turn				\n\
$AI Turn Time Scale: 3, 2.2, 1.6, 1.3, 1								\n\
																		\n\
;; Percentage of the time where AI ships will use the glide attack		\n\
;; when it is an option.												\n\
$Glide Attack Percent: 0, 0, 0, 0, 0									\n\
																		\n\
;; Percentage of the time where AI ships will use circle strafe			\n\
;; when it is an option.												\n\
$Circle Strafe Percent: 0, 0, 0, 0, 0									\n\
																		\n\
;; Percentage of the time where AI ships will use glide to strafe		\n\
;; capital ships when it is an option.									\n""\
$Glide Strafe Percent: 0, 0, 0, 0, 0									\n\
																		\n\
;; Percentage of the time where AI ships will randomly sidethrust in a	\n\
;; dogfight.															\n\
$Random Sidethrust Percent: 0, 0, 0, 0, 0								\n\
																		\n\
;; The amount of time required for the AI to detect 					\n\
;; (and try to break) dogfight stalemate.								\n\
$Stalemate Time Threshold: 0, 0, 0, 0, 0								\n\
																		\n\
;; The maximum distance the AI and target must be within				\n\
;; for a dogfight stalemate												\n\
$Stalemate Distance Threshold: 0, 0, 0, 0, 0							\n\
																		\n\
;; factor applied to the speed at which the player's shields recharge	\n\
$Player Shield Recharge Scale: 4, 2, 1.5, 1.25, 1						\n\
																		\n\
;; factor applied to the speed at which the player's weapons recharge	\n\
$Player Weapon Recharge Scale: 10, 4, 2.5, 2, 1.5						\n\
																		\n\
;; maximum number of turrets on one ship allowed to be attacking a		\n\
;; target at a given time												\n\
$Max Turret Target Ownage: 3, 4, 7, 12, 19								\n\
																		\n\
;; maximum number of turrets on one ship allowed to be attacking the	\n\
;; player at a given time												\n\
$Max Turret Player Ownage: 3, 4, 7, 12, 19								\n""\
																		\n\
;; the minimum percentage of the total assessed damage a player		 	\n\
;; must inflict in order to be awarded a kill							\n\
$Percentage Required For Kill Scale: 0.30, 0.30, 0.30, 0.30, 0.30		\n\
																		\n\
;; the minimum percentage of the total assessed damage a player		 	\n\
;; must inflict in order to be awarded an assist						\n\
$Percentage Required For Assist Scale: 0.15, 0.15, 0.15, 0.15, 0.15		\n\
																		\n\
;; in TvT and Coop missions all teammates will be granted this 		 	\n\
;; percentage of the capships score when someone scores a kill			\n\
$Percentage Awarded For Capship Assist: 0.1, 0.2, 0.35, 0.5, 0.6		\n\
																		\n\
;; the amount to subtract from the player's score if they are			\n\
;; repaired by a support ship											\n\
$Repair Penalty: 10, 20, 35, 50, 60										\n\
																		\n\
;; time delay after bombs have been fired before they can collide		\n\
;; with other weapons (ie. be shot down)								\n\
$Delay Before Allowing Bombs to Be Shot Down: 1.5, 1.5, 1.5, 1.5, 1.5	\n\
																		\n\
;; Chance AI has to fire missiles at player is (value + 1) / 7 in every	\n\
;; 10 second interval													\n""\
$Chance AI Has to Fire Missiles at Player:	0, 1, 2, 3, 4				\n\
																		\n\
;; The maximum amount of delay allowed before the AI will update its	\n\
;; aim. Applies for small ships vs small ships							\n\
$Max Aim Update Delay: 0, 0, 0, 0, 0									\n\
																		\n\
;; The maximum amount of delay allowed before turret AI will update its	\n\
;; aim. Applies for turrets vs small ships								\n\
$Turret Max Aim Update Delay: 0, 0, 0, 0, 0								\n\
																		\n\
;; The maximum amount of delay allowed before turret AI will update its	\n\
;; aim. Applies for turrets vs small ships								\n\
$Detail Distance Multiplier: 0.125, 0.25, 1.0, 4.0, 8.0					\n\
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
;; if set, shields will devote all their charging energy to the weakest	\n\
;; quadrant(s) and not waste energy on fully-charged quadrants			\n\
;; (previously was -smart_shields on the command line)					\n\
$smart shield management: NO											\n\
																		\n""\
;; if set, the AI will properly use brief pulses of afterburner power	\n\
;; instead of afterburning until fuel is exhausted						\n\
$smart afterburner management: NO										\n\
																		\n\
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
;; if set, will not use a minimum speed limit for docked ships			\n""\
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
;; if set, will dampening closer to real newtonian physics				\n\
$use newtonian dampening: NO											\n\
																		\n\
;; if set, beam damage is counted when calculating kills and assists 	\n\
$include beams for kills and assists: NO								\n\
																		\n\
;; if set, kills gain score based on the percentage damage the killer	\n\
;; inflicted on the dead ship											\n\
$score kills based on damage caused: NO									\n\
																		\n\
;; if set, kills gain score based on the percentage damage the player	\n\
;; gaining the assist inflicted on the dead ship						\n\
$score assists based on damage caused: NO								\n\
																		\n\
;; if set, players (rather than just their team) can gain score from 	\n\
;; events in multiplayer												\n\
$allow event and goal scoring in multiplayer: NO						\n\
																		\n\
;; if set, the AI will properly link primaries according to				\n\
;; specified percentages of energy levels instead of retail behavior	\n\
;; where it mistakenly linked according to absolute energy levels		\n\
$fix linked primary weapon decision bug: NO								\n\
																		\n\
;; if set, prevents turrets from targeting bombs beyond maximum			\n\
;; range of the weapons of the turret									\n\
$prevent turrets targeting too distant bombs: NO						\n\
																		\n""\
;; if set, prevents turrets from trying to target subsystems beyond		\n\
;; their fov limits, also keeps the turret subsystem targeting			\n\
;; preference order intact regardless of the angle to the target		\n\
$smart subsystem targeting for turrets: NO								\n\
																		\n\
;; if set, heat-seeking missiles will not home in on stealth ships		\n\
;; (this mirrors the established behavior where heat-seeking missiles	\n\
;; do not home in on ships that are hidden from sensors)				\n\
$fix heat seekers homing on stealth ships bug: NO						\n\
																		\n\
;; allow a player to commit into a multiplayer game without primaries	\n\
$multi allow empty primaries:		NO									\n\
																		\n\
;; allow a player to commit into a multiplayer game without secondaries	\n\
$multi allow empty secondaries:		NO									\n\
																		\n\
;; if set, allows turrets target other weapons that bombs assuming		\n\
;; it is within allowed target priorities								\n\
$allow turrets target weapons freely: NO								\n\
																		\n\
;; if set forces turrets to use only the set turret fov limits and		\n\
;; ignore hard coded limits (with 'fire_down_normals' flag)				\n\
$use only single fov for turrets:		NO								\n\
																		\n\
;; allow AI ships to dodge weapons vertically as well as horizontally	\n\
$allow vertical dodge:	NO												\n\
																		\n""\
;; a disam or disable goal will protect a ship globally and purge		\n\
;; standing attack orders globally. If set to NO, only the ship/wing	\n\
;; given the order to disarm or disable will be affected.				\n\
$disarm or disable cause global ai goal effects:	YES					\n\
																		\n\
;; Fixes a bug where AI class is not properly set if set in the mission	\n\
;; This should be YES if you want anything in AI.tbl to mean anything	\n\
$fix AI class bug:	NO													\n\
																		\n\
;; If set, the AI will NOT make extra effort to avoid ramming the player\n\
;; during dogfights. This results in more aggressive AI.				\n\
$no extra collision avoidance vs player:	NO							\n\
																		\n\
;; If set, the game will check if the dying ship is a player's wingman, \n\
;; before making it give a death scream					 				\n\
$perform less checks for death screams:	NO								\n\
																		\n\
;; If set, allows shield management for all ships						\n\
;; (including capships).												\n\
$all ships manage shields:				NO								\n\
																		\n\
;; If set, ai aims using ship center instead of first gunpoint			\n\
$ai aims from ship center:			NO									\n\
																		\n\
;; If set, prevents fighters from linking their weapons in the first	\n\
;; few minutes of the mission											\n\
$allow primary link delay:				YES								\n\
																		\n\
;; If set, prevents beams from instantly killing all weapons from first	\n\
;; hit, instead allows weapon hitpoints to be used instead				\n\
$allow beams to damage bombs:			NO								\n\
																		\n\
#End																	\n\
";

//=========================================================================

// This is the default table.
// Please note that the {\n\}s should be removed from the end of each line
// if you intend to use this to format your own table.

char *Default_autopilot_table = "\
#Autopilot																\n\
																		\n\
$Link Distance: 1000													\n\
																		\n\
$No Nav Selected:														\n\
	+Msg: Cannot engage autopilot, no navpoint selected.				\n\
	+Snd File: none														\n\
$Gliding:																\n\
	+Msg: Cannot engage autopilot while gliding.						\n\
	+Snd File: none														\n\
$Too Close:																\n\
	+Msg: Cannot engage autopilot: waypoint too close.					\n\
	+Snd File: none														\n\
$Hostiles:																\n\
	+Msg: Cannot engage autopilot: hostile craft near.					\n\
	+Snd File: none														\n\
$Linked:																\n\
	+Msg: Autopilot Linked												\n\
	+Snd File: none														\n\
$Hazard:																\n\
	+Msg: Cannot engage autopilot: Hazard Near							\n\
	+Snd File: none														\n\
$Support Present:														\n\
	+Msg: Cannot engage autopilot: Support Ship Present					\n\
	+Snd File: none														\n\
$Support Working:														\n\
	+Msg: Cannot engage autopilot: Support Ship is rearming or repairing a ship	\n\
	+Snd File: none														\n\
																		\n\
#END																	\n\
";

//=========================================================================

// This is the default table.
// Please note that the {\n\}s should be removed from the end of each line
// if you intend to use this to format your own table.

char *Default_fonts_table = "\
#Fonts																	\n\
																		\n\
$Font: font01.vf														\n\
$Font: font02.vf														\n\
$Font: font03.vf														\n\
																		\n\
#End																	\n\
";

//=========================================================================

// This is the default table.
// Please note that the {\n\}s should be removed from the end of each line
// if you intend to use this to format your own table.

char *Default_post_processing_table = "\
#Effects										\n\
												\n\
$Name:			distort noise					\n\
$Uniform:		noise_amount					\n\
$Define:		FLAG_DISTORT_NOISE				\n\
$AlwaysOn: 		false							\n\
$Default:		0.0								\n\
$Div:			20000							\n\
$Add:			0								\n\
												\n\
$Name:			saturation						\n\
$Uniform:		saturation						\n\
$Define:		FLAG_SATURATION					\n\
$AlwaysOn: 		false							\n\
$Default:		0.9								\n\
$Div:			50								\n\
$Add:			0								\n\
												\n\
$Name:			brightness						\n\
$Uniform:		brightness						\n\
$Define:		FLAG_BRIGHTNESS					\n\
$AlwaysOn: 		false							\n\
$Default:		1.11							\n\
$Div:			50								\n\
$Add:			0								\n\
												\n\
$Name:			contrast						\n\
$Uniform:		contrast						\n\
$Define:		FLAG_CONTRAST					\n\
$AlwaysOn: 		false							\n\
$Default:		1.015							\n\
$Div:			50								\n\
$Add:			0								\n\
												\n\
$Name:			film grain						\n\
$Uniform:		film_grain						\n\
$Define:		FLAG_GRAIN						\n\
$AlwaysOn: 		false							\n\
$Default:		0.1								\n\
$Div:			50								\n\
$Add:			0								\n\
												\n\
$Name:			stripes							\n\
$Uniform:		tv_stripes						\n\
$Define:		FLAG_STRIPES					\n\
$AlwaysOn:		false							\n\
$Default:		0.0								\n\
$Div:			50								\n\
$Add:			0								\n\
												\n\
$Name:			cutoff							\n\
$Uniform:		cutoff							\n\
$Define:		FLAG_CUTOFF						\n\
$AlwaysOn:		false							\n\
$Default:		2.0								\n\
$Div:			50								\n\
$Add:			0.0								\n\
												\n\
$Name:			dithering						\n\
$Uniform:		dither							\n\
$Define:		FLAG_DITH						\n\
$AlwaysOn:		false							\n\
$Default:		0.0								\n\
$Div:			50								\n\
$Add:			0								\n\
												\n\
#End											\n\
";

//===========================================================
//					DEFAULT SHADERS
//===========================================================

char* Default_main_vertex_shader = "\
#ifdef FLAG_ENV_MAP																			\n\
uniform mat4 envMatrix;																		\n\
varying vec3 envReflect;																	\n\
#endif																						\n\
																							\n\
#ifdef FLAG_NORMAL_MAP																		\n\
varying mat3 tbnMatrix;																		\n\
#endif																						\n\
																							\n\
#ifdef FLAG_FOG																				\n\
varying float fogDist;																		\n\
#endif																						\n\
																							\n\
varying vec4 position;																		\n\
varying vec3 lNormal;																		\n\
																							\n\
void main()																					\n\
{																							\n\
	gl_TexCoord[0] = gl_MultiTexCoord0;														\n\
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;						\n\
	gl_FrontColor = gl_Color;																\n\
	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);										\n\
																							\n\
 // Transform the normal into eye space and normalize the result.							\n\
	position = gl_ModelViewMatrix * gl_Vertex;												\n\
	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);									\n\
	lNormal = normal;																		\n\
																							\n\
 #ifdef FLAG_NORMAL_MAP																		\n\
 // Setup stuff for normal maps																\n\
	vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);							\n\
	vec3 b = cross(normal, t) * gl_MultiTexCoord1.w;										\n\
	tbnMatrix = mat3(t, b, normal);															\n\
 #endif																						\n\
																							\n\
 #ifdef FLAG_ENV_MAP																		\n\
 // Environment mapping reflection vector.													\n\
	envReflect = reflect(position.xyz, normal);												\n\
	envReflect = vec3(envMatrix * vec4(envReflect, 0.0));									\n\
	envReflect = normalize(envReflect);														\n\
 #endif																						\n\
																							\n\
 #ifdef FLAG_FOG																			\n\
	fogDist = clamp((gl_Position.z - gl_Fog.start) * 0.75 * gl_Fog.scale, 0.0, 1.0);		\n\
 #endif																						\n\
																							\n""\
 #ifdef __GLSL_CG_DATA_TYPES																\n\
 // Check necessary for ATI specific behavior												\n\
	gl_ClipVertex = (gl_ModelViewMatrix * gl_Vertex);										\n\
 #endif																						\n\
}																							\n\
";

char *Default_main_fragment_shader = "\
#ifdef FLAG_LIGHT																\n\
uniform int n_lights;															\n\
#endif																			\n\
																				\n\
#ifdef FLAG_DIFFUSE_MAP															\n\
uniform sampler2D sBasemap;														\n\
#endif																			\n\
																				\n\
#ifdef FLAG_GLOW_MAP															\n\
uniform sampler2D sGlowmap;														\n\
#endif																			\n\
																				\n\
#ifdef FLAG_SPEC_MAP															\n\
uniform sampler2D sSpecmap;														\n\
#endif																			\n\
																				\n\
#ifdef FLAG_ENV_MAP																\n\
uniform samplerCube sEnvmap;													\n\
uniform bool alpha_spec;														\n\
varying vec3 envReflect;														\n\
#endif																			\n\
																				\n\
#ifdef FLAG_NORMAL_MAP															\n\
uniform sampler2D sNormalmap;													\n\
varying mat3 tbnMatrix;															\n\
#endif																			\n\
																				\n\
#ifdef FLAG_FOG																	\n\
varying float fogDist;															\n\
#endif																			\n\
																				\n\
varying vec4 position;															\n\
varying vec3 lNormal;															\n\
																				\n\
#if SHADER_MODEL == 2															\n\
  #define MAX_LIGHTS 2															\n\
#else																			\n\
  #define MAX_LIGHTS 8															\n\
#endif																			\n\
																				\n\
#define SPEC_INTENSITY_POINT		5.3 // Point light							\n\
#define SPEC_INTENSITY_DIRECTIONAL	3.0 // Directional light					\n\
#define SPECULAR_FACTOR				1.75										\n\
#define SPECULAR_ALPHA				0.1											\n\
#define SPEC_FACTOR_NO_SPEC_MAP		0.6											\n\
#define ENV_ALPHA_FACTOR			0.3											\n\
#define GLOW_MAP_INTENSITY			1.5											\n\
#define AMBIENT_LIGHT_BOOST			1.0											\n\
																				\n""\
void main()																		\n\
{																				\n\
	vec3 eyeDir = vec3(normalize(-position).xyz); // Camera is at (0,0,0) in ModelView space	\n\
	vec4 lightAmbientDiffuse = vec4(0.0, 0.0, 0.0, 1.0);						\n\
	vec4 lightDiffuse = vec4(0.0, 0.0, 0.0, 1.0);								\n\
	vec4 lightAmbient = vec4(0.0, 0.0, 0.0, 1.0); 								\n\
	vec4 lightSpecular = vec4(0.0, 0.0, 0.0, 1.0);								\n\
	vec2 texCoord = gl_TexCoord[0].xy;											\n\
																				\n\
 #ifdef FLAG_LIGHT																\n\
  #ifdef FLAG_NORMAL_MAP														\n\
	// Normal map - convert from DXT5nm											\n\
	vec3 normal;																\n\
	normal.rg = (texture2D(sNormalmap, texCoord).ag * 2.0) - 1.0;				\n\
  #ifdef FLAG_ENV_MAP															\n\
	vec3 envOffset = vec3(0.0);													\n\
	envOffset.xy = normal.xy;													\n\
  #endif																		\n\
	normal.b = sqrt(1.0 - dot(normal.rg, normal.rg));							\n\
	// prevent breaking of normal maps											\n\
	normal = tbnMatrix * normal;												\n\
	float norm = length(normal);												\n\
	if ( length(normal) > 0.0 ) {												\n\
		normal /= norm ;														\n\
	} else {																	\n\
		normal = tbnMatrix * vec3(0.0, 0.0, 1.0);								\n\
	}																			\n\
  #else																			\n\
	vec3 normal = lNormal;														\n\
  #endif																		\n\
																				\n\
	vec3 lightDir;																\n\
	lightAmbient = gl_FrontMaterial.emission + (gl_LightModel.ambient * gl_FrontMaterial.ambient);	\n\
																				\n\
	#pragma optionNV unroll all													\n\
	for (int i = 0; i < MAX_LIGHTS; ++i) {										\n\
	  #if SHADER_MODEL > 2														\n\
		if (i > n_lights) {														\n\
			break;																\n\
		}																		\n\
	  #endif																	\n\
																				\n\
		float specularIntensity = 1.0;											\n\
		float attenuation = 1.0;												\n""\
																				\n\
		// Attenuation and light direction										\n\
	  #if SHADER_MODEL > 2														\n\
		if (gl_LightSource[i].position.w == 1.0) {								\n\
	  #else																		\n\
		if (gl_LightSource[i].position.w == 1.0 && i != 0) {					\n\
	  #endif																	\n\
			// Positional light source											\n\
			float dist = distance(gl_LightSource[i].position.xyz, position.xyz);	\n\
			lightDir = gl_LightSource[i].position.xyz - position.xyz;			\n\
																				\n\
		  #if SHADER_MODEL > 2													\n\
			if (gl_LightSource[i].spotCutoff < 91.0) {  // Tube light			\n\
				vec3 nearest = gl_LightSource[i].position.xyz;					\n\
				float beamlength = length(gl_LightSource[i].spotDirection);		\n\
				vec3 beamDir = normalize(gl_LightSource[i].spotDirection);		\n\
				// Get nearest point on line									\n\
				float neardist = dot(position.xyz - gl_LightSource[i].position.xyz , beamDir);	\n\
				// Move back from the endpoint of the beam along the beam by the distance we calculated	\n\
				nearest = gl_LightSource[i].position.xyz - beamDir * abs(neardist);	\n\
				lightDir = nearest - position.xyz;								\n""\
				dist = length(lightDir);										\n\
			}																	\n\
		  #endif																\n\
																				\n\
			lightDir = normalize(lightDir);										\n\
			attenuation = 1.0 / (gl_LightSource[i].constantAttenuation + (gl_LightSource[i].linearAttenuation *	 dist) + (gl_LightSource[i].quadraticAttenuation * dist * dist));	\n\
			specularIntensity = SPEC_INTENSITY_POINT;							\n\
																				\n\
		} else {																\n\
			// Directional light source											\n\
			lightDir = normalize(gl_LightSource[i].position.xyz);				\n\
			specularIntensity = SPEC_INTENSITY_DIRECTIONAL; // Directional light	\n\
		}																		\n\
																				\n\
		// Ambient and Diffuse													\n\
		lightAmbient += (gl_FrontLightProduct[i].ambient * attenuation);		\n\
		lightDiffuse += ((gl_FrontLightProduct[i].diffuse * max(dot(normal, lightDir), 0.0)) * attenuation); \n\
																				\n\
		// Specular																\n\
		float NdotHV = clamp(dot(normal, normalize(eyeDir + lightDir)), 0.0, 1.0);	\n\
		lightSpecular += ((gl_FrontLightProduct[i].specular * pow(max(0.0, NdotHV), gl_FrontMaterial.shininess)) * attenuation) * specularIntensity;	\n\
	}																			\n\
																				\n\
	lightAmbientDiffuse = lightAmbient + lightDiffuse;							\n\
 #else																			\n\
	lightAmbientDiffuse = gl_Color;												\n\
	lightSpecular = gl_SecondaryColor;											\n\
 #endif																			\n\
																				\n""\
 #ifdef FLAG_DIFFUSE_MAP														\n\
 // Base color																	\n\
	vec4 baseColor = texture2D(sBasemap, texCoord);								\n\
 #else																			\n\
	vec4 baseColor = gl_Color;													\n\
 #endif																			\n\
																				\n\
	vec4 fragmentColor;															\n\
	fragmentColor.rgb = baseColor.rgb * max(lightAmbientDiffuse.rgb * AMBIENT_LIGHT_BOOST, gl_LightModel.ambient.rgb - 0.425);																	\n\
	fragmentColor.a = baseColor.a;												\n\
																				\n\
 #ifdef FLAG_SPEC_MAP															\n\
 // Spec color																	\n\
	fragmentColor.rgb += lightSpecular.rgb * (texture2D(sSpecmap, texCoord).rgb * SPECULAR_FACTOR);	\n\
	fragmentColor.a += (dot(lightSpecular.a, lightSpecular.a) * SPECULAR_ALPHA);	\n\
 #else																			\n\
	fragmentColor.rgb += lightSpecular.rgb * (baseColor.rgb * SPEC_FACTOR_NO_SPEC_MAP);	\n\
 #endif																			\n\
																				\n\
 #ifdef FLAG_ENV_MAP															\n\
 // Env color																	\n\
  #ifdef FLAG_NORMAL_MAP														\n\
	vec3 envReflectNM = envReflect + envOffset;									\n\
	vec3 envIntensity = (alpha_spec) ? vec3(texture2D(sSpecmap, texCoord).a) : texture2D(sSpecmap, texCoord).rgb;	\n\
	fragmentColor.a += (dot(textureCube(sEnvmap, envReflectNM).rgb, textureCube(sEnvmap, envReflectNM).rgb) * ENV_ALPHA_FACTOR);	\n\
	fragmentColor.rgb += textureCube(sEnvmap, envReflectNM).rgb * envIntensity;	\n\
  #else																			\n\
	vec3 envIntensity = (alpha_spec) ? vec3(texture2D(sSpecmap, texCoord).a) : texture2D(sSpecmap, texCoord).rgb;	\n\
	fragmentColor.a += (dot(textureCube(sEnvmap, envReflect).rgb, textureCube(sEnvmap, envReflect).rgb) * ENV_ALPHA_FACTOR);	\n\
	fragmentColor.rgb += textureCube(sEnvmap, envReflect).rgb * envIntensity;	\n\
  #endif																		\n\
 #endif																			\n\
																				\n\
 #ifdef FLAG_GLOW_MAP															\n\
 // Glow color																	\n\
	fragmentColor.rgb += texture2D(sGlowmap, texCoord).rgb * GLOW_MAP_INTENSITY;\n\
 #endif																			\n\
																				\n""\
 #ifdef FLAG_FOG																\n\
	fragmentColor.rgb = mix(fragmentColor.rgb, gl_Fog.color.rgb, fogDist);		\n\
 #endif																			\n\
																				\n\
	gl_FragColor = fragmentColor;												\n\
}																				\n\
";

char* Default_fxaa_vertex_shader = 
"#extension GL_EXT_gpu_shader4 : enable\n"
"uniform float rt_w;\n"
"uniform float rt_h;\n"
"varying vec2 v_rcpFrame;\n"
"noperspective varying vec2 v_pos;\n"
"void main() {\n"
"	gl_Position = gl_Vertex;\n"
"	v_rcpFrame = vec2(1.0/rt_w, 1.0/rt_h);\n"
"	v_pos = gl_Vertex.xy*0.5 + 0.5;\n"
"}\n";

char* Default_fxaa_fragment_shader = "#extension GL_EXT_gpu_shader4 : enable\n"
"#define FXAA_EARLY_EXIT 1\n"
"#define FXAA_DISCARD 1 \n"
"#if SHADER_MODEL == 2\n"
"    #define FXAA_GLSL_120 1\n"
"	#define FXAA_GLSL_130 0\n"
"#endif\n"
"#if SHADER_MODEL > 2\n"
"    #define FXAA_GLSL_130 1\n"
"	#define FXAA_GLSL_120 0\n"
"#endif\n"
"#ifndef FXAA_FAST_PIXEL_OFFSET\n"
"    #ifdef GL_EXT_gpu_shader4\n"
"        #define FXAA_FAST_PIXEL_OFFSET 1\n"
"    #endif\n"
"    #ifdef GL_NV_gpu_shader5\n"
"		#extension GL_NV_gpu_shader5 : enable\n"
"        #define FXAA_FAST_PIXEL_OFFSET 1\n"
"    #endif\n"
"    #ifdef GL_ARB_gpu_shader5\n"
"		#extension GL_ARB_gpu_shader5 : enable\n"
"        #define FXAA_FAST_PIXEL_OFFSET 1\n"
"    #endif\n"
"    #ifndef FXAA_FAST_PIXEL_OFFSET\n"
"        #define FXAA_FAST_PIXEL_OFFSET 0\n"
"    #endif\n"
"#endif\n"
"#ifndef FXAA_GATHER4_ALPHA\n"
"    #ifdef GL_ARB_gpu_shader5\n"
"		#extension GL_ARB_gpu_shader5 : enable\n"
"        #define FXAA_GATHER4_ALPHA 1\n"
"    #endif\n"
"    #ifdef GL_NV_gpu_shader5\n"
"		#extension GL_NV_gpu_shader5 : enable\n"
"        #define FXAA_GATHER4_ALPHA 1\n"
"    #endif\n"
"    #ifndef FXAA_GATHER4_ALPHA\n"
"        #define FXAA_GATHER4_ALPHA 0\n"
"    #endif\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 10)\n"
"	#define FXAA_QUALITY__PS 3\n"
"	#define FXAA_QUALITY__P0 1.5\n"
"	#define FXAA_QUALITY__P1 3.0\n"
"	#define FXAA_QUALITY__P2 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 11)\n"
"	#define FXAA_QUALITY__PS 4\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 3.0\n"
"	#define FXAA_QUALITY__P3 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 12)\n"
"	#define FXAA_QUALITY__PS 5\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 4.0\n"
"	#define FXAA_QUALITY__P4 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 13)\n"
"	#define FXAA_QUALITY__PS 6\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 2.0\n"
"	#define FXAA_QUALITY__P4 4.0\n"
"	#define FXAA_QUALITY__P5 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 14)\n"
"	#define FXAA_QUALITY__PS 7\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 2.0\n"
"	#define FXAA_QUALITY__P4 2.0\n"
"	#define FXAA_QUALITY__P5 4.0\n"
"	#define FXAA_QUALITY__P6 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 25)\n"
"	#define FXAA_QUALITY__PS 8\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 2.0\n"
"	#define FXAA_QUALITY__P4 2.0\n"
"	#define FXAA_QUALITY__P5 2.0\n"
"	#define FXAA_QUALITY__P6 4.0\n"
"	#define FXAA_QUALITY__P7 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 26)\n"
"	#define FXAA_QUALITY__PS 9\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 2.0\n"
"	#define FXAA_QUALITY__P4 2.0\n"
"	#define FXAA_QUALITY__P5 2.0\n"
"	#define FXAA_QUALITY__P6 2.0\n"
"	#define FXAA_QUALITY__P7 4.0\n"
"	#define FXAA_QUALITY__P8 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 27)\n"
"	#define FXAA_QUALITY__PS 10\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 2.0\n"
"	#define FXAA_QUALITY__P4 2.0\n"
"	#define FXAA_QUALITY__P5 2.0\n"
"	#define FXAA_QUALITY__P6 2.0\n"
"	#define FXAA_QUALITY__P7 2.0\n"
"	#define FXAA_QUALITY__P8 4.0\n"
"	#define FXAA_QUALITY__P9 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 28)\n"
"	#define FXAA_QUALITY__PS 11\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.5\n"
"	#define FXAA_QUALITY__P2 2.0\n"
"	#define FXAA_QUALITY__P3 2.0\n"
"	#define FXAA_QUALITY__P4 2.0\n"
"	#define FXAA_QUALITY__P5 2.0\n"
"	#define FXAA_QUALITY__P6 2.0\n"
"	#define FXAA_QUALITY__P7 2.0\n"
"	#define FXAA_QUALITY__P8 2.0\n"
"	#define FXAA_QUALITY__P9 4.0\n"
"	#define FXAA_QUALITY__P10 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY__PRESET == 39)\n"
"	#define FXAA_QUALITY__PS 12\n"
"	#define FXAA_QUALITY__P0 1.0\n"
"	#define FXAA_QUALITY__P1 1.0\n"
"	#define FXAA_QUALITY__P2 1.0\n"
"	#define FXAA_QUALITY__P3 1.0\n"
"	#define FXAA_QUALITY__P4 1.0\n"
"	#define FXAA_QUALITY__P5 1.5\n"
"	#define FXAA_QUALITY__P6 2.0\n"
"	#define FXAA_QUALITY__P7 2.0\n"
"	#define FXAA_QUALITY__P8 2.0\n"
"	#define FXAA_QUALITY__P9 2.0\n"
"	#define FXAA_QUALITY__P10 4.0\n"
"	#define FXAA_QUALITY__P11 8.0\n"
"#endif\n"
"#define FxaaBool bool\n"
"#define FxaaDiscard discard\n"
"#define FxaaFloat float\n"
"#define FxaaFloat2 vec2\n"
"#define FxaaFloat3 vec3\n"
"#define FxaaFloat4 vec4\n"
"#define FxaaHalf float\n"
"#define FxaaHalf2 vec2\n"
"#define FxaaHalf3 vec3\n"
"#define FxaaHalf4 vec4\n"
"#define FxaaInt2 ivec2\n"
"#define FxaaSat(x) clamp(x, 0.0, 1.0)\n"
"#define FxaaTex sampler2D\n"
"#define FxaaTexTop(t, p) texture2DLod(t, p, 0.0)\n"
" #if (FXAA_FAST_PIXEL_OFFSET == 1)\n"
"    #define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)\n"
"#else\n"
"    #define FxaaTexOff(t, p, o, r) texture2DLod(t, p + (o * r), 0.0)\n"
"#endif\n"
"#if (FXAA_GLSL_120 == 1)\n"
"    #if (FXAA_GATHER4_ALPHA == 1)\n"
"        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"
"        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"
"        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"
"        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"
"    #endif\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_GLSL_130 == 1)\n"
"    #if (FXAA_GATHER4_ALPHA == 1)\n"
"        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"
"        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"
"        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"
"        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"
"    #endif\n"
"#endif\n"
"FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }\n"
"FxaaFloat4 FxaaPixelShader(\n"
"    FxaaFloat2 pos,\n"
"    FxaaTex tex,\n"
"    FxaaFloat2 fxaaQualityRcpFrame,\n"
"    FxaaFloat fxaaQualitySubpix,\n"
"    FxaaFloat fxaaQualityEdgeThreshold,\n"
"    FxaaFloat fxaaQualityEdgeThresholdMin\n"
") {\n"
"    FxaaFloat2 posM;\n"
"    posM.x = pos.x;\n"
"    posM.y = pos.y;\n"
"    #if (FXAA_GATHER4_ALPHA == 1)\n"
"        #if (FXAA_DISCARD == 0)\n"
"            FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);\n"
"            #if (FXAA_GREEN_AS_LUMA == 0)\n"
"                #define lumaM rgbyM.w\n"
"            #else\n"
"                #define lumaM rgbyM.y\n"
"            #endif\n"
"        #endif\n"
"        #if (FXAA_GREEN_AS_LUMA == 0)\n"
"            FxaaFloat4 luma4A = FxaaTexAlpha4(tex, posM);\n"
"            FxaaFloat4 luma4B = FxaaTexOffAlpha4(tex, posM, FxaaInt2(-1, -1));\n"
"        #else\n"
"            FxaaFloat4 luma4A = FxaaTexGreen4(tex, posM);\n"
"            FxaaFloat4 luma4B = FxaaTexOffGreen4(tex, posM, FxaaInt2(-1, -1));\n"
"        #endif\n"
"        #if (FXAA_DISCARD == 1)\n"
"            #define lumaM luma4A.w\n"
"        #endif\n"
"        #define lumaE luma4A.z\n"
"        #define lumaS luma4A.x\n"
"        #define lumaSE luma4A.y\n"
"        #define lumaNW luma4B.w\n"
"        #define lumaN luma4B.z\n"
"        #define lumaW luma4B.x\n"
"    #else\n"
"        FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);\n"
"        #if (FXAA_GREEN_AS_LUMA == 0)\n"
"            #define lumaM rgbyM.w\n"
"        #else\n"
"            #define lumaM rgbyM.y\n"
"        #endif\n"
"        FxaaFloat lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0), fxaaQualityRcpFrame.xy));\n"
"    #endif\n"
"    FxaaFloat maxSM = max(lumaS, lumaM);\n"
"    FxaaFloat minSM = min(lumaS, lumaM);\n"
"    FxaaFloat maxESM = max(lumaE, maxSM);\n"
"    FxaaFloat minESM = min(lumaE, minSM);\n"
"    FxaaFloat maxWN = max(lumaN, lumaW);\n"
"    FxaaFloat minWN = min(lumaN, lumaW);\n"
"    FxaaFloat rangeMax = max(maxWN, maxESM);\n"
"    FxaaFloat rangeMin = min(minWN, minESM);\n"
"    FxaaFloat rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;\n"
"    FxaaFloat range = rangeMax - rangeMin;\n"
"    FxaaFloat rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);\n"
"    FxaaBool earlyExit = range < rangeMaxClamped;\n"
"    if(earlyExit)\n"
"        #if (FXAA_DISCARD == 1)\n"
"            FxaaDiscard;\n"
"        #else\n"
"            return rgbyM;\n"
"        #endif\n"
"    #if (FXAA_GATHER4_ALPHA == 0)\n"
"        FxaaFloat lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));\n"
"    #else\n"
"        FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(1, -1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));\n"
"    #endif\n"
"    FxaaFloat lumaNS = lumaN + lumaS;\n"
"    FxaaFloat lumaWE = lumaW + lumaE;\n"
"    FxaaFloat subpixRcpRange = 1.0/range;\n"
"    FxaaFloat subpixNSWE = lumaNS + lumaWE;\n"
"    FxaaFloat edgeHorz1 = (-2.0 * lumaM) + lumaNS;\n"
"    FxaaFloat edgeVert1 = (-2.0 * lumaM) + lumaWE;\n"
"    FxaaFloat lumaNESE = lumaNE + lumaSE;\n"
"    FxaaFloat lumaNWNE = lumaNW + lumaNE;\n"
"    FxaaFloat edgeHorz2 = (-2.0 * lumaE) + lumaNESE;\n"
"    FxaaFloat edgeVert2 = (-2.0 * lumaN) + lumaNWNE;\n"
"    FxaaFloat lumaNWSW = lumaNW + lumaSW;\n"
"    FxaaFloat lumaSWSE = lumaSW + lumaSE;\n"
"    FxaaFloat edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);\n"
"    FxaaFloat edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);\n"
"    FxaaFloat edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;\n"
"    FxaaFloat edgeVert3 = (-2.0 * lumaS) + lumaSWSE;\n"
"    FxaaFloat edgeHorz = abs(edgeHorz3) + edgeHorz4;\n"
"    FxaaFloat edgeVert = abs(edgeVert3) + edgeVert4;\n"
"    FxaaFloat subpixNWSWNESE = lumaNWSW + lumaNESE;\n"
"    FxaaFloat lengthSign = fxaaQualityRcpFrame.x;\n"
"    FxaaBool horzSpan = edgeHorz >= edgeVert;\n"
"    FxaaFloat subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;\n"
"    if(!horzSpan) lumaN = lumaW;\n"
"    if(!horzSpan) lumaS = lumaE;\n"
"    if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;\n"
"    FxaaFloat subpixB = (subpixA * (1.0/12.0)) - lumaM;\n"
"    FxaaFloat gradientN = lumaN - lumaM;\n"
"    FxaaFloat gradientS = lumaS - lumaM;\n"
"    FxaaFloat lumaNN = lumaN + lumaM;\n"
"    FxaaFloat lumaSS = lumaS + lumaM;\n"
"    FxaaBool pairN = abs(gradientN) >= abs(gradientS);\n"
"    FxaaFloat gradient = max(abs(gradientN), abs(gradientS));\n"
"    if(pairN) lengthSign = -lengthSign;\n"
"    FxaaFloat subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);\n"
"    FxaaFloat2 posB;\n"
"    posB.x = posM.x;\n"
"    posB.y = posM.y;\n"
"    FxaaFloat2 offNP;\n"
"    offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;\n"
"    offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;\n"
"    if(!horzSpan) posB.x += lengthSign * 0.5;\n"
"    if( horzSpan) posB.y += lengthSign * 0.5;\n"
"    FxaaFloat2 posN;\n"
"    posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;\n"
"    posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;\n"
"    FxaaFloat2 posP;\n"
"    posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;\n"
"    posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;\n"
"    FxaaFloat subpixD = ((-2.0)*subpixC) + 3.0;\n"
"    FxaaFloat lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));\n"
"    FxaaFloat subpixE = subpixC * subpixC;\n"
"    FxaaFloat lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));\n"
"    if(!pairN) lumaNN = lumaSS;\n"
"    FxaaFloat gradientScaled = gradient * 1.0/4.0;\n"
"    FxaaFloat lumaMM = lumaM - lumaNN * 0.5;\n"
"    FxaaFloat subpixF = subpixD * subpixE;\n"
"    FxaaBool lumaMLTZero = lumaMM < 0.0;\n"
"    lumaEndN -= lumaNN * 0.5;\n"
"    lumaEndP -= lumaNN * 0.5;\n"
"    FxaaBool doneN = abs(lumaEndN) >= gradientScaled;\n"
"    FxaaBool doneP = abs(lumaEndP) >= gradientScaled;\n"
"    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;\n"
"    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;\n"
"    FxaaBool doneNP = (!doneN) || (!doneP);\n"
"    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;\n"
"    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;\n"
"    if(doneNP) {\n"
"        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"        doneN = abs(lumaEndN) >= gradientScaled;\n"
"        doneP = abs(lumaEndP) >= gradientScaled;\n"
"        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;\n"
"        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;\n"
"        doneNP = (!doneN) || (!doneP);\n"
"        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;\n"
"        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;\n"
"        #if (FXAA_QUALITY__PS > 3)\n"
"        if(doneNP) {\n"
"            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"            doneN = abs(lumaEndN) >= gradientScaled;\n"
"            doneP = abs(lumaEndP) >= gradientScaled;\n"
"            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;\n"
"            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;\n"
"            doneNP = (!doneN) || (!doneP);\n"
"            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;\n"
"            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;\n"
"            #if (FXAA_QUALITY__PS > 4)\n"
"            if(doneNP) {\n"
"                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                doneN = abs(lumaEndN) >= gradientScaled;\n"
"                doneP = abs(lumaEndP) >= gradientScaled;\n"
"                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;\n"
"                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;\n"
"                doneNP = (!doneN) || (!doneP);\n"
"                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;\n"
"                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;\n"
"                #if (FXAA_QUALITY__PS > 5)\n"
"                if(doneNP) {\n"
"                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                    doneN = abs(lumaEndN) >= gradientScaled;\n"
"                    doneP = abs(lumaEndP) >= gradientScaled;\n"
"                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;\n"
"                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;\n"
"                    doneNP = (!doneN) || (!doneP);\n"
"                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;\n"
"                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;\n"
"                    #if (FXAA_QUALITY__PS > 6)\n"
"                    if(doneNP) {\n"
"                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                        doneN = abs(lumaEndN) >= gradientScaled;\n"
"                        doneP = abs(lumaEndP) >= gradientScaled;\n"
"                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;\n"
"                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;\n"
"                        doneNP = (!doneN) || (!doneP);\n"
"                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;\n"
"                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;\n"
"                        #if (FXAA_QUALITY__PS > 7)\n"
"                        if(doneNP) {\n"
"                            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                            doneN = abs(lumaEndN) >= gradientScaled;\n"
"                            doneP = abs(lumaEndP) >= gradientScaled;\n"
"                            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;\n"
"                            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;\n"
"                            doneNP = (!doneN) || (!doneP);\n"
"                            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;\n"
"                            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;\n"
"    #if (FXAA_QUALITY__PS > 8)\n"
"    if(doneNP) {\n"
"        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"        doneN = abs(lumaEndN) >= gradientScaled;\n"
"        doneP = abs(lumaEndP) >= gradientScaled;\n"
"        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;\n"
"        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;\n"
"        doneNP = (!doneN) || (!doneP);\n"
"        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;\n"
"        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;\n"
"        #if (FXAA_QUALITY__PS > 9)\n"
"        if(doneNP) {\n"
"            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"            doneN = abs(lumaEndN) >= gradientScaled;\n"
"            doneP = abs(lumaEndP) >= gradientScaled;\n"
"            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;\n"
"            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;\n"
"            doneNP = (!doneN) || (!doneP);\n"
"            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;\n"
"            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;\n"
"            #if (FXAA_QUALITY__PS > 10)\n"
"            if(doneNP) {\n"
"                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                doneN = abs(lumaEndN) >= gradientScaled;\n"
"                doneP = abs(lumaEndP) >= gradientScaled;\n"
"                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;\n"
"                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;\n"
"                doneNP = (!doneN) || (!doneP);\n"
"                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;\n"
"                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;\n"
"                #if (FXAA_QUALITY__PS > 11)\n"
"                if(doneNP) {\n"
"                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                    doneN = abs(lumaEndN) >= gradientScaled;\n"
"                    doneP = abs(lumaEndP) >= gradientScaled;\n"
"                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;\n"
"                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;\n"
"                    doneNP = (!doneN) || (!doneP);\n"
"                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;\n"
"                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;\n"
"                    #if (FXAA_QUALITY__PS > 12)\n"
"                    if(doneNP) {\n"
"                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                        doneN = abs(lumaEndN) >= gradientScaled;\n"
"                        doneP = abs(lumaEndP) >= gradientScaled;\n"
"                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P12;\n"
"                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P12;\n"
"                        doneNP = (!doneN) || (!doneP);\n"
"                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P12;\n"
"                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P12;\n"
"                    }\n"
"                    #endif\n"
"                }\n"
"                #endif\n"
"            }\n"
"            #endif\n"
"        }\n"
"        #endif\n"
"    }\n"
"    #endif\n"
"                        }\n"
"                        #endif\n"
"                    }\n"
"                    #endif\n"
"                }\n"
"                #endif\n"
"            }\n"
"            #endif\n"
"        }\n"
"        #endif\n"
"    }\n"
"    FxaaFloat dstN = posM.x - posN.x;\n"
"    FxaaFloat dstP = posP.x - posM.x;\n"
"    if(!horzSpan) dstN = posM.y - posN.y;\n"
"    if(!horzSpan) dstP = posP.y - posM.y;\n"
"    FxaaBool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;\n"
"    FxaaFloat spanLength = (dstP + dstN);\n"
"    FxaaBool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;\n"
"    FxaaFloat spanLengthRcp = 1.0/spanLength;\n"
"    FxaaBool directionN = dstN < dstP;\n"
"    FxaaFloat dst = min(dstN, dstP);\n"
"    FxaaBool goodSpan = directionN ? goodSpanN : goodSpanP;\n"
"    FxaaFloat subpixG = subpixF * subpixF;\n"
"    FxaaFloat pixelOffset = (dst * (-spanLengthRcp)) + 0.5;\n"
"    FxaaFloat subpixH = subpixG * fxaaQualitySubpix;\n"
"    FxaaFloat pixelOffsetGood = goodSpan ? pixelOffset : 0.0;\n"
"    FxaaFloat pixelOffsetSubpix = max(pixelOffsetGood, subpixH);\n"
"    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;\n"
"    if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;\n"
"    #if (FXAA_DISCARD == 1)\n"
"        return FxaaTexTop(tex, posM);\n"
"    #else\n"
"        return FxaaFloat4(FxaaTexTop(tex, posM).xyz, lumaM);\n"
"    #endif\n"
"}\n"
"uniform sampler2D tex0;\n"
"varying vec2 v_rcpFrame;\n"
"noperspective varying vec2 v_pos;\n"
"void main() {\n"
"	gl_FragColor = FxaaPixelShader(v_pos, tex0, v_rcpFrame, FXAA_QUALITY__SUBPIX, FXAA_QUALITY__EDGE_THRESHOLD, FXAA_QUALITY__EDGE_THRESHOLD_MIN);\n"
"}\n";

char *Default_blur_fragment_shader = "\
varying float blurSize;											\n\
																\n\
uniform sampler2D tex;											\n\
																\n\
#define BLUR_SIZE_DIV 3.0										\n\
																\n\
// Gaussian Blur												\n\
// 512x512 and smaller textures give best results				\n\
// 2 passes required											\n\
void main()														\n\
{																\n\
	// Echelon9 - Due to Apple not implementing array constructors in OS X's		\n\
	// GLSL implementation we need to setup the arrays this way as a workaround		\n\
	float BlurWeights[6];										\n\
																\n\
	BlurWeights[5] = 0.0402;									\n\
	BlurWeights[4] = 0.0623;									\n\
	BlurWeights[3] = 0.0877;									\n\
	BlurWeights[2] = 0.1120;									\n\
	BlurWeights[1] = 0.1297;									\n\
	BlurWeights[0] = 0.1362;									\n\
																\n\
	vec4 sum = texture2D(tex, gl_TexCoord[0].xy) * BlurWeights[0];	\n\
																\n\
#ifdef PASS_0													\n\
	for (int i = 1; i < 6; i++) {								\n\
		sum += texture2D(tex, vec2(clamp(gl_TexCoord[0].x - float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0), gl_TexCoord[0].y)) * BlurWeights[i];	\n\
		sum += texture2D(tex, vec2(clamp(gl_TexCoord[0].x + float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0), gl_TexCoord[0].y)) * BlurWeights[i];	\n\
	}															\n\
#endif															\n\
																\n\
#ifdef PASS_1													\n\
	for (int i = 1; i < 6; i++) {								\n\
		sum += texture2D(tex, vec2(gl_TexCoord[0].x, clamp(gl_TexCoord[0].y - float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0))) * BlurWeights[i];	\n\
		sum += texture2D(tex, vec2(gl_TexCoord[0].x, clamp(gl_TexCoord[0].y + float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0))) * BlurWeights[i];	\n\
	}															\n\
#endif															\n\
																\n\
	gl_FragColor = sum;											\n\
}																\n\
";

char *Default_brightpass_fragment_shader = "\
uniform sampler2D tex;									\n\
														\n\
const float Luminance = 0.08;							\n\
const float fMiddleGray = 0.2;							\n\
const float fWhiteCutoff = 0.4;							\n\
														\n\
// High-pass filter										\n\
void main() {											\n\
	vec4 ColorOut = texture2D(tex, gl_TexCoord[0].xy);	\n\
														\n\
	ColorOut *= fMiddleGray / (Luminance + 0.001);		\n\
	ColorOut *= (1.0 + (ColorOut / (fWhiteCutoff * fWhiteCutoff)));	\n\
	ColorOut -= 6.0;									\n\
														\n\
	ColorOut /= (10.0 + ColorOut); //25.0;				\n\
														\n\
	gl_FragColor = ColorOut;							\n\
}														\n\
";

char *Default_post_fragment_shader = "\
uniform sampler2D tex;									\n\
uniform float timer;									\n\
uniform sampler2D bloomed;								\n\
uniform float bloom_intensity;							\n\
														\n\
#ifdef FLAG_DISTORT_NOISE								\n\
uniform float noise_amount;								\n\
#endif													\n\
														\n\
#ifdef FLAG_SATURATION									\n\
uniform float saturation;								\n\
#endif													\n\
														\n\
#ifdef FLAG_BRIGHTNESS									\n\
uniform float brightness;								\n\
#endif													\n\
														\n\
#ifdef FLAG_CONTRAST									\n\
uniform float contrast;									\n\
#endif													\n\
														\n\
#ifdef FLAG_GRAIN										\n\
uniform float film_grain;								\n\
#endif													\n\
														\n\
#ifdef FLAG_STRIPES										\n\
uniform float tv_stripes;								\n\
#endif													\n\
														\n\
#ifdef FLAG_CUTOFF										\n\
uniform float cutoff;									\n\
#endif													\n\
														\n\
#ifdef FLAG_DITH										\n\
uniform float dither;									\n\
#endif													\n\
														\n\
uniform sampler2D blurred_tex;							\n\
uniform sampler2D depth_tex;							\n\
														\n""\
void main()												\n\
{														\n\
 #ifdef FLAG_DISTORT_NOISE								\n\
 // Distort noise										\n\
	float distort_factor = timer * sin(gl_TexCoord[0].x * gl_TexCoord[0].y * 100.0 + timer);	\n\
	distort_factor = mod(distort_factor, 8.0) * mod(distort_factor, 4.0);	\n\
														\n\
	vec2 distort;										\n\
	distort = vec2(mod(distort_factor, noise_amount), mod(distort_factor, noise_amount + 0.002));	\n\
 #else													\n\
	vec2 distort = vec2(0, 0);							\n\
 #endif													\n\
														\n\
 // Global constant										\n\
	vec4 color_in;										\n\
	vec4 color_out;										\n\
														\n\
 // Bloom												\n\
	if (bloom_intensity > 0.0) {						\n\
		color_in = texture2D(tex, gl_TexCoord[0].xy + distort);	\n\
		vec4 color_bloom = texture2D(bloomed, gl_TexCoord[0].xy + distort);	\n\
		color_in = mix(color_in,  max(color_in + 0.7 * color_bloom, color_bloom), bloom_intensity);	\n\
	} else {											\n\
		color_in = texture2D(tex, gl_TexCoord[0].xy + distort);	\n\
	}													\n\
														\n\
 #ifdef FLAG_SATURATION									\n\
 // Saturation											\n\
	vec4 color_grayscale = color_in;					\n\
	color_grayscale.rgb = vec3(dot(color_in.rgb, vec3(0.299, 0.587, 0.184)));	\n\
	color_out = mix(color_in, color_grayscale, 1.0 - saturation);	\n\
 #else													\n\
	color_out = color_in;								\n\
 #endif													\n\
														\n\
 #ifdef FLAG_BRIGHTNESS									\n\
 // Brightness											\n\
	vec3 Afactor = vec3(brightness);					\n\
	color_out.rgb = color_out.rgb * Afactor;			\n\
 #endif													\n\
														\n\
 #ifdef FLAG_CONTRAST									\n\
 // Contrast											\n\
	vec3 Bfactor = vec3(0.5 - 0.5 * contrast);			\n\
	color_out.rgb = color_out.rgb + Bfactor;			\n\
 #endif													\n\
														\n\
 #ifdef FLAG_GRAIN										\n\
 // Film Grain											\n\
	float x = gl_TexCoord[0].x * gl_TexCoord[0].y * timer * 1000.0;	\n\
	x = mod(x, 13.0) * mod(x, 123.0);					\n\
	float dx = mod(x, 0.01);							\n\
														\n\
	vec3 result = color_out.rgb + color_out.rgb * clamp(0.1 + dx * 100.0, 0.0, 1.0);	\n\
														\n""\
	color_out.rgb = mix(color_out.rgb, result, film_grain);	\n\
 #endif													\n\
														\n\
 #ifdef FLAG_STRIPES									\n\
 // TV-Stripes (Old School)								\n\
	vec2 sc;											\n\
	sc.x = sin(gl_TexCoord[0].y * 2048.0);				\n\
	sc.y = cos(gl_TexCoord[0].y * 2048.0);				\n\
	vec3 stripes = color_out.rgb + color_out.rgb * vec3(sc.x, sc.y, sc.x) * 0.8;	\n\
														\n\
	color_out.rgb = mix(color_out.rgb, stripes, tv_stripes);	\n\
 #endif													\n\
														\n\
 #ifdef FLAG_CUTOFF										\n\
	// Experimental cutoff shader						\n\
	if (cutoff > 0.0) {									\n\
		vec4 color_greyscale;							\n\
		color_greyscale.rgb = vec3(dot(color_in.rgb, vec3(0.299, 0.587, 0.184)));	\n\
		vec4 normalized_col;							\n\
		float col_length = (length(color_out.rgb));		\n\
		if (col_length > 1.0) {							\n\
			normalized_col = ((color_out)/col_length);	\n\
		} else {										\n\
			normalized_col = color_out;					\n\
		}												\n\
		vec3 unit_grey = vec3(0.5773);					\n\
		float sat = dot(normalized_col.rgb, unit_grey);	\n\
		color_out = mix(color_greyscale, color_out, sat * cutoff);	\n\
	}													\n\
 #endif													\n\
														\n\
 #ifdef FLAG_DITH										\n\
 // Dithering											\n\
	float downsampling_factor = 4;						\n\
	float bias = 0.5;									\n\
														\n\
	color_out.rgb = floor(color_out.rgb * downsampling_factor + bias) / downsampling_factor;	\n\
 #endif													\n\
														\n\
	gl_FragColor = color_out;							\n\
}														\n\
";

char *Default_post_vertex_shader = "\
varying float blurSize;						\n\
uniform float bsize;						\n\
											\n\
void main()									\n\
{											\n\
	gl_TexCoord[0] = gl_MultiTexCoord0;		\n\
	gl_Position = gl_Vertex;				\n\
											\n\
	gl_FrontColor = gl_Color;				\n\
	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);	\n\
											\n\
	blurSize = 1.0 / bsize;					\n\
											\n\
 // Check necessary for ATI specific behavior	\n\
 #ifdef __GLSL_CG_DATA_TYPES				\n\
	gl_ClipVertex = (gl_ModelViewMatrix * gl_Vertex);	\n\
 #endif										\n\
}											\n\
";

char* Default_fxaa_prepass_shader ="\
uniform sampler2D tex;											\n\
																\n\
void main() {													\n\
	vec4 color = texture2D(tex, gl_TexCoord[0].xy);				\n\
																\n\
	gl_FragColor = vec4(color.rgb, dot(color.rgb, vec3(0.299, 0.587, 0.114)) );	\n\
}																\n\
";