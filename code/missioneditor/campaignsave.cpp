#include "campaignsave.h"

#include "globalincs/linklist.h"
#include "globalincs/version.h"

#include "mission/missioncampaign.h"
#include "parse/parselo.h"

#include <freespace.h>

// Bodies copied from the old implementations in missionsave.cpp,
// unchanged except for the new class name and include.
int Fred_campaign_save::save_campaign_file(const char* pathname)
{
	// This is original FRED code. These were moved to the call sites as the data should be fully
	// prepared before calling this function.
	// TODO check QtFRED's tree is sorted and saved before calling this function.
	// Campaign_tree_formp->save_tree(); // flush all changes so they get saved.
	// Campaign_tree_viewp->sort_elements();

	reset_parse();
	raw_ptr = Parse_text_raw;
	fred_parse_flag = 0;

	pathname = cf_add_ext(pathname, FS_CAMPAIGN_FILE_EXT);
	fp = cfopen(pathname, "wt", CF_TYPE_MISSIONS);
	if (!fp) {
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

	if (Campaign.type != CAMPAIGN_TYPE_SINGLE) {
		required_string_fred("+Num Players:");
		parse_comments();
		fout(" %d", Campaign.num_players);
	}

	// campaign flags - Goober5000
	if (save_config.save_format != MissionFormat::RETAIL) {
		optional_string_fred("$Flags:");
		parse_comments();
		fout(" %d\n", Campaign.flags);
	}

	if (save_config.save_format != MissionFormat::RETAIL && !Campaign.custom_data.empty()) {
		if (optional_string_fred("$begin_custom_data_map")) {
			parse_comments(2);
		} else {
			fout("\n$begin_custom_data_map");
		}

		for (const auto& pair : Campaign.custom_data) {
			fout("\n   +Val: %s %s", pair.first.c_str(), pair.second.c_str());
		}

		if (optional_string_fred("$end_custom_data_map")) {
			parse_comments();
		} else {
			fout("\n$end_custom_data_map");
		}
	}

	// write out the ships and weapons which the player can start the campaign with
	optional_string_fred("+Starting Ships: (");
	parse_comments(2);
	for (int i = 0; i < ship_info_size(); i++) {
		if (Campaign.ships_allowed[i])
			fout(" \"%s\" ", Ship_info[i].name);
	}
	fout(")\n");

	optional_string_fred("+Starting Weapons: (");
	parse_comments();
	for (int i = 0; i < weapon_info_size(); i++) {
		if (Campaign.weapons_allowed[i])
			fout(" \"%s\" ", Weapon_info[i].name);
	}
	fout(")\n");

	fred_parse_flag = 0;
	for (int i = 0; i < Campaign.num_missions; i++) {
		// Expect to get Campaign.missions ordered from FRED
		cmission& cm = Campaign.missions[i];

		required_string_either_fred("$Mission:", "#End");
		required_string_fred("$Mission:");
		parse_comments(2);
		fout(" %s", cm.name);

		if (strlen(cm.briefing_cutscene)) {
			if (optional_string_fred("+Briefing Cutscene:", "$Mission"))
				parse_comments();
			else
				fout("\n+Briefing Cutscene:");

			fout(" %s", cm.briefing_cutscene);
		}

		required_string_fred("+Flags:", "$Mission:");
		parse_comments();

		// don't save any internal flags
		auto flags_to_save = cm.flags & CMISSION_EXTERNAL_FLAG_MASK;

		// Goober5000
		if (save_config.save_format != MissionFormat::RETAIL) {
			// don't save Bastion flag
			fout(" %d", flags_to_save & ~CMISSION_FLAG_BASTION);

			// new main hall stuff
			if (optional_string_fred("+Main Hall:", "$Mission:"))
				parse_comments();
			else
				fout("\n+Main Hall:");

			fout(" %s", cm.main_hall.c_str());
		} else {
			// save Bastion flag properly
			fout(" %d", flags_to_save | ((cm.main_hall != "") ? CMISSION_FLAG_BASTION : 0));
		}

		if (!cm.substitute_main_hall.empty()) {
			fso_comment_push(";;FSO 3.7.2;;");
			if (optional_string_fred("+Substitute Main Hall:")) {
				parse_comments(1);
				fout(" %s", cm.substitute_main_hall.c_str());
			} else {
				fout_version("\n+Substitute Main Hall: %s", cm.substitute_main_hall.c_str());
			}
			fso_comment_pop();
		} else {
			bypass_comment(";;FSO 3.7.2;; +Substitute Main Hall:");
		}

		if (cm.debrief_persona_index > 0) {
			fso_comment_push(";;FSO 3.6.8;;");
			if (optional_string_fred("+Debriefing Persona Index:")) {
				parse_comments(1);
				fout(" %d", cm.debrief_persona_index);
			} else {
				fout_version("\n+Debriefing Persona Index: %d", cm.debrief_persona_index);
			}
			fso_comment_pop();
		} else {
			bypass_comment(";;FSO 3.6.8;; +Debriefing Persona Index:");
		}

		// new save cmission sexps
		if (optional_string_fred("+Formula:", "$Mission:")) {
			parse_comments();
		} else {
			fout("\n+Formula:");
		}

		{
			SCP_string sexp_out{};
			convert_sexp_to_string(sexp_out, cm.formula, SEXP_SAVE_MODE);
			fout(" %s", sexp_out.c_str());
		}

		bool mission_loop = cm.flags & CMISSION_FLAG_HAS_LOOP;

		Assertion(cm.flags ^ CMISSION_FLAG_HAS_FORK,
			"scpFork campaigns cannot be saved, use axemFork.\n Should be detected on load.");

		if (mission_loop) {
			required_string_fred("\n+Mission Loop:");
			parse_comments();

			if (cm.mission_branch_desc) {
				required_string_fred("+Mission Loop Text:");
				parse_comments();
				fout_ext("\n", "%s", cm.mission_branch_desc);
				fout("\n$end_multi_text");
			}

			if (cm.mission_branch_brief_anim) {
				required_string_fred("+Mission Loop Brief Anim:");
				parse_comments();
				fout_ext("\n", "%s", cm.mission_branch_brief_anim);
				fout("\n$end_multi_text");
			}

			if (cm.mission_branch_brief_sound) {
				required_string_fred("+Mission Loop Brief Sound:");
				parse_comments();
				fout_ext("\n", "%s", cm.mission_branch_brief_sound);
				fout("\n$end_multi_text");
			}

			// write out mission loop formula
			fout("\n+Formula:");
			{
				SCP_string sexp_out{};
				convert_sexp_to_string(sexp_out, cm.mission_loop_formula, SEXP_SAVE_MODE);
				fout(" %s", sexp_out.c_str());
			}
		}

		if (optional_string_fred("+Level:", "$Mission:")) {
			parse_comments();
		} else {
			fout("\n\n+Level:");
		}

		fout(" %d", cm.level);

		if (optional_string_fred("+Position:", "$Mission:")) {
			parse_comments();
		} else {
			fout("\n+Position:");
		}

		fout(" %d", cm.pos);

		fso_comment_pop();
	}

	required_string_fred("#End");
	parse_comments(2);
	token_found = NULL;
	parse_comments();
	fout("\n");

	cfclose(fp);
	fso_comment_pop(true);

	// Mission editor should run the error checker *before* calling the save function
	Assertion(!err, "Nothing in here should have a side effect to raise the mission error saving flag.");

	return err;
}

void Fred_campaign_save::save_campaign_sexp(int node, int link_num)
{
	SCP_string sexp_out;
	Assert(node >= 0);

	// if the link num is -1, then this is a end-of-campaign location
	if (link_num != -1) {
		if (build_sexp_string(sexp_out, node, 2, SEXP_SAVE_MODE)) {
			fout("   (\n      %s\n      ( next-mission \"%s\" )\n   )\n",
				sexp_out.c_str(),
				Campaign.missions[link_num].name);
		} else {
			fout("   ( %s( next-mission \"%s\" ) )\n", sexp_out.c_str(), Campaign.missions[link_num].name);
		}
	} else {
		if (build_sexp_string(sexp_out, node, 2, SEXP_SAVE_MODE)) {
			fout("   (\n      %s\n      ( end-of-campaign )\n   )\n", sexp_out.c_str());
		} else {
			fout("   ( %s( end-of-campaign ) )\n", sexp_out.c_str());
		}
	}
}