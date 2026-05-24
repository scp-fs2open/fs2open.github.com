#pragma once

#include "../AbstractDialogModel.h"

#include "ai/ai.h"
#include "ai/aigoals.h"

namespace fso::fred::dialogs {

constexpr auto ED_MAX_GOALS = MAX_AI_GOALS;
constexpr auto MAX_EDITOR_GOAL_PRIORITY = 200;
constexpr auto TYPE_PATH = 0x1000;
constexpr auto TYPE_SHIP = 0x2000;
constexpr auto TYPE_PLAYER = 0x3000;
constexpr auto TYPE_WING = 0x4000;
constexpr auto TYPE_WAYPOINT = 0x5000;
constexpr auto TYPE_SHIP_CLASS = 0x6000;
constexpr auto TYPE_MASK = 0xf000;
constexpr auto DATA_MASK = 0x0fff;
constexpr auto MAX_VALID = 99;

class ShipGoalsDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	ShipGoalsDialogModel(QObject* parent, EditorViewport* viewport, bool multi, int selfShip, int selfWing);

	const SCP_vector<std::pair<const char*, SCP_set<ai_goal_mode>>>& getAiGoalComboData();
	ai_goal_mode getFirstModeFromComboBox(int whichItem);

	bool apply() override;
	void reject() override;

	void setShip(int shipNum);
	int getShip() const;

	void setWing(int wingNum);
	int getWing() const;

	ai_goal* getGoal() const;

	int getValid(int pos) const;
	static const ai_goal_list* getGoalTypes();
	int getGoalsSize() const;

	void setBehavior(int index, int behavior);
	int getBehavior(int index) const;

	void setObject(int index, int objNum);
	int getObject(int index) const;

	void setSubsys(int index, const SCP_string& subsys);
	SCP_string getSubsys(int index) const;

	void setDock(int index, long long dock);
	int getDock(int index) const;

	void setPriority(int index, int priority);
	int getPriority(int index) const;

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData(bool multi, int selfShip, int selfWing);
	void initialize(ai_goal* goals);
	void initializeMulti();
	void initComboData();
	int verifyOrders();
	void updateItem(int item);

	int _aiGoalListSize = Editor::getAigoal_list_size();
	int _selfShip;
	int _selfWing;
	int _behavior[ED_MAX_GOALS];
	int _object[ED_MAX_GOALS];
	int _priority[ED_MAX_GOALS];
	SCP_string _subsys[ED_MAX_GOALS];
	long long _dock2[ED_MAX_GOALS];
	SCP_vector<std::pair<const char*, SCP_set<ai_goal_mode>>> _aiGoalComboData;
	int _valid[MAX_VALID];
	bool _multiEdit;
	ai_goal* _goalp;
};

} // namespace fso::fred::dialogs
