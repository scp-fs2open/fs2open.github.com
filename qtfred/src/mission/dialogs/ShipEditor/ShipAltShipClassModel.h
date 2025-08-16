#pragma once
#include "../AbstractDialogModel.h"
namespace fso::fred::dialogs {
/**
 * @brief Model for QtFRED's Alt Ship Class dialog
 */
class ShipAltShipClassModel : public AbstractDialogModel {
  private:
	/**
	 * @brief Initialises data for the model
	 */
	void initializeData();

	SCP_vector<alt_class> alt_class_pool;

	bool _multi_edit = false;

	bool _player_flyable_ships_only = true;

	int _num_string_variables = 0;

	int _num_selected_ships = 0;

	SCP_vector<int> _m_selected_ships;

	SCP_vector<int> _m_alt_class_list;

	SCP_vector<SCP_string> _m_set_from_variables;

	SCP_vector<std::reference_wrapper<sexp_variable>> _string_variables;

	SCP_vector<SCP_string> _m_set_from_ship_class;
	SCP_vector<int> _ship_class_indices;
  public:
	/**
	 * @brief Constructor
	 * @param [in] parent The parent dialog.
	 * @param [in] viewport The viewport this dialog is attacted to.
	 */
	ShipAltShipClassModel(QObject* parent, EditorViewport* viewport, bool is_several_ships);
	bool apply() override;
	void reject() override;
};
} // namespace fso::fred::dialogs