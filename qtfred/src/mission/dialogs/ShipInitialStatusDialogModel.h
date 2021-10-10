#pragma once

#include "AbstractDialogModel.h"

#include <object/objectdock.h>

namespace fso {
namespace fred {
namespace dialogs {
typedef struct dockpoint_information {
	int dockee_shipnum;
	int dockee_point;
} dockpoint_information;
constexpr auto BLANKFIELD = 101;
void initial_status_mark_dock_leader_helper(object* objp, dock_function_info* infop, EditorViewport*);
void initial_status_unmark_dock_handled_flag(object* objp, dock_function_info* infop, EditorViewport*);
void reset_arrival_to_false(int shipnum, bool reset_wing, EditorViewport*);
bool set_cue_to_false(int* cue);
class ShipInitialStatusDialogModel : public AbstractDialogModel {

  private:
	void initializeData();

	template <typename T>
	void modify(T& a, const T& b);

	bool _modified = false;

	int m_ship;
	int cur_subsys = -1;

	int m_damage;
	int m_shields;
	int m_force_shields;
	int m_velocity;
	int m_hull;
	int m_has_shields;
	int m_ship_locked;
	int m_weapons_locked;
	SCP_string m_cargo_name;
	int m_primaries_locked;
	int m_secondaries_locked;
	int m_turrets_locked;
	int m_afterburner_locked;
	SCP_string m_team_color_setting;

		int ship_has_scannable_subsystems;

	int num_dock_points;
	// int cur_docker_point;
	// int cur_dockee;
	// int cur_dockee_point;

	dockpoint_information* dockpoint_array;

	bool vchanged;
	bool schanged;
	bool hchanged;

	void set_modified();

	void update_docking_info();
	void undock(object*, object*);
	void dock(object*, int, object*, int);
	void dock_evaluate_all_docked_objects(object* objp,
		dock_function_info* infop,
		void (*function)(object*, dock_function_info*, EditorViewport*));
	void dock_evaluate_tree(object* objp,
		dock_function_info* infop,
		void (*function)(object*, dock_function_info*, EditorViewport*),
		ubyte* visited_bitstring);

  public:
	ShipInitialStatusDialogModel(QObject* parent, EditorViewport* viewport, bool multi);

	bool apply() override;
	void reject() override;
	const bool query_modified();

	void setVelocity(int);
	const int getVelocity();

	void setHull(int);
	const int getHull();

	void setHasShield(int);
	int getHasShield();

	void setShieldHull(int);
	const int getShieldHull();

	void setForceShield(int);
	const int getForceShield();

	void setShipLocked(int);
	const int getShipLocked();

	void setWeaponLocked(int);
	const int getWeaponLocked();

	void setPrimariesDisabled(int);
	const int getPrimariesDisabled();

	void setSecondariesDisabled(int);
	const int getSecondariesDisabled();

	void setTurretsDisabled(int);
	const int getTurretsDisabled();

	void setAfterburnerDisabled(int);
	const int getAfterburnerDisabled();

	void setDamage(int);
	const int getDamage();

	SCP_string getCargo();
	void setCargo(const SCP_string&);

	SCP_string getColour();
	void setColour(const SCP_string&);

	bool m_multi_edit;
	bool m_use_teams = false;

	void change_subsys(int);

	const int getShip();
	const int getnum_dock_points();
	const int getShip_has_scannable_subsystems();
	const dockpoint_information* getdockpoint_array();
	void setDockee(int, int);
	void setDockeePoint(int, int);
};

/**
 * @brief Handles setting a flag on a flagset when the value is inconsistent
 *
 * This is necessary in case multiple ships with inconsistent object flags have been selected in which case
 * that flag may not be edited since it would corrupt the value of that flag. This function simplifies handling
 * that case.
 */
template <typename T>
static void handle_inconsistent_flag(flagset<T>& flags, T flag, int value)
{
	if (value == Qt::Checked) {
		flags.set(flag);
	} else if (value == Qt::Unchecked) {
		flags.remove(flag);
	}
}
template <typename T>
inline void ShipInitialStatusDialogModel::modify(T& a, const T& b)
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