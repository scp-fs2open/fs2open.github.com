#pragma once

#include "AbstractDialogModel.h"

#include "ship/ship.h"
#include "ui/widgets/sexp_tree.h"
#include "mission/util.h"

namespace fso {
namespace fred {
namespace dialogs {

class ShipEditorDialogModel : public AbstractDialogModel {
  private:
	void initializeData();

	template <typename T>
	void modify(T& a, const T& b);

	bool _modified = false;
	int _m_no_departure_warp;
	int _m_no_arrival_warp;
	bool _m_player_ship;
	int _m_departure_tree_formula;
	int _m_arrival_tree_formula;
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
	int _m_score;
	int _m_assist_score;
	int _m_arrival_dist;
	int _m_arrival_target;
	int _m_departure_target;
	int _m_persona;

	SCP_string m_wing;

	int mission_type;

	void set_modified();

	bool update_ship(int ship);
	bool update_data();

	void ship_alt_name_close(int base_ship);
	void ship_callsign_close(int base_ship);

	static int make_ship_list(int* arr);

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

	void setHotkey(int);
	int getHotkey();

	void setPersona(int);
	int getPersona();

	void setScore(int);
	int getScore();

	void setAssist(int);
	int getAssist();

	void setPlayer(bool);
	bool getPlayer();

	void setArrivalLocation(int);
	int getArrivalLocation();

	void setArrivalTarget(int);
	int getArrivalTarget();

	void setArrivalDistance(int);
	int getArrivalDistance();

	void setArrivalDelay(int);
	int getArrivalDelay();

	void setArrivalCue(bool);
	bool getArrivalCue();

	void setArrivalFormula(int, int);
	int getArrivalFormula();

	void setNoArrivalWarp(int);
	int getNoArrivalWarp();

	void setDepartureLocation(int);
	int getDepartureLocation();

	void setDepartureTarget(int);
	int getDepartureTarget();

	void setDepartureDelay(int);
	int getDepartureDelay();

	void setDepartureCue(bool);
	bool getDepartureCue();

	void setDepartureFormula(int, int);
	int getDepartureFormula();
	void setNoDepartureWarp(int);
	int getNoDepartureWarp();

	void OnPrevious();
	void OnNext();
	void OnDeleteShip();
	void OnShipReset();

	static bool wing_is_player_wing(int);

	bool enable = true;
	//bool p_enable;
	//int type;
	//int base_player;
	//int select_sexp_node;
	int player_count;
	int ship_count;
	//int escort_count;
	bool multi_edit;
	//int base_ship;
	int cue_init;
	int total_count;
	int pvalid_count;
	int pship_count; // a total count of the player ships not marked
	int single_ship;
	int player_ship;
	std::set<size_t> ship_orders;
	static int tristate_set(int val, int cur_state);

	//int pship, current_orders;
};

template <typename T>
inline void ShipEditorDialogModel::modify(T& a, const T& b)
{
	if (a != b) {
		a = b;
		set_modified();
		update_data();
		modelChanged();
	}
}
} // namespace dialogs
} // namespace fred
} // namespace fso