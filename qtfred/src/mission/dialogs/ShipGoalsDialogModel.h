#pragma once

#include "AbstractDialogModel.h"

#include "ai/aigoals.h"

namespace fso {
namespace fred {
namespace dialogs {
constexpr auto ED_MAX_GOALS = 10;
constexpr auto MAX_EDITOR_GOAL_PRIORITY = 200;
constexpr auto TYPE_PATH = 0x1000;
constexpr auto TYPE_SHIP = 0x2000;
constexpr auto TYPE_PLAYER = 0x3000;
constexpr auto TYPE_WING = 0x4000;
constexpr auto TYPE_WAYPOINT = 0x5000;
constexpr auto TYPE_SHIP_CLASS = 0x6000;
constexpr auto TYPE_MASK = 0xf000;
constexpr auto DATA_MASK = 0x0fff;

class ShipGoalsDialogModel : public AbstractDialogModel {
  private:
	int Ai_goal_list_size = _editor->getAigoal_list_size();
	void initializeData();
	void initialize(ai_goal* goals, int ship);
	void initialize_multi();

	template <typename T>
	void modify(T& a, const T& b);

	bool _modified = false;

	void set_modified();

	int self_ship, self_wing;
	int m_behavior[ED_MAX_GOALS];
	int m_object[ED_MAX_GOALS];
	int m_priority[ED_MAX_GOALS];
	SCP_string m_subsys[ED_MAX_GOALS];
	int m_dock2[ED_MAX_GOALS];
	//int m_data[ED_MAX_GOALS];
	int valid[99];

	bool m_multi_edit;

	ai_goal* goalp;
	int verify_orders(int ship = -1);

	void update_item(int item);

  public:
	ShipGoalsDialogModel(QObject* parent, EditorViewport* viewport, bool multi, int self_ship, int self_wing);
	bool apply() override;
	void reject() override;
	const bool query_modified();

	void setShip(int);
	int getShip();

	void setWing(int);
	int getWing();
	

	const ai_goal* getGoal();

	const int getValid(int);
	const ai_goal_list* getGoalTypes();
	const int getGoalsSize();

	void setBehavior(int, int);
	const int getBehavior(int);

	void setObject(int, int);
	int getObject(int);

	void setSubsys(int, SCP_string);
	SCP_string getSubsys(int);

	void setDock(int, int);
	int getDock(int);

	void setPriority(int, int);
	const int getPriority(int);
};
template <typename T>
inline void ShipGoalsDialogModel::modify(T& a, const T& b)
{
	if (a != b) {
		a = b;
		set_modified();
		modelChanged();
	}
}
} // namespace dialogs
} // namespace fred
} // namespace fso