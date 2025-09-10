#pragma once

#include "globalincs/pstypes.h"

#include "mission/dialogs/AbstractDialogModel.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"

namespace fso::fred::dialogs {

enum class CampaignFormat {
	Retail,
	FSO
};

enum class CampaignSpecialMode {
	Loop,
	Fork
};

struct CampaignBranchData {
	int id = -1;
	int sexp_formula = -1;
	SCP_string next_mission_name;
	bool is_loop = false;
	bool is_fork = false; // Not currently supported by FRED, but good to have
	SCP_string loop_description;
	SCP_string loop_briefing_anim;
	SCP_string loop_briefing_sound;
};


struct CampaignMissionData {
	SCP_string filename;
	int level = 0;
	int position = 0;
	int graph_x = INT_MIN; // For UI layout only // TODO after missionsave.cpp is refactored, save these to the campaign file
	int graph_y = INT_MIN; // For UI layout only
	int graph_color = -1;  // For UI layout only

	SCP_string briefing_cutscene;
	SCP_string main_hall;
	SCP_string substitute_main_hall;
	int debrief_persona_index = 0;

	CampaignSpecialMode special_mode_hint = CampaignSpecialMode::Loop;

	SCP_vector<CampaignBranchData> branches;
};

struct ICampaignEditorTreeOps {
	virtual ~ICampaignEditorTreeOps() = default;

	// Asks the tree to load a formula and return its internal ID
	virtual int loadSexp(int formula_index) = 0;

	// Asks the tree to save an internal branch and return the new formula ID
	virtual int saveSexp(int internal_node_id) = 0;

	// Asks the tree to create a new, default SEXP (i.e., "true")
	virtual int createDefaultSexp() = 0;

	// Asks the tree to completely rebuild its view from a list of branches
	virtual void rebuildBranchTree(const SCP_vector<CampaignBranchData>& branches, const SCP_string& currentMissionName) = 0;

	// Asks the tree to expand the visual branch for a given internal node ID
	virtual void expandBranch(int internal_node_id) = 0;
};

class CampaignEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT

  public:
	// The constructor now takes our abstract interface
	CampaignEditorDialogModel(QObject* parent, EditorViewport* viewportt, ICampaignEditorTreeOps& tree_ops);
	~CampaignEditorDialogModel() override = default;

	// AbstractDialogModel implementation
	bool apply() override;
	void reject() override;

	static SCP_vector<SCP_string> getCampaignTypes();

	void createNewCampaign();
	void loadCampaignFromFile(const SCP_string& filename);
	void saveCampaign(const SCP_string& filename);
	bool checkValidity();
	CampaignFormat getSaveFormat() const;
	void setSaveFormat(CampaignFormat fmt);

	void setCurrentMissionSelection(int index);
	int getCurrentMissionSelection() const;
	void setCurrentBranchSelection(int branch_index);
	int getCurrentBranchSelection() const;

	// Base Campaign Data
	const SCP_string& getCampaignFilename() const;
	void setCampaignFilename(const SCP_string& filename);
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
	SCP_map<SCP_string, SCP_string> getCustomData() const;
	void setCustomData(const SCP_map<SCP_string, SCP_string>& custom_data);

	// Available Missions
	const SCP_vector<std::pair<SCP_string, bool>>& getAvailableMissionFiles() const;
	void setAvailableMissionsFilter(const SCP_string& filter);
	void addMission(const SCP_string& filename, int level, int pos);
	void removeMission(int mission_index);
	void updateMissionPosition(int mission_index, int new_level, int new_pos);
	int getMissionGraphX(int i) const;
	void setMissionGraphX(int i, int x);
	int getMissionGraphY(int i) const;
	void setMissionGraphY(int i, int y);
	int getMissionGraphColor(int i) const; // -1 = unset, else 0xRRGGBB
	void setMissionGraphColor(int i, int rgb0xRRGGBB);
	CampaignSpecialMode getMissionSpecialMode(int i) const;
	void toggleMissionSpecialMode(int i); 

	// Current Mission
	void setMissionAsFirst(int mission_index);
	SCP_string getCurrentMissionFilename() const;
	void setCurrentMissionFilename(const SCP_string& /*filename*/);
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

	// Sexp tree
	void removeBranchByTreeId(int formula_id);

	// Campaign Graph
	const SCP_vector<CampaignMissionData>& getCampaignMissions() const;
	int getNumBranches() const;
	void addBranch(int from_mission_index, int to_mission_index);
	void addEndBranch(int from_mission_index);
	void addSpecialBranch(int from_mission_index, int to_mission_index);
	void removeBranch(int mission_index, int branch_index);
	void moveBranchUp();
	void moveBranchDown();
	void updateCurrentBranch(int internal_node_id);
	bool getCurrentBranchIsSpecial() const;
	int addBranchIdIfMissing(CampaignBranchData& b); // assigns a unique id once
	CampaignBranchData* findBranchById(int missionIdx, int branchId);

	// Tech
	SCP_vector<std::tuple<SCP_string, int, bool>> getAllowedShips() const;
	void setAllowedShip(int ship_class_index, bool allowed);
	SCP_vector<std::tuple<SCP_string, int, bool>> getAllowedWeapons() const;
	void setAllowedWeapon(int weapon_class_index, bool allowed);

	// Lists
	static SCP_vector<SCP_string> getCutsceneList();
	SCP_vector<SCP_string> getMainhallList() const;

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
	SCP_map<SCP_string, SCP_string> m_custom_data;
	SCP_vector<CampaignMissionData> m_missions;

	// List of mission files in the game directory that are not yet in the campaign
	SCP_vector<std::pair<SCP_string, bool>> m_available_mission_files;
	SCP_string m_available_missions_filter;

	// Pointers/indices to the currently selected items for context
	int m_current_mission_index = -1;
	int m_current_branch_index = -1;

	int _waveId;

	CampaignFormat m_save_format = CampaignFormat::FSO;

	void initializeData(const char* filename = nullptr);
	void parseBranchesFromFormula(CampaignMissionData& mission, int formula_index, bool is_loop);
	void loadAvailableMissions();
	static void clearCampaignGlobal();
	void commitWorkingCopyToGlobal();
	void sortMissions();
	void stopSpeech();
};

} // namespace fso::fred::dialogs