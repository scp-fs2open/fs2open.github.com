/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

/*
 * $Logfile: /Freespace2/code/iff_defs/iff_defs.cpp $
 * $Revision: 1.3 $
 * $Date: 2005-09-30 03:40:40 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/09/29 04:26:08  Goober5000
 * parse fixage
 * --Goober5000
 *
 * Revision 1.1  2005/09/27 05:25:18  Goober5000
 * initial commit of basic IFF code
 * --Goober5000
 *
 */


#include "iff_defs/iff_defs.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"
#include "hud/hud.h"
#include "mission/missionparse.h"


int Num_iffs;
iff_info Iff_info[MAX_IFFS];

int Iff_traitor;

color Iff_colors[MAX_IFF_COLORS][2];


//=============================================================================

// This is the default table
// Please note that the {\n\}s should be removed from the end of each line and
// the {\"}s  should be replaced with {"}s if you intend to use this to format
// your own iff_defs.tbl.

char *default_iff_table = "\
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


// borrowed from ship.cpp, ship_iff_init_colors
int iff_get_alpha_value(bool alpha_on)
{
	int iff_bright_delta = 4;

	if (!alpha_on)
		return (HUD_COLOR_ALPHA_MAX - iff_bright_delta) * 16;
	else 
		return HUD_COLOR_ALPHA_MAX * 16;
}

int iff_init_color(int r, int g, int b)
{
	typedef struct temp_color_t {
		int	r;
		int g;
		int b;
	} temp_color_t;

	int i, idx;
	temp_color_t *c;

	static int num_iff_colors = 0;
	static temp_color_t temp_colors[MAX_IFF_COLORS];


	// make sure we're under the limit
	if (num_iff_colors >= MAX_IFF_COLORS)
	{
		Warning(LOCATION, "Too many iff colors!  Ignoring the rest...\n");
		return 0;
	}


	// find out if this color is in use
	for (i = 0; i < num_iff_colors; i++)
	{
		c = &temp_colors[i];

		if (c->r == r && c->g == g && c->b == b)
			return i;
	}


	// not in use, so add a new slot
	idx = num_iff_colors;
	num_iff_colors++;

	// save the values
	c = &temp_colors[idx];
	c->r = r;
	c->g = g;
	c->b = b;

	// init it
	gr_init_alphacolor(&Iff_colors[idx][0], r, g, b, iff_get_alpha_value(false));
	gr_init_alphacolor(&Iff_colors[idx][1], r, g, b, iff_get_alpha_value(true));


	// return the new slot
	return idx;
}

void iff_init()
{
	char traitor_name[NAME_LENGTH];
	char attack_names[MAX_IFFS][MAX_IFFS][NAME_LENGTH];

	struct {
		char iff_name[NAME_LENGTH];
		int color_index;
	} observed_color_table[MAX_IFFS][MAX_IFFS];

	int num_attack_names[MAX_IFFS];
	int num_observed_colors[MAX_IFFS];


	// Goober5000 - condensed check for table file
	CFILE *idt = cfopen("iff_defs.tbl", "rb");
	int table_exists = (idt != NULL);
	if (table_exists)
		cfclose(idt);

	// Goober5000 - if table doesn't exist, use the default table (see above)
	if (table_exists)
		read_file_text("iff_defs.tbl");
	else
		read_file_text_from_array(default_iff_table);

	reset_parse();	
	

	// before parsing, set up the predefined colors
	// NOTE: THESE MUST OCCUR IN THE ORDER DEFINED IN IFF_DEFS.H!!
	iff_init_color(0xff, 0xff, 0xff);	// IFF_COLOR_SELECTION
	iff_init_color(0x7f, 0x7f, 0x7f);	// IFF_COLOR_MESSAGE
	iff_init_color(0xff, 0xff, 0x00);	// IFF_COLOR_TAGGED


	// parse the table --------------------------------------------------------

	required_string("#IFFs");

	// get the traitor
	required_string("$Traitor IFF:");
	stuff_string(traitor_name, F_NAME, NULL, NAME_LENGTH);

	// begin reading data
	Num_iffs = 0;
	while (required_string_either("#End","$IFF Name:"))
	{
		iff_info *iff;
		int rgb[3];
		int cur_iff;
		
		// make sure we're under the limit
		if (Num_iffs >= MAX_IFFS)
		{
			Warning(LOCATION, "Too many iffs in iffs_defs.tbl!  Max is %d.\n", MAX_IFFS);
			skip_to_start_of_string("#End", NULL);
			break;
		}

		// add new IFF
		iff = &Iff_info[Num_iffs];
		cur_iff = Num_iffs;
		Num_iffs++;


		// get required IFF info ----------------------------------------------

		// get the iff name
		required_string("$IFF Name:");
		stuff_string(iff->iff_name, F_NAME, NULL, NAME_LENGTH);

		// get the iff color
		if (check_for_string("$Colour:"))
			required_string("$Colour:");
		else
			required_string("$Color:");
		stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
		iff->color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);


		// get relationships between IFFs -------------------------------------

		// get the list of iffs attacked
		if (optional_string("$Attacks:"))
			num_attack_names[cur_iff] = stuff_string_list(attack_names[cur_iff], MAX_IFFS);
		else
			num_attack_names[cur_iff] = 0;

		// get the list of observed colors
		num_observed_colors[cur_iff] = 0;
		while (optional_string("+Sees"))
		{
			// get iff observed
			stuff_string_until(observed_color_table[cur_iff][num_observed_colors[cur_iff]].iff_name, "As:", NAME_LENGTH);
			required_string("As:");

			// get color observed
			stuff_int_list(rgb, 3, RAW_INTEGER_TYPE);
			observed_color_table[cur_iff][num_observed_colors[cur_iff]].color_index = iff_init_color(rgb[0], rgb[1], rgb[2]);

			// increment
			num_observed_colors[cur_iff]++;
		}


		// get flags ----------------------------------------------------------

		// get iff flags
		iff->flags = 0;
		if (optional_string("$Flags:"))
		{
			int i;
			char flag_strings[MAX_IFF_FLAGS][NAME_LENGTH];
			int num_strings = stuff_string_list(flag_strings, MAX_IFF_FLAGS);
			for (i = 0; i < num_strings; i++)
			{
				if (!stricmp(NOX("exempt from all teams at war"), flag_strings[i]))
					iff->flags |= IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR;
				else
					Warning(LOCATION, "Bogus string in iff flags: %s\n", flag_strings[i]);
			}
		}

		// get default ship flags
		iff->default_ship_flags = 0;
		if (optional_string("$Default Ship Flags:"))
		{
			int i, j;
			char flag_strings[MAX_PARSE_OBJECT_FLAGS][NAME_LENGTH];
			int num_strings = stuff_string_list(flag_strings, MAX_PARSE_OBJECT_FLAGS);
			for (i = 0; i < num_strings; i++)
			{
				for (j = 0; j < MAX_PARSE_OBJECT_FLAGS; j++)
				{
					if (!stricmp(flag_strings[i], Parse_object_flags[j]))
					{
						iff->default_ship_flags |= (1 << j);
						break;
					}
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS)
				Warning(LOCATION, "Bogus string in iff default ship flags: %s\n", flag_strings[i]);
		}
	}
	
	required_string("#End");


	// now resolve the relationships ------------------------------------------
}
