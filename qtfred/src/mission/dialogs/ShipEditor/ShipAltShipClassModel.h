#pragma once

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {

class ShipAltShipClassModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	ShipAltShipClassModel(QObject* parent, EditorViewport* viewport);
	bool apply() override;
	void reject() override;

	SCP_vector<alt_class> getPool() const;

	static SCP_vector<std::pair<SCP_string, int>> getClasses();
	static SCP_vector<std::pair<SCP_string, int>> getVariables();
	void syncData(const SCP_vector<alt_class>& newPool);

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();

	SCP_vector<alt_class> _altClassPool;
	int _numSelectedShips = 0;
	SCP_vector<int> _selectedShips;
	SCP_vector<int> _altClassList;
};

} // namespace fso::fred::dialogs
