#pragma once

#include "AbstractDialogModel.h"

#include "ship/ship.h"
#include "gamesnd/eventmusic.h"
#include "mission/missionparse.h"
#include "mission/missionmessage.h"
#include "playerman/managepilot.h" // for squad logos
#include "sound/sound.h"

namespace fso::fred::dialogs {


class MissionSpecDialogModel : public AbstractDialogModel {
private:
	void initializeData();
	void prepareSquadLogoList();


	SCP_string _m_created;
	SCP_string _m_modified;
	SCP_string _m_mission_notes;
	SCP_string _m_designer_name;
	SCP_string _m_mission_title;
	SCP_string _m_mission_desc;
	SCP_string _m_squad_filename;
	SCP_string _m_squad_name;
	SCP_string _m_loading_640;
	SCP_string _m_loading_1024;
	int		_m_ai_profile;
	int		_m_event_music;
	SCP_string	_m_substitute_event_music;
	int		_m_command_persona;
	SCP_string	_m_command_sender;
	bool		_m_full_war;
	uint		_m_num_respawns;
	int			_m_max_respawn_delay;
	bool		_m_disallow_support;
	float		_m_max_hull_repair_val;
	float		_m_max_subsys_repair_val;
	bool		_m_contrail_threshold_flag;
	int			_m_contrail_threshold;
	SCP_map<SCP_string, SCP_string> _m_custom_data;
	SCP_vector<custom_string> _m_custom_strings;
	sound_env	_m_sound_env;

	std::array<SCP_string, MAX_STARTING_WINGS> _m_custom_starting_wings;
	std::array<SCP_string, MAX_SQUADRON_WINGS> _m_custom_squadron_wings;
	std::array<SCP_string, MAX_TVT_WINGS> _m_custom_tvt_wings;

	flagset<Mission::Mission_Flags> _m_flags;
	SCP_vector<std::pair<SCP_string, bool>> _m_flag_data;
	SCP_vector<SCP_string> _m_squadLogoList;

	int _m_type;

public:
	MissionSpecDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setMissionTitle(const SCP_string& m_mission_title);
	SCP_string getMissionTitle();

	void setDesigner(const SCP_string& m_designer_name);
	SCP_string getDesigner();

	SCP_string getCreatedTime();

	SCP_string getModifiedTime();

	void setMissionType(int m_type);
	int getMissionType();

	void setNumRespawns(uint);
	uint getNumRespawns();

	void setMaxRespawnDelay(int);
	int getMaxRespawnDelay();

	void setSquadronName(const SCP_string&);
	SCP_string getSquadronName();
	void setSquadronLogo(const SCP_string&);
	SCP_string getSquadronLogo();
	std::vector<std::string> getSquadLogoList() const { return _m_squadLogoList; };

	void setLowResLoadingScreen(const SCP_string&);
	SCP_string getLowResLoadingScren();
	void setHighResLoadingScreen(const SCP_string&);
	SCP_string getHighResLoadingScren();

	void setDisallowSupport(bool);
	bool getDisallowSupport();
	void setHullRepairMax(float);
	int getHullRepairMax();
	void setSubsysRepairMax(float);
	int getSubsysRepairMax();

	void setTrailThresholdFlag(bool);
	bool getTrailThresholdFlag();
	void setTrailDisplaySpeed(int);
	int getTrailDisplaySpeed();

	void setCommandSender(const SCP_string&);
	SCP_string getCommandSender();
	void setCommandPersona(int);
	int getCommandPersona();

	void setEventMusic(int);
	int getEventMusic();
	void setSubEventMusic(const SCP_string&);
	SCP_string getSubEventMusic();

	void setMissionFlag(const SCP_string& flag_name, bool enabled);
	void setMissionFlagDirect(Mission::Mission_Flags flag, bool enabled);
	bool getMissionFlag(Mission::Mission_Flags flag) const;
	const SCP_vector<std::pair<SCP_string, bool>>& getMissionFlagsList();

	void setMissionFullWar(bool enabled);

	void setAIProfileIndex(int m_ai_profile);
	int getAIProfileIndex() const;

	void setMissionDescText(const SCP_string&);
	SCP_string getMissionDescText();

	void setDesignerNoteText(const SCP_string&);
	SCP_string getDesignerNoteText();

	void setCustomData(const SCP_map<SCP_string, SCP_string>& custom_data);
	SCP_map<SCP_string, SCP_string> getCustomData() const;

	void setCustomStrings(const SCP_vector<custom_string>& custom_strings);
	SCP_vector<custom_string> getCustomStrings() const;

	void setSoundEnvironmentParams(const sound_env& env);
	sound_env getSoundEnvironmentParams() const;

	void setCustomStartingWings(const std::array<SCP_string, MAX_STARTING_WINGS>& starting_wings);
	std::array<SCP_string, MAX_STARTING_WINGS> getCustomStartingWings() const;

	void setCustomSquadronWings(const std::array<SCP_string, MAX_SQUADRON_WINGS>& squadron_wings);
	std::array<SCP_string, MAX_SQUADRON_WINGS> getCustomSquadronWings() const;

	void setCustomTvTWings(const std::array<SCP_string, MAX_TVT_WINGS>& tvt_wings);
	std::array<SCP_string, MAX_TVT_WINGS> getCustomTvTWings() const;

};

} // namespace fso::fred::dialogs
