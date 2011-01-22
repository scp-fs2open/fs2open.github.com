/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "freespace2/freespace.h"
#include "MissionSave.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "FredRender.h"
#include "ai/aigoals.h"
#include "starfield/starfield.h"
#include "lighting/lighting.h"
#include "globalincs/linklist.h"
#include "weapon/weapon.h"
#include "mission/missioncampaign.h"
#include "CampaignTreeWnd.h"
#include "CampaignTreeView.h"
#include "CampaignEditorDlg.h"
#include "parse/sexp.h"
#include "mission/missionbriefcommon.h"
#include "Management.h"
#include "gamesnd/eventmusic.h"
#include "starfield/nebula.h"
#include "asteroid/asteroid.h"
#include "missionui/missioncmdbrief.h"
#include "jumpnode/jumpnode.h"
#include "MainFrm.h"
#include "localization/fhash.h"
#include "nebula/neb.h"
#include "osapi/osapi.h"
#include "FredView.h"
#include "cfile/cfile.h"
#include "object/objectdock.h"
#include "object/objectshield.h"
#include "iff_defs/iff_defs.h"
#include "missionui/fictionviewer.h"
#include "globalincs/version.h"
#include "sound/sound.h"
#include "sound/ds.h"


void CFred_mission_save::convert_special_tags_to_retail(char *text, int max_len)
{
	replace_all(Mp, "$quote", "''", max_len);
	replace_all(Mp, "$semicolon", ",", max_len);
}

// Goober5000 - convert $quote and $semicolon to '' and ,
void CFred_mission_save::convert_special_tags_to_retail()
{
	int i, team, stage;

	if ( Format_fs2_open != FSO_FORMAT_RETAIL)
		return;

	for (team = 0; team < Num_teams; team++)
	{
		// command briefing
		for (stage = 0; stage < Cmd_briefs[team].num_stages; stage++)
		{
			convert_special_tags_to_retail(Cmd_briefs[team].stage[stage].text, CMD_BRIEF_TEXT_MAX);
		}

		// briefing
		for (stage = 0; stage < Briefings[team].num_stages; stage++)
		{
			convert_special_tags_to_retail(Briefings[team].stages[stage].new_text, MAX_BRIEF_LEN);
		}

		// debriefing
		for (stage = 0; stage < Debriefings[team].num_stages; stage++)
		{
			convert_special_tags_to_retail(Debriefings[team].stages[stage].new_text, MAX_DEBRIEF_LEN);
		}
	}

	for (i = Num_builtin_messages; i < Num_messages; i++)
	{
		convert_special_tags_to_retail(Messages[i].message, MESSAGE_LENGTH);
	}
}

int CFred_mission_save::save_mission_file(char *pathname)
{
	char backup_name[256], savepath[MAX_PATH_LEN], *p;
	CTime t;

	t = CTime::GetCurrentTime();
	strcpy_s(The_mission.modified, t.Format("%x at %X"));

	strcpy_s(savepath, "");
	p = strrchr(pathname, '\\');
	if ( p ) {
		*p = '\0';
		strcpy_s(savepath, pathname);
		*p = '\\';
		strcat_s(savepath, "\\");
	}
	strcat_s(savepath, "saving.xxx");

	reset_parse();
	fred_parse_flag = 0;
	fp = cfopen(savepath, "wt", CFILE_NORMAL);
	if (!fp)	{
		nprintf(("Error", "Can't open mission file to save.\n"));
		return -1;
	}	

	// Goober5000
	convert_special_tags_to_retail();

	if (save_mission_info())
		err = -2;
	else if (save_plot_info())
		err = -3;
	else if (save_variables())
		err = -3;
//	else if (save_briefing_info())
//		err = -4;
	else if (save_fiction())
		err = -3;
	else if (save_cutscenes())
		err = -4;
	else if (save_cmd_briefs())
		err = -4;
	else if (save_briefing())
		err = -4;
	else if (save_debriefing())
		err = -5;
	else if (save_players())
		err = -6;
	else if (save_objects())
		err = -7;
	else if (save_wings())
		err = -8;
	else if (save_events())
		err = -9;
	else if (save_goals())
		err = -10;
	else if (save_waypoints())
		err = -11;
	else if (save_messages())
		err = -12;
	else if (save_reinforcements())
		err = -13;
	else if (save_bitmaps())
		err = -14;
	else if (save_asteroid_fields())
		err = -15;
	else if (save_music())
		err = -16;
	else {
		required_string_fred("#End");
		parse_comments(2);
		token_found = NULL;
		parse_comments();
		fout("\n");
	}

	cfclose(fp);
	if (err) {
		mprintf(("Mission saving error code #%d\n", err));

	} else {
		strcpy_s(backup_name, pathname);
		if (backup_name[strlen(backup_name) - 4] == '.')
			backup_name[strlen(backup_name) - 4] = 0;

		strcat_s(backup_name, ".bak");
		cf_attrib(pathname, 0, FILE_ATTRIBUTE_READONLY, CF_TYPE_MISSIONS);
		cf_delete(backup_name, CF_TYPE_MISSIONS);
		cf_rename(pathname, backup_name, CF_TYPE_MISSIONS);
		cf_rename(savepath, pathname, CF_TYPE_MISSIONS);
	}

	return err;
}

int CFred_mission_save::autosave_mission_file(char *pathname)
{
	char backup_name[256], name2[256];
	int i, len;
	CTime t;
	
	t = CTime::GetCurrentTime();
	strcpy_s(The_mission.modified, t.Format("%x at %X"));

	len = strlen(pathname);
	strcpy_s(backup_name, pathname);
	strcpy_s(name2, pathname);
	sprintf(backup_name + len, ".%.3d", BACKUP_DEPTH);
	cf_delete(backup_name, CF_TYPE_MISSIONS);
	for (i=BACKUP_DEPTH; i>1; i--) {
		sprintf(backup_name + len, ".%.3d", i - 1);
		sprintf(name2 + len, ".%.3d", i);
		cf_rename(backup_name, name2, CF_TYPE_MISSIONS);
	}
	
	strcpy(backup_name + len, ".001");
	reset_parse();
	fred_parse_flag = 0;
	fp = cfopen(backup_name, "wt", CFILE_NORMAL, CF_TYPE_MISSIONS);
	if (!fp)	{
		nprintf(("Error", "Can't open mission file to save.\n"));
		return -1;
	}

	// Goober5000
	convert_special_tags_to_retail();

	if (save_mission_info())
		err = -2;
	else if (save_plot_info())
		err = -3;
	else if (save_variables())
		err = -3;
//	else if (save_briefing_info())
//		err = -4;
	else if (save_fiction())
		err = -3;
	else if (save_cutscenes())
		err = -4;
	else if (save_cmd_briefs())
		err = -4;
	else if (save_briefing())
		err = -4;
	else if (save_debriefing())
		err = -5;
	else if (save_players())
		err = -6;
	else if (save_objects())
		err = -7;
	else if (save_wings())
		err = -8;
	else if (save_events())
		err = -9;
	else if (save_goals())
		err = -10;
	else if (save_waypoints())
		err = -11;
	else if (save_messages())
		err = -12;
	else if (save_reinforcements())
		err = -13;
	else if (save_bitmaps())
		err = -14;
	else if (save_asteroid_fields())
		err = -15;
	else if (save_music())
		err = -16;
	else {
		required_string_fred("#End");
		parse_comments(2);
		token_found = NULL;
		parse_comments();
		fout("\n");
	}

	cfclose(fp);
	if (err)
		mprintf(("Mission saving error code #%d\n", err));

	return err;
}

int CFred_mission_save::save_mission_info()
{
	required_string_fred("#Mission Info");
	parse_comments(0);

	required_string_fred("$Version:");
	parse_comments(2);
	fout(" %.2f", FRED_MISSION_VERSION);

	// XSTR
	required_string_fred("$Name:");
	parse_comments();
	fout_ext(" ", "%s", The_mission.name);

	required_string_fred("$Author:");
	parse_comments();
	fout(" %s", The_mission.author);

	required_string_fred("$Created:");
	parse_comments();
	fout(" %s", The_mission.created);

	required_string_fred("$Modified:");
	parse_comments();
	fout(" %s", The_mission.modified);

	required_string_fred("$Notes:");
	parse_comments();
	fout("\n%s", The_mission.notes);

	required_string_fred("$End Notes:");
	parse_comments(0);

	// XSTR
	required_string_fred("$Mission Desc:");
	parse_comments(2);
	fout_ext("\n", "%s", The_mission.mission_desc);
	fout("\n");

	required_string_fred("$end_multi_text");
	parse_comments(0);

#if 0
	if (optional_string_fred("+Game Type:"))
		parse_comments(2);
	else
		fout("\n\n+Game Type:");
	fout("\n%s", Game_types[The_mission.game_type]);	
#endif		

	if ( optional_string_fred("+Game Type Flags:")){
		parse_comments(2);
	} else {
		fout("\n+Game Type Flags:");
	}	

	fout(" %d", The_mission.game_type);

	if (optional_string_fred("+Flags:")){
		parse_comments(2);
	} else {
		fout("\n+Flags:");
	}

	fout(" %d", The_mission.flags);

	// maybe write out Nebula intensity
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		Assert(Neb2_awacs > 0.0f);
		fout("\n+NebAwacs: %f\n", Neb2_awacs);

		// storm name
		fout("\n+Storm: %s\n", Mission_parse_storm_name);
	}

	// Goober5000
	if (Format_fs2_open != FSO_FORMAT_RETAIL)
	{
		if (The_mission.contrail_threshold != CONTRAIL_THRESHOLD_DEFAULT)
		{
			fout("\n$Contrail Speed Threshold: %d\n", The_mission.contrail_threshold);
		}
	}

	// For multiplayer missions -- write out the number of player starts and number of respawns
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		if (optional_string_fred("+Num Players:"))
			parse_comments(2);
		else
			fout("\n+Num Players:");

		fout(" %d", Player_starts);

		if (optional_string_fred("+Num Respawns:"))
			parse_comments(2);
		else
			fout("\n+Num Respawns:");

		fout(" %d", The_mission.num_respawns);

		if ( Format_fs2_open != FSO_FORMAT_RETAIL ) {
			if (optional_string_fred("+Max Respawn Time:"))
				parse_comments(2);
			else {
				fso_comment_push(";;FSO 3.6.11;;");
				fout_version("\n+Max Respawn Time:");
				fso_comment_pop();
			}

			fout(" %d", The_mission.max_respawn_delay);
		}
		else {
			bypass_comment(";;FSO 3.6.11;; +Max Respawn Time:");
		}
	}

	if ( Format_fs2_open == FSO_FORMAT_RETAIL )
	{
		if ( optional_string_fred("+Red Alert:"))
			parse_comments(2);
		else
			fout("\n+Red Alert:");

		fout(" %d", (The_mission.flags & MISSION_FLAG_RED_ALERT) ? 1 : 0);
	}

	if ( Format_fs2_open == FSO_FORMAT_RETAIL )
	{
		if ( optional_string_fred("+Scramble:"))
			parse_comments(2);
		else
			fout("\n+Scramble:");

		fout(" %d", (The_mission.flags & MISSION_FLAG_SCRAMBLE) ? 1 : 0);
	}

	if ( optional_string_fred("+Disallow Support:")){
		parse_comments(2);
	} else {
		fout("\n+Disallow Support:");
	}
	// this is compatible with non-SCP variants - Goober5000
	fout(" %d", (The_mission.support_ships.max_support_ships == 0)?1:0 );

	// here be WMCoolmon's hull and subsys repair stuff
	if (Format_fs2_open != FSO_FORMAT_RETAIL)
	{
		if ( optional_string_fred("+Hull Repair Ceiling:")) {
			parse_comments(2);
		} else {
			fout("\n+Hull Repair Ceiling:");
		}
		fout(" %f", The_mission.support_ships.max_hull_repair_val);

		if ( optional_string_fred("+Subsystem Repair Ceiling:")) {
			parse_comments(2);
		} else {
			fout("\n+Subsystem Repair Ceiling:");
		}
		fout(" %f", The_mission.support_ships.max_subsys_repair_val);
	}

	if (Mission_all_attack) {
		if (optional_string_fred("+All Teams Attack")){
			parse_comments();
		} else {
			fout("\n+All Teams Attack");
		}
	}

	if (Entry_delay_time) {
		if (optional_string_fred("+Player Entry Delay:"))
			parse_comments(2);
		else
			fout("\n\n+Player Entry Delay:");

		fout("\n%f", f2fl(Entry_delay_time));
	}

	if (optional_string_fred("+Viewer pos:")){
		parse_comments(2);
	} else {
		fout("\n\n+Viewer pos:");
	}

	save_vector(view_pos);

	if (optional_string_fred("+Viewer orient:")){
		parse_comments();
	} else {
		fout("\n+Viewer orient:");
	}

	save_matrix(view_orient);

	// squadron info
	if(!(The_mission.game_type & MISSION_TYPE_MULTI) && (strlen(The_mission.squad_name) > 0)){
		// squad name
		fout("\n+SquadReassignName: %s", The_mission.squad_name);

		// maybe squad logo
		if(strlen(The_mission.squad_filename) > 0){
			fout("\n+SquadReassignLogo: %s", The_mission.squad_filename);
		}
	}

	// Goober5000 - special wing info
	if (Format_fs2_open != FSO_FORMAT_RETAIL)
	{
		int i;
		fout("\n");

		// starting wings
		if (strcmp(Starting_wing_names[0], "Alpha") || strcmp(Starting_wing_names[1], "Beta") || strcmp(Starting_wing_names[2], "Gamma"))
		{
			fout("\n$Starting wing names: ( ");

			for (i=0; i<MAX_STARTING_WINGS; i++)
			{
				fout ("\"%s\" ", Starting_wing_names[i]);
			}

			fout (")");
		}

		// squadron wings
		if (strcmp(Squadron_wing_names[0], "Alpha") || strcmp(Squadron_wing_names[1], "Beta") || strcmp(Squadron_wing_names[2], "Gamma") || strcmp(Squadron_wing_names[3], "Delta") || strcmp(Squadron_wing_names[4], "Epsilon"))
		{
			fout("\n$Squadron wing names: ( ");

			for (i=0; i<MAX_SQUADRON_WINGS; i++)
			{
				fout("\"%s\" ", Squadron_wing_names[i]);
			}

			fout(")");
		}

		// tvt wings
		if (strcmp(TVT_wing_names[0], "Alpha") || strcmp(TVT_wing_names[1], "Zeta"))
		{
			fout("\n$Team-versus-team wing names: ( ");

			for (i=0; i<MAX_TVT_WINGS; i++)
			{
				fout("\"%s\" ", TVT_wing_names[i]);
			}

			fout(")");
		}
	}

	// Phreak's loading screen stuff
	if (Format_fs2_open != FSO_FORMAT_RETAIL)
	{
		if (strlen(The_mission.loading_screen[GR_640]) > 0)
		{
			fout("\n\n$Load Screen 640:\t%s",The_mission.loading_screen[GR_640]);
		}

		if (strlen(The_mission.loading_screen[GR_1024]) > 0)
		{
			fout("\n$Load Screen 1024:\t%s",The_mission.loading_screen[GR_1024]);
		}
	}

	// Phreak's skybox stuff
	if (strlen(The_mission.skybox_model) > 0)
	{
		char out_str[NAME_LENGTH];
		char *period;

		// kill off any extension, we will add one here
		strcpy_s(out_str, The_mission.skybox_model);
		period = strrchr(out_str, '.');
		if (period != NULL)
			*period = 0;

		if (optional_string_fred("$Skybox Model:")) {
			parse_comments(2);
			fout(" %s.pof", out_str);
		} else {
			fso_comment_push(";;FSO 3.6.0;;");
			fout_version("\n\n$Skybox Model: %s.pof", out_str);
			fso_comment_pop();
		}
	} else {
		bypass_comment(";;FSO 3.6.0;; $Skybox Model:");
	}

	// are skybox flags in use?
	if (The_mission.skybox_flags != DEFAULT_NMODEL_FLAGS) {
		//char out_str[4096];
		if (optional_string_fred("+Skybox Flags:")) {
			parse_comments(1);
			fout( " %d", The_mission.skybox_flags);
		} else {
			fso_comment_push(";;FSO 3.6.11;;");
			fout_version("\n+Skybox Flags: %d", The_mission.skybox_flags);
			fso_comment_pop();
		}
	}
	else {
		bypass_comment(";;FSO 3.6.11;; +Skybox Flags:");
	}

	// Goober5000's AI profile stuff
	int profile_index = (The_mission.ai_profile - Ai_profiles);
	Assert(profile_index >= 0 && profile_index < MAX_AI_PROFILES);

	if (optional_string_fred("$AI Profile:")) {
		parse_comments(2);
		fout(" %s", The_mission.ai_profile->profile_name);
	} else {
		fso_comment_push(";;FSO 3.6.9;;");
		fout_version("\n\n$AI Profile: %s", The_mission.ai_profile->profile_name);
		fso_comment_pop();
	}

	// sound environment (EFX/EAX) - taylor
	sound_env *m_env = &The_mission.sound_environment;
	if ( (m_env->id >= 0) && (m_env->id < (int)EFX_presets.size()) ) {
		EFXREVERBPROPERTIES *prop = &EFX_presets[m_env->id];

		fso_comment_push(";;FSO 3.6.12;;");

		fout_version("\n\n$Sound Environment: %s", prop->name.c_str());

		if (m_env->volume != prop->flGain) {
			fout_version("\n+Volume: %f", m_env->volume);
		}

		if (m_env->damping != prop->flDecayHFRatio) {
			fout_version("\n+Damping: %f", m_env->damping);
		}

		if (m_env->decay != prop->flDecayTime) {
			fout_version("\n+Decay Time: %f", m_env->decay);
		}

		fso_comment_pop();
	}

	return err;
}

int CFred_mission_save::save_plot_info()
{
	if ( Format_fs2_open == FSO_FORMAT_RETAIL )
	{
		if (optional_string_fred("#Plot Info"))
		{
			parse_comments(2);

			// XSTR
			required_string_fred("$Tour:");
			parse_comments(2);
			fout_ext(" ", "Blah");

			required_string_fred("$Pre-Briefing Cutscene:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Pre-Mission Cutscene:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Next Mission Success:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Next Mission Partial:");
			parse_comments();
			fout(" Blah");

			required_string_fred("$Next Mission Failure:");
			parse_comments();
			fout(" Blah");
		}
		else
		{
			fout("\n\n#Plot Info\n\n");

			fout("$Tour: ");
			fout_ext(NULL, "Blah");
			fout("\n");
			fout("$Pre-Briefing Cutscene: Blah\n");
			fout("$Pre-Mission Cutscene: Blah\n");
			fout("$Next Mission Success: Blah\n");
			fout("$Next Mission Partial: Blah\n");
			fout("$Next Mission Failure: Blah\n");

			fout("\n");
		}
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_cutscenes()
{	
	char type[NAME_LENGTH]; 
	char out[MULTITEXT_LENGTH];

	// Let's just assume it has them for now - 
	if (!(The_mission.cutscenes.empty()) ) {
		if (Format_fs2_open != FSO_FORMAT_RETAIL) {
			if (optional_string_fred("#Cutscenes")) {
				parse_comments(2);
			}
			else {
				fout_version("\n\n#Cutscenes\n\n");
			}

			for (uint i = 0; i < The_mission.cutscenes.size(); i++) {
				if ( strlen(The_mission.cutscenes[i].cutscene_name) ) {
					// determine the name of this cutscene type
					switch (The_mission.cutscenes[i].type) {
						case MOVIE_PRE_FICTION:
							strcpy_s(type, "$Fiction Viewer Cutscene:");  
							break; 
						case MOVIE_PRE_CMD_BRIEF:
							strcpy_s(type, "$Command Brief Cutscene:");  
							break; 
						case MOVIE_PRE_BRIEF:
							strcpy_s(type, "$Briefing Cutscene:");  
							break; 
						case MOVIE_PRE_GAME:
							strcpy_s(type, "$Pre-game Cutscene:");  
							break; 
						case MOVIE_PRE_DEBRIEF:
							strcpy_s(type, "$Debriefing Cutscene:");  
							break; 
						default: 
							Int3(); 
							continue; 
					}
					
					if (optional_string_fred(type)) {
						parse_comments();
						fout(" %s", The_mission.cutscenes[i].cutscene_name); 
					}
					else {
						fout_version("%s %s\n", type, The_mission.cutscenes[i].cutscene_name); 
					}

					required_string_fred("+formula:");
					parse_comments(); 
					convert_sexp_to_string(The_mission.cutscenes[i].formula, out, SEXP_SAVE_MODE, 4096);
					fout(" %s", out);
				}
			}
			required_string_fred("#end"); 
			parse_comments();
		}
		else {
			MessageBox(NULL, "Warning: This mission contains cutscene data, but you are saving in the retail mission format. This infomration will be lost", "Incompatibility with retail mission format", MB_OK);
		}
	}

	fso_comment_pop(true);
	return err; 
}

int CFred_mission_save::save_fiction()
{
	if (mission_has_fiction())
	{
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if (optional_string_fred("#Fiction Viewer"))
				parse_comments();
			else
				fout("\n\n#Fiction Viewer");

			fout("\n");

			// save file
			required_string_fred("$File:");
			parse_comments();
			fout(" %s", fiction_file());

			// save font
			if (strlen(fiction_font()) > 0)
			{
				if (optional_string_fred("$Font:"))
					parse_comments();
				else
					fout("\n$Font:");
				fout(" %s", fiction_font());
			}
			else
				optional_string_fred("$Font:");
		}
		else
		{
			MessageBox(NULL, "Warning: This mission contains fiction viewer data, but you are saving in the retail mission format.", "Incompatibility with retail mission format", MB_OK);
		}
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_cmd_brief()
{
	int stage;

	stage = 0;
	required_string_fred("#Command Briefing");
	parse_comments(2);

	if (The_mission.game_type & MISSION_TYPE_MULTI)
		return err;  // no command briefings allowed in multiplayer missions.

	for (stage=0; stage<Cur_cmd_brief->num_stages; stage++) {
		required_string_fred("$Stage Text:");
		parse_comments(2);

		// XSTR
		fout_ext("\n", "%s", Cur_cmd_brief->stage[stage].text);

		required_string_fred("$end_multi_text", "$Stage Text:");
		parse_comments();

		required_string_fred("$Ani Filename:");
		parse_comments();
		fout(" %s", Cur_cmd_brief->stage[stage].ani_filename);

		required_string_fred("+Wave Filename:", "$Ani Filename:");
		parse_comments();
		fout(" %s", Cur_cmd_brief->stage[stage].wave_filename);

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_cmd_briefs()
{
	int i;

	for (i=0; i<Num_teams; i++) {
		Cur_cmd_brief = &Cmd_briefs[i];
		save_cmd_brief();
	}

	return err;
}

int CFred_mission_save::save_briefing()
{
	int			i,j,k, nb;
	char			out[4096];
	brief_stage	*bs;
	brief_icon	*bi;

	for ( nb = 0; nb < Num_teams; nb++ ) {

		required_string_fred("#Briefing");
		parse_comments(2);

		required_string_fred("$start_briefing");
		parse_comments();

		Assert(Briefings[nb].num_stages <= MAX_BRIEF_STAGES);
		required_string_fred("$num_stages:");
		parse_comments();
		fout(" %d", Briefings[nb].num_stages);

		for (i=0; i<Briefings[nb].num_stages; i++) {
			bs = &Briefings[nb].stages[i];

			required_string_fred("$start_stage");
			parse_comments();

			required_string_fred("$multi_text");
			parse_comments();

			// XSTR
			sprintf(out,"%s", bs->new_text);
			fout_ext("\n", out);

			required_string_fred("$end_multi_text", "$start_stage");
			parse_comments();

			if (!drop_white_space(bs->voice)[0]){
				strcpy_s(bs->voice, "None");
			}

			required_string_fred("$voice:");
			parse_comments();
			fout(" %s", bs->voice);

			required_string_fred("$camera_pos:");
			parse_comments();
			save_vector(bs->camera_pos);
			
			required_string_fred("$camera_orient:");
			parse_comments();
			save_matrix(bs->camera_orient);

			required_string_fred("$camera_time:");
			parse_comments();
			fout(" %d", bs->camera_time);

			required_string_fred("$num_lines:");
			parse_comments();
			fout(" %d", bs->num_lines);

			for (k=0; k<bs->num_lines; k++) {
				required_string_fred("$line_start:");
				parse_comments();
				fout(" %d", bs->lines[k].start_icon);

				required_string_fred("$line_end:");
				parse_comments();
				fout(" %d", bs->lines[k].end_icon);

				fso_comment_pop();
			}

			required_string_fred("$num_icons:");
			parse_comments();
			Assert(bs->num_icons <= MAX_STAGE_ICONS );
			fout(" %d", bs->num_icons);

			required_string_fred("$Flags:");
			parse_comments();
			fout(" %d", bs->flags);

			required_string_fred("$Formula:");
			parse_comments();
			convert_sexp_to_string(bs->formula, out, SEXP_SAVE_MODE, 4096);
			fout(" %s", out);

			for ( j = 0; j < bs->num_icons; j++ ) {
				bi = &bs->icons[j];

				required_string_fred("$start_icon");
				parse_comments();

				required_string_fred("$type:");
				parse_comments();
				fout(" %d", bi->type);

				required_string_fred("$team:");
				parse_comments();
				fout(" %s", Iff_info[bi->team].iff_name);

				required_string_fred("$class:");
				parse_comments();
				if (bi->ship_class < 0)
					bi->ship_class = 0;

				fout(" %s", Ship_info[bi->ship_class].name);

				required_string_fred("$pos:");
				parse_comments();
				save_vector(bi->pos);

				if (drop_white_space(bi->label)[0]) {
					if (optional_string_fred("$label:"))
						parse_comments();
					else
						fout("\n$label:");

					fout(" %s", bi->label);
				}

				if (optional_string_fred("+id:"))
					parse_comments(); 
				else
					fout("\n+id:");

				fout(" %d", bi->id);

				required_string_fred("$hlight:");
				parse_comments();
				fout(" %d", (bi->flags & BI_HIGHLIGHT)?1:0 );

				if (Format_fs2_open != FSO_FORMAT_RETAIL)
				{
					required_string_fred("$mirror:");
					parse_comments();
					fout(" %d", (bi->flags & BI_MIRROR_ICON)?1:0 );
				}

				required_string_fred("$multi_text");
				parse_comments();

//				sprintf(out,"\n%s", bi->text);
//				fout(out);

				required_string_fred("$end_multi_text");
				parse_comments();

				required_string_fred("$end_icon");
				parse_comments();

				fso_comment_pop();
			}

			required_string_fred("$end_stage");
			parse_comments();

			fso_comment_pop();
		}
		required_string_fred("$end_briefing");
		parse_comments();

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_debriefing()
{
	int j, i;
	char out[4096];

	for ( j = 0; j < Num_teams; j++ ) {

		Debriefing = &Debriefings[j];

		required_string_fred("#Debriefing_info");
		parse_comments(2);

		required_string_fred("$Num stages:");
		parse_comments(2);
		fout(" %d", Debriefing->num_stages);

		for (i=0; i<Debriefing->num_stages; i++) {
			required_string_fred("$Formula:");
			parse_comments(2);
			convert_sexp_to_string(Debriefing->stages[i].formula, out, SEXP_SAVE_MODE, 4096);
			fout(" %s", out);

			// XSTR
			required_string_fred("$Multi text");
			parse_comments();
			fout_ext("\n   ", "%s", Debriefing->stages[i].new_text);

			required_string_fred("$end_multi_text");
			parse_comments();

			if (!drop_white_space(Debriefing->stages[i].voice)[0]){
				strcpy_s(Debriefing->stages[i].voice, "None");
			}

			required_string_fred("$Voice:");
			parse_comments();
			fout(" %s", Debriefing->stages[i].voice);

			// XSTR
			required_string_fred("$Recommendation text:");
			parse_comments();
			fout_ext("\n   ", "%s", Debriefing->stages[i].new_recommendation_text);

			required_string_fred("$end_multi_text");
			parse_comments();

			fso_comment_pop();
		}
	}

	fso_comment_pop(true);

	return err;
}

int sexp_variable_block_count();
// save variables
int CFred_mission_save::save_variables()
{
	char *type;
	char number[] = "number";
	char string[] = "string";
	char block[] = "block";
	int i;
	int num_block_vars = 0;

	// sort sexp_variables
	sexp_variable_sort();

	// get count
	int num_variables = sexp_variable_count();

	if (Format_fs2_open == FSO_FORMAT_RETAIL) {
		generate_special_explosion_block_variables();
		num_block_vars = num_block_variables();
	}
	int total_variables = num_variables + num_block_vars;

	if (total_variables > 0) {

		// write 'em out
		required_string_fred("#Sexp_variables");
		parse_comments(2);

		required_string_fred("$Variables:");
		parse_comments(2);

		fout("\n(");
//		parse_comments();

		for (i=0; i<num_variables; i++) {
			if (Sexp_variables[i].type & SEXP_VARIABLE_NUMBER) {
				type = number;
			} else {
				type = string;
			}
			// index "var name" "default" "type"
			fout("\n\t\t%d\t\t\"%s\"\t\t\"%s\"\t\t\"%s\"", i, Sexp_variables[i].variable_name, Sexp_variables[i].text, type);

			// persistent and network variables
			if (Format_fs2_open != FSO_FORMAT_RETAIL)
			{
				// Network variable - Karajorma
				if (Sexp_variables[i].type & SEXP_VARIABLE_NETWORK) {
					fout("\t\t\"%s\"", "network-variable");
				}

				// player-persistent - Goober5000
				if (Sexp_variables[i].type & SEXP_VARIABLE_PLAYER_PERSISTENT) {
					fout("\t\t\"%s\"", "player-persistent");
				// campaign-persistent - Goober5000
				} else if (Sexp_variables[i].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT) {
					fout("\t\t\"%s\"", "campaign-persistent");
				}
			}

//			parse_comments();
		}

		for (i=MAX_SEXP_VARIABLES-num_block_vars; i<MAX_SEXP_VARIABLES; i++) {
			type = block;
			fout("\n\t\t%d\t\t\"%s\"\t\t\"%s\"\t\t\"%s\"", i, Block_variables[i].variable_name, Block_variables[i].text, type);
		}

		fout("\n)");

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}


int CFred_mission_save::save_players()
{
	int i, j;
	int var_idx;
	int used_pool[MAX_WEAPON_TYPES];

	// write out alternate name list
	if(Mission_alt_type_count > 0){
		fout("\n\n#Alternate Types:\n");

		// write them all out
		for(i=0; i<Mission_alt_type_count; i++){
			fout("$Alt: %s\n", Mission_alt_types[i]);
		}

		// end
		fout("\n#end\n");
	}

	// write out callsign list
	if(Mission_callsign_count > 0){
		fout("\n\n#Callsigns:\n");

		// write them all out
		for(i=0; i<Mission_callsign_count; i++){
			fout("$Callsign: %s\n", Mission_callsigns[i]);
		}

		// end
		fout("\n#end\n");
	}

	required_string_fred("#Players");
	parse_comments(2);
	fout("\t\t;! %d total\n", Player_starts);

	for (i=0; i<Num_teams; i++) {
		required_string_fred("$Starting Shipname:");
		parse_comments();
		Assert(Player_start_shipnum >= 0);
		fout(" %s", Ships[Player_start_shipnum].ship_name);
		
		required_string_fred("$Ship Choices:");
		parse_comments();
		fout(" (\n");

		for (j=0; j<Team_data[i].num_ship_choices; j++) {
			// Check to see if a variable name should be written for the class rather than a number
			if (strlen(Team_data[i].ship_list_variables[j])) {
				var_idx = get_index_sexp_variable_name(Team_data[i].ship_list_variables[j]);
				Assert (var_idx > -1 && var_idx < MAX_SEXP_VARIABLES); 
			
				fout("\t@%s\t", Sexp_variables[var_idx].variable_name);
			}
			else {
				fout("\t\"%s\"\t", Ship_info[Team_data[i].ship_list[j]].name); 
			}

			// Now check if we should write a variable or a number for the amount of ships available
			if (strlen(Team_data[i].ship_count_variables[j])) {
				var_idx = get_index_sexp_variable_name(Team_data[i].ship_count_variables[j]);
				Assert (var_idx > -1 && var_idx < MAX_SEXP_VARIABLES); 
			
				fout("@%s\n", Sexp_variables[var_idx].variable_name);			
			}
			else {
				fout("%d\n", Team_data[i].ship_count[j]);
			}
		}

		fout(")");

		if (optional_string_fred("+Weaponry Pool:", "$Starting Shipname:")){
			parse_comments(2);
		} else {
			fout("\n\n+Weaponry Pool:");
		}

		fout(" (\n");
		generate_weaponry_usage_list(i, used_pool); 
		for (j=0; j<Team_data[i].num_weapon_choices; j++) {
			// first output the weapon name or a variable that sets it 
			if (strlen(Team_data[i].weaponry_pool_variable[j])) {
				var_idx = get_index_sexp_variable_name(Team_data[i].weaponry_pool_variable[j]);
				Assert (var_idx > -1 && var_idx < MAX_SEXP_VARIABLES); 

				fout("\t@%s\t", Sexp_variables[var_idx].variable_name); 
			}
			else {
				fout("\t\"%s\"\t", Weapon_info[Team_data[i].weaponry_pool[j]].name);
			}

			// now output the amount of this weapon or a variable that sets it. If this weapon is in the used pool and isn't
			// set by a variable we should add the amount of weapons used by the wings to it and zero the entry so we know 
			// that we have dealt with it
			if (strlen(Team_data[i].weaponry_amount_variable[j])) {
				var_idx = get_index_sexp_variable_name(Team_data[i].weaponry_amount_variable[j]);
				Assert (var_idx > -1 && var_idx < MAX_SEXP_VARIABLES); 

				fout ("@%s\n", Sexp_variables[var_idx].variable_name); 			
			}
			else {
				if (strlen(Team_data[i].weaponry_pool_variable[j])) {
					fout ("%d\n", Team_data[i].weaponry_count[j]);
				}
				else {
					fout ("%d\n", Team_data[i].weaponry_count[j] + used_pool[Team_data[i].weaponry_pool[j]]);
					used_pool[Team_data[i].weaponry_pool[j]] = 0; 
				}
			}
		}

		// now we add anything left in the used pool as a static entry
		for (j=0; j<Num_weapon_types; j++){
			if (used_pool[j] > 0){
				fout("\t\"%s\"\t%d\n", Weapon_info[j].name, used_pool[j]);
			}
		}

		fout(")");

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

// Goober5000
void CFred_mission_save::save_single_dock_instance(ship *shipp, dock_instance *dock_ptr)
{
	Assert(shipp && dock_ptr);
	Assert(dock_ptr->docked_objp->type == OBJ_SHIP || dock_ptr->docked_objp->type == OBJ_START);

	// get ships and objects
	object *objp = &Objects[shipp->objnum];
	object *other_objp = dock_ptr->docked_objp;
	ship *other_shipp = &Ships[other_objp->instance];

	// write other ship
	if (optional_string_fred("+Docked With:", "$Name:"))
		parse_comments();
	else
		fout("\n+Docked With:");
	fout(" %s", other_shipp->ship_name);


	// Goober5000 - hm, Volition seems to have reversed docker and dockee here

	// write docker (actually dockee) point
	required_string_fred("$Docker Point:", "$Name:");
	parse_comments();
	fout(" %s", model_get_dock_name(Ship_info[other_shipp->ship_info_index].model_num, dock_find_dockpoint_used_by_object(other_objp, objp)));

	// write dockee (actually docker) point
	required_string_fred("$Dockee Point:", "$Name:");
	parse_comments();
	fout(" %s", model_get_dock_name(Ship_info[shipp->ship_info_index].model_num, dock_find_dockpoint_used_by_object(objp, other_objp)));

	fso_comment_pop(true);
}

int CFred_mission_save::save_objects()
{
	char out[4096];
	int i, j, k, z;
	ai_info *aip;
	object *objp;
	ship_info *sip;
	
	required_string_fred("#Objects");
	parse_comments(2);
	fout("\t\t;! %d total\n", ship_get_num_ships() );

	for (i=z=0; i<MAX_SHIPS; i++) {
		if (Ships[i].objnum < 0){
			continue;
		}

		j = Objects[Ships[i].objnum].type;
		if ((j != OBJ_SHIP) && (j != OBJ_START)){
			continue;
		}

		objp = &Objects[Ships[i].objnum];
		sip = &Ship_info[Ships[i].ship_info_index];
		required_string_either_fred("$Name:", "#Wings");
		required_string_fred("$Name:");
		parse_comments(z ? 2 : 1);
		fout(" %s\t\t;! Object #%d\n", Ships[i].ship_name, i);

		required_string_fred("$Class:");
		parse_comments(0);
		fout(" %s", Ship_info[Ships[i].ship_info_index].name);

		//alt classes stuff
		if (Format_fs2_open != FSO_FORMAT_RETAIL) {
			if ((int)Ships[i].s_alt_classes.size()) {
				for (k = 0; k < (int)Ships[i].s_alt_classes.size() ; k++) {
					// is this a variable?
					if (Ships[i].s_alt_classes[k].variable_index != -1) {
						fout_version("\n;;FSO 3.6.10;; $Alt Ship Class: @%s", Sexp_variables[Ships[i].s_alt_classes[k].variable_index].variable_name);  
					}
					else {
						fout_version("\n;;FSO 3.6.10;; $Alt Ship Class: \"%s\"", Ship_info[Ships[i].s_alt_classes[k].ship_class].name);
					}

					// default class?					
					if (Ships[i].s_alt_classes[k].default_to_this_class) {
						fout_version("\n;;FSO 3.6.10;; +Default Class:");
					}
				}

			}
		}

		// optional alternate type name
		if(strlen(Fred_alt_names[i])){
			fout("\n$Alt: %s\n", Fred_alt_names[i]);
		}

		// optional callsign
		if(strlen(Fred_callsigns[i])){
			fout("\n$Callsign: %s\n", Fred_callsigns[i]);
		}

		required_string_fred("$Team:");
		parse_comments();
		fout(" %s", Iff_info[Ships[i].team].iff_name);

		required_string_fred("$Location:");
		parse_comments();
		save_vector(Objects[Ships[i].objnum].pos);

		required_string_fred("$Orientation:");
		parse_comments();
		save_matrix(Objects[Ships[i].objnum].orient);

		if (Format_fs2_retail)
		{
			required_string_fred("$IFF:");
			parse_comments();
			fout(" %s", "IFF 1");
		}

		Assert(Ships[i].ai_index >= 0);
		aip = &Ai_info[Ships[i].ai_index];

		required_string_fred("$AI Behavior:");
		parse_comments();
		fout(" %s", Ai_behavior_names[aip->behavior]);

		if (Ships[i].weapons.ai_class != Ship_info[Ships[i].ship_info_index].ai_class) {
			if (optional_string_fred("+AI Class:", "$Name:"))
				parse_comments();
			else
				fout("\n+AI Class:");

			fout(" %s", Ai_class_names[Ships[i].weapons.ai_class]);
		}

		save_ai_goals(Ai_info[Ships[i].ai_index].goals, i);

		// XSTR
		required_string_fred("$Cargo 1:");
		parse_comments();
		fout_ext(" ", "%s", Cargo_names[Ships[i].cargo1]);

		save_common_object_data(&Objects[Ships[i].objnum], &Ships[i]);

		if (Ships[i].wingnum >= 0){
			Ships[i].arrival_location = ARRIVE_AT_LOCATION;
		}

		required_string_fred("$Arrival Location:");
		parse_comments();
		fout(" %s", Arrival_location_names[Ships[i].arrival_location]);

		if (Ships[i].arrival_location != ARRIVE_AT_LOCATION)
		{
			if (optional_string_fred("+Arrival Distance:", "$Name:")){
				parse_comments();
			} else {
				fout("\n+Arrival Distance:");
			}

			fout(" %d", Ships[i].arrival_distance);
			if (optional_string_fred("$Arrival Anchor:", "$Name:")){
				parse_comments();
			} else {
				fout("\n$Arrival Anchor:");
			}

			z = Ships[i].arrival_anchor;
			if (z & SPECIAL_ARRIVAL_ANCHOR_FLAG)
			{
				// get name
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, z, Format_fs2_retail);

				// save it
				fout(" %s", tmp);
			}
			else if (z >= 0)
			{
				fout(" %s", Ships[z].ship_name);
			}
			else
			{
				fout(" <error>");
			}
		}

		// Goober5000
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if ((Ships[i].arrival_location == ARRIVE_FROM_DOCK_BAY) && (Ships[i].arrival_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Ships[i].arrival_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Arrival Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Ships[i].arrival_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Ships[i].arrival_delay)
		{
			if (optional_string_fred("+Arrival Delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Arrival Delay:");

			fout(" %d", Ships[i].arrival_delay);
		}

		required_string_fred("$Arrival Cue:");
		parse_comments();
		convert_sexp_to_string(Ships[i].arrival_cue, out, SEXP_SAVE_MODE, 4096);
		fout(" %s", out);

		required_string_fred("$Departure Location:");
		parse_comments();
		fout(" %s", Departure_location_names[Ships[i].departure_location]);

		if ( Ships[i].departure_location != DEPART_AT_LOCATION )
		{
			required_string_fred("$Departure Anchor:");
			parse_comments();
			
			if ( Ships[i].departure_anchor >= 0 )
				fout(" %s", Ships[Ships[i].departure_anchor].ship_name );
			else
				fout(" <error>");
		}

		// Goober5000
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if ((Ships[i].departure_location == DEPART_AT_DOCK_BAY) && (Ships[i].departure_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Ships[i].departure_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Departure Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Ships[i].departure_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Ships[i].departure_delay)
		{
			if (optional_string_fred("+Departure delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Departure delay:");

			fout(" %d", Ships[i].departure_delay);
		}

		required_string_fred("$Departure Cue:");
		parse_comments();
		convert_sexp_to_string(Ships[i].departure_cue, out, SEXP_SAVE_MODE, 4096);
		fout(" %s", out);

		required_string_fred("$Determination:");
		parse_comments();
		fout(" 10"); // dummy value for backwards compatibility

		if (optional_string_fred("+Flags:", "$Name:")) {
			parse_comments();
			fout (" (");
		} else
			fout("\n+Flags: (");

		if (Ships[i].flags & SF_CARGO_REVEALED)
			fout(" \"cargo-known\"");
		if (Ships[i].flags & SF_IGNORE_COUNT)
			fout(" \"ignore-count\"");
		if (objp->flags & OF_PROTECTED)
			fout(" \"protect-ship\"");
		if (Ships[i].flags & SF_REINFORCEMENT)
			fout(" \"reinforcement\"");
		if (objp->flags & OF_NO_SHIELDS)
			fout(" \"no-shields\"");
		if (Ships[i].flags & SF_ESCORT)
			fout(" \"escort\"");
		if (objp->type == OBJ_START)
			fout(" \"player-start\"");
		if (Ships[i].flags & SF_NO_ARRIVAL_MUSIC)
			fout(" \"no-arrival-music\"");
		if (Ships[i].flags & SF_NO_ARRIVAL_WARP)
			fout(" \"no-arrival-warp\"");
		if (Ships[i].flags & SF_NO_DEPARTURE_WARP)
			fout(" \"no-departure-warp\"");
		if (Ships[i].flags & SF_LOCKED)
			fout(" \"locked\"");
		if (Objects[Ships[i].objnum].flags & OF_INVULNERABLE)
			fout(" \"invulnerable\"");
		if (Ships[i].flags & SF_HIDDEN_FROM_SENSORS)
			fout(" \"hidden-from-sensors\"");
		if (Ships[i].flags & SF_SCANNABLE)
			fout(" \"scannable\"");
		if (Ai_info[Ships[i].ai_index].ai_flags & AIF_KAMIKAZE)
			fout(" \"kamikaze\"");
		if (Ai_info[Ships[i].ai_index].ai_flags & AIF_NO_DYNAMIC)
			fout(" \"no-dynamic\"");
		if (Ships[i].flags & SF_RED_ALERT_STORE_STATUS)
			fout(" \"red-alert-carry\"");
		if (objp->flags & OF_BEAM_PROTECTED)
			fout(" \"beam-protect-ship\"");
		if (objp->flags & OF_FLAK_PROTECTED)
			fout(" \"flak-protect-ship\"");
		if (objp->flags & OF_LASER_PROTECTED)
			fout(" \"laser-protect-ship\"");
		if (objp->flags & OF_MISSILE_PROTECTED)
			fout(" \"missile-protect-ship\"");
		if (Ships[i].ship_guardian_threshold != 0)
			fout(" \"guardian\"");
		if (objp->flags & OF_SPECIAL_WARPIN)
			fout(" \"special-warp\"");
		if (Ships[i].flags & SF_VAPORIZE)
			fout(" \"vaporize\"");
		if (Ships[i].flags2 & SF2_STEALTH)
			fout(" \"stealth\"");
		if (Ships[i].flags2 & SF2_FRIENDLY_STEALTH_INVIS)
			fout(" \"friendly-stealth-invisible\"");
		if (Ships[i].flags2 & SF2_DONT_COLLIDE_INVIS)
			fout(" \"don't-collide-invisible\"");
		fout(" )");

		// flags2 added by Goober5000 --------------------------------
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if (optional_string_fred("+Flags2:", "$Name:")) {
				parse_comments();
				fout (" (");
			} else
				fout("\n+Flags2: (");

			if (Ships[i].flags2 & SF2_PRIMITIVE_SENSORS)
				fout(" \"primitive-sensors\"");
			if (Ships[i].flags2 & SF2_NO_SUBSPACE_DRIVE)
				fout(" \"no-subspace-drive\"");
			if (Ships[i].flags2 & SF2_NAVPOINT_CARRY)
				fout(" \"nav-carry-status\"");
			if (Ships[i].flags2 & SF2_AFFECTED_BY_GRAVITY)
				fout(" \"affected-by-gravity\"");
			if (Ships[i].flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING)
				fout(" \"toggle-subsystem-scanning\"");
			if (Objects[i].flags & OF_TARGETABLE_AS_BOMB)
				fout(" \"targetable-as-bomb\"");
			if (Ships[i].flags2 & SF2_NO_BUILTIN_MESSAGES)
				fout(" \"no-builtin-messages\"");
			if (Ships[i].flags2 & SF2_PRIMARIES_LOCKED)
				fout(" \"primaries-locked\"");
			if (Ships[i].flags2 & SF2_SECONDARIES_LOCKED)
				fout(" \"secondaries-locked\"");
			if (Ships[i].flags2 & SF2_NO_DEATH_SCREAM)
				fout(" \"no-death-scream\"");
			if (Ships[i].flags2 & SF2_ALWAYS_DEATH_SCREAM)
				fout(" \"always-death-scream\"");
			if (Ships[i].flags2 & SF2_NAVPOINT_NEEDSLINK)
				fout(" \"nav-needslink\"");
			if (Ships[i].flags2 & SF2_HIDE_SHIP_NAME)
				fout(" \"hide-ship-name\"");
			if (Ships[i].flags2 & SF2_SET_CLASS_DYNAMICALLY)
				fout(" \"set-class-dynamically\"");
			if (Ships[i].flags2 & SF2_LOCK_ALL_TURRETS_INITIALLY)
				fout(" \"lock-all-turrets\"");
			if (Ships[i].flags2 & SF2_AFTERBURNER_LOCKED)
				fout(" \"afterburners-locked\"");
			if (Ships[i].flags2 & SF2_FORCE_SHIELDS_ON)
				fout(" \"force-shields-on\"");
			fout(" )");
		}
		// -----------------------------------------------------------

		fout("\n+Respawn priority: %d", Ships[i].respawn_priority);	// HA!  Newline added by Goober5000

		if (Ships[i].flags & SF_ESCORT) {
			if (optional_string_fred("+Escort priority:", "$Name:")) {
				parse_comments();
			} else {
				fout("\n+Escort priority:");
			}

			fout(" %d", Ships[i].escort_priority);
		}

		// special explosions
		if (Format_fs2_open != FSO_FORMAT_RETAIL) {
			if (Ships[i].use_special_explosion) {
				if (optional_string_fred("$Special Explosion:", "$Name:")) {
					parse_comments();

					required_string_fred("+Special Exp Damage:"); 
					parse_comments();
					fout(" %f", Ships[i].special_exp_damage);

					required_string_fred("+Special Exp Blast:"); 
					parse_comments();
					fout(" %f", Ships[i].special_exp_blast);

					required_string_fred("+Special Exp Inner Radius:"); 
					parse_comments();
					fout(" %f", Ships[i].special_exp_inner);

					required_string_fred("+Special Exp Outer Radius:"); 
					parse_comments();
					fout(" %f", Ships[i].special_exp_outer);

					if (Ships[i].use_shockwave && (Ships[i].special_exp_shockwave_speed > 0)) {
						optional_string_fred("+Special Exp Shockwave Speed:"); 
						parse_comments();
						fout(" %f", Ships[i].special_exp_shockwave_speed);
					}
					else {
						bypass_comment(";;FSO 3.6.13;; +Special Exp Shockwave Speed:");
					}
				}
				else {
					fso_comment_push(";;FSO 3.6.13;;");
					fout_version("\n$Special Explosion:");

					fout_version("\n+Special Exp Damage:"); 
					fout(" %f", Ships[i].special_exp_damage);

					fout_version("\n+Special Exp Blast:"); 
					fout(" %f", Ships[i].special_exp_blast);

					fout_version("\n+Special Exp Inner Radius:"); 
					fout(" %f", Ships[i].special_exp_inner);

					fout_version("\n+Special Exp Outer Radius:"); 
					fout(" %f", Ships[i].special_exp_outer);

					if (Ships[i].use_shockwave && (Ships[i].special_exp_shockwave_speed > 0)) {
						fout_version("\n+Special Exp Shockwave Speed:"); 
						fout(" %f", Ships[i].special_exp_shockwave_speed);
					}

					fso_comment_pop();
				}
			}
			else {
				bypass_comment(";;FSO 3.6.13;; +Special Exp Shockwave Speed:");
			}
		}
			// retail format special explosions
		else {
			if (Ships[i].use_special_explosion) {
				int special_exp_index;

				if (has_special_explosion_block_index(&Ships[i], &special_exp_index)) {
					fout("\n+Special Exp index:");
					fout(" %d", special_exp_index);
				}
				else {
					CString text = "You are saving in the retail mission format, but ";
					text += "the mission has too many special explosions defined. \"";
					text += Ships[i].ship_name;
					text += "\" has therefore lost any special explosion data that was defined for it. ";
					text += "\" Either remove special explosions or SEXP variables if you need it to have one ";
					MessageBox(NULL, text, "Too many variables!", MB_OK);
					
				}				
			}			
		}

		// Goober5000 ------------------------------------------------
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if (Ships[i].special_hitpoints) {
				if (optional_string_fred("+Special Hitpoints:", "$Name:")) {
					parse_comments();
				} else {
					fso_comment_push(";;FSO 3.6.13;;");
					fout_version("\n+Special Hitpoints:");
					fso_comment_pop();
				}

				fout(" %d", Ships[i].special_hitpoints);
			}
			else {
				bypass_comment(";;FSO 3.6.13;; +Special Hitpoints:");
			}

			if (Ships[i].special_shield >= 0) {
				if (optional_string_fred("+Special Shield Points:", "$Name:")) {
					parse_comments();
				} else {
					fso_comment_push(";;FSO 3.6.13;;");
					fout_version("\n+Special Shield Points:");
					fso_comment_pop();
				}

				fout(" %d", Ships[i].special_shield);
			}
			else {
				bypass_comment(";;FSO 3.6.13;; +Special Shield Points:");
			}
		}
		// -----------------------------------------------------------

		if ( Ai_info[Ships[i].ai_index].ai_flags & AIF_KAMIKAZE ) {
			if ( optional_string_fred("+Kamikaze Damage:", "$Name:")){
				parse_comments();
			} else {
				fout("\n+Kamikaze Damage:");
			}

			fout(" %d", (int)(Ai_info[Ships[i].ai_index].kamikaze_damage) );
		}

		if (Ships[i].hotkey != -1) {
			if (optional_string_fred("+Hotkey:", "$Name:")){
				parse_comments();
			} else {
				fout("\n+Hotkey:");
			}

			fout(" %d", Ships[i].hotkey);
		}

		// mwa -- new code to save off information about initially docked ships.
		// Goober5000 - newer code to save off information about initially docked ships. ;)
		if (object_is_docked(&Objects[Ships[i].objnum]))
		{
			// possible incompatibility
			if ( Format_fs2_open == FSO_FORMAT_RETAIL && !dock_check_docked_one_on_one(&Objects[Ships[i].objnum]))
			{
				static bool warned = false;
				if (!warned)
				{
					CString text = "You are saving in the retail mission format, but \"";
					text += Ships[i].ship_name;
					text += "\" is docked to more than one ship.  If you wish to run this mission in retail, ";
					text += "you should remove the additional ships and save the mission again.";
					MessageBox(NULL, text, "Incompatibility with retail mission format", MB_OK);

					warned = true;	// to avoid zillions of boxes
				}
			}

			// save one-on-one groups as if they were retail
			if (dock_check_docked_one_on_one(&Objects[Ships[i].objnum]))
			{
				// retail format only saved information for non-leaders
				if (!(Ships[i].flags & SF_DOCK_LEADER))
				{
					save_single_dock_instance(&Ships[i], Objects[Ships[i].objnum].dock_list);
				}
			}
			// multiply docked
			else
			{
				// save all instances for all ships
				for (dock_instance *dock_ptr = Objects[Ships[i].objnum].dock_list; dock_ptr != NULL; dock_ptr = dock_ptr->next)
				{
					save_single_dock_instance(&Ships[i], dock_ptr);
				}
			}
		}

		// check the ship flag about killing off the ship before a missino starts.  Write out the appropriate
		// variable if necessary
		if ( Ships[i].flags & SF_KILL_BEFORE_MISSION ) {
			if ( optional_string_fred("+Destroy At:", "$Name:"))
				parse_comments();
			else
				fout("\n+Destroy At: ");

			fout(" %d", Ships[i].final_death_time );
		}

		// possibly write out the orders that this ship will accept.  We'll only do it if the orders
		// are not the default set of orders
		if ( Ships[i].orders_accepted != ship_get_default_orders_accepted( &Ship_info[Ships[i].ship_info_index]) ) {
			if ( optional_string_fred("+Orders Accepted:", "$Name:") )
				parse_comments();
			else
				fout("\n+Orders Accepted:");

			fout(" %d\t\t;! note that this is a bitfield!!!", Ships[i].orders_accepted);
		}

		if (Ships[i].group >= 0) {
			if (optional_string_fred("+Group:", "$Name:"))
				parse_comments();
			else
				fout("\n+Group:");

			fout(" %d", Ships[i].group);
		}

		// always write out the score to ensure backwards compatibility. If the score is the same as the value 
		// in the table write out a flag to tell the game to simply use whatever is in the table instead
		if (Ship_info[Ships[i].ship_info_index].score == Ships[i].score ) {
			if ( optional_string_fred("+Use Table Score:", "$Name:") ) {
				parse_comments();
			} else {
				fso_comment_push(";;FSO 3.6.10;;");
				fout_version("\n+Use Table Score:");
				fso_comment_pop();
			}		
		}
		else {
			bypass_comment(";;FSO 3.6.10;; +Use Table Score:");
		}

		if (optional_string_fred("+Score:", "$Name:"))
			parse_comments();
		else
			fout("\n+Score:");

		fout(" %d", Ships[i].score);
	
		
		if (Format_fs2_open != FSO_FORMAT_RETAIL && Ships[i].assist_score_pct != 0) {
			if ( optional_string_fred("+Assist Score Percentage:") ) {
				parse_comments();
			} else {
				fso_comment_push(";;FSO 3.6.10;;");
				fout_version("\n+Assist Score Percentage:");
				fso_comment_pop();
			}
			
			fout(" %f", Ships[i].assist_score_pct);
		}
		else {
			bypass_comment(";;FSO 3.6.10;; +Assist Score Percentage:");
		}

		// deal with the persona for this ship as well.
		if ( Ships[i].persona_index != -1 ) {
			if (optional_string_fred("+Persona Index:", "$Name:"))
				parse_comments();
			else
				fout("\n+Persona Index:");

			fout(" %d", Ships[i].persona_index);
		}

		// Goober5000 - deal with texture replacement ----------------
		k = 0;
		if (Fred_num_texture_replacements > 0) {
			bool needs_header = true;

			while (k < Fred_num_texture_replacements) {
				if ( !stricmp(Ships[i].ship_name, Fred_texture_replacements[k].ship_name) ) {
					if (needs_header) {
						if (optional_string_fred("$Texture Replace:")) {
							parse_comments(1);
						} else {
							fso_comment_push(";;FSO 3.6.8;;");
							fout_version("\n$Texture Replace:");
						}

						needs_header = false;
					}

					// write out this entry
					if (optional_string_fred("+old:")) {
						parse_comments(1);
						fout(" %s", Fred_texture_replacements[k].old_texture);
					} else {
						fout_version("\n+old: %s", Fred_texture_replacements[k].old_texture);
					}

					if (optional_string_fred("+new:")) {
						parse_comments(1);
						fout(" %s", Fred_texture_replacements[k].new_texture);
					} else {
						fout_version("\n+new: %s", Fred_texture_replacements[k].new_texture);
					}
				}

				k++;	// increment down the list of texture replacements
			}

			fso_comment_pop();
		} else {
			bypass_comment(";;FSO 3.6.8;; $Texture Replace:");
		}

		// end of texture replacement -------------------------------

		z++;

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_common_object_data(object *objp, ship *shipp)
{
	int j, z;
	ship_subsys *ptr = NULL;
	ship_info *sip   = NULL;
	ship_weapon *wp  = NULL;
	float temp_max_hull_strength;

	sip = &Ship_info[shipp->ship_info_index];

	if ((int) objp->phys_info.speed) {
		if (optional_string_fred("+Initial Velocity:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Initial Velocity:");

		fout(" %d", (int) objp->phys_info.speed);
	}

	// Goober5000
	if (Format_fs2_open != FSO_FORMAT_RETAIL && (shipp->special_hitpoints))
	{
		temp_max_hull_strength = (float)shipp->special_hitpoints;
	}
	else
	{
		temp_max_hull_strength = sip->max_hull_strength;
	}

	if ((int) objp->hull_strength != temp_max_hull_strength) {
		if (optional_string_fred("+Initial Hull:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Initial Hull:");

		fout(" %d", (int) objp->hull_strength);
	}

	if ((int) shield_get_strength(objp) != 100) {
		if (optional_string_fred("+Initial Shields:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Initial Shields:");

		fout(" %d", (int) objp->shield_quadrant[0]);
	}

	// save normal ship weapons info
	required_string_fred("+Subsystem:", "$Name:");
	parse_comments();
	fout(" Pilot");

	wp = &shipp->weapons;
	z = 0;
	j = wp->num_primary_banks;
	while (j-- && (j >= 0))
		if (wp->primary_bank_weapons[j] != sip->primary_bank_weapons[j])
			z = 1;

	if (z) {
		if (optional_string_fred("+Primary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Primary Banks:");

		fout(" ( ");
		for (j=0; j<wp->num_primary_banks; j++)
			fout("\"%s\" ", Weapon_info[wp->primary_bank_weapons[j]].name);

		fout(")");
	}

	z = 0;
	j = wp->num_secondary_banks;
	while (j-- && (j >= 0))
		if (wp->secondary_bank_weapons[j] != sip->secondary_bank_weapons[j])
			z = 1;

	if (z) {
		if (optional_string_fred("+Secondary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Secondary Banks:");

		fout(" ( ");
		for (j=0; j<wp->num_secondary_banks; j++)
			fout("\"%s\" ", Weapon_info[wp->secondary_bank_weapons[j]].name);

		fout(")");
	}

	z = 0;
	j = wp->num_secondary_banks;
	while (j-- && (j >= 0))
		if (wp->secondary_bank_ammo[j] != 100)
			z = 1;

	if (z) {
		if (optional_string_fred("+Sbank Ammo:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Sbank Ammo:");

		fout(" ( ");
		for (j=0; j<wp->num_secondary_banks; j++)
			fout("%d ", wp->secondary_bank_ammo[j]);

		fout(")");
	}

	ptr = GET_FIRST(&shipp->subsys_list);
	Assert(ptr);

	while (ptr != END_OF_LIST(&shipp->subsys_list) && ptr) {
		// Crashing here!
		if ( (ptr->current_hits) || (ptr->system_info && ptr->system_info->type == SUBSYSTEM_TURRET) || (ptr->subsys_cargo_name != -1)) {
			if (optional_string_fred("+Subsystem:", "$Name:"))
				parse_comments();
			else
				fout("\n+Subsystem:");

			fout(" %s", ptr->system_info->subobj_name);
		}

		if (ptr->current_hits) {
			if (optional_string_fred("$Damage:", "$Name:", "+Subsystem:"))
				parse_comments();
			else
				fout("\n$Damage:");

			fout(" %d", (int) ptr->current_hits);
		}

		if (ptr->subsys_cargo_name != -1) {
			if (optional_string_fred("+Cargo Name:", "$Name:", "+Subsystem:"))
				parse_comments();
			else
				fout("\n+Cargo Name:");

			fout_ext(NULL, "%s", Cargo_names[ptr->subsys_cargo_name]);
		}

		if (ptr->system_info->type == SUBSYSTEM_TURRET)
			save_turret_info(ptr, shipp - Ships);

		ptr = GET_NEXT(ptr);

		fso_comment_pop();
	}

/*	for (j=0; j<shipp->status_count; j++) {
		required_string_fred("$Status Description:");
		parse_comments(-1);
		fout(" %s", Status_desc_names[shipp->status_type[j]]);

		required_string_fred("$Status:");
		parse_comments(-1);
		fout(" %s", Status_type_names[shipp->status[j]]);

		required_string_fred("$Target:");
		parse_comments(-1);
		fout(" %s", Status_target_names[shipp->target[j]]);
	}*/

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_wings()
{
	char out[4096];
	int i, j, z, ship, count = 0;

	fred_parse_flag = 0;
	required_string_fred("#Wings");
	parse_comments(2);
	fout("\t\t;! %d total", Num_wings);

	for (i=0; i<MAX_WINGS; i++) {
		if (!Wings[i].wave_count)
			continue;

		count++;
		required_string_either_fred("$Name:", "#Events");
		required_string_fred("$Name:");
		parse_comments(2);
		fout(" %s", Wings[i].name);

		// squad logo - Goober5000
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if (strlen(Wings[i].wing_squad_filename) > 0)
			{
				if (optional_string_fred("+Squad Logo:", "$Name:"))
					parse_comments();
				else
					fout("\n+Squad Logo:");

				fout(" %s", Wings[i].wing_squad_filename);
			}
		}

		required_string_fred("$Waves:");
		parse_comments();
		fout(" %d", Wings[i].num_waves);

		required_string_fred("$Wave Threshold:");
		parse_comments();
		fout(" %d", Wings[i].threshold);

		required_string_fred("$Special Ship:");
		parse_comments();
		fout(" %d\t\t;! %s\n", Wings[i].special_ship,
			Ships[Wings[i].ship_index[Wings[i].special_ship]].ship_name);

		required_string_fred("$Arrival Location:");
		parse_comments();
		fout(" %s", Arrival_location_names[Wings[i].arrival_location]);

		if (Wings[i].arrival_location != ARRIVE_AT_LOCATION)
		{
			if (optional_string_fred("+Arrival Distance:", "$Name:"))
				parse_comments();
			else
				fout("\n+Arrival Distance:");

			fout(" %d", Wings[i].arrival_distance);
			if (optional_string_fred("$Arrival Anchor:", "$Name:"))
				parse_comments();
			else
				fout("\n$Arrival Anchor:");

			z = Wings[i].arrival_anchor;
			if (z & SPECIAL_ARRIVAL_ANCHOR_FLAG)
			{
				// get name
				char tmp[NAME_LENGTH + 15];
				stuff_special_arrival_anchor_name(tmp, z, Format_fs2_retail);

				// save it
				fout(" %s", tmp);
			}
			else if (z >= 0)
			{
				fout(" %s", Ships[z].ship_name);
			}
			else
			{
				fout(" <error>");
			}
		}

		// Goober5000
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if ((Wings[i].arrival_location == ARRIVE_FROM_DOCK_BAY) && (Wings[i].arrival_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Wings[i].arrival_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Arrival Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Wings[i].arrival_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Wings[i].arrival_delay)
		{
			if (optional_string_fred("+Arrival delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Arrival delay:");

			fout(" %d", Wings[i].arrival_delay);
		}

		required_string_fred("$Arrival Cue:");
		parse_comments();
		convert_sexp_to_string(Wings[i].arrival_cue, out, SEXP_SAVE_MODE, 4096);
		fout(" %s", out);

		required_string_fred("$Departure Location:");
		parse_comments();
		fout(" %s", Departure_location_names[Wings[i].departure_location]);

		if ( Wings[i].departure_location != DEPART_AT_LOCATION )
		{
			required_string_fred("$Departure Anchor:");
			parse_comments();

			if ( Wings[i].departure_anchor >= 0 )
				fout(" %s", Ships[Wings[i].departure_anchor].ship_name );
			else
				fout(" <error>");
		}

		// Goober5000
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			if ((Wings[i].departure_location == DEPART_AT_DOCK_BAY) && (Wings[i].departure_path_mask > 0))
			{
				int j, anchor_shipnum;
				polymodel *pm;

				anchor_shipnum = Wings[i].departure_anchor;
				Assert(anchor_shipnum >= 0 && anchor_shipnum < MAX_SHIPS);

				fout("\n+Departure Paths: ( ");

				pm = model_get(Ship_info[Ships[anchor_shipnum].ship_info_index].model_num);
				for (j = 0; j < pm->ship_bay->num_paths; j++)
				{
					if (Wings[i].departure_path_mask & (1 << j))
					{
						fout("\"%s\" ", pm->paths[pm->ship_bay->path_indexes[j]].name);
					}
				}

				fout(")");
			}
		}

		if (Wings[i].departure_delay)
		{
			if (optional_string_fred("+Departure delay:", "$Name:"))
				parse_comments();
			else
				fout("\n+Departure delay:");

			fout(" %d", Wings[i].departure_delay);
		}

		required_string_fred("$Departure Cue:");
		parse_comments();
		convert_sexp_to_string(Wings[i].departure_cue, out, SEXP_SAVE_MODE, 4096);
		fout(" %s", out);

		required_string_fred("$Ships:");
		parse_comments();
		fout(" (\t\t;! %d total\n", Wings[i].wave_count);

		for (j=0; j<Wings[i].wave_count; j++) {
			ship = Wings[i].ship_index[j];
//			if (Objects[Ships[ship].objnum].type == OBJ_START)
//				fout("\t\"Player 1\"\n");
//			else
				fout("\t\"%s\"\n", Ships[Wings[i].ship_index[j]].ship_name);
		}

		fout(")");

		save_ai_goals(Wings[i].ai_goals, -1);

		if (Wings[i].hotkey != -1) {
			if (optional_string_fred("+Hotkey:", "$Name:"))
				parse_comments();
			else
				fout("\n+Hotkey:");

			fout(" %d", Wings[i].hotkey);
		}

		if ( optional_string_fred("+Flags:", "$Name:")) {
			parse_comments();
			fout( " (" );
		} else 
			fout("\n+Flags: (");

		if ( Wings[i].flags & WF_IGNORE_COUNT )
			fout(" \"ignore-count\"");
		if ( Wings[i].flags & WF_REINFORCEMENT )
			fout(" \"reinforcement\"");
		if ( Wings[i].flags & WF_NO_ARRIVAL_MUSIC )
			fout(" \"no-arrival-music\"");
		if ( Wings[i].flags & WF_NO_ARRIVAL_MESSAGE )
			fout(" \"no-arrival-message\"");
		if ( Wings[i].flags & WF_NO_ARRIVAL_WARP )
			fout(" \"no-arrival-warp\"");
		if ( Wings[i].flags & WF_NO_DEPARTURE_WARP )
			fout(" \"no-departure-warp\"");
		if ( Wings[i].flags & WF_NO_DYNAMIC )
			fout( " \"no-dynamic\"" );

		fout(" )");

		if (Wings[i].wave_delay_min) {
			if (optional_string_fred("+Wave Delay Min:", "$Name:"))
				parse_comments();
			else
				fout("\n+Wave Delay Min:");

			fout(" %d", Wings[i].wave_delay_min);
		}

		if (Wings[i].wave_delay_max) {
			if (optional_string_fred("+Wave Delay Max:", "$Name:"))
				parse_comments();
			else
				fout("\n+Wave Delay Max:");

			fout(" %d", Wings[i].wave_delay_max);
		}

		fso_comment_pop();
	}

	fso_comment_pop(true);

	Assert(count == Num_wings);
	return err;
}

int CFred_mission_save::save_goals()
{
	char out[4096];
	int i;

	fred_parse_flag = 0;
	required_string_fred("#Goals");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_goals);

	for (i=0; i<Num_goals; i++) {
		int type;

		required_string_either_fred("$Type:", "#Waypoints");
		required_string_fred("$Type:");
		parse_comments(i ? 2 : 1);

		type = Mission_goals[i].type & GOAL_TYPE_MASK;
		fout(" %s", Goal_type_names[type]);

		if (*Mission_goals[i].name) {
			if (optional_string_fred("+Name:", "$Type:"))
				parse_comments();
			else
				fout("\n+Name:");

			fout(" %s", Mission_goals[i].name);
		}

		// XSTR
		required_string_fred("$MessageNew:");
		parse_comments();
		fout_ext(" ", "%s", Mission_goals[i].message);
		fout("\n");
		required_string_fred("$end_multi_text");
		parse_comments(0);

		required_string_fred("$Formula:");
		parse_comments();
		convert_sexp_to_string(Mission_goals[i].formula, out, SEXP_SAVE_MODE, 4096);
		fout(" %s", out);

		if ( Mission_goals[i].type & INVALID_GOAL ) {
			if (optional_string_fred("+Invalid", "$Type:"))
				parse_comments();
			else
				fout("\n+Invalid");
		}

		if ( Mission_goals[i].flags & MGF_NO_MUSIC ) {
			if (optional_string_fred("+No music", "$Type:"))
				parse_comments();
			else
				fout("\n+No music");
		}

		if ( Mission_goals[i].score != 0 ) {
			if ( optional_string_fred("+Score:", "$Type:"))
				parse_comments();
			else
				fout("\n+Score:");
			fout(" %d", Mission_goals[i].score );
		}

		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ) {
			if ( optional_string_fred("+Team:", "$Type:"))
				parse_comments();
			else
				fout("\n+Team:");
			fout(" %d", Mission_goals[i].team );
		}

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_waypoints()
{
	int i;
	//object *ptr;

	fred_parse_flag = 0;
	required_string_fred("#Waypoints");
	parse_comments(2);
	fout("\t\t;! %d lists total\n", Num_waypoint_lists);

	for ( jump_node *jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() )
	{
		required_string_fred("$Jump Node:", "$Jump Node Name:");
		parse_comments(2);
		save_vector(jnp->get_obj()->pos);

		required_string_fred("$Jump Node Name:", "$Jump Node:");
		parse_comments();
		fout(" %s", jnp->get_name_ptr());
		
		if(jnp->is_special_model())
		{
			if ( optional_string_fred("+Model File:", "$Jump Node:"))
				parse_comments();
			else
				fout("\n+Model File:");

			int model = jnp->get_modelnum();
			polymodel *pm = model_get(model);
			fout(" %s", pm->filename );
		}

		if(jnp->is_colored())
		{
			if ( optional_string_fred("+Alphacolor:", "$Jump Node:"))
				parse_comments();
			else
				fout("\n+Alphacolor:");

			color jn_color = jnp->get_color();
			fout(" %u %u %u %u", jn_color.red, jn_color.green, jn_color.blue, jn_color.alpha );
		}

		int hidden_is_there = optional_string_fred("+Hidden:", "$Jump Node:");
		if(hidden_is_there)
			parse_comments();

		if(hidden_is_there || jnp->is_hidden())
		{
			if(!hidden_is_there)
				fout("\n+Hidden:");

			if(jnp->is_hidden())
				fout(" %s", "true");
			else
				fout(" %s", "false");
		}

		fso_comment_pop();
	}

	for (i=0; i<Num_waypoint_lists; i++)
	{
		required_string_either_fred("$Name:", "#Messages");
		required_string_fred("$Name:");
		parse_comments(i ? 2 : 1);
		fout(" %s", Waypoint_lists[i].name);

		required_string_fred("$List:");
		parse_comments();
		fout(" (\t\t;! %d points in list\n", Waypoint_lists[i].count);

		save_waypoint_list(Waypoint_lists[i]);
		fout(")");

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_waypoint_list(waypoint_list &w)
{
	int i;

	for (i=0; i<w.count; i++)
		fout("\t( %f, %f, %f )\n", w.waypoints[i].xyz.x, w.waypoints[i].xyz.y, w.waypoints[i].xyz.z);

	return 0;
}

int CFred_mission_save::save_messages()
{
	int i;

	fred_parse_flag = 0;
	required_string_fred("#Messages");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_messages-Num_builtin_messages);

	// Goober5000 - special Command info
	if (Format_fs2_open != FSO_FORMAT_RETAIL)
	{
		if (stricmp(The_mission.command_sender, DEFAULT_COMMAND))
			fout("\n$Command Sender: %s", The_mission.command_sender);

		if (The_mission.command_persona != Default_command_persona)
			fout("\n$Command Persona: %s", Personas[The_mission.command_persona].name);
	}

	for (i=Num_builtin_messages; i<Num_messages; i++) {
		required_string_either_fred("$Name:", "#Reinforcements");
		required_string_fred("$Name:");
		parse_comments(2);
		fout(" %s", Messages[i].name);

		// team
		required_string_fred("$Team:");
		parse_comments(1);
		if((Messages[i].multi_team < 0) || (Messages[i].multi_team >= 2)){
			fout(" %d", -1);
		} else {
			fout(" %d", Messages[i].multi_team);
		}

		// XSTR
		required_string_fred("$MessageNew:");
		parse_comments();
		fout_ext(" ", "%s", Messages[i].message);
		fout("\n");
		required_string_fred("$end_multi_text");
		parse_comments(0);

		if ( Messages[i].persona_index != -1 ) {
			if ( optional_string_fred("+Persona:", "$Name:"))
				parse_comments();
			else
				fout("\n+Persona:");

			fout(" %s", Personas[Messages[i].persona_index].name );
		}

		if (Messages[i].avi_info.name) {
			if (optional_string_fred("+AVI Name:", "$Name:"))
				parse_comments();
			else
				fout("\n+AVI Name:");

			fout(" %s", Messages[i].avi_info.name);
		}

		if (Messages[i].wave_info.name) {
			if (optional_string_fred("+Wave Name:", "$Name:"))
				parse_comments();
			else
				fout("\n+Wave Name:");

			fout(" %s", Messages[i].wave_info.name);
		}

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_vector(vec3d &v)
{
	fout(" %f, %f, %f", v.xyz.x, v.xyz.y, v.xyz.z);
	return 0;
}

int CFred_mission_save::save_matrix(matrix &m)
{
	fout("\n\t%f, %f, %f,\n", m.vec.rvec.xyz.x, m.vec.rvec.xyz.y, m.vec.rvec.xyz.z);
	fout("\t%f, %f, %f,\n",   m.vec.uvec.xyz.x, m.vec.uvec.xyz.y, m.vec.uvec.xyz.z);
	fout("\t%f, %f, %f",      m.vec.fvec.xyz.x, m.vec.fvec.xyz.y, m.vec.fvec.xyz.z);
	return 0;
}

// Goober5000 - move past the comment without copying it to the output file
// (used for special FSO comment tags)
void CFred_mission_save::bypass_comment(char *comment)
{
	char *ch = strstr(raw_ptr, comment);
	if (ch != NULL)
	{
		char *writep = ch;
		char *readp = strchr(writep, '\n');

		// copy all characters past it
		while (*readp != '\0')
		{
			*writep = *readp;

			writep++;
			readp++;
		}

		*writep = '\0';
	}
}

// saves comments from previous campaign/mission file
void CFred_mission_save::parse_comments(int newlines)
{
	char *comment_start = NULL;
	int state = 0, same_line = 0, first_comment = 1, tab = 0, flag = 0;

	if (newlines < 0) {
		newlines = -newlines;
		tab = 1;
	}

	if (newlines)
		same_line = 1;

	if (fred_parse_flag || !Token_found_flag || !token_found || (token_found && (*Mission_text_raw == EOF_CHAR))) {
		while (newlines-- > 0)
			fout("\n");

		if (tab)
			fout("\t");

		if (token_found)
			fout("%s", token_found);

		return;
	}

	while (*raw_ptr != EOF_CHAR) {
		if (!state) {
			if (token_found && (*raw_ptr == *token_found))
				if (!strnicmp(raw_ptr, token_found, strlen(token_found))) {
					same_line = newlines - 1 + same_line;
					while (same_line-- > 0)
						fout("\n");
					
					if (tab) {
						fout("\t");
						fout("%s", token_found);
					} else {
						fout("%s", token_found);
					}

					return;
				}

			if ((*raw_ptr == '/') && (raw_ptr[1] == '*')) {
				comment_start = raw_ptr;
				state = 1;
			}

			if ((*raw_ptr == ';') && (raw_ptr[1] != '!')) {
				comment_start = raw_ptr;
				state = 2;

				// check for a FSO version comment, but if we can't understand it then
				// just handle it as a regular comment
				if ( (raw_ptr[1] == ';') && (raw_ptr[2] == 'F') && (raw_ptr[3] == 'S') && (raw_ptr[4] == 'O') ) {
					int major, minor, build, revis;
					int s_num = scan_fso_version_string(raw_ptr, &major, &minor, &build, &revis);
					
					// hack for releases
					if (FS_VERSION_REVIS < 1000) {
						s_num = 3;
					}

					if ( (s_num == 3) && (major <= FS_VERSION_MAJOR) && (minor <= FS_VERSION_MINOR) && (build <= FS_VERSION_BUILD) ) {
						state = 3;
					} else if ( (s_num == 4) && (major <= FS_VERSION_MAJOR) && (minor <= FS_VERSION_MINOR) && (build <= FS_VERSION_BUILD) && (revis <= FS_VERSION_REVIS) ) {
						state = 3;
					} else {
						state = 4;
					}
				}
			}

			if ((*raw_ptr == '/') && (raw_ptr[1] == '/')) {
				comment_start = raw_ptr;
				state = 2;
			}

			if (*raw_ptr == '\n')
				flag = 1;

			if (flag && state && !(state == 3) )
				fout("\n");

		} else {
			if (*raw_ptr == '\n') {
				if (state == 2) {
					if (first_comment && !flag)
						fout("\t\t");

					*raw_ptr = 0;
					fout("%s\n", comment_start);
					*raw_ptr = '\n';
					state = first_comment = same_line = flag = 0;
				} else if (state == 4) {
					same_line = newlines - 2 + same_line;
					while (same_line-- > 0)
						fout("\n");

					if (*(raw_ptr-1) == '\r') {
						*(raw_ptr-1) = '\0';
					} else {
						*raw_ptr = 0;
					}

					fout("%s\n", comment_start);

					if (*(raw_ptr-1) == '\0') {
						*(raw_ptr-1) = '\r';
					} else {
						*raw_ptr = '\n';
					}

					state = first_comment = same_line = flag = 0;
				}
			}

			if ((*raw_ptr == '*') && (raw_ptr[1] == '/') && (state == 1)) {
				if (first_comment && !flag)
					fout("\t\t");

				state = raw_ptr[2];
				raw_ptr[2] = 0;
				fout("%s", comment_start);
				raw_ptr[2] = (char)state;
				state = first_comment = flag = 0;
			}

			if ( (*raw_ptr == ';') && (raw_ptr[1] == ';') && (state == 3) ) {
				state = raw_ptr[2];
				raw_ptr[2] = 0;
				fso_comment_pop();
				fso_comment_push(comment_start);
				raw_ptr[2] = (char)state;
				state = first_comment = flag = 0;
				raw_ptr++;
			}
		}

		raw_ptr++;
	}

	return;
}

int CFred_mission_save::fout_version(char *format, ...)
{
	char str[16384];
	char *ch = NULL;
	va_list args;
	
	if (err) {
		return err;
	}

	// output the version first thing, but skip the special case where we use
	// fout_version() for multiline value strings (typically indicated by an initial space)
	if ( (Format_fs2_open == FSO_FORMAT_COMPATIBILITY_MODE) && (*format != ' ') && !fso_ver_comment.empty() ) {
		int len = 0;
		while (*format == '\n') {
			str[len++] = '\n';
			format++;
		}

		str[len] = '\0';

		strcat_s(str, fso_ver_comment.back().c_str());
		strcat_s(str, " ");

		cfputs(str, fp);

		memset(str, 0, sizeof(str));
	}

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	Assert(strlen(str) < 16384);

	// this could be a multi-line string, so we've got to handle it all properly
	if ( (Format_fs2_open == FSO_FORMAT_COMPATIBILITY_MODE) && !fso_ver_comment.empty() ) {
		bool first_line = true;
		char *str_p = str;

		ch = strchr(str, '\n');

		// if we have something, and it's not just at the end, then process it specially
		if ( (ch != NULL) && (*(ch+1) != '\0') ) {
			do {
				if ( *(ch+1) != '\0' ) {
					*ch = '\0';

					if (first_line) {
						first_line = false;
					} else {
						cfputs((char*)fso_ver_comment.back().c_str(), fp);
						cfputs(" ", fp);
					}

					cfputs(str_p, fp);
					cfputc('\n', fp);

					str_p = ch+1;
				} else {
					if (first_line) {
						first_line = false;
					} else {
						cfputs((char*)fso_ver_comment.back().c_str(), fp);
						cfputs(" ", fp);
					}

					cfputs(str_p, fp);

					str_p = ch+1;

					break;
				}
			} while ( (ch = strchr(str_p, '\n')) != NULL );

			// be sure to account for any ending elements too
			if ( strlen(str_p) ) {
				cfputs((char*)fso_ver_comment.back().c_str(), fp);
				cfputs(" ", fp);
				cfputs(str_p, fp);
			}

			return 0;
		}
	}

	cfputs(str, fp);

	return 0;
}

int CFred_mission_save::fout(char *format, ...)
{
	char str[16384];
	va_list args;
	
	if (err){
		return err;
	}

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	Assert(strlen(str) < 16384);

	cfputs(str, fp);
	return 0;
}

int CFred_mission_save::fout_ext(char *pre_str, char *format, ...)
{
	char str[16384];
	char str_out[16384] = "";
	va_list args;
	int str_id;
	
	if (err){
		return err;
	}

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);
	Assert(strlen(str) < 16384);

	memset(str_out, 0, sizeof(str_out));

	if (pre_str) {
		strcpy_s(str_out, pre_str);
	}

	// lookup the string in the hash table
	str_id = fhash_string_exists(str);

	// doesn't exist, so assign it an ID of -1 and stick it in the table
	if (str_id <= -2) {
		sprintf(str_out+strlen(str_out), " XSTR(\"%s\", -1)", str);

		// add the string to the table		
		fhash_add_str(str, -1);
	}
	// _does_ exist, so just write it out as it is
	else {
		sprintf(str_out+strlen(str_out), " XSTR(\"%s\", %d)", str, str_id);
	}

	// this could be a multi-line string, so we've got to handle it all properly
	if ( !fso_ver_comment.empty() ) {
		bool first_line = true;
		char *str_p = str_out;

		char *ch = strchr(str_out, '\n');

		// if we have something, and it's not just at the end, then process it specially
		if ( (ch != NULL) && (*(ch+1) != '\0') ) {
			do {
				if ( *(ch+1) != '\0' ) {
					*ch = '\0';

					if (first_line) {
						first_line = false;
					} else {
						cfputs((char*)fso_ver_comment.back().c_str(), fp);
						cfputs(" ", fp);
					}

					cfputs(str_p, fp);
					cfputc('\n', fp);

					str_p = ch+1;
				} else {
					if (first_line) {
						first_line = false;
					} else {
						cfputs((char*)fso_ver_comment.back().c_str(), fp);
						cfputs(" ", fp);
					}

					cfputs(str_p, fp);

					str_p = ch+1;

					break;
				}
			} while ( (ch = strchr(str_p, '\n')) != NULL );

			// be sure to account for any ending elements too
			if ( strlen(str_p) ) {
				cfputs((char*)fso_ver_comment.back().c_str(), fp);
				cfputs(" ", fp);
				cfputs(str_p, fp);
			}

			return 0;
		}
	}

	cfputs(str_out, fp);

	return 0;
}

void CFred_mission_save::save_ai_goals(ai_goal *goalp, int ship)
{
	char *str = NULL, buf[80];
	int i, valid, flag = 1;

	for (i=0; i<MAX_AI_GOALS; i++) {
		if (goalp[i].ai_mode == AI_GOAL_NONE)
			continue;

		if (flag) {
			if (optional_string_fred("$AI Goals:", "$Name:"))
				parse_comments();
			else
				fout("\n$AI Goals:");

			fout(" ( goals ");
			flag = 0;
		}

		if (goalp[i].ai_mode == AI_GOAL_CHASE_ANY) {
			fout("( ai-chase-any %d ) ", goalp[i].priority);

		} else if (goalp[i].ai_mode == AI_GOAL_UNDOCK) {
			fout("( ai-undock %d ) ", goalp[i].priority);

		} else if (goalp[i].ai_mode == AI_GOAL_KEEP_SAFE_DISTANCE) {
			fout("( ai-keep-safe-distance %d ) ", goalp[i].priority);
		
		} else if (goalp[i].ai_mode == AI_GOAL_PLAY_DEAD) {
			fout("( ai-play-dead %d ) ", goalp[i].priority);

		} else if (goalp[i].ai_mode == AI_GOAL_WARP) {
			fout("( ai-warp-out %d ) ", goalp[i].priority);
		
		} else {
			valid = 1;
			if (!goalp[i].ship_name) {
				Warning(LOCATION, "Ai goal has no target where one is required");

			} else {
				sprintf(buf, "\"%s\"", goalp[i].ship_name);
				switch (goalp[i].ai_mode) {
					case AI_GOAL_WAYPOINTS:
						str = "ai-waypoints";
						break;

					case AI_GOAL_WAYPOINTS_ONCE:
						str = "ai-waypoints-once";
						break;

					case AI_GOAL_DESTROY_SUBSYSTEM:
						if (goalp[i].docker.index == -1 || !goalp[i].docker.index) {
							valid = 0;
							Warning(LOCATION, "AI destroy subsystem goal invalid subsystem name\n");

						} else {
							sprintf(buf, "\"%s\" \"%s\"", goalp[i].ship_name, goalp[i].docker.name);
							str = "ai-destroy-subsystem";
						}

						break;

					case AI_GOAL_DOCK:
						if (ship < 0) {
							valid = 0;
							Warning(LOCATION, "Wings aren't allowed to have a docking goal\n");
							
						} else if (goalp[i].docker.index == -1 || !goalp[i].docker.index) {
							valid = 0;
							Warning(LOCATION, "AI dock goal for \"%s\" has invalid docker point "
								"(docking with \"%s\")\n", Ships[ship].ship_name, goalp[i].ship_name);

						} else if (goalp[i].dockee.index == -1 || !goalp[i].dockee.index) {
							valid = 0;
							Warning(LOCATION, "AI dock goal for \"%s\" has invalid dockee point "
								"(docking with \"%s\")\n", Ships[ship].ship_name, goalp[i].ship_name);

						} else {
							sprintf(buf, "\"%s\" \"%s\" \"%s\"", goalp[i].ship_name,
								goalp[i].docker.name, goalp[i].dockee.name);

							str = "ai-dock";
						}
						break;

					case AI_GOAL_CHASE:
						str = "ai-chase";
						break;

					case AI_GOAL_CHASE_WING:
						str = "ai-chase-wing";
						break;

					case AI_GOAL_GUARD:
						str = "ai-guard";
						break;

					case AI_GOAL_GUARD_WING:
						str = "ai-guard-wing";
						break;

					case AI_GOAL_DISABLE_SHIP:
						str = "ai-disable-ship";
						break;

					case AI_GOAL_DISARM_SHIP:
						str = "ai-disarm-ship";
						break;

					case AI_GOAL_IGNORE:
						str = "ai-ignore";
						break;

					case AI_GOAL_IGNORE_NEW:
						str = "ai-ignore-new";
						break;

					case AI_GOAL_EVADE_SHIP:
						str = "ai-evade-ship";
						break;

					case AI_GOAL_STAY_NEAR_SHIP:
						str = "ai-stay-near-ship";
						break;

					case AI_GOAL_STAY_STILL:
						str = "ai-stay-still";
						break;

					default:
						Assert(0);
				}

				if (valid)
					fout("( %s %s %d ) ", str, buf, goalp[i].priority);
			}
		}

		fso_comment_pop();
	}

	if (!flag)
		fout(")");

	fso_comment_pop(true);
}

int CFred_mission_save::save_events()
{
	char out[4096];
	int i;

	fred_parse_flag = 0;
	required_string_fred("#Events");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_mission_events);

	for (i=0; i<Num_mission_events; i++) {
		required_string_either_fred("$Formula:", "#Goals");
		required_string_fred("$Formula:");
		parse_comments(i ? 2 : 1);
		convert_sexp_to_string(Mission_events[i].formula, out, SEXP_SAVE_MODE, 4096);
		fout(" %s", out);

		if (*Mission_events[i].name) {
			if (optional_string_fred("+Name:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Name:");
			}

			fout(" %s", Mission_events[i].name);
		}

		if ( optional_string_fred("+Repeat Count:", "$Formula:")){
			parse_comments();
		} else {
			fout("\n+Repeat Count:");
		}

		fout(" %d", Mission_events[i].repeat_count);

		if (Format_fs2_open != FSO_FORMAT_RETAIL && Mission_events[i].trigger_count != 1 ) {
			if ( optional_string_fred("+Trigger Count:", "$Formula:")){
				parse_comments();
			} else {
				fso_comment_push(";;FSO 3.6.11;;");
				fout_version("\n+Trigger Count:");
				fso_comment_pop(); 
			}

			fout(" %d", Mission_events[i].trigger_count);
		}

		if ( optional_string_fred("+Interval:", "$Formula:")){
			parse_comments();
		} else {
			fout("\n+Interval:");
		}

		fout(" %d", Mission_events[i].interval);

		if ( Mission_events[i].score != 0 ) {
			if ( optional_string_fred("+Score:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Score:");
			}
			fout(" %d", Mission_events[i].score);
		}

		if ( Mission_events[i].chain_delay >= 0 ) {
			if ( optional_string_fred("+Chained:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Chained:");
			}

			fout(" %d", Mission_events[i].chain_delay);
		}

		//XSTR
		if (Mission_events[i].objective_text) {
			if (optional_string_fred("+Objective:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Objective:");
			}

			fout_ext(" ", "%s", Mission_events[i].objective_text);
		}

		//XSTR
		if (Mission_events[i].objective_key_text) {
			if (optional_string_fred("+Objective key:", "$Formula:")){
				parse_comments();
			} else {
				fout("\n+Objective key:");
			}

			fout_ext(" ", "%s", Mission_events[i].objective_key_text);
		}

		// save team
		if (Mission_events[i].team >= 0){
			if (optional_string_fred("+Team:")){
				parse_comments();
			} else {
				fout("\n+Team:");
			} 
			fout(" %d", Mission_events[i].team);
		}

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_reinforcements()
{
	int i, j, type;

	fred_parse_flag = 0;
	required_string_fred("#Reinforcements");
	parse_comments(2);
	fout("\t\t;! %d total\n", Num_reinforcements);

	for (i=0; i<Num_reinforcements; i++) {
		required_string_either_fred("$Name:", "#Background bitmaps");
		required_string_fred("$Name:");
		parse_comments(i ? 2 : 1);
		fout(" %s", Reinforcements[i].name);

		type = TYPE_ATTACK_PROTECT;
		for (j=0; j<MAX_SHIPS; j++)
			if ((Ships[j].objnum != -1) && !stricmp(Ships[j].ship_name, Reinforcements[i].name)) {
				if (Ship_info[Ships[j].ship_info_index].flags & SIF_SUPPORT)
					type = TYPE_REPAIR_REARM;
				break;
			}

		required_string_fred("$Type:");
		parse_comments();
		fout(" %s", Reinforcement_type_names[type]);

		required_string_fred("$Num times:");
		parse_comments();
		fout(" %d", Reinforcements[i].uses);

		if ( optional_string_fred("+Arrival Delay:", "$Name:"))
			parse_comments();
		else
			fout("\n+Arrival Delay:");
		fout(" %d", Reinforcements[i].arrival_delay );

		if (optional_string_fred("+No Messages:", "$Name:"))
			parse_comments();
		else
			fout("\n+No Messages:");
		fout(" (");
		for (j = 0; j < MAX_REINFORCEMENT_MESSAGES; j++) {
			if ( strlen(Reinforcements[i].no_messages[j]) )
				fout(" \"%s\"", Reinforcements[i].no_messages[j]);
		}
		fout(" )");

		if (optional_string_fred("+Yes Messages:", "$Name:"))
			parse_comments();
		else
			fout("\n+Yes Messages:");
		fout(" (");
		for (j = 0; j < MAX_REINFORCEMENT_MESSAGES; j++) {
			if ( strlen(Reinforcements[i].yes_messages[j]) )
				fout(" \"%s\"", Reinforcements[i].yes_messages[j]);
		}
		fout(" )");

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_bitmaps()
{	
	int i;
	uint j;

	fred_parse_flag = 0;
	required_string_fred("#Background bitmaps");
	parse_comments(2);
	fout("\t\t;! %d total\n", stars_get_num_bitmaps());

	required_string_fred("$Num stars:");
	parse_comments();
	fout(" %d", Num_stars);

	required_string_fred("$Ambient light level:");
	parse_comments();
	fout(" %d", The_mission.ambient_light_level);

	// neb2 stuff
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		required_string_fred("+Neb2:");
		parse_comments();
		fout(" %s\n", Neb2_texture_name);

		required_string_fred("+Neb2Flags:");
		parse_comments();
		fout(" %d\n", Neb2_poof_flags);
	}
	// neb 1 stuff
	else {
		if (Nebula_index >= 0) {
			if (optional_string_fred("+Nebula:")){
				parse_comments();
			} else {
				fout("\n+Nebula:");
			}		
			fout(" %s", Nebula_filenames[Nebula_index]);		

			required_string_fred("+Color:");
			parse_comments();
			fout(" %s", Nebula_colors[Mission_palette]);

			required_string_fred("+Pitch:");
			parse_comments();
			fout(" %d", Nebula_pitch);

			required_string_fred("+Bank:");
			parse_comments();
			fout(" %d", Nebula_bank);

			required_string_fred("+Heading:");
			parse_comments();
			fout(" %d", Nebula_heading);
		}
	}

	fso_comment_pop();

	// Goober5000 - save all but the lowest priority using the special comment tag
	for (i = 0; i < Num_backgrounds; i++)
	{
		bool tag = (i < Num_backgrounds - 1);
		background_t *background = &Backgrounds[i];

		if (optional_string_fred("$Bitmap List:")) {
			parse_comments(2);
		} else {
			fso_comment_push(";;FSO 3.6.9;;");
			fout_version("\n\n$Bitmap List:");
		}

		if ( !tag ) {
			fso_comment_pop(true);
		}

		// save suns by filename
		for (j = 0; j < background->suns.size(); j++)
		{
			starfield_list_entry *sle = &background->suns[j];

			// filename
			required_string_fred("$Sun:");
			parse_comments();
			fout(" %s", sle->filename);

			// angles
	 		required_string_fred("+Angles:");
			parse_comments();
			fout(" %f %f %f", sle->ang.p, sle->ang.b, sle->ang.h);

			// scale
	 		required_string_fred("+Scale:");
			parse_comments();
			fout(" %f", sle->scale_x);
		}

		// save background bitmaps by filename
		for (j = 0; j < background->bitmaps.size(); j++)
		{
			starfield_list_entry *sle = &background->bitmaps[j];

			// filename
			required_string_fred("$Starbitmap:");
	 		parse_comments();
			fout(" %s", sle->filename);

			// angles
	 		required_string_fred("+Angles:");
			parse_comments();
			fout(" %f %f %f", sle->ang.p, sle->ang.b, sle->ang.h);

			// scale
	 		required_string_fred("+ScaleX:");
			parse_comments();
			fout(" %f", sle->scale_x);
			required_string_fred("+ScaleY:");
			parse_comments();
			fout(" %f", sle->scale_y);

			// div
	 		required_string_fred("+DivX:");
	 		parse_comments();
			fout(" %d", sle->div_x);
	 		required_string_fred("+DivY:");
			parse_comments();
			fout(" %d", sle->div_y);
		}

		fso_comment_pop();
 	}

	// taylor's environment map thingy
	if (strlen(The_mission.envmap_name) > 0) {
		if (optional_string_fred("$Environment Map:")) {
			parse_comments(2);
			fout(" %s", The_mission.envmap_name);
		} else {
			fso_comment_push(";;FSO 3.6.9;;");
			fout_version("\n\n$Environment Map: %s", The_mission.envmap_name);
			fso_comment_pop();
		}
	} else {
		bypass_comment(";;FSO 3.6.9;; $Environment Map:");
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_asteroid_fields()
{
	int i, idx;

	fred_parse_flag = 0;
	required_string_fred("#Asteroid Fields");
	parse_comments(2);

	for (i=0; i<1 /*MAX_ASTEROID_FIELDS*/; i++) {
		if (!Asteroid_field.num_initial_asteroids)
			continue;

		required_string_fred("$Density:");
		parse_comments(2);
		fout(" %d", Asteroid_field.num_initial_asteroids);

		// field type
		if (optional_string_fred("+Field Type:")){
			parse_comments();
		} else {
			fout("\n+Field Type:");
		}
		fout(" %d", Asteroid_field.field_type);

		// debris type
		if (optional_string_fred("+Debris Genre:")){
			parse_comments();
		} else {
			fout("\n+Debris Genre:");
		}
		fout(" %d", Asteroid_field.debris_genre);

		// field_debris_type (only if ship genre)
		if (Asteroid_field.debris_genre == DG_SHIP) {
			for (int idx=0; idx<3; idx++) {
				if (Asteroid_field.field_debris_type[idx] != -1) {
					if (optional_string_fred("+Field Debris Type:")){
						parse_comments();
					} else {
						fout("\n+Field Debris Type:");
					}
					fout(" %d", Asteroid_field.field_debris_type[idx]);
				}
			}
		} else {
			// asteroid subtypes stored in field_debris_type as -1 or 1
			for (idx=0; idx<3; idx++) {
				if (Asteroid_field.field_debris_type[idx] != -1) {
					if (optional_string_fred("+Field Debris Type:")){
						parse_comments();
					} else {
						fout("\n+Field Debris Type:");
					}
					fout(" %d", idx);
				}
			}
		}


		required_string_fred("$Average Speed:");
		parse_comments();
		fout(" %f", vm_vec_mag(&Asteroid_field.vel));

		required_string_fred("$Minimum:");
		parse_comments();
		save_vector(Asteroid_field.min_bound);

		required_string_fred("$Maximum:");
		parse_comments();
		save_vector(Asteroid_field.max_bound);

		if (Asteroid_field.has_inner_bound == 1) {
			if (optional_string_fred("+Inner Bound:")){
				parse_comments();
			} else {
				fout("\n+Inner Bound:");
			}

			required_string_fred("$Minimum:");
			parse_comments();
			save_vector(Asteroid_field.inner_min_bound);

			required_string_fred("$Maximum:");
			parse_comments();
			save_vector(Asteroid_field.inner_max_bound);
		}

		fso_comment_pop();
	}

	fso_comment_pop(true);

	return err;
}

int CFred_mission_save::save_music()
{
	required_string_fred("#Music");
	parse_comments(2);

	required_string_fred("$Event Music:");
	parse_comments(2);
	if (Current_soundtrack_num < 0)
		fout(" None");
	else
		fout(" %s", Soundtracks[Current_soundtrack_num].name);

	// Goober5000 - save using the special comment prefix
	if (stricmp(The_mission.substitute_event_music_name, "None")) {
		if (optional_string_fred("$Substitute Event Music:")) {
			parse_comments(1);
			fout(" %s", The_mission.substitute_event_music_name);
		} else {
			fso_comment_push(";;FSO 3.6.9;;");
			fout_version("\n$Substitute Event Music: %s", The_mission.substitute_event_music_name);
			fso_comment_pop();
		}
	} else {
		bypass_comment(";;FSO 3.6.9;; $Substitute Event Music:");
	}

	required_string_fred("$Briefing Music:");
	parse_comments();
	if (Mission_music[SCORE_BRIEFING] < 0)
		fout(" None");
	else
		fout(" %s", Spooled_music[Mission_music[SCORE_BRIEFING]].name);

	// Goober5000 - save using the special comment prefix
	if (stricmp(The_mission.substitute_briefing_music_name, "None")) {
		if (optional_string_fred("$Substitute Briefing Music:")) {
			parse_comments(1);
			fout(" %s", The_mission.substitute_briefing_music_name);
		} else {
			fso_comment_push(";;FSO 3.6.9;;");
			fout_version("\n$Substitute Briefing Music: %s", The_mission.substitute_briefing_music_name);
			fso_comment_pop();
		}
	} else {
		bypass_comment(";;FSO 3.6.9;; $Substitute Briefing Music:");
	}

	// avoid keeping the old one around
	bypass_comment(";;FSO 3.6.8;; $Substitute Music:");

	// old stuff
	if (Mission_music[SCORE_DEBRIEF_SUCCESS] != event_music_get_spooled_music_index("Success")) {
		if (optional_string_fred("$Debriefing Success Music:")) {
			parse_comments(1);
		} else {
			fout("\n$Debriefing Success Music:");
		}
		fout(" %s", Mission_music[SCORE_DEBRIEF_SUCCESS] < 0 ? "None" : Spooled_music[Mission_music[SCORE_DEBRIEF_SUCCESS]].name);
	}
	if (Mission_music[SCORE_DEBRIEF_AVERAGE] != event_music_get_spooled_music_index("Average")) {
		if (optional_string_fred("$Debriefing Average Music:")) {
			parse_comments(1);
		} else {
			fout("\n$Debriefing Average Music:");
		}
		fout(" %s", Mission_music[SCORE_DEBRIEF_AVERAGE] < 0 ? "None" : Spooled_music[Mission_music[SCORE_DEBRIEF_AVERAGE]].name);
	}
	if (Mission_music[SCORE_DEBRIEF_FAIL] != event_music_get_spooled_music_index("Failure")) {
		if (optional_string_fred("$Debriefing Fail Music:")) {
			parse_comments(1);
		} else {
			fout("\n$Debriefing Fail Music:");
		}
		fout(" %s", Mission_music[SCORE_DEBRIEF_FAIL] < 0 ? "None" : Spooled_music[Mission_music[SCORE_DEBRIEF_FAIL]].name);
	}

	// Goober5000 - save using the special comment prefix
	if (mission_has_fiction() && Mission_music[SCORE_FICTION_VIEWER] >= 0) {
		if (optional_string_fred("$Fiction Viewer Music:")) {
			parse_comments(1);
			fout(" %s", Spooled_music[Mission_music[SCORE_FICTION_VIEWER]].name);
		} else {
			fso_comment_push(";;FSO 3.6.11;;");
			fout_version("\n$Fiction Viewer Music: %s", Spooled_music[Mission_music[SCORE_FICTION_VIEWER]].name);
			fso_comment_pop();
		}
	} else {
		bypass_comment(";;FSO 3.6.11;; $Fiction Viewer Music:");
	}

	fso_comment_pop(true);

	return err;
}

void CFred_mission_save::save_turret_info(ship_subsys *ptr, int ship)
{
	int i, z;
	ship_weapon *wp = &ptr->weapons;

	if (wp->ai_class != Ship_info[Ships[ship].ship_info_index].ai_class) {
		if (optional_string_fred("+AI Class:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+AI Class:");

		fout(" %s", Ai_class_names[wp->ai_class]);
	}

	z = 0;
	i = wp->num_primary_banks;
	while (i--)
		if (wp->primary_bank_weapons[i] != ptr->system_info->primary_banks[i])
			z = 1;

	if (z) {
		if (optional_string_fred("+Primary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Primary Banks:");

		fout(" ( ");
		for (i=0; i<wp->num_primary_banks; i++)
			fout("\"%s\" ", Weapon_info[wp->primary_bank_weapons[i]].name);

		fout(")");
	}

	z = 0;
	i = wp->num_secondary_banks;
	while (i--)
		if (wp->secondary_bank_weapons[i] != ptr->system_info->secondary_banks[i])
			z = 1;

	if (z) {
		if (optional_string_fred("+Secondary Banks:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Secondary Banks:");

		fout(" ( ");
		for (i=0; i<wp->num_secondary_banks; i++)
			fout("\"%s\" ", Weapon_info[wp->secondary_bank_weapons[i]].name);

		fout(")");
	}

	z = 0;
	i = wp->num_secondary_banks;
	while (i--)
		if (wp->secondary_bank_ammo[i] != 100)
			z = 1;

	if (z) {
		if (optional_string_fred("+Sbank Ammo:", "$Name:", "+Subsystem:"))
			parse_comments();
		else
			fout("\n+Sbank Ammo:");

		fout(" ( ");
		for (i=0; i<wp->num_secondary_banks; i++)
			fout("%d ", wp->secondary_bank_ammo[i]);

		fout(")");
	}

	fso_comment_pop(true);
}

int CFred_mission_save::save_campaign_file(char *pathname)
{
	int i, j, m, flag;

	Campaign_tree_formp->save_tree();  // flush all changes so they get saved.
	Campaign_tree_viewp->sort_elements();
	reset_parse();
	fred_parse_flag = 0;

	pathname = cf_add_ext(pathname, FS_CAMPAIGN_FILE_EXT);
	fp = cfopen(pathname, "wt", CFILE_NORMAL, CF_TYPE_MISSIONS);
	if (!fp)	{
		nprintf(("Error", "Can't open campaign file to save.\n"));
		return -1;
	}

	required_string_fred("$Name:");
	parse_comments(0);
	fout(" %s", Campaign.name);

	Assert((Campaign.type >= 0) && (Campaign.type < MAX_CAMPAIGN_TYPES));
	required_string_fred("$Type:");
	parse_comments();
	fout(" %s", campaign_types[Campaign.type]);

	// XSTR
	if (Campaign.desc) {
		required_string_fred("+Description:");
		parse_comments();
		fout_ext("\n", "%s", Campaign.desc);
		fout("\n$end_multi_text");
	}

	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		required_string_fred("+Num Players:");
		parse_comments();
		fout(" %d", Campaign.num_players);
	}	

	// campaign flags - Goober5000
	if (Format_fs2_open != FSO_FORMAT_RETAIL)
	{
		optional_string_fred("$Flags:");
		parse_comments();
		fout(" %d\n", Campaign.flags);
	}

	// write out the ships and weapons which the player can start the campaign with
	optional_string_fred("+Starting Ships: (");
	parse_comments(2);
	for (i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		if ( Campaign.ships_allowed[i] )
			fout(" \"%s\" ", Ship_info[i].name );
	}
	fout( ")\n" );

	optional_string_fred("+Starting Weapons: (");
	parse_comments();
	for (i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		if ( Campaign.weapons_allowed[i] )
			fout(" \"%s\" ", Weapon_info[i].name );
	}
	fout( ")\n" );

	fred_parse_flag = 0;
	for (i=0; i<Campaign.num_missions; i++) {
		m = Sorted[i];
		required_string_either_fred("$Mission:", "#End");
		required_string_fred("$Mission:");
		parse_comments(2);
		fout(" %s", Campaign.missions[m].name);

		if ( strlen(Campaign.missions[i].briefing_cutscene) ) {
			if (optional_string_fred("+Briefing Cutscene:", "$Mission"))
				parse_comments();
			else
				fout("\n+Briefing Cutscene:");

			fout( " %s", Campaign.missions[i].briefing_cutscene );
		}

		required_string_fred("+Flags:", "$Mission:");
		parse_comments();

		// Goober5000
		if (Format_fs2_open != FSO_FORMAT_RETAIL)
		{
			// don't save Bastion flag
			fout(" %d", Campaign.missions[m].flags & ~CMISSION_FLAG_BASTION);

			// new main hall stuff
			if (optional_string_fred("+Main Hall:", "$Mission:"))
				parse_comments();
			else
				fout("\n+Main Hall:");

			fout(" %d", Campaign.missions[m].main_hall);
		}
		else
		{
			// save Bastion flag properly
			fout(" %d", Campaign.missions[m].flags | ((Campaign.missions[m].main_hall > 0) ? CMISSION_FLAG_BASTION : 0));
		}

		if ( (Campaign.missions[m].debrief_persona_index >= 0) && (Campaign.missions[m].debrief_persona_index <= 0xff) ) {
			if (optional_string_fred("+Debriefing Persona Index:")) {
				parse_comments(1);
				fout(" %d", Campaign.missions[m].debrief_persona_index);
			} else {
				fso_comment_push(";;FSO 3.6.8;;");
				fout_version("\n+Debriefing Persona Index: %d", Campaign.missions[m].debrief_persona_index);
				fso_comment_pop();
			}
		} else {
			bypass_comment(";;FSO 3.6.8;; +Debriefing Persona Index:");
		}

		// save campaign link sexp
		bool mission_loop = false;
		flag = 0;
		for (j=0; j<Total_links; j++) {
			if (Links[j].from == m) {
				if (!flag) {
					if (optional_string_fred("+Formula:", "$Mission:"))
						parse_comments();
					else
						fout("\n+Formula:");

					fout(" ( cond\n");
					flag = 1;
				}

				//save_campaign_sexp(Links[j].sexp, Campaign.missions[Links[j].to].name);
				if (Links[j].mission_loop) {
					mission_loop = true;
				} else {
					save_campaign_sexp(Links[j].sexp, Links[j].to);
				}
			}
		}

		if (flag) {
			fout(")");
		}

		// now save campaign loop sexp
		if (mission_loop) {
			required_string_fred("\n+Mission Loop:");
			parse_comments();

			int num_mission_loop = 0;
			for (j=0; j<Total_links; j++) {
				if ( (Links[j].from == m) && (Links[j].mission_loop) ) {

					num_mission_loop++;

					// maybe write out mission loop descript
					if ((num_mission_loop == 1) && Links[j].mission_loop_txt) {
						required_string_fred("+Mission Loop Text:");
						parse_comments();
						fout_ext("\n", "%s", Links[j].mission_loop_txt);
						fout("\n$end_multi_text");
					}

					// maybe write out mission loop descript
					if ((num_mission_loop == 1) && Links[j].mission_loop_brief_anim) {
						required_string_fred("+Mission Loop Brief Anim:");
						parse_comments();
						fout_ext("\n", "%s", Links[j].mission_loop_brief_anim);
						fout("\n$end_multi_text");
					}

					// maybe write out mission loop descript
					if ((num_mission_loop == 1) && Links[j].mission_loop_brief_sound) {
						required_string_fred("+Mission Loop Brief Sound:");
						parse_comments();
						fout_ext("\n", "%s", Links[j].mission_loop_brief_sound);
						fout("\n$end_multi_text");
					}

					if (num_mission_loop == 1) {
						// write out mission loop formula
						fout("\n+Formula:");
						fout(" ( cond\n");
						save_campaign_sexp(Links[j].sexp, Links[j].to);
						fout(")");
					}
				}
			}
			if (num_mission_loop > 1) {
				char buffer[1024];
				sprintf(buffer, "Multiple branching loop error from mission %s\nEdit campaign for *at most* 1 loop from each mission.", Campaign.missions[m].name);
				MessageBox((HWND)os_get_window(), buffer, "Error", MB_OK);
			}
		}

		if (optional_string_fred("+Level:", "$Mission:")){
			parse_comments();
		} else {
			fout("\n\n+Level:");
		}

		fout(" %d", Campaign.missions[m].level);

		if (optional_string_fred("+Position:", "$Mission:")){
			parse_comments();
		} else {
			fout("\n+Position:");
		}

		fout(" %d", Campaign.missions[m].pos);

		fso_comment_pop();
	}

	required_string_fred("#End");
	parse_comments(2);
	token_found = NULL;
	parse_comments();
	fout("\n");

	cfclose(fp);
	if (err)
		mprintf(("Campaign saving error code #%d\n", err));
	else
		Campaign_wnd->error_checker();

	fso_comment_pop(true);

	return err;
}

void CFred_mission_save::save_campaign_sexp(int node, int link_num)
{
	char out[4096];

	Sexp_string = out;
	*out = 0;
	Assert(node >= 0);

	// if the link num is -1, then this is a end-of-campaign location
	if ( link_num != -1 ) {
		if (build_sexp_string(node, 2, SEXP_SAVE_MODE, 4096)) {
			fout("   (\n      %s\n      ( next-mission \"%s\" )\n   )\n", out, Campaign.missions[link_num].name);
		} else {
			fout("   ( %s( next-mission \"%s\" ) )\n", out, Campaign.missions[link_num].name);
		}
	} else {
		if (build_sexp_string(node, 2, SEXP_SAVE_MODE, 4096)) {
			fout("   (\n      %s\n      ( end-of-campaign )\n   )\n", out);
		} else {
			fout("   ( %s( end-of-campaign ) )\n", out );
		}
	}
}

void CFred_mission_save::fso_comment_push(char *ver)
{
	if ( fso_ver_comment.empty() ) {
		fso_ver_comment.push_back( std::string(ver) );
		return;
	}

	std::string before = fso_ver_comment.back();

	int major, minor, build, revis;
	int in_major, in_minor, in_build, in_revis;
	int elem1, elem2;

	elem1 = scan_fso_version_string( fso_ver_comment.back().c_str(), &major, &minor, &build, &revis );
	elem2 = scan_fso_version_string( ver, &in_major, &in_minor, &in_build, &in_revis );
	
	// check consistency
	if (elem1 == 3 && elem2 == 4 || elem1 == 4 && elem2 == 3) {
		elem1 = elem2 = 3;
	} else if ((elem1 >= 3 && elem2 >= 3) && (revis < 1000 || in_revis < 1000)) {
		elem1 = elem2 = 3;
	}

	if ( (elem1 == 3) && ((major > in_major) || (minor > in_minor) || (build > in_build)) ) {
		// the push'd version is older than our current version, so just push a copy of the previous version
		fso_ver_comment.push_back( before );
	} else if ( (elem1 == 4) && ((major > in_major) || (minor > in_minor) || (build > in_build) || (revis > in_revis)) ) {
		// the push'd version is older than our current version, so just push a copy of the previous version
		fso_ver_comment.push_back( before );
	} else {
		fso_ver_comment.push_back( std::string(ver) );
	}
}

void CFred_mission_save::fso_comment_pop(bool pop_all)
{
	if ( fso_ver_comment.empty() ) {
		return;
	}

	if (pop_all) {
		fso_ver_comment.clear();
		return;
	}

	fso_ver_comment.pop_back();
}

