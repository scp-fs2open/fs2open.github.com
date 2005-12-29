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
//**********

//:PART 2:
//**********
def_file Default_files[] = {
	{"species_defs.tbl",		Default_species_table},
	{"iff_defs.tbl",			Default_iff_table},
	{"shiptypes.tbl",			Default_shiptypes_table},
};

static int Num_default_files = sizeof(Default_files)/sizeof(def_file);
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
	Warning(LOCATION, "Default table '%s' missing from executable - contact a coder.", filename);
	return NULL;
}

//:PART 3:
//**********
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// This is the default table
// Please note that the {\n\}s should be removed from the end of each line
// if you intend to use this to format your own species_defs.tbl.

char *Default_species_table = "\
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

//=============================================================================

// This is the default table
// Please note that the {\n\}s should be removed from the end of each line and
// the {\"}s  should be replaced with {"}s if you intend to use this to format
// your own iff_defs.tbl.

char *Default_iff_table = "\
																			\n\
#IFFs																		\n\
																			\n\
;; Every iff_defs.tbl must contain a Traitor entry.  Traitors attack one	\n\
;; another (required by the dogfighting code) but it is up to you to		\n\
;; decide who attacks the traitor or whom else the traitor attacks.			\n\
$Traitor IFF: Traitor														\n\
																			\n\
;------------------------													\n\
; Friendly																	\n\
;------------------------													\n\
$IFF Name: Friendly															\n\
$Color: ( 0, 255, 0 )														\n\
$Attacks: ( \"Hostile\" \"Neutral\" \"Traitor\" )							\n\
$Default Ship Flags: ( \"cargo-known\" )									\n\
																			\n\
;------------------------													\n\
; Hostile																	\n\
;------------------------													\n\
$IFF Name: Hostile															\n\
$Color: ( 255, 0, 0 )														\n\
$Attacks: ( \"Friendly\" \"Neutral\" \"Traitor\" )							\n\
+Sees Friendly As: ( 255, 0, 0 )											\n\
+Sees Hostile As: ( 0, 255, 0 )												\n\
																			\n\
;------------------------													\n\
; Neutral																	\n\
;------------------------													\n\
$IFF Name: Neutral															\n\
$Color: ( 255, 0, 0 )														\n\
$Attacks: ( \"Friendly\" \"Traitor\" )										\n\
+Sees Friendly As: ( 255, 0, 0 )											\n\
+Sees Hostile As: ( 0, 255, 0 )												\n\
+Sees Neutral As: ( 0, 255, 0 )												\n\
																			\n\
;------------------------													\n\
; Unknown																	\n\
;------------------------													\n\
$IFF Name: Unknown															\n\
$Color: ( 255, 0, 255 )														\n\
$Attacks: ( \"Hostile\" )													\n\
+Sees Neutral As: ( 0, 255, 0 )												\n\
+Sees Traitor As: ( 0, 255, 0 )												\n\
$Flags: ( \"exempt from all teams at war\" )								\n\
																			\n\
;------------------------													\n\
; Traitor																	\n\
;------------------------													\n\
$IFF Name: Traitor															\n\
$Color: ( 255, 0, 0 )														\n\
$Attacks: ( \"Friendly\" \"Hostile\" \"Neutral\" \"Traitor\" )				\n\
+Sees Friendly As: ( 255, 0, 0 )											\n\
																			\n\
#End																		\n\
";

//=============================================================================

//=============================================================================
//char *Default_shiptypes_table = "";

char *Default_shiptypes_table = "\
#Ship types\n\
""$Name:					Navbuoy\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		10.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Turrets attack this:	YES\n\
\n\
""$Name:					Sentry gun\n\
$Counts for Alone:		YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			0.10\n\
$EMP Multiplier:		10.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Accept Player Orders:	NO\n\
	+Auto attacks:			YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
\n\
""$Name:					Escape pod\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Warp Pushable:			YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		10.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			600.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Turrets attack this:	YES\n\
\n\
""$Name:					Cargo\n\
$Scannable:				YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			0.10\n\
$EMP Multiplier:		10.0\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Passive docks:			(\"cargo\")\n\
\n\
""$Name:					Support\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Warp Pushable:			YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		3.5\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"dock\" \"undock\" \"waypoints\" \"waypoints once\" \"stay near ship\" \"keep safe dist\" \"stay still\" \"play dead\")\n\
	+Accept Player Orders:	YES\n\
	+Player orders:			(\"rearm me\" \"abort rearm\" \"stay near me\" \"stay near ship\" \"keep safe dist\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Active docks:			(\"support\")\n\
\n\
;;WMC - Stealth ships always have another type, so this isn't used\n\
""$Name:					Stealth\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		4.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"attack besides\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\")\n\
	+Auto attacks:			YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Fighter\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Warp Pushable:			YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		4.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"attack besides\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\")\n\
	+Auto attacks:			YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Bomber\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Warp Pushable:			YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		4.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"attack besides\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\")\n\
	+Auto attacks:			YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
;;WMC - This fighter/bomber type doesn't seem to be used anywhere, because no ship is set as both fighter and bomber\n\
""$Name:					Fighter/bomber\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Warp Pushable:			YES\n\
$Max Debris Speed:		200\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		4.0\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"waypoints\" \"waypoints once\" \"depart\" \"attack subsys\" \"attack wing\" \"guard ship\" \"disable ship\" \"disarm ship\" \"attack any\" \"ignore ship\" \"guard wing\" \"evade ship\" \"stay still\" \"play dead\" \"stay near ship\" \"keep safe dist\" \"attack besides\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"disable ship\" \"disarm ship\" \"guard ship\" \"ignore ship\" \"form on wing\" \"cover me\" \"attack any\" \"depart\" \"disable subsys\")\n\
	+Auto attacks:			YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Transport\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		150\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		2.0\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			500.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"dock\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Freighter\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Scannable:				YES\n\
$Max Debris Speed:		150\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		1.75\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			600.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"dock\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Active docks:			(\"cargo\")\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					AWACS\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		150\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.8\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			600.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Gas Miner\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		150\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		1.0\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			600.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Cruiser\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		150\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.9\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			600.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Corvette\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		150\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.3333\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			600.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"dock\" \"waypoints\" \"waypoints once\" \"depart\" \"undock\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"attack ship\" \"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Capital\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Warp Pushes:			YES\n\
$Max Debris Speed:		100\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.2\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			750.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"waypoints\" \"waypoints once\" \"depart\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Accept Player Orders:	YES\n\
	+Player Orders:			(\"depart\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Super cap\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Warp Pushes:			YES\n\
$Max Debris Speed:		100\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.075\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			1000.0\n\
$AI:\n\
	+Valid goals:			(\"fly to ship\" \"attack ship\" \"attack wing\" \"waypoints\" \"waypoints once\" \"depart\" \"stay still\" \"play dead\" \"stay near ship\")\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Can Form Wing:			YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Drydock\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		100\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.5\n\
$Beams Easily Hit:		YES\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			750.0\n\
$AI:\n\
	+Accept Player Orders:	YES\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Passive docks:			(\"support\")\n\
\n\
""$Name:					Knossos Device\n\
$Counts for Alone:		YES\n\
$Praise Destruction:	YES\n\
$On Hotkey List:		YES\n\
$Target as Threat:		YES\n\
$Show Attack Direction:	YES\n\
$Max Debris Speed:		100\n\
$FF Multiplier:			1.0\n\
$EMP Multiplier:		0.10\n\
$Fog:\n\
	+Start dist:			10.0\n\
	+Compl dist:			1000.0\n\
$AI:\n\
	+Auto attacks:			YES\n\
	+Attempt Broadside:		YES\n\
	+Guards attack this:	YES\n\
	+Turrets attack this:	YES\n\
	+Passive docks:			(\"support\")\n\
#End\n\
";