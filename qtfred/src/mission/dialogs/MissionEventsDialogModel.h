#pragma once

#include "AbstractDialogModel.h"

#include "missioneditor/sexp_annotation_model.h"
#include "missioneditor/sexp_tree_model.h"

#include <mission/missiongoals.h>
#include <mission/missionmessage.h>
#include <mission/missionparse.h>

namespace fso::fred::dialogs {

class MissionEventsDialogModel : public AbstractDialogModel {
	Q_OBJECT

  public:
	MissionEventsDialogModel(QObject* parent, EditorViewport* viewport, SexpTreeModel& tree_model);

	bool apply() override;
	void reject() override;

	bool eventIsValid() const;
	bool messageIsValid() const;

	void setCurrentlySelectedEvent(int event);
	void setCurrentlySelectedEventByFormula(int formula);
	int getCurrentlySelectedEvent() const;
	SCP_vector<mission_event>& getEventList();
	void deleteRootNode(int node);
	void renameRootNode(int node, const SCP_string& name);
	void changeRootNodeFormula(int old, int node);
	void reorderByRootFormulaOrder(const SCP_vector<int>& newOrderedFormulas);

	void setCurrentlySelectedMessage(int msg);
	int getCurrentlySelectedMessage() const;
	const SCP_vector<SCP_string>& getHeadAniList();
	const SCP_vector<SCP_string>& getWaveList();

	// Session-persistent list of head ANIs discovered outside the built-in defaults
	static void addExtraHeadAni(const SCP_string& name);
	static void clearBrowsedHeadAnis();
	static const SCP_vector<SCP_string>& getExtraHeadAnis();
	const SCP_vector<std::pair<SCP_string, int>>& getPersonaList();
	const SCP_vector<std::pair<SCP_string, int>>& getTeamList();

	// Event Management
	void createEvent();
	void insertEvent();
	void deleteEvent();
	void renameEvent(int id, const SCP_string& name);
	int getFormula() const;
	void setFormula(int node);
	int getRepeatCount() const;
	void setRepeatCount(int count);
	int getTriggerCount() const;
	void setTriggerCount(int count);
	int getIntervalTime() const;
	void setIntervalTime(int time);
	bool getChained() const;
	void setChained(bool chained);
	int getChainDelay() const;
	void setChainDelay(int delay);
	bool getUseMsecs() const;
	void setUseMsecs(bool useMsecs);
	int getEventScore() const;
	void setEventScore(int score);
	int getEventTeam() const;
	void setEventTeam(int team);
	SCP_string getEventDirectiveText() const;
	void setEventDirectiveText(const SCP_string& text);
	SCP_string getEventDirectiveKeyText() const;
	void setEventDirectiveKeyText(const SCP_string& text);

	// Event Logging
	bool getLogTrue() const;
	void setLogTrue(bool log);
	bool getLogFalse() const;
	void setLogFalse(bool log);
	bool getLogLogPrevious() const;
	void setLogLogPrevious(bool log);
	bool getLogAlwaysFalse() const;
	void setLogAlwaysFalse(bool log);
	bool getLogFirstRepeat() const;
	void setLogFirstRepeat(bool log);
	bool getLogLastRepeat() const;
	void setLogLastRepeat(bool log);
	bool getLogFirstTrigger() const;
	void setLogFirstTrigger(bool log);
	bool getLogLastTrigger() const;
	void setLogLastTrigger(bool log);

	// Event Annotations. 'key' is an annotation key as used by SexpAnnotationModel:
	// a tree_nodes[] index (>= 0) for a regular node, or rootKey(formula) (<= -2)
	// for an annotation on a labeled root.
	void setNodeAnnotation(int key, const SCP_string& note);
	void setNodeBgColor(int key, int r, int g, int b, bool has_color);

	// Message Management
	void createMessage();
	void insertMessage();
	void deleteMessage();
	void moveMessageUp();
	void moveMessageDown();
	void moveMessageToTop();
	void moveMessageToBottom();
	SCP_string getMessageName() const;
	void setMessageName(const SCP_string& name);
	SCP_string getMessageText() const;
	void setMessageText(const SCP_string& text);
	SCP_string getMessageNote() const;
	void setMessageNote(const SCP_string& note);
	SCP_string getMessageAni() const;
	void setMessageAni(const SCP_string& ani);
	SCP_string getMessageWave() const;
	void setMessageWave(const SCP_string& wave);
	int getMessagePersona() const;
	void setMessagePersona(int persona);
	int getMessageTeam() const;
	void setMessageTeam(int team);

	void autoSelectPersona();
	void playMessageWave();
	const SCP_vector<MMessage>& getMessageList() const;
	static bool getMissionIsMultiTeam();

	void initializeData();

	void setModified();

 signals:
	// Widget operations — dialog connects these to update the tree widget
	void treeCleared();
	void subtreeAdded(const SCP_string& name, NodeImage image, int formula);
	void defaultRootBuilt(const SCP_string& name, int after_root_formula, int new_formula);
	void rootSelected(int formula);
	void eventDeleteRequested();
	void topLevelIndexRequested(int formula, int desired_index);
	void annotationApplied(int key, const SCP_string& note, int r, int g, int b, bool has_color);

  private:

	void initializeEvents();
	void initializeEventAnnotations();
	void initializeTeamList();
	static mission_event makeDefaultEvent();

	int buildDefaultTreeStructure(const SCP_string& name);

	void applyAnnotations();

	void initializeMessages();
	void initializeHeadAniList();
	void initializeWaveList();
	void initializePersonaList();

	bool checkMessageNameConflict(const SCP_string& name);
	SCP_string makeUniqueMessageName(const SCP_string& name) const;

	SexpTreeModel& m_tree_model;

	SCP_vector<mission_event> m_events;
	SexpAnnotationModel m_annotation_model;
	SCP_vector<int> m_sig;
	int m_cur_event = -1;

	SCP_vector<MMessage> m_messages;
	int m_cur_msg = -1;
	int m_wave_id = -1;

	SCP_vector<SCP_string> m_head_ani_list;
	SCP_vector<SCP_string> m_wave_list;

	static SCP_vector<SCP_string> s_extra_head_anis;
	SCP_vector<std::pair<SCP_string, int>> m_persona_list;
	SCP_vector<std::pair<SCP_string, int>> m_team_list;
};

} // namespace fso::fred::dialogs
