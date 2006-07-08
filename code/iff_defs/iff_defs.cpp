/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * $Logfile: /Freespace2/code/iff_defs/iff_defs.cpp $
 * $Revision: 1.10 $
 * $Date: 2006-07-08 19:35:52 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2006/04/20 06:32:07  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.8  2006/02/13 00:20:45  Goober5000
 * more tweaks, plus clarification of checks for the existence of files
 * --Goober5000
 *
 * Revision 1.7  2006/01/28 04:33:06  Goober5000
 * fix all teams at war IFF behavior
 * --Goober5000
 *
 * Revision 1.6  2006/01/16 11:02:23  wmcoolmon
 * Various warning fixes, scripting globals fix; added "plr" and "slf" global variables for in-game hooks; various lua functions; GCC fixes for scripting.
 *
 * Revision 1.5  2006/01/13 03:31:09  Goober5000
 * übercommit of custom IFF stuff :)
 *
 * Revision 1.4  2005/12/29 08:08:36  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 1.3  2005/09/30 03:40:40  Goober5000
 * hooray for more work on the iff code
 * --Goober5000
 *
 * Revision 1.2  2005/09/29 04:26:08  Goober5000
 * parse fixage
 * --Goober5000
 *
 * Revision 1.1  2005/09/27 05:25:18  Goober5000
 * initial commit of basic IFF code
 * --Goober5000
 *
 */

#include "globalincs/def_files.h"
#include "iff_defs/iff_defs.h"
#include "parse/parselo.h"
#include "hud/hud.h"
#include "mission/missionparse.h"
#include "ship/ship.h"


int Num_iffs;
iff_info Iff_info[MAX_IFFS];

int Iff_traitor;

// global only to file
color Iff_colors[MAX_IFF_COLORS][2];		// AL 1-2-97: Create two IFF colors, regular and bright


// borrowed from ship.cpp, ship_iff_init_colors
int iff_get_alpha_value(bool is_bright)
{
	int iff_bright_delta = 4;

	if (is_bright == false)
		return (HUD_COLOR_ALPHA_MAX - iff_bright_delta) * 16;
	else 
		return HUD_COLOR_ALPHA_MAX * 16;
}

// init a color and add it to the Iff_colors array
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


	Assert(r >= 0 && r <= 255);
	Assert(g >= 0 && g <= 255);
	Assert(b >= 0 && b <= 255);


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

// parse the table
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

	// Goober5000 - if table doesn't exist, use the default table
	if (cf_exists_full("iff_defs.tbl", CF_TYPE_TABLES))
		read_file_text("iff_defs.tbl");
	else
		read_file_text_from_array(defaults_get_file("iff_defs.tbl"));

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
				if (!stricmp(NOX("support allowed"), flag_strings[i]))
					iff->flags |= IFFF_SUPPORT_ALLOWED;
				else if (!stricmp(NOX("exempt from all teams at war"), flag_strings[i]))
					iff->flags |= IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR;
				else
					Warning(LOCATION, "Bogus string in iff flags: %s\n", flag_strings[i]);
			}
		}

		// get default ship flags
		iff->default_ship_flags = 0;
		if (optional_string("$Default Ship Flags:"))
		{
			int i, j = 0;
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

		// again
		iff->default_ship_flags2 = 0;
		if (optional_string("$Default Ship Flags2:"))
		{
			int i, j = 0;
			char flag_strings[MAX_PARSE_OBJECT_FLAGS_2][NAME_LENGTH];
			int num_strings = stuff_string_list(flag_strings, MAX_PARSE_OBJECT_FLAGS_2);
			for (i = 0; i < num_strings; i++)
			{
				for (j = 0; j < MAX_PARSE_OBJECT_FLAGS_2; j++)
				{
					if (!stricmp(flag_strings[i], Parse_object_flags_2[j]))
					{
						iff->default_ship_flags2 |= (1 << j);
						break;
					}
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS_2)
				Warning(LOCATION, "Bogus string in iff default ship flags2: %s\n", flag_strings[i]);
		}
	}
	
	required_string("#End");


	// now resolve the relationships ------------------------------------------

	// first get the traitor
	Iff_traitor = iff_lookup(traitor_name);
	if (Iff_traitor < 0)
	{
		Iff_traitor = 0;
		Warning(LOCATION, "Traitor IFF %s not found in iff_defs.tbl!  Defaulting to %s.\n", traitor_name, Iff_info[Iff_traitor].iff_name);
	}

	// next get the attackees and colors
	for (int cur_iff = 0; cur_iff < Num_iffs; cur_iff++)
	{
		iff_info *iff = &Iff_info[cur_iff];

		// clear the iffs to be attacked
		iff->attackee_bitmask = 0;
		iff->attackee_bitmask_all_teams_at_war = 0;

		// clear the observed colors
		for (int j = 0; j < MAX_IFFS; j++)
			iff->observed_color_index[j] = -1;

		// resolve the list names
		for (int list_index = 0; list_index < MAX_IFFS; list_index++)
		{
			// are we within the number of attackees listed?
			if (list_index < num_attack_names[cur_iff])
			{
				// find out who
				int target_iff = iff_lookup(attack_names[cur_iff][list_index]);

				// valid?
				if (target_iff >= 0)
					iff->attackee_bitmask |= iff_get_mask(target_iff);
				else
					Warning(LOCATION, "Attack target IFF %s not found for IFF %s in iff_defs.tbl!\n", attack_names[cur_iff][list_index], iff->iff_name);
			}

			// are we within the number of colors listed?
			if (list_index < num_observed_colors[cur_iff])
			{
				// find out who
				int target_iff = iff_lookup(observed_color_table[cur_iff][list_index].iff_name);

				// valid?
				if (target_iff >= 0)
					iff->observed_color_index[target_iff] = observed_color_table[cur_iff][list_index].color_index;
				else
					Warning(LOCATION, "Observed color IFF %s not found for IFF %s in iff_defs.tbl!\n", observed_color_table[cur_iff][list_index].iff_name, iff->iff_name);
			}
		}

		// resolve the all teams at war relationships
		if (iff->flags & IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR)
		{
			// exempt, so use standard attacks
			iff->attackee_bitmask_all_teams_at_war = iff->attackee_bitmask;
		}
		else
		{
			// nonexempt, so build bitmask of all other nonexempt teams
			for (int other_iff = 0; other_iff < Num_iffs; other_iff++)
			{
				// skip myself (unless I attack myself normally)
				if ((other_iff == cur_iff) && !iff_x_attacks_y(cur_iff, cur_iff))
					continue;

				// skip anyone exempt
				if (Iff_info[other_iff].flags & IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR)
					continue;

				// add everyone else
				iff->attackee_bitmask_all_teams_at_war |= iff_get_mask(other_iff);
			}
		}
	}
}

// find the iff name
int iff_lookup(char *iff_name)
{
	// bogus
	Assert(iff_name);
	if(iff_name == NULL)
		return -1;

	for (int i = 0; i < Num_iffs; i++)
		if (!stricmp(iff_name, Iff_info[i].iff_name))
			return i;

	return -1;
}

// get the mask, taking All Teams At War into account
int iff_get_attackee_mask(int attacker_team)
{
	Assert(attacker_team >= 0 && attacker_team < Num_iffs);

	//	All teams attack all other teams.
	if (Mission_all_attack)
	{
		return Iff_info[attacker_team].attackee_bitmask_all_teams_at_war;
	}
	// normal
	else
	{
		return Iff_info[attacker_team].attackee_bitmask;
	}
}

// rather slower, since it has to construct a mask
int iff_get_attacker_mask(int attackee_team)
{
	Assert(attackee_team >= 0 && attackee_team < Num_iffs);

	int i, attacker_bitmask = 0;
	for (i = 0; i < Num_iffs; i++)
	{
		if (iff_x_attacks_y(i, attackee_team))
			attacker_bitmask |= iff_get_mask(i);
	}

	return attacker_bitmask;
}

// similar to above; >0 if true, 0 if false
int iff_x_attacks_y(int team_x, int team_y)
{
	return iff_matches_mask(team_y, iff_get_attackee_mask(team_x));
}

// generate a mask for a team
int iff_get_mask(int team)
{
	return (1 << team);
}

// see if the mask contains the team
int iff_matches_mask(int team, int mask)
{
	return (iff_get_mask(team) & mask) ? 1 : 0;
}

// get the color from the color index
color *iff_get_color(int color_index, int is_bright)
{
	return &Iff_colors[color_index][is_bright];
}

// get the color index, taking objective vs. subjective into account
color *iff_get_color_by_team(int team, int seen_from_team, int is_bright)
{
	Assert(team >= 0 && team < Num_iffs);
	Assert(seen_from_team < Num_iffs);
	Assert(is_bright == 0 || is_bright == 1);


	// is this guy being seen by anyone?
	if (seen_from_team < 0)
		return &Iff_colors[Iff_info[team].color_index][is_bright];


	// Goober5000 - base the following on "sees X as" from iff code
	// c.f. AL's comment:

	// AL 12-26-97:	it seems IFF color needs to be set relative to the player team.  If
	//						the team in question is the same as the player, then it should be 
	//						drawn friendly.  If the team is different than the player's, then draw the
	//						appropriate IFF.


	// assume an observed color is defined; if not, use normal color
	int color_index = Iff_info[seen_from_team].observed_color_index[team];
	if (color_index < 0)
		color_index = Iff_info[team].color_index;


	return &Iff_colors[color_index][is_bright];
}
