#pragma once

#include "globalincs/pstypes.h"

#include "mission/dialogs/AbstractDialogModel.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"

namespace fso::fred::dialogs {

/**
 * @brief A simple data structure to hold the working copy of a campaign mission's branch.
 */
struct CampaignBranchData {
	int sexp_formula = -1;
	SCP_string next_mission_name;
	bool is_loop = false;
	bool is_fork = false; // Not currently supported by FRED, but good to have
	SCP_string loop_description;
	SCP_string loop_briefing_anim;
	SCP_string loop_briefing_sound;
};

/**
 * @brief A simple data structure to hold the working copy of a mission within the campaign.
 */
struct CampaignMissionData {
	SCP_string name;
	int level = 0;
	int position = 0;

	SCP_string briefing_cutscene;
	SCP_string main_hall;
	SCP_string substitute_main_hall;
	int debrief_persona_index = 0;

	SCP_vector<CampaignBranchData> branches;
};

/**
 * @brief An abstract interface that defines the contract for how the model
 * communicates with the UI's SEXP tree widget. This keeps the model
 * completely decoupled from any specific UI implementation.
 */
struct ICampaignEditorTreeOps {
	virtual ~ICampaignEditorTreeOps() = default;

	// Asks the tree to load a formula and return its internal ID
	virtual int loadSexp(int formula_index) = 0;

	// Asks the tree to save an internal branch and return the new formula ID
	virtual int saveSexp(int internal_node_id) = 0;

	// Asks the tree to create a new, default SEXP (i.e., "true")
	virtual int createDefaultSexp() = 0;

	// Asks the tree to completely rebuild its view from a list of branches
	virtual void rebuildBranchTree(const SCP_vector<CampaignBranchData>& branches) = 0;

	// Asks the tree to expand the visual branch for a given internal node ID
	virtual void expandBranch(int internal_node_id) = 0;
};

/**
 * @brief The data model for the Campaign Editor. Manages all campaign data
 * and business logic in a UI-agnostic way.
 */
class CampaignEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT

  public:
	// The constructor now takes our abstract interface
	CampaignEditorDialogModel(QObject* parent, EditorViewport* viewportt, ICampaignEditorTreeOps& tree_ops);
	~CampaignEditorDialogModel() override = default;

	// AbstractDialogModel implementation
	bool apply() override;
	void reject() override;

	void loadCampaignFromFile(const SCP_string& filename);
	void saveCampaign(const SCP_string& filename);
	bool checkValidity();

	void setCurrentMissionSelection(int index);
	void setCurrentBranchSelection(int branch_index);

	// Base Campaign Data
	const SCP_string& getCampaignName() const;
	void setCampaignName(const SCP_string& name);
	const SCP_string& getCampaignDescription() const;
	void setCampaignDescription(const SCP_string& descr);
	int getCampaignType() const;
	void setCampaignType(int type);
	bool getCampaignTechReset() const;
	void setCampaignTechReset(bool reset);
	int getCampaignNumPlayers() const;
	void setCampaignNumPlayers(int num_players);

	// Available Missions
	const SCP_vector<SCP_string>& getAvailableMissionFiles() const;
	void addMission(const SCP_string& filename, int level, int pos);
	void removeMission(int mission_index);
	void updateMissionPosition(int mission_index, int new_level, int new_pos);

	// Current Mission
	SCP_string getCurrentMissionFilename() const;
	void setCurrentMissionFilename(const SCP_string& filename);
	SCP_string getCurrentMissionBriefingCutscene() const;
	void setCurrentMissionBriefingCutscene(const SCP_string& cutscene);
	SCP_string getCurrentMissionMainhall() const;
	void setCurrentMissionMainhall(const SCP_string& mainhall);
	SCP_string getCurrentMissionSubstituteMainhall() const;
	void setCurrentMissionSubstituteMainhall(const SCP_string& mainhall);
	int getCurrentMissionDebriefingPersona() const;
	void setCurrentMissionDebriefingPersona(int persona_index);

	// Loop Data
	SCP_string getCurrentBranchLoopDescription() const;
	void setCurrentBranchLoopDescription(const SCP_string& descr);
	SCP_string getCurrentBranchLoopAnim() const;
	void setCurrentBranchLoopAnim(const SCP_string& anim);
	SCP_string getCurrentBranchLoopVoice() const;
	void setCurrentBranchLoopVoice(const SCP_string& voice);
	void testCurrentBranchLoopVoice();

	// Campaign Graph
	const SCP_vector<CampaignMissionData>& getCampaignMissions() const;
	void addBranch(int from_mission_index, int to_mission_index);
	void removeBranch(int mission_index, int branch_index);
	void updateCurrentBranch(int internal_node_id);

	// Tech
	const SCP_vector<bool>& getAllowedShips() const;
	void setAllowedShip(int ship_class_index, bool allowed);
	const SCP_vector<bool>& getAllowedWeapons() const;
	void setAllowedWeapon(int weapon_class_index, bool allowed);

  private:
	// The model's only link to the SEXP tree UI
	ICampaignEditorTreeOps& m_tree_ops;

	SCP_string m_campaign_filename;
	SCP_string m_campaign_name;
	SCP_string m_campaign_descr;
	int m_campaign_type = CAMPAIGN_TYPE_SINGLE;
	int m_num_players = -1;
	int m_flags = 0;
	SCP_vector<bool> m_ships_allowed;
	SCP_vector<bool> m_weapons_allowed;
	SCP_vector<CampaignMissionData> m_missions;

	// List of mission files in the game directory that are not yet in the campaign
	SCP_vector<SCP_string> m_available_mission_files;

	// Pointers/indices to the currently selected items for context
	int m_current_mission_index = -1;
	int m_current_branch_index = -1;

	int _waveId;

	void initializeData(const char* filename = nullptr);
	void parseBranchesFromFormula(CampaignMissionData& mission, int formula_index, bool is_loop);
	void loadAvailableMissions();
	static void clearCampaignGlobal();
	void commitWorkingCopyToGlobal();
	void sortMissions();
	void stopSpeech();
};

} // namespace fso::fred::dialogs