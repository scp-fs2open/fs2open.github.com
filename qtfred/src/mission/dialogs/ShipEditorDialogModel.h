#pragma once

#include "AbstractDialogModel.h"

#include "mission/util.h"
#include "ship/ship.h"
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

	bool enable = true;
	int player_count;
	int ship_count;
	bool multi_edit;
	int cue_init;
	int total_count;
	int pvalid_count;
	int pship_count; // a total count of the player ships not marked
	int single_ship;
	int player_ship;

	std::set<size_t> ship_orders;

		bool texenable = true;

  public:
	ShipEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setShipName(const SCP_string& m_ship_name);
	SCP_string getShipName() const;

	void setShipClass(const int);
	int getShipClass() const;

	void setAIClass(const int);
	int getAIClass() const;

	void setTeam(const int);
	int getTeam() const;

	void setCargo(const SCP_string&);
	SCP_string getCargo() const;

	void setAltName(const SCP_string&);
	SCP_string getAltName() const;

	void setCallsign(const SCP_string&);
	SCP_string getCallsign() const;

	SCP_string getWing() const;

	void setHotkey(const int);
	int getHotkey() const;

	void setPersona(const int);
	int getPersona() const;

	void setScore(const int);
	int getScore() const;

	void setAssist(const int);
	int getAssist() const;

	void setPlayer(const bool);
	bool getPlayer() const;

	void setArrivalLocation(const int);
	int getArrivalLocation() const;

	void setArrivalTarget(const int);
	int getArrivalTarget() const;

	void setArrivalDistance(const int);
	int getArrivalDistance() const;

	void setArrivalDelay(const int);
	int getArrivalDelay() const;

	void setArrivalCue(const bool);
	bool getArrivalCue() const;

	void setArrivalFormula(const int, const int);
	int getArrivalFormula() const;

	void setNoArrivalWarp(const int);
	int getNoArrivalWarp() const;

	void setDepartureLocation(const int);
	int getDepartureLocation() const;

	void setDepartureTarget(const int);
	int getDepartureTarget() const;

	void setDepartureDelay(const int);
	int getDepartureDelay() const;

	void setDepartureCue(const bool);
	bool getDepartureCue() const;

	void setDepartureFormula(const int, const int);
	int getDepartureFormula() const;
	void setNoDepartureWarp(const int);
	int getNoDepartureWarp() const;

	void OnPrevious();
	void OnNext();
	void OnDeleteShip();
	void OnShipReset();

	static bool wing_is_player_wing(const int);
	std::set<size_t> getShipOrders() const;

	bool getTexEditEnable() const;
	static int tristate_set(const int val, const int cur_state);

	/**

	int getSingleShip() const;
	bool getIfMultipleShips() const;
	int getNumSelectedPlayers() const;
	int getNumPlayers() const;
	bool getUIEnable() const;
	int getNumSelectedShips() const;
	int getUseCue() const;
	int getNumSelectedObjects() const;
	int getNumValidPlayers() const;
	int getIfPlayerShip() const;
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