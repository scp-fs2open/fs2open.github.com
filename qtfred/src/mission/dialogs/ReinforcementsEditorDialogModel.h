#pragma once

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"

namespace fso {
namespace fred {
namespace dialogs {

	class ReinforcementsDialogModel : public AbstractDialogModel {
	public:

		ReinforcementsDialogModel(QObject* parent, EditorViewport* viewport);

		bool apply() override;
		void reject() override;

		void initializeData();

		bool numberLineEditUpdateRequired();
		bool listUpdateRequired();

		SCP_vector<SCP_string> getShipPoolList();
		SCP_vector<SCP_string> getReinforcementList();
		int getUseCount();
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
		bool _numberLineEditUpdateRequired;
		bool _listUpdateRequired;

		SCP_vector<SCP_string> _shipWingPool; // the list of ships and wings that are not yet reinforcements
		SCP_vector<std::tuple<SCP_string,int,int>> _reinforcementList; // use to store the name of the ship, the use count, and the delay before arriving
		SCP_vector<SCP_string> _selectedReinforcements;
		SCP_vector<int> _selectedReinforcementIndices; // keeps track of what ships are currently selected in order to adjust them more easily, in reverse order.
	};

}
}
}