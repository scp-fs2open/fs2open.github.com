#pragma once

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"

namespace fso::fred::dialogs {

class ReinforcementsDialogModel : public AbstractDialogModel {
  public:
	ReinforcementsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void initializeData();

	SCP_vector<SCP_string> getShipPoolList();
	SCP_vector<SCP_string> getReinforcementList();
	int getUseCount();
	bool getUseCountEnabled(const SCP_string& name);
	int getBeforeArrivalDelay();

	void setUseCount(int count);
	void setBeforeArrivalDelay(int delay);

	void addToReinforcements(const SCP_vector<SCP_string>& namesIn);
	void removeFromReinforcements(const SCP_vector<SCP_string>& namesIn);

	void selectReinforcement(const SCP_vector<SCP_string>& namesIn);

	void moveReinforcementsUp();
	void moveReinforcementsDown();

	void updateSelectedIndices();

  private:
	SCP_vector<SCP_string> _shipWingPool; // the list of ships and wings that are not yet reinforcements
	SCP_vector<SCP_string> _useCountEnabled; // the list of reinforcements that have a use count enabled
	SCP_vector<std::tuple<SCP_string, int, int>> _reinforcementList; // use to store the name of the ship, the use count, and the delay before arriving
	SCP_vector<SCP_string> _selectedReinforcements;
	SCP_vector<int> _selectedReinforcementIndices; // keeps track of what ships are currently selected in order to
												   // adjust them more easily, in reverse order.
};

} // namespace fso::fred::dialogs