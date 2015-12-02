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
extern char *Default_controlconfig_table;
extern char *Default_mod_table;
extern char *Default_post_processing_table;
extern char* Default_main_vertex_shader;
extern char* Default_main_fragment_shader;
extern char* Default_main_geometry_shader;
extern char* Default_fxaa_vertex_shader;
extern char* Default_fxaa_fragment_shader;
extern char* Default_blur_fragment_shader;
extern char* Default_brightpass_fragment_shader;
extern char* Default_post_fragment_shader;
extern char* Default_post_vertex_shader;
extern char* Default_fxaa_prepass_shader;
extern char* Default_effect_vertex_shader;
extern char* Default_effect_fragment_shader;
extern char* Default_effect_particle_fragment_shader;
extern char* Default_effect_distortion_vertex_shader;
extern char* Default_effect_distortion_fragment_shader;
extern char* Default_effect_ribbon_geometry_shader;
extern char* Default_effect_screen_geometry_shader;
extern char* Default_shadowdebug_vertex_shader;
extern char* Default_shadowdebug_fragment_shader;
extern char* Default_lightshaft_fragment_shader;
extern char* Default_video_vertex_shader;
extern char* Default_video_fragment_shader;
extern char* Default_deferred_vertex_shader;
extern char* Default_deferred_fragment_shader;
extern char* Default_deferred_clear_vertex_shader;
extern char* Default_deferred_clear_fragment_shader;
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
	{ "controlconfigdefaults.tbl",  Default_controlconfig_table },
	{ "game_settings.tbl",		Default_mod_table},
	{ "post_processing.tbl",	Default_post_processing_table},
	{ "main-f.sdr",				Default_main_fragment_shader},
	{ "main-v.sdr",				Default_main_vertex_shader},
	{ "main-g.sdr",				Default_main_geometry_shader},
	{ "fxaa-v.sdr",				Default_fxaa_vertex_shader},
	{ "fxaa-f.sdr",				Default_fxaa_fragment_shader},
	{ "blur-f.sdr",				Default_blur_fragment_shader},
	{ "brightpass-f.sdr",		Default_brightpass_fragment_shader},
	{ "post-f.sdr",				Default_post_fragment_shader},
	{ "post-v.sdr",				Default_post_vertex_shader},
	{ "fxaapre-f.sdr",			Default_fxaa_prepass_shader},
	{ "effect-v.sdr",			Default_effect_vertex_shader},
	{ "effect-f.sdr",			Default_effect_fragment_shader},
	{ "effect-particle-f.sdr",	Default_effect_particle_fragment_shader},
    { "effect-distort-v.sdr",   Default_effect_distortion_vertex_shader},
	{ "effect-distort-f.sdr",	Default_effect_distortion_fragment_shader},
	{ "effect-screen-g.sdr",	Default_effect_screen_geometry_shader},
	{ "shadowdebug-v.sdr",		Default_shadowdebug_vertex_shader},
	{ "shadowdebug-f.sdr",		Default_shadowdebug_fragment_shader},
	{ "ls-f.sdr",				Default_lightshaft_fragment_shader},
	{ "video-v.sdr",			Default_video_vertex_shader},
	{ "video-f.sdr",			Default_video_fragment_shader},
	{ "deferred-v.sdr",			Default_deferred_vertex_shader},
	{ "deferred-f.sdr",			Default_deferred_fragment_shader},
	{ "deferred-clear-v.sdr",	Default_deferred_clear_vertex_shader},
	{ "deferred-clear-f.sdr",	Default_deferred_clear_fragment_shader}
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

char *Default_mod_table = "\
;; Mod.tbl should be used for settings which affect the entire mod and	\n\
;; only very rarely need to be changed (if ever).						\n\
																		\n\
#CAMPAIGN SETTINGS														\n\
																		\n\
;; Sets default campaign file the game will look for with new pilots	\n\
$Default Campaign File Name: FreeSpace2									\n\
																		\n\
																		\n\
#HUD SETTINGS															\n\
																		\n\
;; Sets the delay before a directive will appear on the screen (ms)		\n\
$Directive Wait Time: 3000												\n\
																		\n\
;; If set, shows the HUD during in-game cutscenes						\n\
$Cutscene camera displays HUD: NO										\n\
																		\n\
;; If set, uses the colors of the ANI instead of the colors of the HUD	\n\
$Full color head animations: NO											\n\
																		\n\
																		\n\
#SEXP SETTINGS															\n\
																		\n\
;; When set, this makes the argument SEXPs loop through all the SEXPs	\n\
;; before it moves on to the next argument. Default behaviour is the 	\n\
;; exact opposite, each SEXP is called for all arguments.				\n\
$Loop SEXPs Then Arguments:	NO											\n\
																		\n\
;; When set, this makes the event chaining behavior act as people		\n\
;; expected it to be in Mantis #82.										\n\
$Use Alternate Chaining Behavior: NO									\n\
																		\n\
																		\n\
#GRAPHICS SETTINGS														\n\
																		\n\
;; When set, this enables the loading of external shader files			\n\
$Enable external shaders: NO											\n\
																		\n\
																		\n\
#OTHER SETTINGS															\n\
																		\n\
;; If not set, a hit to a turret's barrels will not count as a hit to	\n\
;; the turret unless it is also within the turret base's radius.		\n\
$Fixed Turret Collisions: NO											\n\
																		\n""\
;; If not set, hits will damage nearest subsystem first, rather than the\n\
;; impacted physical subsystem first.									\n\
$Damage Impacted Subsystem First: NO									\n\
																		\n\
;; used when no ani is specified or ship_select_3d is active			\n\
$Default ship select effect: FS2										\n\
																		\n\
;; used when no ani is specified or weapon_select_3d is active			\n\
$Default weapon select effect: FS2										\n\
																		\n\
;; Enable weapons to inherit their parent object's collision group setting \n\
$Weapons inherit parent collision group: NO								\n\
																		\n\
#END																	\n\
";


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
$Max Missiles Locked on Player: 2, 3, 4, 7, 99							\n\
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
;; Sets the factor applied to the speed at which the player's shields	\n\
;; recharge																\n\
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
;; Size of the player autoaim cone for each difficulty level  			\n\
;; Only affects the player. If the ship has autoaim, the wider FOV value\n\
;; will be used. Uses convergence.										\n\
$Player Autoaim FOV: 0, 0, 0, 0, 0										\n\
																		\n\
;; The multiplier that affects at what range LOD switching will occur.	\n\
;; NOTE THAT THIS IS NOT BY DIFFICULTY LEVEL (it's by model detail level\n\
;; in the Options menu)                                                 \n\
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
;; specified percentages of energy levels, instead of retail behavior	\n\
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
;; if set, allows turrets target other weapons than bombs assuming		\n\
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
;; If set makes beam turrets use same FOV rules as other weapons do.	\n\
;; Prevents beam from a turret from following the target beyond the		\n\
;; turret's FOV.														\n\
$force beam turrets to use normal fov: NO								\n\
																		\n\
;; Fixes a bug where AI class is not properly set if set in the mission	\n\
;; This should be YES if you want anything in AI.tbl to mean anything	\n\
$fix AI class bug:	NO													\n\
																		\n\
;; TBD																	\n\
$turrets ignore targets radius in range checks: NO						\n\
																		\n\
;; If set, the AI will NOT make extra effort to avoid ramming the player\n\
;; during dogfights. This results in more aggressive AI.				\n\
$no extra collision avoidance vs player:	NO							\n\
																		\n\
;; If set, the game will not check if the dying ship is a player's		\n\
;; wingman, or if the maximum number of screams have been played, or 	\n\
;; even if the dying ship is on the player's team before making it give	\n\
;; a death scream										 				\n\
$perform fewer checks for death screams:	NO							\n\
																		\n\
;; TBD																	\n\
$advanced turret fov edge checks: NO									\n\
																		\n\
;; TBD																	\n\
$require turrets to have target in fov: NO								\n\
																		\n\
;; If set, allows shield management for all ships						\n\
;; (including capships).												\n\
$all ships manage shields:				NO								\n\
																		\n\
;; If set, ai aims using ship center instead of first gunpoint			\n\
$ai aims from ship center:			NO									\n\
																		\n\
;; If set, allows AI fighters to link their weapons at the beginning of	\n\
;; a mission instead of imposing a delay of 30s to 120s					\n\
$allow primary link at mission start:	NO								\n\
																		\n\
;; If set, prevents beams from instantly killing all weapons from first	\n\
;; hit, instead allows weapon hitpoints to be used instead				\n\
$allow beams to damage bombs:			NO								\n\
																		\n\
;; TBD																	\n\
$disable weapon damage scaling for player:	NO							\n""\
																		\n\
;; TBD																	\n\
$countermeasures affect aspect seekers:	NO								\n\
																		\n\
;; TBD																	\n\
$ai path mode:	normal													\n\
																		\n\
;; TBD																	\n\
$no warp camera:	NO													\n\
																		\n\
;; If set, this flag overrides the retail behavior whereby a ship		\n\
;; assigned to guard a ship in a wing will instead guard the entire wing\n\
$ai guards specific ship in wing:	NO									\n\
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

char *Default_controlconfig_table = "\
                                    #ControlConfigOverride \n\
                                    \
                                    #End\n\
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

char* Default_main_vertex_shader =
"#ifdef APPLE\n"
" #extension GL_ARB_draw_instanced : enable\n"
"#else\n"
" #ifdef __GLSL_CG_DATA_TYPES\n"
"  #extension GL_EXT_draw_instanced : enable\n"
" #else\n"
"  #extension GL_ARB_draw_instanced : enable\n"
" #endif\n"
"#endif\n"
"#ifdef FLAG_ENV_MAP\n"
"uniform mat4 envMatrix;\n"
"varying vec3 envReflect;\n"
"#endif\n"
"#ifdef FLAG_NORMAL_MAP\n"
"varying mat3 tbnMatrix;\n"
"#endif\n"
"#ifdef FLAG_FOG\n"
"varying float fogDist;\n"
"#endif\n"
"#ifdef FLAG_TRANSFORM\n"
"#extension GL_ARB_texture_buffer_object : enable\n"
"attribute float model_id;\n"
"uniform samplerBuffer transform_tex;\n"
"uniform int buffer_matrix_offset;\n"
 "#ifdef FLAG_SHADOW_MAP\n"
"varying float not_visible_g;\n"
 "#else\n"
"varying float not_visible;\n"
 "#endif\n"
"#endif\n"
"#ifdef FLAG_SHADOW_MAP\n"
"varying float Instance;\n"
"uniform mat4 shadow_proj_matrix[4];\n"
"#endif\n"
"#ifdef FLAG_SHADOWS\n"
"uniform mat4 shadow_mv_matrix;\n"
"uniform mat4 shadow_proj_matrix[4];\n"
"uniform mat4 model_matrix;\n"
"varying vec4 shadow_vec[4];\n"
"varying vec4 pos_shadow;\n"
"#endif\n"
"#ifdef FLAG_THRUSTER\n"
"uniform float thruster_scale;\n"
"#endif\n"
"#ifdef FLAG_CLIP\n"
"uniform int use_clip_plane;\n"
"uniform vec3 clip_normal;\n"
"uniform vec3 clip_position;\n"
"//uniform vec4 clip_plane;\n"
"uniform mat4 world_matrix;\n"
"#ifdef FLAG_SHADOW_MAP\n"
"varying float clip_distance_g;\n"
"#else\n"
"varying float clip_distance;\n"
"#endif\n"
"#endif\n"
"varying vec4 position;\n"
"varying vec3 lNormal;\n"
"#ifdef FLAG_TRANSFORM\n"
"#define TEXELS_PER_MATRIX 4\n"
"void getModelTransform(inout mat4 transform, inout float invisible, int id, int matrix_offset)\n"
"{\n"
"	transform[0] = texelFetch(transform_tex, (matrix_offset + id) * TEXELS_PER_MATRIX);\n"
"	transform[1] = texelFetch(transform_tex, (matrix_offset + id) * TEXELS_PER_MATRIX + 1);\n"
"	transform[2] = texelFetch(transform_tex, (matrix_offset + id) * TEXELS_PER_MATRIX + 2);\n"
"	transform[3] = texelFetch(transform_tex, (matrix_offset + id) * TEXELS_PER_MATRIX + 3);\n"
"	invisible = transform[3].w;\n"
"	transform[3].w = 1.0;\n"
"}\n"
"#endif\n"
"#ifdef FLAG_SHADOWS\n"
"vec4 transformToShadowMap(int i, vec4 pos)\n"
"{\n"
"	vec4 shadow_proj;\n"
"	shadow_proj = shadow_proj_matrix[i] * pos;\n"
"	shadow_proj += 1.0;\n"
"	shadow_proj *= 0.5;\n"
"	shadow_proj.w = shadow_proj.z;\n"
"	shadow_proj.z = float(i);\n"
"	return shadow_proj;\n"
"}\n"
"#endif\n"
"void main()\n"
"{\n"
"	mat4 orient = mat4(1.0);\n"
"	mat4 scale = mat4(1.0);\n"
" #ifdef FLAG_TRANSFORM\n"
"	float invisible;"
"	getModelTransform(orient, invisible, int(model_id), buffer_matrix_offset);\n"
"  #ifdef FLAG_SHADOW_MAP\n"
"	not_visible_g = invisible;\n"
"  #else\n"
"	not_visible = invisible;\n"
"  #endif\n"
" #endif\n"
"	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"	vec4 vertex = gl_Vertex;\n"
" #ifdef FLAG_THRUSTER\n"
"	if(vertex.z < -1.5) {\n"
"		vertex.z *= thruster_scale;\n"
"	}\n"
" #endif\n"
"	position = gl_ModelViewMatrix * orient * vertex;\n"
" #ifdef FLAG_SHADOW_MAP\n"
"	gl_Position = position;\n"
"  #ifdef APPLE\n"
"	 Instance = float(gl_InstanceIDARB);\n"
"  #else\n"
"	 Instance = float(gl_InstanceID);\n"
"  #endif\n"
" #else\n"
"	gl_Position = gl_ProjectionMatrix * position;\n"
" #endif\n"
"	gl_FrontColor = gl_Color;\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
" // Transform the normal into eye space and normalize the result.\n"
"	vec3 normal = normalize(gl_NormalMatrix * mat3(orient) * gl_Normal);\n"
"	lNormal = normal;\n"
" #ifdef FLAG_SHADOWS\n"
"		pos_shadow = shadow_mv_matrix * model_matrix * orient * gl_Vertex;\n"
"		shadow_vec[0] = transformToShadowMap(0, pos_shadow);\n"
"		shadow_vec[1] = transformToShadowMap(1, pos_shadow);\n"
"		shadow_vec[2] = transformToShadowMap(2, pos_shadow);\n"
"		shadow_vec[3] = transformToShadowMap(3, pos_shadow);\n"
" #endif\n"
" #ifdef FLAG_NORMAL_MAP\n"
" // Setup stuff for normal maps\n"
"	vec3 t = normalize(gl_NormalMatrix * mat3(orient) * gl_MultiTexCoord1.xyz);\n"
"	vec3 b = cross(normal, t) * gl_MultiTexCoord1.w;\n"
"	tbnMatrix = mat3(t, b, normal);\n"
" #endif\n"
" #ifdef FLAG_ENV_MAP\n"
" // Environment mapping reflection vector.\n"
"	envReflect = reflect(position.xyz, normal);\n"
"	envReflect = vec3(envMatrix * vec4(envReflect, 0.0));\n"
"	envReflect = normalize(envReflect);\n"
" #endif\n"
" #ifdef FLAG_FOG\n"
"	fogDist = clamp((gl_Position.z - gl_Fog.start) * 0.75 * gl_Fog.scale, 0.0, 1.0);\n"
" #endif\n"
" #ifdef FLAG_CLIP\n"
"   float clip_dist = 0.0;\n"
"	if(use_clip_plane == 1) clip_dist = dot(normalize((world_matrix*orient*vertex).xyz - clip_position), clip_normal);\n"
"  #ifdef FLAG_SHADOW_MAP\n"
"   clip_distance_g = clip_dist;\n"
"  #else\n"
"   clip_distance = clip_dist;\n"
"  #endif\n"
" #endif\n"
"	gl_ClipVertex = (gl_ModelViewMatrix * orient * vertex);\n"
"}";

char *Default_main_fragment_shader = 
"#extension GL_EXT_texture_array : enable\n"
"#ifdef FLAG_LIGHT\n"
"uniform int n_lights;\n"
"uniform float light_factor;\n"
"#endif\n"
"#ifdef FLAG_DIFFUSE_MAP\n"
"uniform sampler2D sBasemap;\n"
"uniform int desaturate;\n"
"uniform vec3 desaturate_clr;\n"
"uniform int blend_alpha;\n"
"#endif\n"
"#ifdef FLAG_GLOW_MAP\n"
"uniform sampler2D sGlowmap;\n"
"#endif\n"
"#ifdef FLAG_SPEC_MAP\n"
"uniform sampler2D sSpecmap;\n"
"#endif\n"
"#ifdef FLAG_ENV_MAP\n"
"uniform samplerCube sEnvmap;\n"
"uniform bool alpha_spec;\n"
"varying vec3 envReflect;\n"
"#endif\n"
"#ifdef FLAG_NORMAL_MAP\n"
"uniform sampler2D sNormalmap;\n"
"varying mat3 tbnMatrix;\n"
"#endif\n"
"#ifdef FLAG_FOG\n"
"varying float fogDist;\n"
"#endif\n"
"#ifdef FLAG_ANIMATED\n"
"uniform sampler2D sFramebuffer;\n"
"uniform int effect_num;\n"
"uniform float anim_timer;\n"
"uniform float vpwidth;\n"
"uniform float vpheight;\n"
"#endif\n"
"#ifdef FLAG_TRANSFORM\n"
"varying float not_visible;\n"
"#endif\n"
"#ifdef FLAG_TEAMCOLOR\n"
"uniform vec3 base_color;\n"
"uniform vec3 stripe_color;\n"
"vec2 teamMask = vec2(0.0, 0.0);\n"
"#endif\n"
"#ifdef FLAG_MISC_MAP\n"
"uniform sampler2D sMiscmap;\n"
"#endif\n"
"#ifdef FLAG_SHADOWS\n"
"varying vec4 shadow_vec[4];\n"
"varying vec4 pos_shadow;\n"
"//uniform sampler2DArrayShadow shadow_map;\n"
"uniform sampler2DArray shadow_map;\n"
"//uniform sampler2D shadow_map;\n"
"uniform float znear;\n"
"uniform float zfar;\n"
"uniform float veryneardist;\n"
"uniform float neardist;\n"
"uniform float middist;\n"
"uniform float fardist;\n"
"#endif\n"
"#ifdef FLAG_CLIP\n"
"uniform int use_clip_plane;\n"
"varying float clip_distance;\n"
"#endif\n"
"varying vec4 position;\n"
"varying vec3 lNormal;\n"
"\n"
"#if SHADER_MODEL == 2\n"
"  #define MAX_LIGHTS 2\n"
"#else\n"
"  #define MAX_LIGHTS 8\n"
"#endif\n"
"#define SPEC_INTENSITY_POINT			5.3 // Point light\n"
"#define SPEC_INTENSITY_DIRECTIONAL		3.0 // Directional light\n"
"#define SPECULAR_FACTOR				1.75\n"
"#define SPECULAR_ALPHA					0.1\n"
"#define SPEC_FACTOR_NO_SPEC_MAP		0.6\n"
"#define ENV_ALPHA_FACTOR				0.3\n"
"#define GLOW_MAP_INTENSITY				1.5\n"
"#define AMBIENT_LIGHT_BOOST			1.0\n"
"#define VARIANCE_SHADOW_SCALE			1000000.0\n"
"#define VARIANCE_SHADOW_SCALE_INV		1.0/VARIANCE_SHADOW_SCALE\n"
"\n"
"#ifdef FLAG_SHADOWS\n"
"vec2 sampleShadowMap(vec2 uv, vec2 offset_uv, int cascade, float shadowMapSizeInv)\n"
"{\n"
"	return texture2DArray(shadow_map, vec3(uv + offset_uv * shadowMapSizeInv, float(cascade))).xy;\n"
"}\n"
"\n"
"float computeShadowFactor(vec2 moments, float bias)\n"
"{\n"
"	float shadow = 1.0;\n"
"	if((moments.x - bias) > pos_shadow.z)\n"
"	{\n"
"		// variance shadow mapping using Chebychev's Formula\n"
"		float variance = moments.y * VARIANCE_SHADOW_SCALE - moments.x * moments.x;\n"
"		float mD = moments.x - bias - pos_shadow.z;\n"
"		shadow = variance / (variance + mD * mD);\n"
"		shadow = clamp(shadow, 0.0, 1.0);\n"
"	}\n"
"	return shadow;\n"
"}\n"
"\n"
"float sampleNoPCF(int cascade)\n"
"{\n"
"	return computeShadowFactor(sampleShadowMap(shadow_vec[cascade].xy, vec2(0.0, 0.0), cascade, 1.0/1024.0), 0.05);\n"
"}\n"
"\n"
"float samplePoissonPCF(int cascade)\n"
"{\n"
"	if(cascade > 3 || cascade < 0) return 1.0;\n"
"	vec2 poissonDisc[16];\n"
"	poissonDisc[0] = vec2(-0.76275, -0.3432573);\n"
"	poissonDisc[1] = vec2(-0.5226235, -0.8277544);\n"
"	poissonDisc[2] = vec2(-0.3780261, 0.01528688);\n"
"	poissonDisc[3] = vec2(-0.7742821, 0.4245702);\n"
"	poissonDisc[4] = vec2(0.04196143, -0.02622231);\n"
"	poissonDisc[5] = vec2(-0.2974772, -0.4722782);\n"
"	poissonDisc[6] = vec2(-0.516093, 0.71495);\n"
"	poissonDisc[7] = vec2(-0.3257416, 0.3910343);\n"
"	poissonDisc[8] = vec2(0.2705966, 0.6670476);\n"
"	poissonDisc[9] = vec2(0.4918377, 0.1853267);\n"
"	poissonDisc[10] = vec2(0.4428544, -0.6251478);\n"
"	poissonDisc[11] = vec2(-0.09204347, 0.9267113);\n"
"	poissonDisc[12] = vec2(0.391505, -0.2558275);\n"
"	poissonDisc[13] = vec2(0.05605913, -0.7570801);\n"
"	poissonDisc[14] = vec2(0.81772, -0.02475523);\n"
"	poissonDisc[15] = vec2(0.6890262, 0.5191521);\n"
"	float maxUVOffset[4];\n"
"	maxUVOffset[0] = 1.0/300.0;\n"
"	maxUVOffset[1] = 1.0/250.0;\n"
"	maxUVOffset[2] = 1.0/200.0;\n"
"	maxUVOffset[3] = 1.0/200.0;\n"
"	vec2 sum = sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[0], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[1], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[2], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[3], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[4], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[5], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[6], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[7], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[8], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[9], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[10], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[11], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[12], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[13], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[14], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	sum += sampleShadowMap(shadow_vec[cascade].xy, poissonDisc[15], cascade, maxUVOffset[cascade])*(1.0/16.0);\n"
"	return computeShadowFactor(sum, 0.1);\n"
"}\n"
"float getShadowValue()\n"
"{\n"
"	// Valathil's Shadows\n"
"	float depth = -position.z;\n"
"	int cascade = 4;\n"
"	cascade -= int(step(depth, fardist));\n"
"	cascade -= int(step(depth, middist));\n"
"	cascade -= int(step(depth, neardist));\n"
"	cascade -= int(step(depth, veryneardist));\n"
"	float cascade_start_dist[5];\n"
"	cascade_start_dist[0] = 0.0;\n"
"	cascade_start_dist[1] = veryneardist;\n"
"	cascade_start_dist[2] = neardist;\n"
"	cascade_start_dist[3] = middist;\n"
"	cascade_start_dist[4] = fardist;\n"
"	if(cascade > 3 || cascade < 0) return 1.0;\n"
"	float dist_threshold = (cascade_start_dist[cascade+1] - cascade_start_dist[cascade])*0.2;\n"
"	if(cascade_start_dist[cascade+1] - dist_threshold > depth)\n"
"		return samplePoissonPCF(cascade);\n"
"	return mix(samplePoissonPCF(cascade), samplePoissonPCF(cascade+1), smoothstep(cascade_start_dist[cascade+1] - dist_threshold, cascade_start_dist[cascade+1], depth));\n"
"}\n"
"#endif\n"
"void main()\n"
"{\n"
"#ifdef FLAG_TRANSFORM\n"
"	if(not_visible >= 0.9) { discard; }\n"
"#endif\n"
"#ifdef FLAG_SHADOW_MAP\n"
" #ifdef FLAG_CLIP\n"
"	if(use_clip_plane == 1) { if(clip_distance <= 0.0) { discard; } }\n"
" #endif\n"
"	// need depth and depth squared for variance shadow maps\n"
"	gl_FragData[0] = vec4(position.z, position.z * position.z * VARIANCE_SHADOW_SCALE_INV, 0.0, 1.0);\n"
"	return;\n"
"#endif\n" 
"	vec3 eyeDir = vec3(normalize(-position).xyz);\n"
"	vec3 lightAmbientDiffuse = vec3(0.0, 0.0, 0.0);\n"
"   vec3 lightDiffuse = vec3(0.0, 0.0, 0.0);\n"
"   vec3 lightAmbient = vec3(0.0, 0.0, 0.0);\n"
"   vec4 lightSpecular = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	vec2 texCoord = gl_TexCoord[0].xy;\n"
"	vec4 baseColor = gl_Color;\n"
"	vec4 posData = vec4(position.xyz,1.0);\n"
"	vec4 normData = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	vec4 specData = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	vec3 unitNormal = normalize(lNormal);\n"
"	vec3 normal = unitNormal;\n"
"#ifdef FLAG_NORMAL_MAP\n"
"   // Normal map - convert from DXT5nm\n"
"   vec2 normalSample;\n"
"   normal.rg = normalSample = (texture2D(sNormalmap, texCoord).ag * 2.0) - 1.0;\n"
"   normal.b = sqrt(1.0 - dot(normal.rg, normal.rg));\n"
"   normal = tbnMatrix * normal;\n"
"   float norm = length(normal);\n"
"   // prevent breaking of normal maps\n"
"   if (norm > 0.0)\n"
"      normal /= norm;\n"
"   else\n"
"      normal = unitNormal;\n"
"#endif\n"
"	normData = vec4(normal,1.0);\n"
"\n"
"#ifdef FLAG_ANIMATED\n"
"   vec2 distort = vec2(cos(position.x*position.w*0.005+anim_timer*20.0)*sin(position.y*position.w*0.005),sin(position.x*position.w*0.005+anim_timer*20.0)*cos(position.y*position.w*0.005))*0.03;\n"
"#endif\n"
"\n"
"#ifdef FLAG_DIFFUSE_MAP\n"
"	vec2 diffuseTexCoord = texCoord;\n"
" #ifdef FLAG_ANIMATED\n"
"   if (effect_num == 2) {\n"
"      diffuseTexCoord = texCoord + distort*(1.0-anim_timer);\n"
"   }\n"
" #endif\n"
"	baseColor = texture2D(sBasemap, diffuseTexCoord);\n"
"	if ( blend_alpha == 0 && baseColor.a < 0.95 ) discard; // if alpha blending is not on, discard transparent pixels\n"
"	// premultiply alpha if blend_alpha is 1. assume that our blend function is srcColor + (1-Alpha)*destColor.\n"
"	// if blend_alpha is 2, assume blend func is additive and don't modify color\n"
"	if(blend_alpha == 1) baseColor.rgb = baseColor.rgb * baseColor.a;\n"
"#endif\n"
"\n"
"   specData = vec4(baseColor.rgb * SPEC_FACTOR_NO_SPEC_MAP, 1.0);\n"
"#ifdef FLAG_SPEC_MAP\n"
"   vec4 specColour = texture2D(sSpecmap, texCoord);\n"
"   specData = vec4(specColour.rgb * SPECULAR_FACTOR, 1.0);\n"
"#endif\n"
"\n"
"#ifdef FLAG_MISC_MAP\n"
" #ifdef FLAG_TEAMCOLOR\n"
"	vec2 teamMask = vec2(0.0, 0.0);\n"
"	teamMask = texture2D(sMiscmap, texCoord).rg;\n"
"	vec3 base = base_color - vec3(0.5);\n"
"	vec3 stripe = stripe_color - vec3(0.5);\n"
"	baseColor.rgb += (base * teamMask.x) + (stripe * teamMask.y);\n"
" #endif\n"
"#endif\n"
"\n"
"	float shadow = 1.0;\n"
"#ifdef FLAG_SHADOWS\n"
"	shadow = getShadowValue();\n"
"#endif\n"
"\n"
"#ifdef FLAG_LIGHT\n"
"	vec3 lightDir;\n"
"   lightAmbient = vec3(gl_FrontMaterial.emission + (gl_LightModel.ambient * gl_FrontMaterial.ambient));\n"
"   float dist;\n"
"   #pragma optionNV unroll all\n"
"   for (int i = 0; i < MAX_LIGHTS; ++i) {\n"
" #if SHADER_MODEL > 2\n"
"      if (i > n_lights)\n"
"         break;\n"
" #endif\n"
"      if(i > 0)\n"
"         shadow = 1.0;\n"
"      float specularIntensity = SPEC_INTENSITY_DIRECTIONAL;\n"
"      lightDir = normalize(gl_LightSource[i].position.xyz);\n"
"      float attenuation = 1.0;\n"
" #ifndef FLAG_DEFERRED\n"
"  #if SHADER_MODEL > 2\n"
"      if (gl_LightSource[i].position.w == 1.0) {\n"
"  #else\n"
"      if (gl_LightSource[i].position.w == 1.0 && i != 0) {\n"
"  #endif\n"
"          // Positional light source\n"
"          dist = distance(gl_LightSource[i].position.xyz, position.xyz);\n"
"          lightDir = (gl_LightSource[i].position.xyz - position.xyz);\n"
"  #if SHADER_MODEL > 2\n"
"          if (gl_LightSource[i].spotCutoff < 91.0) {  // Tube light\n"
"              float beamlength = length(gl_LightSource[i].spotDirection);\n"
"              vec3 beamDir = normalize(gl_LightSource[i].spotDirection);\n"
"              // Get nearest point on line\n"
"              float neardist = dot(position.xyz - gl_LightSource[i].position.xyz , beamDir);\n"
"              // Move back from the endpoint of the beam along the beam by the distance we calculated\n"
"              vec3 nearest = gl_LightSource[i].position.xyz - beamDir * abs(neardist);\n"
"              lightDir = nearest - position.xyz;\n"
"              dist = length(lightDir);\n"
"          }\n"
"  #endif\n"
"          lightDir = normalize(lightDir);\n"
"          attenuation = 1.0 / (gl_LightSource[i].constantAttenuation + (gl_LightSource[i].linearAttenuation * dist) + (gl_LightSource[i].quadraticAttenuation * dist * dist));\n"
"          specularIntensity = SPEC_INTENSITY_POINT;\n"
"      }\n"
" #endif\n"
"      // Attenuation and light direction\n"
"      vec3 half_vec = normalize(lightDir + eyeDir);\n"
"      // Ambient and Diffuse\n"
"      lightAmbient += (gl_FrontLightProduct[i].ambient.rgb * attenuation) * shadow;\n"
"      lightDiffuse += (gl_FrontLightProduct[i].diffuse.rgb * light_factor * (max(dot(normal, lightDir), 0.0)) * attenuation) * shadow;\n"
"      // Specular\n"
"      float NdotHV = clamp(dot(normal, half_vec), 0.0, 1.0);\n"
"      lightSpecular += ((gl_FrontLightProduct[i].specular * pow(NdotHV, gl_FrontMaterial.shininess)) * attenuation) * specularIntensity * shadow;\n"
"   }\n"
"	lightAmbientDiffuse = lightAmbient + lightDiffuse;\n"
"#else\n"
"   lightAmbientDiffuse = gl_Color.rgb;\n"
"   lightSpecular = gl_SecondaryColor;\n"
"#endif\n"
"\n"
"	baseColor.rgb *= max(lightAmbientDiffuse.rgb * AMBIENT_LIGHT_BOOST, gl_LightModel.ambient.rgb - 0.425);\n"
"	baseColor.rgb += lightSpecular.rgb * specData.rgb;\n"
"#ifdef FLAG_ENV_MAP\n"
"   vec3 envReflectNM = envReflect;\n"
" #ifdef FLAG_NORMAL_MAP\n"
"   envReflectNM += vec3(normalSample, 0.0);\n"
" #endif\n"
"   vec4 envColour = textureCube(sEnvmap, envReflectNM);\n"
"   vec3 envIntensity = (alpha_spec) ? vec3(specColour.a) : specColour.rgb;\n"
"   baseColor.rgb += envColour.rgb * envIntensity;\n"
"#endif\n"
"\n"
"#ifdef FLAG_GLOW_MAP\n"
"   baseColor.rgb += texture2D(sGlowmap, texCoord).rgb * GLOW_MAP_INTENSITY;\n"
"#endif\n"
"\n"
"#ifdef FLAG_FOG\n"
"	vec3 fogColor = gl_Fog.color.rgb;\n"
" #ifdef FLAG_DIFFUSE_MAP\n"
"	if(blend_alpha == 1) fogColor *= baseColor.a;\n"
" #endif\n"
"   baseColor.rgb = mix(baseColor.rgb, fogColor, fogDist);\n"
"	specData.rgb *= fogDist;\n"
"#endif\n"
"\n"
"#ifdef FLAG_DIFFUSE_MAP\n"
"	if(desaturate == 1) {\n"
"		baseColor.rgb = desaturate_clr * dot(vec3(1.0), baseColor.rgb) * 0.3333333;\n"
"	}\n"
"#endif\n"
"#ifdef FLAG_ANIMATED\n"
"   if (effect_num == 0) {\n"
"      float shinefactor = 1.0/(1.0 + pow(abs((fract(abs(texCoord.x))-anim_timer) * 1000.0), 2.0)) * 1000.0;\n"
"      baseColor.rgb = baseColor.rgb + vec3(shinefactor);\n"
"      baseColor.a = baseColor.a * clamp(shinefactor * (fract(abs(texCoord.x))-anim_timer) * -10000.0,0.0,1.0);\n"
"   }\n"
"   if (effect_num == 1) {\n"
"      float shinefactor = 1.0/(1.0 + pow(abs(position.y-anim_timer), 2.0));\n"
"      baseColor.rgb = baseColor.rgb + vec3(shinefactor);\n"
" #ifdef FLAG_LIGHT\n"
"      baseColor.a = baseColor.a;\n"
" #else\n"
"      // ATI Wireframe fix *grumble*\n"
"      baseColor.a = clamp((position.y-anim_timer) * 10000.0,0.0,1.0);\n"
" #endif\n"
"   }\n"
"   if (effect_num == 2) {\n"
"      vec2 screenPos = gl_FragCoord.xy * vec2(vpwidth,vpheight);\n"
"      baseColor.a = baseColor.a;\n"
"      float cloak_interp = (sin(position.x*position.w*0.005+anim_timer*20.0)*sin(position.y*position.w*0.005)*0.5)-0.5;\n"
" #ifdef FLAG_LIGHT\n"
"      baseColor.rgb = mix(texture2D(sFramebuffer, screenPos + distort*anim_timer + anim_timer*0.1*normal.xy).rgb,baseColor.rgb,clamp(cloak_interp+anim_timer*2.0,0.0,1.0));\n"
" #else\n"
"      baseColor.rgb = mix(texture2D(sFramebuffer, screenPos + distort*anim_timer + anim_timer*0.1*lNormal.xy).rgb,baseColor.rgb,clamp(cloak_interp+anim_timer*2.0,0.0,1.0));\n"
" #endif\n"
"   }\n"
"#endif\n"
"#ifdef FLAG_CLIP\n"
"	// for some odd reason if we try to discard the pixel early for plane clipping, it screws up glow maps so let's just do it down here.\n"
"	if(use_clip_plane == 1) { if(clip_distance <= 0.0) { discard; } }\n"
"#endif\n"
"   gl_FragData[0] = baseColor;\n"
"#ifdef FLAG_DEFERRED\n"
"	gl_FragData[1] = posData;\n"
"	gl_FragData[2] = normData;\n"
"	gl_FragData[3] = specData;\n"
"#endif\n"
"}";

char* Default_main_geometry_shader =
"#extension GL_EXT_geometry_shader4 : enable\n"
"uniform mat4 shadow_proj_matrix[4];\n"
"varying in float Instance[3];\n"
"varying out vec4 position;\n"
"#ifdef FLAG_TRANSFORM\n"
"varying in float not_visible_g[];\n"
"varying out float not_visible;\n"
"#endif\n"
"#ifdef FLAG_CLIP\n"
"varying in float clip_distance_g[];\n"
"varying out float clip_distance;\n"
"#endif\n"
"void main(void)\n"
"{\n"
"	int instanceID = int(Instance[0]);\n"
"   for(int vert = 0; vert < gl_VerticesIn; vert++)\n"
"	{\n"
"		gl_Position = shadow_proj_matrix[instanceID] * gl_PositionIn[vert];\n"
"		if(gl_Position.z < -1.0)\n"
"			gl_Position.z = -1.0;\n"
"       position = gl_PositionIn[vert];\n"
"		gl_ClipVertex = gl_ClipVertexIn[vert];\n"
"		gl_Layer = instanceID;\n"
"#ifdef FLAG_TRANSFORM\n"
"		not_visible = not_visible_g[0];\n"
"#endif\n"
"#ifdef FLAG_CLIP\n"
"		clip_distance = clip_distance_g[0];\n"
"#endif\n"
"		EmitVertex();\n"
"	}\n"
"	EndPrimitive();\n"
"}";

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
"}";

char* Default_fxaa_fragment_shader = 
"#extension GL_EXT_gpu_shader4 : enable\n"
"#define FXAA_EARLY_EXIT 1\n"
"#define FXAA_DISCARD 1\n"
"#if SHADER_MODEL == 2\n"
"	#define FXAA_GLSL_120 1\n"
"	#define FXAA_GLSL_130 0\n"
"#endif\n"
"#if SHADER_MODEL > 2\n"
"	#define FXAA_GLSL_120 0\n"
"	#define FXAA_GLSL_130 1\n"
"#endif\n"
"#ifndef FXAA_FAST_PIXEL_OFFSET\n"
"	#ifdef GL_EXT_gpu_shader4\n"
"		#define FXAA_FAST_PIXEL_OFFSET 1\n"
"	#endif\n"
"	#ifdef GL_NV_gpu_shader5\n"
"		#extension GL_NV_gpu_shader5 : enable\n"
"		#define FXAA_FAST_PIXEL_OFFSET 1\n"
"	#endif\n"
"	#ifdef GL_ARB_gpu_shader5\n"
"		#extension GL_ARB_gpu_shader5 : enable\n"
"		#define FXAA_FAST_PIXEL_OFFSET 1\n"
"	#endif\n"
"	#ifndef FXAA_FAST_PIXEL_OFFSET\n"
"		#define FXAA_FAST_PIXEL_OFFSET 0\n"
"	#endif\n"
"#endif\n"
"#ifndef FXAA_GATHER4_ALPHA\n"
"	#ifdef GL_ARB_gpu_shader5\n"
"		#extension GL_ARB_gpu_shader5 : enable\n"
"		#define FXAA_GATHER4_ALPHA 1\n"
"	#endif\n"
"	#ifdef GL_NV_gpu_shader5\n"
"		#extension GL_NV_gpu_shader5 : enable\n"
"		#define FXAA_GATHER4_ALPHA 1\n"
"	#endif\n"
"	#ifndef FXAA_GATHER4_ALPHA\n"
"		#define FXAA_GATHER4_ALPHA 0\n"
"	#endif\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 10)\n"
"	#define FXAA_QUALITY_PS 3\n"
"	#define FXAA_QUALITY_P0 1.5\n"
"	#define FXAA_QUALITY_P1 3.0\n"
"	#define FXAA_QUALITY_P2 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 11)\n"
"	#define FXAA_QUALITY_PS 4\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 3.0\n"
"	#define FXAA_QUALITY_P3 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 12)\n"
"	#define FXAA_QUALITY_PS 5\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 4.0\n"
"	#define FXAA_QUALITY_P4 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 13)\n"
"	#define FXAA_QUALITY_PS 6\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 2.0\n"
"	#define FXAA_QUALITY_P4 4.0\n"
"	#define FXAA_QUALITY_P5 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 14)\n"
"	#define FXAA_QUALITY_PS 7\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 2.0\n"
"	#define FXAA_QUALITY_P4 2.0\n"
"	#define FXAA_QUALITY_P5 4.0\n"
"	#define FXAA_QUALITY_P6 12.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 25)\n"
"	#define FXAA_QUALITY_PS 8\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 2.0\n"
"	#define FXAA_QUALITY_P4 2.0\n"
"	#define FXAA_QUALITY_P5 2.0\n"
"	#define FXAA_QUALITY_P6 4.0\n"
"	#define FXAA_QUALITY_P7 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 26)\n"
"	#define FXAA_QUALITY_PS 9\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 2.0\n"
"	#define FXAA_QUALITY_P4 2.0\n"
"	#define FXAA_QUALITY_P5 2.0\n"
"	#define FXAA_QUALITY_P6 2.0\n"
"	#define FXAA_QUALITY_P7 4.0\n"
"	#define FXAA_QUALITY_P8 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 27)\n"
"	#define FXAA_QUALITY_PS 10\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 2.0\n"
"	#define FXAA_QUALITY_P4 2.0\n"
"	#define FXAA_QUALITY_P5 2.0\n"
"	#define FXAA_QUALITY_P6 2.0\n"
"	#define FXAA_QUALITY_P7 2.0\n"
"	#define FXAA_QUALITY_P8 4.0\n"
"	#define FXAA_QUALITY_P9 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 28)\n"
"	#define FXAA_QUALITY_PS 11\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.5\n"
"	#define FXAA_QUALITY_P2 2.0\n"
"	#define FXAA_QUALITY_P3 2.0\n"
"	#define FXAA_QUALITY_P4 2.0\n"
"	#define FXAA_QUALITY_P5 2.0\n"
"	#define FXAA_QUALITY_P6 2.0\n"
"	#define FXAA_QUALITY_P7 2.0\n"
"	#define FXAA_QUALITY_P8 2.0\n"
"	#define FXAA_QUALITY_P9 4.0\n"
"	#define FXAA_QUALITY_P10 8.0\n"
"#endif\n"
"#if (FXAA_QUALITY_PRESET == 39)\n"
"	#define FXAA_QUALITY_PS 12\n"
"	#define FXAA_QUALITY_P0 1.0\n"
"	#define FXAA_QUALITY_P1 1.0\n"
"	#define FXAA_QUALITY_P2 1.0\n"
"	#define FXAA_QUALITY_P3 1.0\n"
"	#define FXAA_QUALITY_P4 1.0\n"
"	#define FXAA_QUALITY_P5 1.5\n"
"	#define FXAA_QUALITY_P6 2.0\n"
"	#define FXAA_QUALITY_P7 2.0\n"
"	#define FXAA_QUALITY_P8 2.0\n"
"	#define FXAA_QUALITY_P9 2.0\n"
"	#define FXAA_QUALITY_P10 4.0\n"
"	#define FXAA_QUALITY_P11 8.0\n"
"#endif\n"
"#if (FXAA_GLSL_120 == 1) || (FXAA_GLSL_130 == 1)\n"
"	#define FxaaBool bool\n"
"	#define FxaaDiscard discard\n"
"	#define FxaaFloat float\n"
"	#define FxaaFloat2 vec2\n"
"	#define FxaaFloat3 vec3\n"
"	#define FxaaFloat4 vec4\n"
"	#define FxaaHalf float\n"
"	#define FxaaHalf2 vec2\n"
"	#define FxaaHalf3 vec3\n"
"	#define FxaaHalf4 vec4\n"
"	#define FxaaInt2 ivec2\n"
"	#define FxaaSat(x) clamp(x, 0.0, 1.0)\n"
"	#define FxaaTex sampler2D\n"
"#else\n"
"	#define FxaaBool bool\n"
"	#define FxaaDiscard clip(-1)\n"
"	#define FxaaFloat float\n"
"	#define FxaaFloat2 float2\n"
"	#define FxaaFloat3 float3\n"
"	#define FxaaFloat4 float4\n"
"	#define FxaaHalf half\n"
"	#define FxaaHalf2 half2\n"
"	#define FxaaHalf3 half3\n"
"	#define FxaaHalf4 half4\n"
"	#define FxaaSat(x) saturate(x)\n"
"#endif\n"
"#if (FXAA_GLSL_120 == 1)\n"
"	#define FxaaTexTop(t, p) texture2DLod(t, p, 0.0)\n"
"	#if (FXAA_FAST_PIXEL_OFFSET == 1)\n"
"		#define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)\n"
"	#else\n"
"		#define FxaaTexOff(t, p, o, r) texture2DLod(t, p + (o * r), 0.0)\n"
"	#endif\n"
"	#if (FXAA_GATHER4_ALPHA == 1)\n"
"		#define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"
"		#define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"
"		#define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"
"		#define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"
"	#endif\n"
"#endif\n"
"#if (FXAA_GLSL_130 == 1)\n"
"	#define FxaaTexTop(t, p) texture2DLod(t, p, 0.0)\n"
"	#define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)\n"
"	#if (FXAA_GATHER4_ALPHA == 1)\n"
"		#define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"
"		#define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"
"		#define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"
"		#define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"
"	#endif\n"
"#endif\n"
"FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }\n"
"FxaaFloat4 FxaaPixelShader(\n"
"	FxaaFloat2 pos,\n"
"	FxaaTex tex,\n"
"	FxaaFloat2 fxaaQualityRcpFrame,\n"
"	FxaaFloat fxaaQualitySubpix,\n"
"	FxaaFloat fxaaQualityEdgeThreshold,\n"
"	FxaaFloat fxaaQualityEdgeThresholdMin\n"
") {\n"
"	FxaaFloat2 posM;\n"
"	posM.x = pos.x;\n"
"	posM.y = pos.y;\n"
"	#if (FXAA_GATHER4_ALPHA == 1)\n"
"		#if (FXAA_DISCARD == 0)\n"
"			FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);\n"
"			#define lumaM rgbyM.y\n"
"		#endif\n"
"		FxaaFloat4 luma4A = FxaaTexAlpha4(tex, posM);\n"
"		FxaaFloat4 luma4B = FxaaTexOffAlpha4(tex, posM, FxaaInt2(-1, -1));\n"
"		#if (FXAA_DISCARD == 1)\n"
"			#define lumaM luma4A.w\n"
"		#endif\n"
"		#define lumaE luma4A.z\n"
"		#define lumaS luma4A.x\n"
"		#define lumaSE luma4A.y\n"
"		#define lumaNW luma4B.w\n"
"		#define lumaN luma4B.z\n"
"		#define lumaW luma4B.x\n"
"	#else\n"
"		FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);\n"
"		#define lumaM rgbyM.w\n"
"		FxaaFloat lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0), fxaaQualityRcpFrame.xy));\n"
"	#endif\n"
"	FxaaFloat maxSM = max(lumaS, lumaM);\n"
"	FxaaFloat minSM = min(lumaS, lumaM);\n"
"	FxaaFloat maxESM = max(lumaE, maxSM);\n"
"	FxaaFloat minESM = min(lumaE, minSM);\n"
"	FxaaFloat maxWN = max(lumaN, lumaW);\n"
"	FxaaFloat minWN = min(lumaN, lumaW);\n"
"	FxaaFloat rangeMax = max(maxWN, maxESM);\n"
"	FxaaFloat rangeMin = min(minWN, minESM);\n"
"	FxaaFloat rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;\n"
"	FxaaFloat range = rangeMax - rangeMin;\n"
"	FxaaFloat rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);\n"
"	FxaaBool earlyExit = range < rangeMaxClamped;\n"
"	if (earlyExit)\n"
"		#if (FXAA_DISCARD == 1)\n"
"			FxaaDiscard;\n"
"		#else\n"
"			return rgbyM;\n"
"		#endif\n"
"	#if (FXAA_GATHER4_ALPHA == 0)\n"
"		FxaaFloat lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));\n"
"	#else\n"
"		FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(1, -1), fxaaQualityRcpFrame.xy));\n"
"		FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));\n"
"	#endif\n"
"	FxaaFloat lumaNS = lumaN + lumaS;\n"
"	FxaaFloat lumaWE = lumaW + lumaE;\n"
"	FxaaFloat subpixRcpRange = 1.0/range;\n"
"	FxaaFloat subpixNSWE = lumaNS + lumaWE;\n"
"	FxaaFloat edgeHorz1 = (-2.0 * lumaM) + lumaNS;\n"
"	FxaaFloat edgeVert1 = (-2.0 * lumaM) + lumaWE;\n"
"	FxaaFloat lumaNESE = lumaNE + lumaSE;\n"
"	FxaaFloat lumaNWNE = lumaNW + lumaNE;\n"
"	FxaaFloat edgeHorz2 = (-2.0 * lumaE) + lumaNESE;\n"
"	FxaaFloat edgeVert2 = (-2.0 * lumaN) + lumaNWNE;\n"
"	FxaaFloat lumaNWSW = lumaNW + lumaSW;\n"
"	FxaaFloat lumaSWSE = lumaSW + lumaSE;\n"
"	FxaaFloat edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);\n"
"	FxaaFloat edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);\n"
"	FxaaFloat edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;\n"
"	FxaaFloat edgeVert3 = (-2.0 * lumaS) + lumaSWSE;\n"
"	FxaaFloat edgeHorz = abs(edgeHorz3) + edgeHorz4;\n"
"	FxaaFloat edgeVert = abs(edgeVert3) + edgeVert4;\n"
"	FxaaFloat subpixNWSWNESE = lumaNWSW + lumaNESE;\n"
"	FxaaFloat lengthSign = fxaaQualityRcpFrame.x;\n"
"	FxaaBool horzSpan = edgeHorz >= edgeVert;\n"
"	FxaaFloat subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;\n"
"	if (!horzSpan) lumaN = lumaW;\n"
"	if (!horzSpan) lumaS = lumaE;\n"
"	if (horzSpan) lengthSign = fxaaQualityRcpFrame.y;\n"
"	FxaaFloat subpixB = (subpixA * (1.0/12.0)) - lumaM;\n"
"	FxaaFloat gradientN = lumaN - lumaM;\n"
"	FxaaFloat gradientS = lumaS - lumaM;\n"
"	FxaaFloat lumaNN = lumaN + lumaM;\n"
"	FxaaFloat lumaSS = lumaS + lumaM;\n"
"	FxaaBool pairN = abs(gradientN) >= abs(gradientS);\n"
"	FxaaFloat gradient = max(abs(gradientN), abs(gradientS));\n"
"	if (pairN) lengthSign = -lengthSign;\n"
"	FxaaFloat subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);\n"
"	FxaaFloat2 posB;\n"
"	posB.x = posM.x;\n"
"	posB.y = posM.y;\n"
"	FxaaFloat2 offNP;\n"
"	offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;\n"
"	offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;\n"
"	if (!horzSpan) posB.x += lengthSign * 0.5;\n"
"	if (horzSpan) posB.y += lengthSign * 0.5;\n"
"	FxaaFloat2 posN;\n"
"	posN.x = posB.x - offNP.x * FXAA_QUALITY_P0;\n"
"	posN.y = posB.y - offNP.y * FXAA_QUALITY_P0;\n"
"	FxaaFloat2 posP;\n"
"	posP.x = posB.x + offNP.x * FXAA_QUALITY_P0;\n"
"	posP.y = posB.y + offNP.y * FXAA_QUALITY_P0;\n"
"	FxaaFloat subpixD = ((-2.0)*subpixC) + 3.0;\n"
"	FxaaFloat lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));\n"
"	FxaaFloat subpixE = subpixC * subpixC;\n"
"	FxaaFloat lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));\n"
"	if (!pairN) lumaNN = lumaSS;\n"
"	FxaaFloat gradientScaled = gradient * 1.0/4.0;\n"
"	FxaaFloat lumaMM = lumaM - lumaNN * 0.5;\n"
"	FxaaFloat subpixF = subpixD * subpixE;\n"
"	FxaaBool lumaMLTZero = lumaMM < 0.0;\n"
"	lumaEndN -= lumaNN * 0.5;\n"
"	lumaEndP -= lumaNN * 0.5;\n"
"	FxaaBool doneN = abs(lumaEndN) >= gradientScaled;\n"
"	FxaaBool doneP = abs(lumaEndP) >= gradientScaled;\n"
"	if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P1;\n"
"	if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P1;\n"
"	FxaaBool doneNP = (!doneN) || (!doneP);\n"
"	if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P1;\n"
"	if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P1;\n"
"	if (doneNP) {\n"
"		if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"		if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"		if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"		if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"		doneN = abs(lumaEndN) >= gradientScaled;\n"
"		doneP = abs(lumaEndP) >= gradientScaled;\n"
"		if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P2;\n"
"		if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P2;\n"
"		doneNP = (!doneN) || (!doneP);\n"
"		if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P2;\n"
"		if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P2;\n"
"		#if (FXAA_QUALITY_PS > 3)\n"
"		if (doneNP) {\n"
"			if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"			if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"			if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"			if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"			doneN = abs(lumaEndN) >= gradientScaled;\n"
"			doneP = abs(lumaEndP) >= gradientScaled;\n"
"			if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P3;\n"
"			if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P3;\n"
"			doneNP = (!doneN) || (!doneP);\n"
"			if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P3;\n"
"			if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P3;\n"
"			#if (FXAA_QUALITY_PS > 4)\n"
"			if (doneNP) {\n"
"				if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"				if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"				if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"				if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"				doneN = abs(lumaEndN) >= gradientScaled;\n"
"				doneP = abs(lumaEndP) >= gradientScaled;\n"
"				if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P4;\n"
"				if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P4;\n"
"				doneNP = (!doneN) || (!doneP);\n"
"				if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P4;\n"
"				if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P4;\n"
"				#if (FXAA_QUALITY_PS > 5)\n"
"				if (doneNP) {\n"
"					if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"					if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"					if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"					if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"					doneN = abs(lumaEndN) >= gradientScaled;\n"
"					doneP = abs(lumaEndP) >= gradientScaled;\n"
"					if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P5;\n"
"					if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P5;\n"
"					doneNP = (!doneN) || (!doneP);\n"
"					if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P5;\n"
"					if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P5;\n"
"					#if (FXAA_QUALITY_PS > 6)\n"
"					if (doneNP) {\n"
"						if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"						if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"						if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"						if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"						doneN = abs(lumaEndN) >= gradientScaled;\n"
"						doneP = abs(lumaEndP) >= gradientScaled;\n"
"						if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P6;\n"
"						if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P6;\n"
"						doneNP = (!doneN) || (!doneP);\n"
"						if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P6;\n"
"						if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P6;\n"
"						#if (FXAA_QUALITY_PS > 7)\n"
"						if (doneNP) {\n"
"							if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"							if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"							if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"							if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"							doneN = abs(lumaEndN) >= gradientScaled;\n"
"							doneP = abs(lumaEndP) >= gradientScaled;\n"
"							if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P7;\n"
"							if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P7;\n"
"							doneNP = (!doneN) || (!doneP);\n"
"							if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P7;\n"
"							if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P7;\n"
"	#if (FXAA_QUALITY_PS > 8)\n"
"	if (doneNP) {\n"
"		if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"		if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"		if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"		if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"		doneN = abs(lumaEndN) >= gradientScaled;\n"
"		doneP = abs(lumaEndP) >= gradientScaled;\n"
"		if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P8;\n"
"		if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P8;\n"
"		doneNP = (!doneN) || (!doneP);\n"
"		if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P8;\n"
"		if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P8;\n"
"		#if (FXAA_QUALITY_PS > 9)\n"
"		if (doneNP) {\n"
"			if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"			if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"			if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"			if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"			doneN = abs(lumaEndN) >= gradientScaled;\n"
"			doneP = abs(lumaEndP) >= gradientScaled;\n"
"			if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P9;\n"
"			if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P9;\n"
"			doneNP = (!doneN) || (!doneP);\n"
"			if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P9;\n"
"			if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P9;\n"
"			#if (FXAA_QUALITY_PS > 10)\n"
"			if (doneNP) {\n"
"				if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"				if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"				if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"				if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"				doneN = abs(lumaEndN) >= gradientScaled;\n"
"				doneP = abs(lumaEndP) >= gradientScaled;\n"
"				if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P10;\n"
"				if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P10;\n"
"				doneNP = (!doneN) || (!doneP);\n"
"				if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P10;\n"
"				if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P10;\n"
"				#if (FXAA_QUALITY_PS > 11)\n"
"				if (doneNP) {\n"
"					if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"					if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"					if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"					if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"					doneN = abs(lumaEndN) >= gradientScaled;\n"
"					doneP = abs(lumaEndP) >= gradientScaled;\n"
"					if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P11;\n"
"					if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P11;\n"
"					doneNP = (!doneN) || (!doneP);\n"
"					if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P11;\n"
"					if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P11;\n"
"					#if (FXAA_QUALITY_PS > 12)\n"
"					if (doneNP) {\n"
"						if (!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"						if (!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"						if (!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"						if (!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"						doneN = abs(lumaEndN) >= gradientScaled;\n"
"						doneP = abs(lumaEndP) >= gradientScaled;\n"
"						if (!doneN) posN.x -= offNP.x * FXAA_QUALITY_P12;\n"
"						if (!doneN) posN.y -= offNP.y * FXAA_QUALITY_P12;\n"
"						doneNP = (!doneN) || (!doneP);\n"
"						if (!doneP) posP.x += offNP.x * FXAA_QUALITY_P12;\n"
"						if (!doneP) posP.y += offNP.y * FXAA_QUALITY_P12;\n"
"					}\n"
"					#endif\n"
"				}\n"
"				#endif\n"
"			}\n"
"			#endif\n"
"		}\n"
"		#endif\n"
"	}\n"
"	#endif\n"
"						}\n"
"						#endif\n"
"					}\n"
"					#endif\n"
"				}\n"
"				#endif\n"
"			}\n"
"			#endif\n"
"		}\n"
"		#endif\n"
"	}\n"
"	FxaaFloat dstN = posM.x - posN.x;\n"
"	FxaaFloat dstP = posP.x - posM.x;\n"
"	if (!horzSpan) dstN = posM.y - posN.y;\n"
"	if (!horzSpan) dstP = posP.y - posM.y;\n"
"	FxaaBool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;\n"
"	FxaaFloat spanLength = (dstP + dstN);\n"
"	FxaaBool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;\n"
"	FxaaFloat spanLengthRcp = 1.0/spanLength;\n"
"	FxaaBool directionN = dstN < dstP;\n"
"	FxaaFloat dst = min(dstN, dstP);\n"
"	FxaaBool goodSpan = directionN ? goodSpanN : goodSpanP;\n"
"	FxaaFloat subpixG = subpixF * subpixF;\n"
"	FxaaFloat pixelOffset = (dst * (-spanLengthRcp)) + 0.5;\n"
"	FxaaFloat subpixH = subpixG * fxaaQualitySubpix;\n"
"	FxaaFloat pixelOffsetGood = goodSpan ? pixelOffset : 0.0;\n"
"	FxaaFloat pixelOffsetSubpix = max(pixelOffsetGood, subpixH);\n"
"	if (!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;\n"
"	if (horzSpan) posM.y += pixelOffsetSubpix * lengthSign;\n"
"	#if (FXAA_DISCARD == 1)\n"
"		return FxaaTexTop(tex, posM);\n"
"	#else\n"
"		return FxaaFloat4(FxaaTexTop(tex, posM).xyz, lumaM);\n"
"	#endif\n"
"}\n"
"uniform sampler2D tex0;\n"
"varying vec2 v_rcpFrame;\n"
"noperspective varying vec2 v_pos;\n"
"void main() {\n"
"	gl_FragColor = FxaaPixelShader(v_pos, tex0, v_rcpFrame, FXAA_QUALITY_SUBPIX, FXAA_QUALITY_EDGE_THRESHOLD, FXAA_QUALITY_EDGE_THRESHOLD_MIN);\n"
"}";

char *Default_blur_fragment_shader = 
"varying float blurSize;\n"
"uniform sampler2D tex;\n"
"#define BLUR_SIZE_DIV 3.0\n"
"// Gaussian Blur\n"
"// 512x512 and smaller textures give best results\n"
"// 2 passes required\n"
"void main()\n"
"{\n"
"	// Echelon9 - Due to Apple not implementing array constructors in OS X's\n"
"	// GLSL implementation we need to setup the arrays this way as a workaround\n"
"	float BlurWeights[6];\n"
"	BlurWeights[5] = 0.0402;\n"
"	BlurWeights[4] = 0.0623;\n"
"	BlurWeights[3] = 0.0877;\n"
"	BlurWeights[2] = 0.1120;\n"
"	BlurWeights[1] = 0.1297;\n"
"	BlurWeights[0] = 0.1362;\n"
"	vec4 sum = texture2D(tex, gl_TexCoord[0].xy) * BlurWeights[0];\n"
"#ifdef PASS_0\n"
"	for (int i = 1; i < 6; i++) {\n"
"		sum += texture2D(tex, vec2(clamp(gl_TexCoord[0].x - float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0), gl_TexCoord[0].y)) * BlurWeights[i];\n"
"		sum += texture2D(tex, vec2(clamp(gl_TexCoord[0].x + float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0), gl_TexCoord[0].y)) * BlurWeights[i];\n"
"	}\n"
"#endif\n"
"#ifdef PASS_1\n"
"	for (int i = 1; i < 6; i++) {\n"
"		sum += texture2D(tex, vec2(gl_TexCoord[0].x, clamp(gl_TexCoord[0].y - float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0))) * BlurWeights[i];\n"
"		sum += texture2D(tex, vec2(gl_TexCoord[0].x, clamp(gl_TexCoord[0].y + float(i) * (blurSize/BLUR_SIZE_DIV), 0.0, 1.0))) * BlurWeights[i];\n"
"	}\n"
"#endif\n"
"	gl_FragColor = sum;\n"
"}";

char *Default_brightpass_fragment_shader = 
"uniform sampler2D tex;\n"
"const float Luminance = 0.08;\n"
"const float fMiddleGray = 0.2;\n"
"const float fWhiteCutoff = 0.4;\n"
"// High-pass filter\n"
"void main() {\n"
"	vec4 ColorOut = texture2D(tex, gl_TexCoord[0].xy);\n"
"	ColorOut *= fMiddleGray / (Luminance + 0.001);\n"
"	ColorOut *= (1.0 + (ColorOut / (fWhiteCutoff * fWhiteCutoff)));\n"
"	ColorOut -= 6.0;\n"
"	ColorOut /= (10.0 + ColorOut);\n"
"	gl_FragColor = ColorOut;\n"
"}";

char *Default_post_fragment_shader = 
"uniform sampler2D tex;\n"
"uniform float timer;\n"
"uniform sampler2D bloomed;\n"
"uniform float bloom_intensity;\n"
"#ifdef FLAG_DISTORT_NOISE\n"
"uniform float noise_amount;\n"
"#endif\n"
"#ifdef FLAG_SATURATION\n"
"uniform float saturation;\n"
"#endif\n"
"#ifdef FLAG_BRIGHTNESS\n"
"uniform float brightness;\n"
"#endif\n"
"#ifdef FLAG_CONTRAST\n"
"uniform float contrast;\n"
"#endif\n"
"#ifdef FLAG_GRAIN\n"
"uniform float film_grain;\n"
"#endif\n"
"#ifdef FLAG_STRIPES\n"
"uniform float tv_stripes;\n"
"#endif\n"
"#ifdef FLAG_CUTOFF\n"
"uniform float cutoff;\n"
"#endif\n"
"#ifdef FLAG_DITH\n"
"uniform float dither;\n"
"#endif\n"
"uniform sampler2D blurred_tex;\n"
"uniform sampler2D depth_tex;\n"
"void main()\n"
"{\n"
" #ifdef FLAG_DISTORT_NOISE\n"
" // Distort noise\n"
"	float distort_factor = timer * sin(gl_TexCoord[0].x * gl_TexCoord[0].y * 100.0 + timer);\n"
"	distort_factor = mod(distort_factor, 8.0) * mod(distort_factor, 4.0);\n"
"	vec2 distort;\n"
"	distort = vec2(mod(distort_factor, noise_amount), mod(distort_factor, noise_amount + 0.002));\n"
" #else\n"
"	vec2 distort = vec2(0, 0);\n"
" #endif\n"
" // Global constant\n"
"	vec4 color_in;\n"
"	vec4 color_out;\n"
" // Bloom\n"
"	if (bloom_intensity > 0.0) {\n"
"		color_in = texture2D(tex, gl_TexCoord[0].xy + distort);\n"
"		vec4 color_bloom = texture2D(bloomed, gl_TexCoord[0].xy + distort);\n"
"		color_in = mix(color_in,  max(color_in + 0.7 * color_bloom, color_bloom), bloom_intensity);\n"
"	} else {\n"
"		color_in = texture2D(tex, gl_TexCoord[0].xy + distort);\n"
"	}\n"
" #ifdef FLAG_SATURATION\n"
" // Saturation\n"
"	vec4 color_grayscale = color_in;\n"
"	color_grayscale.rgb = vec3(dot(color_in.rgb, vec3(0.299, 0.587, 0.184)));\n"
"	color_out = mix(color_in, color_grayscale, 1.0 - saturation);\n"
" #else\n"
"	color_out = color_in;\n"
" #endif\n"
" #ifdef FLAG_BRIGHTNESS\n"
" // Brightness\n"
"	vec3 Afactor = vec3(brightness);\n"
"	color_out.rgb = color_out.rgb * Afactor;\n"
" #endif\n"
" #ifdef FLAG_CONTRAST\n"
" // Contrast\n"
"	vec3 Bfactor = vec3(0.5 - 0.5 * contrast);\n"
"	color_out.rgb = color_out.rgb + Bfactor;\n"
" #endif\n"
" #ifdef FLAG_GRAIN\n"
" // Film Grain\n"
"	float x = gl_TexCoord[0].x * gl_TexCoord[0].y * timer * 1000.0;\n"
"	x = mod(x, 13.0) * mod(x, 123.0);\n"
"	float dx = mod(x, 0.01);\n"
"	vec3 result = color_out.rgb + color_out.rgb * clamp(0.1 + dx * 100.0, 0.0, 1.0);\n"
"	color_out.rgb = mix(color_out.rgb, result, film_grain);\n"
" #endif\n"
" #ifdef FLAG_STRIPES\n"
" // TV-Stripes (Old School)\n"
"	vec2 sc;\n"
"	sc.x = sin(gl_TexCoord[0].y * 2048.0);\n"
"	sc.y = cos(gl_TexCoord[0].y * 2048.0);\n"
"	vec3 stripes = color_out.rgb + color_out.rgb * vec3(sc.x, sc.y, sc.x) * 0.8;\n"
"	color_out.rgb = mix(color_out.rgb, stripes, tv_stripes);\n"
" #endif\n"
" #ifdef FLAG_CUTOFF\n"
"	// Experimental cutoff shader\n"
"	if (cutoff > 0.0) {\n"
"		vec4 color_greyscale;\n"
"		color_greyscale.rgb = vec3(dot(color_in.rgb, vec3(0.299, 0.587, 0.184)));\n"
"		vec4 normalized_col;\n"
"		float col_length = (length(color_out.rgb));\n"
"		if (col_length > 1.0) {\n"
"			normalized_col = ((color_out)/col_length);\n"
"		} else {\n"
"			normalized_col = color_out;\n"
"		}\n"
"		vec3 unit_grey = vec3(0.5773);\n"
"		float sat = dot(normalized_col.rgb, unit_grey);\n"
"		color_out = mix(color_greyscale, color_out, sat * cutoff);\n"
"	}\n"
" #endif\n"
" #ifdef FLAG_DITH\n"
" // Dithering\n"
"	float downsampling_factor = 4;\n"
"	float bias = 0.5;\n"
"	color_out.rgb = floor(color_out.rgb * downsampling_factor + bias) / downsampling_factor;\n"
" #endif\n"
"	color_out.a = 1.0;\n"
"	gl_FragColor = color_out;\n"
"}";

char *Default_post_vertex_shader = 
"varying float blurSize;\n"
"uniform float bsize;\n"
"void main()\n"
"{\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_Position = gl_Vertex;\n"
"	gl_FrontColor = gl_Color;\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	blurSize = 1.0 / bsize;\n"
"// Check necessary for ATI specific behavior\n"
" #ifdef __GLSL_CG_DATA_TYPES\n"
"	gl_ClipVertex = (gl_ModelViewMatrix * gl_Vertex);\n"
" #endif\n"
"}";

char* Default_fxaa_prepass_shader = 
"uniform sampler2D tex;\n"
"void main() {\n"
"	vec4 color = texture2D(tex, gl_TexCoord[0].xy);\n"
"	gl_FragColor = vec4(color.rgb, dot(color.rgb, vec3(0.299, 0.587, 0.114)) );\n"
"}";

char* Default_effect_vertex_shader = 
"attribute float radius;\n"
"#ifdef FLAG_EFFECT_GEOMETRY\n"
" attribute vec3 uvec;\n"
" varying vec3 up_g;\n"
" varying float radius_g;\n"
"#else\n"
" varying float radius_p;\n"
" varying vec4 position_p;\n"
"#endif\n"
"#ifdef FLAG_EFFECT_DISTORTION\n"
"varying float offset_out;\n"
"uniform float use_offset;\n"
"#endif\n"
"void main()\n"
"{\n"
"	#ifdef FLAG_EFFECT_GEOMETRY\n"
"	 radius_g = radius;\n"
"	 up_g = uvec;\n"
"	 gl_Position = gl_ModelViewMatrix * gl_Vertex;\n"
"	#else\n"
"	 radius_p = radius;\n"
"	 gl_Position = ftransform();\n"
"	 position_p = gl_ModelViewMatrix * gl_Vertex;\n"
"	#endif\n"
"	#ifdef FLAG_EFFECT_DISTORTION\n"
"	offset_out = radius * use_offset;\n"
"	#endif\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_FrontColor = gl_Color;\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
" #ifdef  __GLSL_CG_DATA_TYPES\n"
"	// Check necessary for ATI specific behavior\n"
"	gl_ClipVertex = (gl_ModelViewMatrix * gl_Vertex);\n"
" #endif\n"
"}";

char* Default_effect_particle_fragment_shader =
"uniform sampler2D baseMap;\n"
"uniform sampler2D depthMap;\n"
"uniform float window_width;\n"
"uniform float window_height;\n"
"uniform float nearZ;\n"
"uniform float farZ;\n"
"uniform int linear_depth;\n"
"varying float radius_p;\n"
"varying vec4 position_p;\n"
"void main()\n"
"{\n"
"	vec4 fragmentColor = texture2D(baseMap, gl_TexCoord[0].xy)*gl_Color.a;\n"
"	vec2 offset = vec2(radius_p * abs(0.5 - gl_TexCoord[0].x) * 2.0, radius_p * abs(0.5 - gl_TexCoord[0].y) * 2.0);\n"
"	float offset_len = length(offset);\n"
"	if ( offset_len > radius_p ) {\n"
"		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
"		return;\n"
"	}\n"
"	vec2 depthCoord = vec2(gl_FragCoord.x / window_width, gl_FragCoord.y / window_height );\n"
"	vec4 sceneDepth = texture2D(depthMap, depthCoord);\n"
"	float sceneDepthLinear;\n"
"	float fragDepthLinear;\n"
"	if ( linear_depth == 1 ) {\n"
"		sceneDepthLinear = -sceneDepth.z;\n"
"		fragDepthLinear = -position_p.z;\n"
"	} else {\n"
"		sceneDepthLinear = ( 2.0 * farZ * nearZ ) / ( farZ + nearZ - sceneDepth.x * (farZ-nearZ) );\n"
"		fragDepthLinear = ( 2.0 * farZ * nearZ ) / ( farZ + nearZ - gl_FragCoord.z * (farZ-nearZ) );\n"
"	}\n"
"	// assume UV of 0.5, 0.5 is the centroid of this sphere volume\n"
"	float depthOffset = sqrt(pow(radius_p, 2.0) - pow(offset_len, 2.0));\n"
"	float frontDepth = fragDepthLinear - depthOffset;\n"
"	float backDepth = fragDepthLinear + depthOffset;\n"
"	float ds = min(sceneDepthLinear, backDepth) - max(nearZ, frontDepth);\n"
"	fragmentColor = fragmentColor * ( ds / (depthOffset*2.0) );\n"
"	gl_FragColor = fragmentColor;\n"
"}";

char* Default_effect_distortion_vertex_shader =
"attribute float radius;\n"
"varying float offset_out;\n"
"uniform float use_offset;\n"
"void main()\n"
"{\n"
"   gl_Position = ftransform();\n"
"	offset_out = radius * use_offset;\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_FrontColor = gl_Color;\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
" #ifdef  __GLSL_CG_DATA_TYPES\n"
"	// Check necessary for ATI specific behavior\n"
"	gl_ClipVertex = (gl_ModelViewMatrix * gl_Vertex);\n"
" #endif\n"
"}";

char* Default_effect_distortion_fragment_shader =
"uniform sampler2D baseMap;\n"
"uniform sampler2D depthMap;\n"
"uniform float window_width;\n"
"uniform float window_height;\n"
"uniform sampler2D distMap;\n"
"uniform sampler2D frameBuffer;\n"
"varying float offset_out;\n"
"void main()\n"
"{\n"
"	vec2 depthCoord = vec2(gl_FragCoord.x / window_width, gl_FragCoord.y / window_height);\n"
"	vec4 fragmentColor = texture2D(baseMap, gl_TexCoord[0].xy)*gl_Color.a;\n"
"	vec2 distortion = texture2D(distMap, gl_TexCoord[0].xy+vec2(0.0, offset_out)).rg;\n"
"	float alpha = clamp(dot(fragmentColor.rgb,vec3(0.3333))*10.0,0.0,1.0);\n"
"	distortion = ((distortion - 0.5) * 0.01) * alpha;\n"
"	gl_FragColor = texture2D(frameBuffer,depthCoord+distortion);\n"
"	gl_FragColor.a = alpha;\n"
"}";

char* Default_effect_fragment_shader = 
"uniform sampler2D baseMap;\n"
"void main()\n"
"{\n"
"	vec4 fragmentColor = texture2D(baseMap, gl_TexCoord[0].xy)*gl_Color.a;\n"
"	gl_FragColor = fragmentColor;\n"
"}";

char* Default_effect_screen_geometry_shader =
"#extension GL_EXT_geometry_shader4 : enable\n"
"varying in vec3 up_g[];\n"
"varying in float radius_g[];\n"
"varying out float radius_p;\n"
"varying out vec4 position_p;\n"
"void main(void)\n"
"{\n"
"	vec3 forward_vec = vec3(0.0, 0.0, 1.0);\n"
"	vec3 up_vec = normalize(up_g[0]);\n"
"	vec3 right_vec = cross(forward_vec, up_vec);\n"
"	vec4 pos = vec4(0.0, 0.0, 0.0, 0.0);\n"
"	right_vec = normalize(right_vec);\n"
"	pos = (gl_PositionIn[0] - vec4(radius_g[0] * up_vec, 0.0) - vec4(radius_g[0] * right_vec, 0.0));\n"
"	gl_Position = gl_ProjectionMatrix * pos;\n"
"	position_p = pos;\n"
"	gl_TexCoord[0] = vec4(0.0, 0.0, 0.0, 0.0);\n"
"	radius_p = radius_g[0];\n"
"	gl_FrontColor = gl_FrontColorIn[0];\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	EmitVertex();\n"

"	pos = (gl_PositionIn[0] - vec4(radius_g[0] * up_vec, 0.0) + vec4(radius_g[0] * right_vec, 0.0));\n"
"	gl_Position = gl_ProjectionMatrix * pos;\n"
"	position_p = pos;\n"
"	gl_TexCoord[0] = vec4(0.0, 1.0, 0.0, 0.0);\n"
"	radius_p = radius_g[0];\n"
"	gl_FrontColor = gl_FrontColorIn[0];\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	EmitVertex();\n"

"	pos = (gl_PositionIn[0] + vec4(radius_g[0] * up_vec, 0.0) - vec4(radius_g[0] * right_vec, 0.0));\n"
"	gl_Position = gl_ProjectionMatrix * pos;\n"
"	position_p = pos;\n"
"	gl_TexCoord[0] = vec4(1.0, 0.0, 0.0, 0.0);\n"
"	radius_p = radius_g[0];\n"
"	gl_FrontColor = gl_FrontColorIn[0];\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	EmitVertex();\n"

"	pos = (gl_PositionIn[0] + vec4(radius_g[0] * up_vec, 0.0) + vec4(radius_g[0] * right_vec, 0.0));\n"
"	gl_Position = gl_ProjectionMatrix * pos;\n"
"	position_p = pos;\n"
"	gl_TexCoord[0] = vec4(1.0, 1.0, 0.0, 0.0);\n"
"	radius_p = radius_g[0];\n"
"	gl_FrontColor = gl_FrontColorIn[0];\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	EmitVertex();\n"
"	EndPrimitive();\n"
"}";

char* Default_shadowdebug_vertex_shader = 
"void main()\n"
"{\n"
"	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"	gl_FrontColor = vec4(1.0);\n"
"	gl_Position = gl_Vertex;\n"
" #ifdef  __GLSL_CG_DATA_TYPES\n"
"	// Check necessary for ATI specific behavior\n"
"	gl_ClipVertex = (gl_ModelViewMatrix * gl_Vertex);\n"
" #endif\n"
"}\n";

char* Default_shadowdebug_fragment_shader =
"#extension GL_EXT_texture_array : enable\n"
"uniform sampler2DArray shadow_map;\n"
"//uniform sampler2D shadow_map;\n"
"uniform int index;\n"
"void main()\n"
"{\n"
"	vec3 texcoord = vec3(gl_TexCoord[0].xy, float(index));\n"
"	gl_FragColor = vec4(texture2DArray(shadow_map, texcoord).rgb,1.0);\n"
"	//gl_FragColor = vec4(texture2D(shadow_map, gl_TexCoord[0].xy).rgb,1.0);\n"
"}\n"; 

char* Default_lightshaft_fragment_shader =
"uniform sampler2D scene;\n"
"uniform sampler2D cockpit;\n"
"uniform vec2 sun_pos;\n"
"uniform float density;\n"
"uniform float weight;\n"
"uniform float falloff;\n"
"uniform float intensity;\n"
"uniform float cp_intensity;\n"
"void main()\n"
"{\n"
"	vec2 step = vec2( gl_TexCoord[0].st - sun_pos.xy );\n"
"	vec2 pos = gl_TexCoord[0].st;\n"
"	step *= 1.0 / float(SAMPLE_NUM) * density;\n"
"	float decay = 1.0;\n"
"	vec4 sum = vec4(0.0);\n"
"	vec4 mask = texture2D(cockpit, gl_TexCoord[0].st);\n"
"	if (mask.r < 1.0) {\n"
"		gl_FragColor = vec4(cp_intensity);\n"
"		return;\n"
"	}\n"
"	for(int i=0; i < SAMPLE_NUM ; i++) {\n"
"		pos.st -= step;\n"
"		vec4 tex_sample = texture2D(scene, pos);\n"
"		if (tex_sample.r == 1.0)\n"
"			sum += decay * weight;\n"
"		decay *= falloff;\n"
"	}\n"
"	gl_FragColor = sum * intensity;\n"
"	gl_FragColor.a = 1.0;\n"
"}";

char *Default_video_vertex_shader = 
"void main()\n"
"{\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;\n"
"	gl_FrontColor = vec4(1.0);\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"}";

char * Default_video_fragment_shader = 
"uniform sampler2D ytex;\n"
"uniform sampler2D utex;\n"
"uniform sampler2D vtex;\n"
"void main()\n"
"{\n"
"	vec3 val = vec3(texture2D(ytex, gl_TexCoord[0].st).r - 0.0625, texture2D(utex, gl_TexCoord[0].st).r - 0.5, texture2D(vtex, gl_TexCoord[0].st).r - 0.5);\n"
"	gl_FragColor.r = dot(val, vec3(1.1640625, 0.0, 1.59765625));\n"
"	gl_FragColor.g = dot(val, vec3(1.1640625, -0.390625, -0.8125));\n"
"	gl_FragColor.b = dot(val, vec3(1.1640625, 2.015625, 0.0));\n"
"	gl_FragColor.a = 1.0;\n"
"}";

char *Default_deferred_vertex_shader =
"uniform vec3 scale;\n"
"uniform int lightType;\n"
"\n"
"varying vec3 lightPosition;\n"
"varying vec3 beamVec;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(gl_Vertex.xyz * scale,1.0);\n"
"	lightPosition = gl_ModelViewMatrix[3].xyz;\n"
"	if(lightType == 1)\n"
"		beamVec = vec3(gl_ModelViewMatrix * vec4(0.0, 0.0, -scale.z, 0.0));\n"
"}";

char *Default_deferred_fragment_shader =
"uniform sampler2D ColorBuffer;\n"
"uniform sampler2D NormalBuffer;\n"
"uniform sampler2D PositionBuffer;\n"
"uniform sampler2D SpecBuffer;\n"
"uniform float specFactor;\n"
"uniform float invScreenWidth;\n"
"uniform float invScreenHeight;\n"
"uniform int lightType;\n"
"varying vec3 lightPosition;\n"
"uniform float lightRadius;\n"
"varying vec3 beamVec;\n"
"uniform vec3 diffuseLightColor;\n"
"uniform vec3 specLightColor;\n"
"uniform float coneAngle;\n"
"uniform float coneInnerAngle;\n"
"uniform bool dualCone;\n"
"uniform vec3 coneDir;\n"
"\n"
"#define SPEC_INTENSITY_POINT			5.3 // Point light\n"
"#define SPEC_INTENSITY_DIRECTIONAL		3.0 // Directional light\n"
"#define SPECULAR_FACTOR				1.75\n"
"#define SPECULAR_ALPHA					0.1\n"
"#define SPEC_FACTOR_NO_SPEC_MAP		0.6\n"
"#define ENV_ALPHA_FACTOR				0.3\n"
"#define GLOW_MAP_INTENSITY				1.5\n"
"#define AMBIENT_LIGHT_BOOST			1.0\n"
"void main()\n"
"{\n"
"	vec2 screenPos = gl_FragCoord.xy * vec2(invScreenWidth, invScreenHeight);\n"
"	vec3 position = texture2D(PositionBuffer, screenPos).xyz;\n"
"\n"
"	if(abs(dot(position, position)) < 0.1)\n"
"		discard;\n"
"	vec3 lightDir = lightPosition - position.xyz;\n"
"	float dist = length(lightDir);\n"
"	float attenuation = (1.0 - dist/lightRadius);\n"
"	if(lightType == 2)\n"
"	{\n"
"		float coneDot = dot(normalize(-lightDir), coneDir);\n"
"		if(dualCone) {\n"
"			if(abs(coneDot) < coneAngle)\n"
"				discard;\n"
"			else\n"
"				attenuation *= smoothstep(coneAngle, coneInnerAngle, abs(coneDot));\n"
"		} else {\n"
"			if(coneDot < coneAngle)\n"
"				discard;\n"
"			else\n"
"				attenuation *= smoothstep(coneAngle, coneInnerAngle, coneDot);\n"
"		}\n"
"	}\n"
"	if(dist > lightRadius && lightType != 1)\n"
"		discard;\n"
"	vec3 color = texture2D(ColorBuffer, screenPos).rgb;\n"
"	vec4 normal = texture2D(NormalBuffer, screenPos);\n"
"	vec3 specfactor = texture2D(SpecBuffer, screenPos).rgb;\n"
"	vec3 eyeDir = normalize(-position);\n"
"\n"
"	if(lightType == 1)\n"
"	{\n"
"		float beamLength = length(beamVec);\n"
"		vec3 beamDir = beamVec / beamLength;\n"
"		// Get nearest point on line\n"
"		float neardist = clamp(dot(lightDir, beamDir), 0.0, beamLength);\n"
"		// Move back from the endpoint of the beam along the beam by the distance we calculated\n"
"		vec3 nearest = lightPosition - beamDir * neardist;\n"
"		lightDir = nearest - position.xyz;\n"
"		dist = length(lightDir);\n"
"		if(dist > lightRadius)\n"
"			discard;\n"
"	}\n"
"	lightDir /= dist;\n"
"	vec3 halfVec = normalize(lightDir + eyeDir);\n"
"	float NdotHV = clamp(dot(normal.xyz, halfVec), 0.0, 1.0);\n"
"	gl_FragData[0].rgb = color * (diffuseLightColor * (max(dot(normal.xyz, lightDir), 0.0)) * attenuation);\n"
"   gl_FragData[0].rgb += pow(NdotHV, specFactor) * SPEC_INTENSITY_POINT * specfactor * specLightColor * attenuation;\n"
"	gl_FragData[0].a = 1.0;\n"
"}";

char *Default_deferred_clear_vertex_shader =
"void main()\n"
"{\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_Position = gl_Vertex;\n"
"	gl_FrontColor = vec4(1.0);\n"
"	gl_FrontSecondaryColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"}";

char *Default_deferred_clear_fragment_shader =
"void main()\n"
"{\n"
"   gl_FragData[0] = vec4(0.0, 0.0, 0.0, 1.0); // color\n"
"	gl_FragData[1] = vec4(0.0, 0.0, -1000000.0, 1.0); // position\n"
"	gl_FragData[2] = vec4(0.0, 0.0, 0.0, 1.0); // normal\n"
"	gl_FragData[3] = vec4(0.0, 0.0, 0.0, 1.0); // specular\n"
"}\n";
