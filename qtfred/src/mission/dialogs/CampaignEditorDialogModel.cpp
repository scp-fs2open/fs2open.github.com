#include "CampaignEditorDialogModel.h"

#include "cfile/cfile.h"
#include "mission/missionparse.h"
#include "../src/mission/missionsave.h"
#include "parse/sexp.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "sound/audiostr.h"
#include "cutscene/cutscenes.h"
#include "menuui/mainhallmenu.h"

extern int Skip_packfile_search; // from cfilesystem.cpp

namespace fso::fred::dialogs {

CampaignEditorDialogModel::CampaignEditorDialogModel(QObject* parent, fso::fred::EditorViewport* viewport, ICampaignEditorTreeOps& tree_ops)
	: AbstractDialogModel(parent, viewport), m_tree_ops(tree_ops)
{
	initializeData();
}

bool CampaignEditorDialogModel::apply()
{
	stopSpeech();
	clearCampaignGlobal();
	return true;
}

void CampaignEditorDialogModel::reject()
{
	stopSpeech();
	clearCampaignGlobal();
}

void CampaignEditorDialogModel::initializeData(const char* filename)
{
	// Clear the internal state to ensure a fresh start.
	m_missions.clear();
	m_ships_allowed.clear();
	m_weapons_allowed.clear();
	m_available_mission_files.clear();

	// Decide whether to load from the global struct or set up a new campaign.
	if (filename) {
		// LOADING AN EXISTING CAMPAIGN

		// This assumes the global 'Campaign' struct has just been populated by a
		// call to mission_campaign_load() in the loadCampaignFromFile() method.

		// Copy simple properties from the global Campaign struct
		m_campaign_filename = Campaign.filename;
		m_campaign_name = Campaign.name;
		m_campaign_descr = Campaign.desc ? Campaign.desc : "";
		m_campaign_type = Campaign.type;
		m_num_players = Campaign.num_players;
		m_flags = Campaign.flags;
		m_custom_data = Campaign.custom_data;

		// Copy mission data from the global Campaign struct
		m_missions.reserve(Campaign.num_missions);
		for (int i = 0; i < Campaign.num_missions; ++i) {
			const auto& source_mission = Campaign.missions[i];
			auto& dest_mission = m_missions.emplace_back();

			dest_mission.filename = source_mission.name;
			dest_mission.level = source_mission.level;
			dest_mission.position = source_mission.pos;
			dest_mission.briefing_cutscene = source_mission.briefing_cutscene;
			dest_mission.main_hall = source_mission.main_hall;
			dest_mission.substitute_main_hall = source_mission.substitute_main_hall;
			dest_mission.debrief_persona_index = source_mission.debrief_persona_index;
			bool retail_bastion = (source_mission.flags & CMISSION_FLAG_BASTION) != 0;

			// Normalize explicit main hall flag
			if (retail_bastion) {
				dest_mission.main_hall = retail_bastion ? "1" : "0";
			}

			// Parse the SEXP formulas to build the branch data for this mission.
			parseBranchesFromFormula(dest_mission, source_mission.formula, false);
			if (source_mission.flags & CMISSION_FLAG_HAS_LOOP) {
				parseBranchesFromFormula(dest_mission, source_mission.mission_loop_formula, true);
			}

			bool anyLoop = false, anyFork = false;
			for (const auto& b : dest_mission.branches) {
				anyLoop |= b.is_loop;
				anyFork |= b.is_fork;
			}

			auto mode = CampaignSpecialMode::Loop;

			// If branches exist, effective mode follows the branch type.
			if (anyLoop || anyFork) {
				mode = anyLoop ? CampaignSpecialMode::Loop : CampaignSpecialMode::Fork;
			}

			dest_mission.special_mode_hint = mode;
		}

		// Copy ship and weapon permissions from the global Campaign struct
		m_ships_allowed.assign(Campaign.ships_allowed, Campaign.ships_allowed + MAX_SHIP_CLASSES);
		m_weapons_allowed.assign(Campaign.weapons_allowed, Campaign.weapons_allowed + MAX_WEAPON_TYPES);

	} else {
		// CREATING A NEW CAMPAIGN

		// Set up a clean, default state within the model.
		m_campaign_filename = "";
		m_campaign_name = "Unnamed";
		m_campaign_descr = "";
		m_campaign_type = CAMPAIGN_TYPE_SINGLE;
		m_num_players = 0;
		m_flags = CF_DEFAULT_VALUE;

		m_ships_allowed.assign(MAX_SHIP_CLASSES, false);
		m_weapons_allowed.assign(MAX_WEAPON_TYPES, false);
	}

	// Load the list of available mission files from the directory.
	loadAvailableMissions();

	// Set initial selection states to none.
	m_current_mission_index = -1;
	m_current_branch_index = -1;

	// Mark the model as unmodified since this is a fresh load or new state.
	_modified = false;
}

void CampaignEditorDialogModel::parseBranchesFromFormula(CampaignMissionData& mission, int formula_index, bool is_loop)
{
	if (formula_index < 0 || stricmp(CTEXT(formula_index), "cond")) {
		return;
	}

	// The formula is a 'cond' expression. We walk its branches.
	for (int branch_sexp = CDR(formula_index); branch_sexp != -1; branch_sexp = CDR(branch_sexp)) {
		auto& new_branch = mission.branches.emplace_back();
		addBranchIdIfMissing(new_branch);
		new_branch.is_loop = is_loop;

		// The first part of the branch is the condition SEXP.
		int condition_sexp = CAR(CAR(branch_sexp));

		// The second part is the action
		int action_sexp = CADR(CAR(branch_sexp));
		if (!stricmp(CTEXT(action_sexp), "next-mission")) {
			new_branch.next_mission_name = CTEXT(CDR(action_sexp));
		} else {
			// This is an "end-of-campaign" branch, so the name is empty.
			new_branch.next_mission_name = "";
		}

		// The model commands the UI's tree to load the SEXP.
		// The tree returns an internal ID, which we store.
		new_branch.sexp_formula = m_tree_ops.loadSexp(condition_sexp);
	}

	// If this was a loop, find the original cmission to copy the descriptive text.
	if (is_loop) {
		const cmission* source_cmission = nullptr;
		for (int i = 0; i < Campaign.num_missions; ++i) {
			if (mission.filename == Campaign.missions[i].name) {
				source_cmission = &Campaign.missions[i];
				break;
			}
		}

		if (source_cmission) {
			// All branches can have loop data. We apply the single
			// set of metadata from the file to all of them.
			for (auto& branch : mission.branches) {
				if (branch.is_loop) {
					branch.loop_description =
						source_cmission->mission_branch_desc ? source_cmission->mission_branch_desc : "";
					branch.loop_briefing_anim =
						source_cmission->mission_branch_brief_anim ? source_cmission->mission_branch_brief_anim : "";
					branch.loop_briefing_sound =
						source_cmission->mission_branch_brief_sound ? source_cmission->mission_branch_brief_sound : "";
				}
			}
		}
	}
}

void CampaignEditorDialogModel::loadAvailableMissions()
{	
	// Compatibility check lambda
	auto is_mission_compatible = [&](const mission& mission_info) -> bool {
		if (m_campaign_type == CAMPAIGN_TYPE_SINGLE) {
			return (mission_info.game_type & (MISSION_TYPE_SINGLE | MISSION_TYPE_TRAINING));
		}
		if (m_campaign_type == CAMPAIGN_TYPE_MULTI_COOP) {
			return (mission_info.game_type & MISSION_TYPE_MULTI_COOP) &&
				   (m_num_players == -1 || m_num_players == mission_info.num_players);
		}
		if (m_campaign_type == CAMPAIGN_TYPE_MULTI_TEAMS) {
			return (mission_info.game_type & MISSION_TYPE_MULTI_TEAMS) &&
				   (m_num_players == -1 || m_num_players == mission_info.num_players);
		}
		return false;
	};

	m_available_mission_files.clear();

	SCP_string search_pattern = "*" + SCP_string(FS_MISSION_FILE_EXT);

	// Get all editable mission files
	SCP_vector<SCP_string> editable_files;
	::Skip_packfile_search = 1; // This global flag forces the search to ignore VPs
	cf_get_file_list(editable_files, CF_TYPE_MISSIONS, search_pattern.c_str(), CF_SORT_NAME);
	::Skip_packfile_search = 0;

	// For quick lookups, put the editable filenames into a set
	std::unordered_set<SCP_string> editable_set(editable_files.begin(), editable_files.end());

	// Get all mission files including packaged ones
	SCP_vector<SCP_string> all_files;
	cf_get_file_list(all_files, CF_TYPE_MISSIONS, search_pattern.c_str(), CF_SORT_NAME);

	// Get missions already in the campaign
	std::unordered_set<SCP_string> active_missions;
	for (const auto& mission_data : m_missions) {
		active_missions.insert(mission_data.filename);
	}

	// Build the final list of available missions
	for (const auto& filename : all_files) {
		// Skip missions already in the campaign
		if (active_missions.count(filename + FS_MISSION_FILE_EXT)) {
			continue;
		}

		mission mission_info;
		get_mission_info(filename.c_str(), &mission_info);

		if (is_mission_compatible(mission_info)) {
			// Check if the filename exists in our set of loose, editable files
			bool is_editable = (editable_set.count(filename) > 0);
			m_available_mission_files.emplace_back(filename + FS_MISSION_FILE_EXT, is_editable);
		}
	}
}

void CampaignEditorDialogModel::clearCampaignGlobal()
{
	mission_campaign_clear();
}

void CampaignEditorDialogModel::commitWorkingCopyToGlobal()
{
	// Clear the global struct first
	mission_campaign_clear();

	// Copy simple properties
	strcpy_s(Campaign.name, m_campaign_name.c_str());
	Campaign.desc = m_campaign_descr.empty() ? nullptr : strdup(m_campaign_descr.c_str());
	Campaign.type = m_campaign_type;
	Campaign.num_players = m_num_players;
	Campaign.flags = m_flags;
	Campaign.num_missions = static_cast<int>(m_missions.size());
	Campaign.custom_data = m_custom_data;

	// Copy ship and weapon permissions
	for (int i = 0; i < MAX_SHIP_CLASSES; ++i) {
		Campaign.ships_allowed[i] = m_ships_allowed[i];
	}
	for (int i = 0; i < MAX_WEAPON_TYPES; ++i) {
		Campaign.weapons_allowed[i] = m_weapons_allowed[i];
	}

	// Copy mission data
	for (int i = 0; i < Campaign.num_missions; ++i) {
		const auto& source_mission = m_missions[i];
		auto& dest_mission = Campaign.missions[i];

		dest_mission.name = strdup(source_mission.filename.c_str());
		dest_mission.level = source_mission.level;
		dest_mission.pos = source_mission.position;
		SCP_string cutscene = source_mission.briefing_cutscene;
		if (cutscene == "<None>") {
			cutscene = "";
		}
		strcpy_s(dest_mission.briefing_cutscene, cutscene.c_str());

		if (m_save_format == CampaignFormat::Retail) {
			// It's unlikely that we could have any other value in this case but 1 or 0 but let's be explicit.
			dest_mission.main_hall = (source_mission.main_hall == "1") ? "1" : "0";
			if (dest_mission.main_hall == "1") {
				dest_mission.flags |= CMISSION_FLAG_BASTION;
			}
		} else {
			// persist the explicit +Main Hall: string; default to "0" if empty.
			SCP_string hall = source_mission.main_hall;
			if (hall == "<None>") {
				hall = "";
			}
			dest_mission.main_hall = hall.empty() ? "0" : hall;
		}

		SCP_string sub_hall = source_mission.substitute_main_hall;
		if (sub_hall == "<None>") {
			sub_hall = "";
		}
		dest_mission.substitute_main_hall = sub_hall;
		dest_mission.debrief_persona_index = source_mission.debrief_persona_index;

		// Convert the CampaignBranchData back into the cmission formula structure
		dest_mission.formula = -1;
		dest_mission.mission_loop_formula = -1;

		// We need to build a 'cond' SEXP for normal branches...
		int* normal_branch_ptr = &dest_mission.formula;
		bool normal_branches_exist = false;

		// ...and another 'cond' SEXP for loop branches
		int* loop_branch_ptr = &dest_mission.mission_loop_formula;
		bool loop_branches_exist = false;

		for (const auto& branch : source_mission.branches) {
			// First, save the branch's SEXP tree to get the final formula index
			int final_sexp_formula = m_tree_ops.saveSexp(branch.sexp_formula);

			// Create the next-mission part of the branch
			int action_sexp;
			if (!branch.next_mission_name.empty()) {
				action_sexp = alloc_sexp("next-mission",
					SEXP_ATOM,
					SEXP_ATOM_OPERATOR,
					-1,
					alloc_sexp(branch.next_mission_name.c_str(), SEXP_ATOM, SEXP_ATOM_STRING, -1, -1));
			} else {
				action_sexp = alloc_sexp("end-of-campaign", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			}

			// Create the full arm of the cond
			int cond_arm = alloc_sexp("",
				SEXP_LIST,
				-1,
				alloc_sexp("", SEXP_LIST, -1, final_sexp_formula, alloc_sexp("", SEXP_LIST, -1, action_sexp, -1)),
				-1);

			// Add the arm to the correct SEXP list
			if (branch.is_loop) {
				if (!loop_branches_exist) {
					// This is the first loop branch; create the parent 'cond' operator
					*loop_branch_ptr = alloc_sexp("cond", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
					loop_branch_ptr = &Sexp_nodes[*loop_branch_ptr].rest; // Point to the first argument slot
					loop_branches_exist = true;

					// Set the single loop properties on the cmission from the first loop branch we find
					dest_mission.flags |= CMISSION_FLAG_HAS_LOOP;
					dest_mission.mission_branch_desc = branch.loop_description.empty() ? nullptr : strdup(branch.loop_description.c_str());
					dest_mission.mission_branch_brief_anim = branch.loop_briefing_anim.empty() ? nullptr : strdup(branch.loop_briefing_anim.c_str());
					dest_mission.mission_branch_brief_sound = branch.loop_briefing_sound.empty() ? nullptr : strdup(branch.loop_briefing_sound.c_str());
				}
				*loop_branch_ptr = cond_arm;
				loop_branch_ptr = &Sexp_nodes[*loop_branch_ptr].rest;
			} else {
				if (!normal_branches_exist) {
					// This is the first normal branch; create the parent 'cond' operator
					*normal_branch_ptr = alloc_sexp("cond", SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
					normal_branch_ptr = &Sexp_nodes[*normal_branch_ptr].rest; // Point to the first argument slot
					normal_branches_exist = true;
				}
				*normal_branch_ptr = cond_arm;
				normal_branch_ptr = &Sexp_nodes[*normal_branch_ptr].rest;
			}
		}
	}
}

void CampaignEditorDialogModel::sortMissions()
{
	std::sort(m_missions.begin(),
		m_missions.end(),
		[](const CampaignMissionData& a, const CampaignMissionData& b) -> bool {
			if (a.level != b.level) {
				return a.level < b.level;
			}
			return a.position < b.position;
		});
}

void CampaignEditorDialogModel::stopSpeech()
{
	if (_waveId >= -1) {
		audiostream_close_file(_waveId, false);
		_waveId = -1;
	}
}

SCP_vector<SCP_string> CampaignEditorDialogModel::getCampaignTypes()
{
	SCP_vector<SCP_string> types;
	for (auto& type : campaign_types) {
		types.emplace_back(type);
	}
	
	return types;
}

void CampaignEditorDialogModel::createNewCampaign()
{
	stopSpeech();

	// First, clear the global state to ensure a clean load.
	clearCampaignGlobal();

	// Initialize the model to a clean "new campaign" state.
	initializeData();
}
	

void CampaignEditorDialogModel::loadCampaignFromFile(const SCP_string& filename)
{
	stopSpeech();

	// First, clear the global state to ensure a clean load.
	clearCampaignGlobal();

	// Attempt to load the selected file into the global Campaign struct.
	// We pass the full path via the filename argument now.
	if (mission_campaign_load(filename.c_str(), filename.c_str(), nullptr, 0) != 0) {
		// Load failed. Reset the model to a clean "new campaign" state.
		initializeData(nullptr);
		clearCampaignGlobal(); // Ensure cleanup after failed load
		return;
	}

	// Load was successful. Now, copy the data from the global struct
	// into our model's private working copy.
	initializeData(filename.c_str());

	// Immediately clear the global struct again now that we have our safe working copy.
	clearCampaignGlobal();
}

void CampaignEditorDialogModel::saveCampaign(const SCP_string& filename)
{
	stopSpeech();
	
	SCP_string target_filename = filename.empty() ? m_campaign_filename : filename;

	// First, validate the filename itself.
	if (target_filename.empty()) {
		// Cannot save if there's no filename. The UI should have prompted for one.
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Save Error",
			"No filename provided.",
			{DialogButton::Ok});
		return;
	}

	if (!checkValidity()) {
		return;
	}

	// Copy our working data to the global Campaign struct.
	commitWorkingCopyToGlobal();

	// Call the global save function.
	CFred_mission_save mission_saver;
	if (mission_saver.save_campaign_file(target_filename.c_str())) {
		// Save failed, clean up the global.
		clearCampaignGlobal();
		return;
	}

	// On success, update our internal state.
	modify(m_campaign_filename, target_filename);

	// Clean up the global struct now that the save is complete.
	clearCampaignGlobal();
}

bool CampaignEditorDialogModel::checkValidity()
{
	SCP_string error_message;

	// Campaign Name
	if (m_campaign_name.empty() || m_campaign_name == "Unnamed") { // Checking against "unnamed" is arbitrary and I hate it
		error_message += "Campaign must have a valid name.\n";
	}

	// First Mission
	// TODO: We will need a way to set the first mission. For now, we'll assume the first in the list.
	if (!m_missions.empty() && m_current_mission_index == -1) { // Assuming first mission is the one at index 0
		// This logic will need to be updated once we have a way to set the root mission.
		// For now, we'll check if there's at least one mission.
	} else if (m_missions.empty()) {
		error_message += "Campaign must have at least one mission.\n";
	}

	// Duplicate Mission Names
	std::unordered_set<SCP_string> mission_names;
	for (const auto& mission : m_missions) {
		if (!mission_names.insert(mission.filename).second) {
			error_message += "Mission \"" + mission.filename + "\" is included more than once in the campaign.\n";
		}
	}

	// Multiplayer Player Count Mismatch
	if (m_campaign_type != CAMPAIGN_TYPE_SINGLE) {
		for (const auto& mission_data : m_missions) {
			mission mission_info;
			get_mission_info(mission_data.filename.c_str(), &mission_info);
			if (mission_info.num_players != m_num_players) {
				error_message += "Mission \"" + mission_data.filename + "\" has " +
								 std::to_string(mission_info.num_players) + " players, but the campaign is set to " +
								 std::to_string(m_num_players) + " players.\n";
			}
		}
	}

	// Broken Branches
	for (const auto& mission : m_missions) {
		for (const auto& branch : mission.branches) {
			if (!branch.next_mission_name.empty()) {
				// Check if the target mission actually exists in the campaign
				auto it = std::find_if(m_missions.begin(), m_missions.end(), [&](const CampaignMissionData& m) {
					return m.filename == branch.next_mission_name;
				});

				if (it == m_missions.end()) {
					error_message += "Mission \"" + mission.filename +
									 "\" has a broken branch pointing to a non-existent mission \"" +
									 branch.next_mission_name + "\".\n";
				}
			}
		}
	}

	// TODO: Add deeper validation of SEXP logic (e.g., always-false branches, last branch is always true)

	// If we found any errors, display them all and return false.
	if (!error_message.empty()) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Campaign Errors",
			"Please correct the following issues before saving:\n\n" + error_message,
			{DialogButton::Ok});
		return false;
	}

	// If we get here, everything is valid.
	return true;
}

CampaignFormat CampaignEditorDialogModel::getSaveFormat() const
{
	return m_save_format;
}

void CampaignEditorDialogModel::setSaveFormat(CampaignFormat fmt)
{
	if (m_save_format != fmt) {
		m_save_format = fmt;
		set_modified();
	}
}

void CampaignEditorDialogModel::setCurrentMissionSelection(int index)
{
	stopSpeech();
	
	// Update the model's tracked mission index.
	m_current_mission_index = index;

	// When a new mission is selected, any previous branch selection is no longer valid.
	m_current_branch_index = -1;

	// Notify the tree UI to load the branches for the newly selected mission.
	if (SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		const auto& mission = m_missions[m_current_mission_index];
		m_tree_ops.rebuildBranchTree(mission.branches, mission.filename);
	} else {
		m_tree_ops.rebuildBranchTree({}, "");
	}
}

int CampaignEditorDialogModel::getCurrentMissionSelection() const
{
	return m_current_mission_index;
}

void CampaignEditorDialogModel::setCurrentBranchSelection(int branch_index)
{
	stopSpeech();
	
	m_current_branch_index = branch_index;
}

int CampaignEditorDialogModel::getCurrentBranchSelection() const
{
	return m_current_branch_index;
}

const SCP_string& CampaignEditorDialogModel::getCampaignFilename() const
{
	return m_campaign_filename;
}

void CampaignEditorDialogModel::setCampaignFilename(const SCP_string& filename)
{
	SCP_string truncated_filename = filename.substr(0, MAX_FILENAME_LEN - 1);
	modify(m_campaign_filename, truncated_filename);
}

const SCP_string& CampaignEditorDialogModel::getCampaignName() const
{
	return m_campaign_name;
}

void CampaignEditorDialogModel::setCampaignName(const SCP_string& name)
{
	SCP_string truncated_name = name.substr(0, NAME_LENGTH - 1);
	modify(m_campaign_name, truncated_name);
}

const SCP_string& CampaignEditorDialogModel::getCampaignDescription() const
{
	return m_campaign_descr;
}

void CampaignEditorDialogModel::setCampaignDescription(const SCP_string& descr)
{
	SCP_string truncated_descr = descr.substr(0, MISSION_DESC_LENGTH - 1);
	modify(m_campaign_descr, truncated_descr);
}

int CampaignEditorDialogModel::getCampaignType() const
{
	return m_campaign_type;
}

void CampaignEditorDialogModel::setCampaignType(int type)
{
	// Only proceed if the type has actually changed.
	if (m_campaign_type == type) {
		return;
	}

	// If missions have already been added to the campaign, prevent the type from being changed.
	if (!m_missions.empty()) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Type Locked",
			"The campaign type cannot be changed after missions have been added.\n"
			"To change the type, please remove all missions first.",
			{DialogButton::Ok});
		return;
	}

	modify(m_campaign_type, type);

	// Update the number of players based on the new type.
	if (type == CAMPAIGN_TYPE_SINGLE) {
		// Single-player campaigns always have 0 players.
		modify(m_num_players, 0);
	} else {
		// For new multiplayer campaigns, reset players to -1.
		// This will allow the user to set a number or have it be set by the first mission.
		modify(m_num_players, -1);
	}

	// Changing the campaign type affects which missions are compatible,
	// so we must reload the available missions list.
	loadAvailableMissions();
}

bool CampaignEditorDialogModel::getCampaignTechReset() const
{
	return (m_flags & CF_CUSTOM_TECH_DATABASE) != 0;
}

void CampaignEditorDialogModel::setCampaignTechReset(bool reset)
{
	// Create a copy of the current flags to modify.
	int new_flags = m_flags;

	if (reset) {
		new_flags |= CF_CUSTOM_TECH_DATABASE;
	} else {
		new_flags &= ~CF_CUSTOM_TECH_DATABASE;
	}

	modify(m_flags, new_flags);
}

int CampaignEditorDialogModel::getCampaignNumPlayers() const
{
	return m_num_players;
}

void CampaignEditorDialogModel::setCampaignNumPlayers(int num_players)
{
	// Only proceed if the number has actually changed.
	if (m_num_players == num_players) {
		return;
	}

	// If missions have already been added to the campaign, prevent the type from being changed.
	if (!m_missions.empty()) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Players Locked",
			"The number of players cannot be changed after missions have been added.\n"
			"To change the number of players, please remove all missions first.",
			{DialogButton::Ok});
		return;
	}

	modify(m_num_players, num_players);

	// If the player count changed, we must reload the available missions list
	// to apply the new filter.
	loadAvailableMissions();
}

void CampaignEditorDialogModel::setCustomData(const SCP_map<SCP_string, SCP_string>& custom_data)
{
	modify(m_custom_data, custom_data);
	set_modified();
}

SCP_map<SCP_string, SCP_string> CampaignEditorDialogModel::getCustomData() const
{
	return m_custom_data;
}

const SCP_vector<std::pair<SCP_string, bool>>& CampaignEditorDialogModel::getAvailableMissionFiles() const
{
	// If there's no filter, return the full list
	if (m_available_missions_filter.empty()) {
		return m_available_mission_files;
	}
	
	// Otherwise, build and return a temporary, filtered list.
	static SCP_vector<std::pair<SCP_string, bool>> filtered_list;
	filtered_list.clear();

	for (const auto& mission_pair : m_available_mission_files) {
		const SCP_string& filename = mission_pair.first;

		// Check if the filename contains the filter text
		if (stristr(filename.c_str(), m_available_missions_filter.c_str())) {
			filtered_list.push_back(mission_pair);
		}
	}

	return filtered_list;
}

void CampaignEditorDialogModel::setAvailableMissionsFilter(const SCP_string& filter)
{
	m_available_missions_filter = filter;
}

void CampaignEditorDialogModel::addMission(const SCP_string& filename, int level, int pos)
{
	// Check that the mission isn't already in the campaign.
	for (const auto& mission : m_missions) {
		if (mission.filename == filename) {
			// Mission is already in the campaign; do nothing.
			return;
		}
	}
	
	// Before adding, check if this is the first mission for a multiplayer campaign.
	if (m_campaign_type != CAMPAIGN_TYPE_SINGLE && m_missions.empty()) {
		// If so, this mission's player count sets the standard for the whole campaign.
		mission mission_info;
		get_mission_info(filename.c_str(), &mission_info);
		modify(m_num_players, mission_info.num_players);
	}

	// Create and populate the new mission data
	auto& new_mission = m_missions.emplace_back();
	new_mission.filename = filename;
	new_mission.level = level;
	new_mission.position = pos;

	// Adding or removing missions changes the list of available files.
	loadAvailableMissions();
	set_modified();
}

void CampaignEditorDialogModel::removeMission(int mission_index)
{
	if (!SCP_vector_inbounds(m_missions, mission_index)) {
		return;
	}

	// Check if this is the last mission being removed.
	const bool was_last_mission = (m_missions.size() == 1);

	m_missions.erase(m_missions.begin() + mission_index);

	// If the last mission was removed from a multiplayer campaign, reset the player count.
	if (was_last_mission && m_campaign_type != CAMPAIGN_TYPE_SINGLE) {
		modify(m_num_players, -1);
	}

	// Adding or removing missions changes the list of available files.
	loadAvailableMissions();
	set_modified();
}

void CampaignEditorDialogModel::updateMissionPosition(int mission_index, int new_level, int new_pos)
{
	if (!SCP_vector_inbounds(m_missions, mission_index)) {
		return;
	}

	// Get the name of the mission we are moving so we can find it again after the sort.
	SCP_string mission_name_to_find = m_missions[mission_index].filename;

	// Update the mission's position data.
	auto& mission = m_missions[mission_index];
	modify(mission.level, new_level);
	modify(mission.position, new_pos);

	// Sort the entire vector based on the new positions.
	sortMissions();

	// Find the new index of our mission and update the selection.
	for (int i = 0; i < static_cast<int>(m_missions.size()); ++i) {
		if (m_missions[i].filename == mission_name_to_find) {
			setCurrentMissionSelection(i);
			break;
		}
	}
}

int CampaignEditorDialogModel::getMissionGraphX(int i) const
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return INT_MIN;
	}
	return m_missions[i].graph_x; // INT_MIN means "unset"
}

void CampaignEditorDialogModel::setMissionGraphX(int i, int x)
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return;
	}
	auto& m = m_missions[i];
	if (m.graph_x != x) {
		modify(m.graph_x, x);
	}
}

int CampaignEditorDialogModel::getMissionGraphY(int i) const
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return INT_MIN;
	}
	return m_missions[i].graph_y; // INT_MIN means "unset"
}

void CampaignEditorDialogModel::setMissionGraphY(int i, int y)
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return;
	}
	auto& m = m_missions[i];
	if (m.graph_y != y) {
		modify(m.graph_y, y);
	}
}

int CampaignEditorDialogModel::getMissionGraphColor(int i) const
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return -1;
	}
	return m_missions[i].graph_color; // -1 means "unset"
}

void CampaignEditorDialogModel::setMissionGraphColor(int i, int rgb0xRRGGBB)
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return;
	}
	// Accept -1 (unset) or mask to 24-bit RGB
	const int normalized = (rgb0xRRGGBB < 0) ? -1 : (rgb0xRRGGBB & 0x00FFFFFF);

	auto& m = m_missions[i];
	if (m.graph_color != normalized) {
		modify(m.graph_color, normalized);
	}
}

CampaignSpecialMode CampaignEditorDialogModel::getMissionSpecialMode(int i) const
{
	if (!SCP_vector_inbounds(m_missions, i)) {
		return CampaignSpecialMode::Loop; // default visual mode
	}
	const auto& m = m_missions[i];

	// If any special branches exist, derive mode from them
	for (const auto& b : m.branches) {
		if (b.is_loop)
			return CampaignSpecialMode::Loop;
	}
	for (const auto& b : m.branches) {
		if (b.is_fork)
			return CampaignSpecialMode::Fork;
	}
	// Otherwise use the editor hint
	return m.special_mode_hint;
}

void CampaignEditorDialogModel::toggleMissionSpecialMode(int i)
{
	if (!SCP_vector_inbounds(m_missions, i))
		return;

	bool has_special_branches = false;
	const auto& ms = m_missions[i].branches;
	has_special_branches = std::any_of(ms.begin(), ms.end(), [](const CampaignBranchData& b) { return b.is_loop || b.is_fork; });

	// If special branches already exist, mode is locked
	if (has_special_branches)
		return;

	CampaignSpecialMode current = getMissionSpecialMode(i);
	CampaignSpecialMode mode = (current == CampaignSpecialMode::Loop) ? CampaignSpecialMode::Fork : CampaignSpecialMode::Loop;

	modify(m_missions[i].special_mode_hint, mode);
}

void CampaignEditorDialogModel::setMissionAsFirst(int mission_index)
{
	if (!SCP_vector_inbounds(m_missions, mission_index)) {
		return;
	}
	// Move the selected mission to the front of the list.
	auto mission = m_missions[mission_index];
	m_missions.erase(m_missions.begin() + mission_index);
	m_missions.insert(m_missions.begin(), mission);
	set_modified();
}

SCP_string CampaignEditorDialogModel::getCurrentMissionFilename() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "";
	}
	return m_missions[m_current_mission_index].filename;
}

// Changing a mission's filename is a complex operation beyond a simple setter,
// as it's the unique ID for the mission. This would be better handled by a
// dedicated "replace mission" action, so we will leave this unimplemented for now.
void CampaignEditorDialogModel::setCurrentMissionFilename(const SCP_string& /*filename*/)
{
	// Not implemented.
}

SCP_string CampaignEditorDialogModel::getCurrentMissionBriefingCutscene() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "<None>";
	}

	SCP_string cutscene = m_missions[m_current_mission_index].briefing_cutscene;
	return cutscene.empty() ? "<None>" : cutscene;
}

void CampaignEditorDialogModel::setCurrentMissionBriefingCutscene(const SCP_string& cutscene)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}

	auto& mission = m_missions[m_current_mission_index];
	modify(mission.briefing_cutscene, cutscene.substr(0, NAME_LENGTH - 1));
}

SCP_string CampaignEditorDialogModel::getCurrentMissionMainhall() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "<None>";
	}

	SCP_string hall = m_missions[m_current_mission_index].main_hall;

	return hall.empty() ? "<None>" : hall;
}

void CampaignEditorDialogModel::setCurrentMissionMainhall(const SCP_string& mainhall)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}

	auto& mission = m_missions[m_current_mission_index];
	modify(mission.main_hall, mainhall);
}

SCP_string CampaignEditorDialogModel::getCurrentMissionSubstituteMainhall() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "<None>";
	}

	SCP_string hall = m_missions[m_current_mission_index].substitute_main_hall;

	return hall.empty() ? "<None>" : hall;
}

void CampaignEditorDialogModel::setCurrentMissionSubstituteMainhall(const SCP_string& mainhall)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}

	auto& mission = m_missions[m_current_mission_index];
	modify(mission.substitute_main_hall, mainhall);
}

int CampaignEditorDialogModel::getCurrentMissionDebriefingPersona() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return 0;
	}
	return m_missions[m_current_mission_index].debrief_persona_index;
}

void CampaignEditorDialogModel::setCurrentMissionDebriefingPersona(int persona_index)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}

	auto& mission = m_missions[m_current_mission_index];
	modify(mission.debrief_persona_index, persona_index);
}

SCP_string CampaignEditorDialogModel::getCurrentBranchLoopDescription() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "";
	}
	const auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return "";
	}

	const auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return "";
	}

	return mission.branches[m_current_branch_index].loop_description;
}

void CampaignEditorDialogModel::setCurrentBranchLoopDescription(const SCP_string& descr)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}
	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return;
	}

	auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return;
	}

	modify(branch.loop_description, descr.substr(0, MISSION_DESC_LENGTH - 1));
}

SCP_string CampaignEditorDialogModel::getCurrentBranchLoopAnim() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "";
	}
	const auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return "";
	}

	const auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return "";
	}

	return mission.branches[m_current_branch_index].loop_briefing_anim;
}

void CampaignEditorDialogModel::setCurrentBranchLoopAnim(const SCP_string& anim)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}
	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return;
	}

	auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return;
	}

	modify(branch.loop_briefing_anim, anim.substr(0, NAME_LENGTH - 1));
}

SCP_string CampaignEditorDialogModel::getCurrentBranchLoopVoice() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return "";
	}
	const auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return "";
	}

	const auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return "";
	}

	return mission.branches[m_current_branch_index].loop_briefing_sound;
}

void CampaignEditorDialogModel::setCurrentBranchLoopVoice(const SCP_string& voice)
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}
	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return;
	}

	auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return;
	}

	modify(branch.loop_briefing_sound, voice.substr(0, NAME_LENGTH - 1));
}

void CampaignEditorDialogModel::testCurrentBranchLoopVoice()
{
	stopSpeech();

	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}

	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return;
	}

	const auto& branch = mission.branches[m_current_branch_index];
	if (!branch.is_loop && !branch.is_fork) {
		return;
	}

	_waveId = audiostream_open(branch.loop_briefing_sound.c_str(), ASF_EVENTMUSIC);
	audiostream_play(_waveId, 1.0f, 0);
}

void CampaignEditorDialogModel::removeBranchByTreeId(int formula_id)
{
	// Ensure we have a valid mission context
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}

	auto& mission = m_missions[m_current_mission_index];

	// Find the branch row that corresponds to this formula root
	int branch_index = -1;
	for (size_t i = 0; i < mission.branches.size(); ++i) {
		if (mission.branches[i].sexp_formula == formula_id) {
			branch_index = static_cast<int>(i);
			break;
		}
	}
	if (branch_index < 0) {
		return;
	}

	removeBranch(m_current_mission_index, branch_index);

	m_current_branch_index = -1; // set no branch selected

	// Rebuild the visual tree from the model's authoritative state
	if (SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		auto& cur = m_missions[m_current_mission_index];
		m_tree_ops.rebuildBranchTree(cur.branches, cur.filename);
	}
}

const SCP_vector<CampaignMissionData>& CampaignEditorDialogModel::getCampaignMissions() const
{
	return m_missions;
}

int CampaignEditorDialogModel::getNumBranches() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return 0;
	}
	return static_cast<int>(m_missions[m_current_mission_index].branches.size());
}

void CampaignEditorDialogModel::addBranch(int from_mission_index, int to_mission_index)
{
	if (!SCP_vector_inbounds(m_missions, from_mission_index) || !SCP_vector_inbounds(m_missions, to_mission_index)) {
		return;
	}

	auto& from_mission = m_missions[from_mission_index];
	const auto& to_mission_name = m_missions[to_mission_index].filename;

	// Prevent creating a duplicate branch to the same mission
	for (const auto& existing_branch : from_mission.branches) {
		if (!existing_branch.is_loop && !existing_branch.is_fork &&
			existing_branch.next_mission_name == to_mission_name) {
			return;
		}
	}

	// Create the new branch data
	auto& new_branch = from_mission.branches.emplace_back();
	addBranchIdIfMissing(new_branch);
	new_branch.next_mission_name = to_mission_name;
	new_branch.is_loop = false;
	new_branch.is_fork = false;

	// Ask the UI's tree to create a default SEXP ("true") for this new branch
	new_branch.sexp_formula = m_tree_ops.createDefaultSexp();

	set_modified();

	if (m_current_mission_index == from_mission_index) { // only rebuild if we're on the affected mission
		m_tree_ops.rebuildBranchTree(from_mission.branches, from_mission.filename);
	}
}

void CampaignEditorDialogModel::moveBranchUp()
{
	// Ensure a mission and a branch are currently selected.
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}
	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index) || m_current_branch_index == 0) {
		return;
	}
	// Swap the selected branch with the one above it.
	std::swap(mission.branches[m_current_branch_index], mission.branches[m_current_branch_index - 1]);
	set_modified();
	
	m_current_branch_index = -1; // set no branch selected
	// Rebuild the visual tree from the model's authoritative state
	m_tree_ops.rebuildBranchTree(mission.branches, mission.filename);
}

void CampaignEditorDialogModel::moveBranchDown()
{
	// Ensure a mission and a branch are currently selected.
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}
	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index) || m_current_branch_index == static_cast<int>(mission.branches.size()) - 1) {
		return;
	}
	// Swap the selected branch with the one below it.
	std::swap(mission.branches[m_current_branch_index], mission.branches[m_current_branch_index + 1]);
	set_modified();
	
	m_current_branch_index = -1; // set no branch selected
	// Rebuild the visual tree from the model's authoritative state
	m_tree_ops.rebuildBranchTree(mission.branches, mission.filename);
}

void CampaignEditorDialogModel::addEndBranch(int from_mission_index)
{
	if (!SCP_vector_inbounds(m_missions, from_mission_index)) {
		return;
	}
	auto& from = m_missions[from_mission_index];

	// Prevent duplicate "end" branch (empty next)
	for (const auto& b : from.branches) {
		if (!b.is_loop && !b.is_fork && b.next_mission_name.empty()) {
			return;
		}
	}

	auto& nb = from.branches.emplace_back();
	nb.next_mission_name.clear(); // END
	nb.is_loop = false;
	nb.is_fork = false;
	nb.sexp_formula = m_tree_ops.createDefaultSexp();

	set_modified();
	if (m_current_mission_index == from_mission_index) { // only rebuild if we're on the affected mission
		m_tree_ops.rebuildBranchTree(from.branches, from.filename);
	}
}

void CampaignEditorDialogModel::addSpecialBranch(int from_mission_index, int to_mission_index)
{
	if (!SCP_vector_inbounds(m_missions, from_mission_index) || !SCP_vector_inbounds(m_missions, to_mission_index)) {
		return;
	}
	auto& from = m_missions[from_mission_index];
	const auto& to_name = m_missions[to_mission_index].filename;

	// Determine special flavor from the mission's current mode
	const bool asLoop = (from.special_mode_hint == CampaignSpecialMode::Loop);
	const bool asFork = (from.special_mode_hint == CampaignSpecialMode::Fork);

	// No mixing here
	for (const auto& b : from.branches) {
		if ((b.is_loop || b.is_fork) && (b.is_loop != asLoop || b.is_fork != asFork)) {
			// Conflicting special types present
			return;
		}
	}

	// Prevent duplicate special edge to same target
	for (const auto& b : from.branches) {
		if (b.next_mission_name == to_name && b.is_loop == asLoop && b.is_fork == asFork) {
			return;
		}
	}

	auto& nb = from.branches.emplace_back();
	nb.next_mission_name = to_name;
	nb.is_loop = asLoop;
	nb.is_fork = asFork;
	nb.sexp_formula = m_tree_ops.createDefaultSexp();

	set_modified();
	if (m_current_mission_index == from_mission_index) { // only rebuild if we're on the affected mission
		m_tree_ops.rebuildBranchTree(from.branches, from.filename);
	}
}


void CampaignEditorDialogModel::removeBranch(int mission_index, int branch_index)
{
	if (!SCP_vector_inbounds(m_missions, mission_index)) {
		return;
	}
	auto& mission = m_missions[mission_index];
	if (!SCP_vector_inbounds(mission.branches, branch_index)) {
		return;
	}

	// The model's only job is to update its own data.
	mission.branches.erase(mission.branches.begin() + branch_index);

	set_modified();
}

void CampaignEditorDialogModel::updateCurrentBranch(int internal_node_id)
{
	// Ensure a mission and a branch are currently selected.
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return;
	}
	auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return;
	}

	// Tell the tree to save the specified branch.
	// The tree will serialize its internal model for that branch into a new SEXP
	// and return the new formula index.
	int new_sexp_formula = m_tree_ops.saveSexp(internal_node_id);

	// Update the model's data with the new formula.
	// The 'modify' helper also handles setting the modified flag.
	auto& branch = mission.branches[m_current_branch_index];
	modify(branch.sexp_formula, new_sexp_formula);

	m_tree_ops.expandBranch(internal_node_id);
}

bool CampaignEditorDialogModel::getCurrentBranchIsSpecial() const
{
	if (!SCP_vector_inbounds(m_missions, m_current_mission_index)) {
		return false;
	}
	const auto& mission = m_missions[m_current_mission_index];
	if (!SCP_vector_inbounds(mission.branches, m_current_branch_index)) {
		return false;
	}
	const auto& branch = mission.branches[m_current_branch_index];
	return branch.is_loop || branch.is_fork;
}

int CampaignEditorDialogModel::addBranchIdIfMissing(CampaignBranchData& b)
{
	if (b.id >= 0) {
		return b.id;
	}

	// Find the current max id across all missions to avoid collisions,
	// then assign the next integer.
	int maxId = -1;
	for (const auto& mission : m_missions) {
		for (const auto& br : mission.branches) {
			maxId = std::max(maxId, br.id);
		}
	}
	b.id = maxId + 1;
	return b.id;
}

CampaignBranchData* CampaignEditorDialogModel::findBranchById(int missionIdx, int branchId)
{
	if (!SCP_vector_inbounds(m_missions, missionIdx) || branchId < 0) {
		return nullptr;
	}
	auto& branches = m_missions[missionIdx].branches;
	for (auto& br : branches) {
		if (br.id == branchId) {
			return &br;
		}
	}
	return nullptr;
}


SCP_vector<std::tuple<SCP_string, int, bool>> CampaignEditorDialogModel::getAllowedShips() const
{
	SCP_vector<std::tuple<SCP_string, int, bool>> ship_list;
	for (int i = 0; i < static_cast<int>(Ship_info.size()); i++) {
		if (Ship_info[i].flags[Ship::Info_Flags::Player_ship]) {
			ship_list.emplace_back(Ship_info[i].name, i, m_ships_allowed[i]);
		}
	}
	return ship_list;
}

void CampaignEditorDialogModel::setAllowedShip(int ship_class_index, bool allowed)
{
	if (SCP_vector_inbounds(m_ships_allowed, ship_class_index)) {
		if (m_ships_allowed[ship_class_index] != allowed) {
			m_ships_allowed[ship_class_index] = allowed;
			set_modified();
		}
	}
}

SCP_vector<std::tuple<SCP_string, int, bool>> CampaignEditorDialogModel::getAllowedWeapons() const
{
	SCP_vector<std::tuple<SCP_string, int, bool>> weapon_list;
	for (int i = 0; i < static_cast<int>(Weapon_info.size()); i++) {
		if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Player_allowed]) {
			weapon_list.emplace_back(Weapon_info[i].name, i, m_weapons_allowed[i]);
		}
	}
	return weapon_list;
}

void CampaignEditorDialogModel::setAllowedWeapon(int weapon_class_index, bool allowed)
{
	if (SCP_vector_inbounds(m_weapons_allowed, weapon_class_index)) {
		if (m_weapons_allowed[weapon_class_index] != allowed) {
			m_weapons_allowed[weapon_class_index] = allowed;
			set_modified();
		}
	}
}

SCP_vector<SCP_string> CampaignEditorDialogModel::getCutsceneList()
{
	SCP_vector<SCP_string> out;

	out.emplace_back("<None>");

	for (const auto& cs : Cutscenes) {
		out.emplace_back(cs.filename);
	}

	return out;
}

SCP_vector<SCP_string> CampaignEditorDialogModel::getMainhallList() const
{
	SCP_vector<SCP_string> out;
	if (m_save_format == CampaignFormat::FSO) {
		out.emplace_back("<None>");

		// De-dupe by name (some mods define multiple resolutions/variants per mainhall)
		std::unordered_set<SCP_string> seen;
		for (const auto& mh : Main_hall_defines) {
			if (mh.empty()) { // shouldn't happen but let's be safe
				continue;
			}
			const auto& hall = mh[0];
			out.emplace_back(hall.name);
		}
	} else if (m_save_format == CampaignFormat::Retail) {
		out.emplace_back("0");
		out.emplace_back("1");
	}

	return out;
}

} // namespace fso::fred::dialogs