#pragma once

#include "AbstractDialogModel.h"

#include "ship/ship.h"
#include "gamesnd/eventmusic.h"
#include "mission/missionparse.h"
#include "mission/missionmessage.h"

namespace fso {
namespace fred {
namespace dialogs {


class MissionSpecDialogModel : public AbstractDialogModel {
private:
	void initializeData();

	template<typename T> 
	void modify(T &a, const T &b);

	bool _modified = false;

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

	flagset<Mission::Mission_Flags> _m_flags;

	int _m_type;

	void set_modified();

public:
	MissionSpecDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setMissionTitle(SCP_string m_mission_title);
	SCP_string getMissionTitle();

	void setDesigner(SCP_string m_designer_name);
	SCP_string getDesigner();

	SCP_string getCreatedTime();

	SCP_string getModifiedTime();

	void setMissionType(int m_type);
	int getMissionType();

	void setNumRespawns(uint);
	uint getNumRespawns();

	void setMaxRespawnDelay(int);
	int getMaxRespawnDelay();

	void setSquadronName(SCP_string);
	SCP_string getSquadronName();
	void setSquadronLogo(SCP_string);
	SCP_string getSquadronLogo();

	void setLowResLoadingScreen(SCP_string);
	SCP_string getLowResLoadingScren();
	void setHighResLoadingScreen(SCP_string);
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

	void setCommandSender(SCP_string);
	SCP_string getCommandSender();
	void setCommandPersona(int);
	int getCommandPersona();

	void setEventMusic(int);
	int getEventMusic();
	void setSubEventMusic(SCP_string);
	SCP_string getSubEventMusic();

	void setMissionFlag(Mission::Mission_Flags flag, bool enabled);
	const flagset<Mission::Mission_Flags>& getMissionFlags() const;

	void setMissionFullWar(bool enabled);

	void setAIProfileIndex(int m_ai_profile);
	int getAIProfileIndex() const;

	void setMissionDescText(SCP_string);
	SCP_string getMissionDescText();

	void setDesignerNoteText(SCP_string);
	SCP_string getDesignerNoteText();

	bool query_modified();
};

template<typename T>
inline void MissionSpecDialogModel::modify(T &a, const T &b) {
	if (a != b) {
		a = b;
		set_modified();
		modelChanged();
	}
}

}
}
}