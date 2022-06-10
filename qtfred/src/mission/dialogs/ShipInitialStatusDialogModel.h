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
void initial_status_unmark_dock_handled_flag(object* objp);
void reset_arrival_to_false(int shipnum, bool reset_wing, EditorViewport*);
bool set_cue_to_false(int* cue);
class ShipInitialStatusDialogModel : public AbstractDialogModel {

  private:
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

	dockpoint_information* dockpoint_array;

	void set_modified();

	void update_docking_info();
	void undock(object*, object*);
	void dock(object*, int, object*, int);
	void dock_evaluate_all_docked_objects(object* objp,
		dock_function_info* infop,
		void (*function)(object*, dock_function_info*, EditorViewport*));
	void dock_evaluate_all_docked_objects(object* objp, dock_function_info* infop, void (*function)(object*));
	void dock_evaluate_tree(object* objp,
		dock_function_info* infop,
		void (*function)(object*, dock_function_info*, EditorViewport*),
		ubyte* visited_bitstring);
	void
	dock_evaluate_tree(object* objp, dock_function_info* infop, void (*function)(object*), ubyte* visited_bitstring);
	bool m_multi_edit;
	bool m_use_teams = false;

  public:
	ShipInitialStatusDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
	void initializeData(bool);

	bool apply() override;
	void reject() override;
	bool query_modified();

	void setVelocity(const int);
	int getVelocity() const;

	void setHull(const int);
	int getHull() const;

	void setHasShield(const int);
	int getHasShield() const;

	void setShieldHull(const int);
	int getShieldHull() const;

	void setForceShield(const int);
	int getForceShield() const;

	void setShipLocked(const int);
	int getShipLocked() const;

	void setWeaponLocked(const int);
	int getWeaponLocked() const;

	void setPrimariesDisabled(const int);
	int getPrimariesDisabled() const;

	void setSecondariesDisabled(const int);
	int getSecondariesDisabled() const;

	void setTurretsDisabled(const int);
	int getTurretsDisabled() const;

	void setAfterburnerDisabled(const int);
	int getAfterburnerDisabled() const;

	void setDamage(const int);
	int getDamage() const;

	SCP_string getCargo() const;
	void setCargo(const SCP_string&);

	SCP_string getColour() const;
	void setColour(const SCP_string&);

	void change_subsys(const int);

	int getShip() const;
	int getnum_dock_points() const;
	int getShip_has_scannable_subsystems() const;
	dockpoint_information* getdockpoint_array() const;
	void setDockee(const int, const int);
	void setDockeePoint(const int, const int);

	bool getUseTeamcolours() const;
	bool getIfMultpleShips() const;
};

/**
 * @brief Handles setting a flag on a flagset when the value is inconsistent
 *
 * This is necessary in case multiple ships with inconsistent object flags have been selected in which case
 * that flag may not be edited since it would corrupt the value of that flag. This function simplifies handling
 * that case.
 * @warning Constains QT code neeeds updating if moved to non QT enviromnet
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