#pragma once

#include "AbstractDialogModel.h"

#include "ui/widgets/sexp_tree.h"

namespace fso {
namespace fred {
namespace dialogs {

class ShipEditorDialogModel : public AbstractDialogModel {
  private:
	void initializeData();

	template <typename T>
	void modify(T& a, const T& b);

	bool _modified = false;

	bool _m_no_departure_warp;
	bool _m_no_arrival_warp;
	bool _m_player_ship;
	int _m_destroy_spin;
	int _m_departure_delay_spin;
	int _m_arrival_delay_spin;
	sexp_tree _m_departure_tree;
	sexp_tree _m_arrival_tree;
	SCP_string _m_ship_name;
	SCP_string _m_cargo1;
	SCP_string _m_alt_name;
	SCP_string _m_callsign;
	int _m_ship_class;
	int _m_team;
	int _m_arrival_location;
	int _m_departure_location;
	int _m_ai_class;
	int _m_arrival_delay;
	int _m_departure_delay;
	int _m_hotkey;
	bool _m_update_arrival;
	bool _m_update_departure;
	int _m_destroy_value;
	int _m_score;
	int _m_assist_score;
	int _m_arrival_dist;
	int _m_kdamage;
	int _m_arrival_target;
	int _m_departure_target;
	int _m_persona;

	SCP_string m_wing;
	bool m_player;

	int mission_type;

	void set_modified();

	bool update_ship(int ship);
	bool update_data(int redraw);

	void ship_alt_name_close(int base_ship);
	void ship_callsign_close(int base_ship);

  public:
	ShipEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setShipName(const SCP_string& m_ship_name);
	SCP_string getShipName();

	void setShipClass(int);
	int getShipClass();

	void setAIClass(int);
	int getAIClass();

	void setTeam(int);
	int getTeam();

	void setCargo(const SCP_string&);
	SCP_string getCargo();

	void setAltName(const SCP_string&);
	SCP_string getAltName();

	void setCallsign(const SCP_string&);
	SCP_string getCallsign();

	SCP_string getWing();

	bool wing_is_player_wing(int);

	int select_sexp_node;
	int player_count;
	int ship_count;
	int multi_edit;
	int base_ship;
	int cue_init;
	int total_count;
	int pvalid_count;
	int pship_count; // a total count of the player ships not marked
	int player_ship, single_ship;
	int ship_orders;
	int tristate_set(int val, int cur_state);
};

template <typename T>
inline void ShipEditorDialogModel::modify(T& a, const T& b)
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