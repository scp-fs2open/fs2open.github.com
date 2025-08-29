#pragma once

#include "AbstractDialogModel.h"

#include "ui/widgets/sexp_tree.h"

#include <mission/missiongoals.h>
#include <mission/missionmessage.h>
#include <mission/missionparse.h>

namespace fso::fred::dialogs {

	struct IEventTreeOps {
		using Handle = void*;

		virtual ~IEventTreeOps() = default;

		// Called after the tree is loaded, to allow for any post-load operations.
		virtual int load_sub_tree(int formula, bool allow_empty = false, const char* default_body = "do-nothing") = 0;

		// deselects all nodes
		virtual void post_load() = 0;

		// adds the tree and sets the image
		virtual void add_sub_tree(const SCP_string& name, NodeImage image, int formula) = 0;

		// Insert a new top-level root with the given name and the default body:
		//   when -> true -> do-nothing
		// If after_root >= 0, place visually after that root; otherwise append.
		// Returns the new root formula id (stored on the root item).
		virtual int build_default_root(const SCP_string& name, int after_root) = 0;

		// Serialize root back into compact SEXP form and return root id.
		virtual int save_tree(int root_formula) = 0;

		// Used for the "insert at index 0" special case.
		virtual void ensure_top_level_index(int root_formula, int desired_index) = 0;

		// Optional: select/highlight the root in the UI.
		virtual void select_root(int root_formula) = 0;

		// Clear the tree
		virtual void clear() = 0;

		// Delete the selected event
		virtual void delete_event() = 0;

		// Navigation
		virtual Handle parent_of(Handle node) = 0;    // nullptr if root
		virtual int index_in_parent(Handle node) = 0; // 0..N-1, or -1 if no parent
		virtual int root_formula_of(Handle node) = 0;

		// Discovery
		virtual bool is_handle_valid(Handle node) = 0;
		virtual Handle get_root_by_formula(int formula) = 0;
		virtual int child_count(Handle node) = 0;
		virtual Handle child_at(Handle node, int idx) = 0;

		// Annotations
		virtual void set_node_note(Handle node, const SCP_string& note) = 0;
		virtual void set_node_bg_color(Handle node, int r, int g, int b, bool has_color) = 0;
	};

class MissionEventsDialogModel : public AbstractDialogModel {
  public:
	MissionEventsDialogModel(QObject* parent, EditorViewport* viewport, IEventTreeOps& tree_ops);

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

	// Event Annotations
	void setNodeAnnotation(IEventTreeOps::Handle h, const SCP_string& note);
	void setNodeBgColor(IEventTreeOps::Handle h, int r, int g, int b, bool has_color);

	// Message Management
	void createMessage();
	void insertMessage();
	void deleteMessage();
	void moveMessageUp();
	void moveMessageDown();
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

	void setModified();

  private:
	void initializeData();

	void initializeEvents();
	int findFormulaByOriginalEventIndex(int orig) const;
	void initializeEventAnnotations();
	SCP_list<int> buildPathForHandle(IEventTreeOps::Handle h) const;
	static bool isDefaultAnnotation(const event_annotation& ea);
	IEventTreeOps::Handle resolveHandleFromPath(const SCP_list<int>& path) const;
	event_annotation& ensureAnnotationByPath(const SCP_list<int>& path);
	void initializeTeamList();
	static mission_event makeDefaultEvent();

	void applyAnnotations();

	void initializeMessages();
	void initializeHeadAniList();
	void initializeWaveList();
	void initializePersonaList();

	bool checkMessageNameConflict(const SCP_string& name);
	SCP_string makeUniqueMessageName(const SCP_string& name) const;

	IEventTreeOps& m_event_tree_ops;

	SCP_vector<mission_event> m_events;
	SCP_vector<event_annotation> m_event_annotations;
	SCP_vector<int> m_sig;
	int m_cur_event = -1;

	SCP_vector<MMessage> m_messages;
	int m_cur_msg = -1;
	int m_wave_id = -1;

	SCP_vector<SCP_string> m_head_ani_list;
	SCP_vector<SCP_string> m_wave_list;
	SCP_vector<std::pair<SCP_string, int>> m_persona_list;
	SCP_vector<std::pair<SCP_string, int>> m_team_list;
};

} // namespace fso::fred::dialogs