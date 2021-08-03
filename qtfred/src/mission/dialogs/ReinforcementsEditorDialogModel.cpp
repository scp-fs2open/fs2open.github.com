#include "ReinforcementsEditorDialogModel.h"
#include "ship/ship.h"
#include <algorithm>

namespace fso {
namespace fred {
namespace dialogs {

ReinforcementsDialogModel::ReinforcementsDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

void ReinforcementsDialogModel::initializeData()
{
	for (int i = 0; i < Num_reinforcements; i++) {
		_reinforcementList.emplace_back(Reinforcements[i].name, Reinforcements[i].uses, Reinforcements[i].arrival_delay);
	}

	// add the wings to the model's internal storage
	for (auto& currentWing : Wings) {

		if (currentWing.wave_count == 0) {
			continue;
		}

		// no players allowed in reinforcements!
		for (auto& playerTest : currentWing.ship_index) {
			if (playerTest > -1 && (Objects[Ships[playerTest].objnum].type == OBJ_START)){
				continue;
			}
		}

		bool found = false;
		for (auto& reinforcement : _reinforcementList) {
			if (std::get<0>(reinforcement) == currentWing.name) {
				found = true;
				break;
			}
		}

		if (!found) {
			_shipWingPool.push_back(currentWing.name);
		}
	}

	// add other ships to the remaining storage.
	for (auto& currentShip : Ships) {

		if (!strlen(currentShip.ship_name) || currentShip.wingnum >= 0 || Objects[currentShip.objnum].type == OBJ_START) {
			continue;
		}

		bool found = false;

		for (auto& reinforcement : _reinforcementList) {
			if (std::get<0>(reinforcement) == currentShip.ship_name) {
				found = true;
				break;
			}
		}

		if (!found) {
			_shipWingPool.push_back(currentShip.ship_name);
		}
	}

	_selectedReinforcements.clear();
	_selectedReinforcementIndices.clear();
	_numberLineEditUpdateRequired = true;
	_listUpdateRequired = true;
	modelChanged();
}

bool ReinforcementsDialogModel::apply() 
{
	Num_reinforcements = static_cast<int>(_reinforcementList.size());

	int i = 0;

	// Properly set all reinforcement info.
	for (auto& modelReinforcement : _reinforcementList) {
		strcpy_s(Reinforcements[i].name, std::get<0>(modelReinforcement).c_str());
		Reinforcements[i].uses = std::get<1>(modelReinforcement);
		Reinforcements[i].arrival_delay = std::get<2>(modelReinforcement);
		Reinforcements[i].type = 0;
		memset( Reinforcements[i].no_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
		memset( Reinforcements[i].yes_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );

		i++;
	}

	_shipWingPool.clear();
	_numberLineEditUpdateRequired = false;
	_listUpdateRequired = false;
	_selectedReinforcementIndices.clear();

	return true;
}

void ReinforcementsDialogModel::reject() 
{
	_shipWingPool.clear();
	_reinforcementList.clear();
	_selectedReinforcementIndices.clear();
	_numberLineEditUpdateRequired = false;
	_listUpdateRequired = false;
}

bool ReinforcementsDialogModel::numberLineEditUpdateRequired()
{
	return _numberLineEditUpdateRequired;
}

bool ReinforcementsDialogModel::listUpdateRequired()
{
	return _listUpdateRequired;
}

void ReinforcementsDialogModel::addToReinforcements(const SCP_vector<SCP_string>& namesIn)
{
	for (auto& newReinforcement : namesIn) {

		for (auto candidate = _shipWingPool.begin(); candidate != _shipWingPool.end(); candidate++) {
			if (*candidate == newReinforcement) {
				// the default use number is 1, the default delay is zero
				_reinforcementList.emplace_back(*candidate, 1, 0);
				_shipWingPool.erase(candidate); // not efficient, but order matters.
				break;
			}
		}
	}

	while (_reinforcementList.size() > MAX_REINFORCEMENTS) {
		_shipWingPool.push_back(std::get<0>(_reinforcementList.back()));
		_reinforcementList.pop_back();
	}

	_listUpdateRequired = true;
	_numberLineEditUpdateRequired = true;
	modelChanged();
}

void ReinforcementsDialogModel::removeFromReinforcements(const SCP_vector<SCP_string>& namesIn) 
{
	SCP_vector<SCP_string> removalList;

	for (auto& removal : namesIn) {
		for (auto candidate = _reinforcementList.begin(); candidate != _reinforcementList.end(); candidate++) {
			if (std::get<0>(*candidate) == removal) {
				removalList.push_back(std::get<0>(*candidate));
				_shipWingPool.push_back(removal);
				_reinforcementList.erase(candidate); // not efficient, but order matters.
				break;
			}
		}
	}

	_selectedReinforcements.clear();
	updateSelectedIndices();

	_listUpdateRequired = true;
	_numberLineEditUpdateRequired = true;
	modelChanged();
}

// remember to call this and getReinforcementList together
SCP_vector<SCP_string> ReinforcementsDialogModel::getShipPoolList()
{
	return _shipWingPool;
}

// remember to call this and getShipPoolList together
SCP_vector<SCP_string> ReinforcementsDialogModel::getReinforcementList()
{
	_listUpdateRequired = false;

	SCP_vector<SCP_string> list;

	for (auto& currentReinforcement : _reinforcementList)
	{
		list.push_back(std::get<0>(currentReinforcement));
	}

	return list;
}

int ReinforcementsDialogModel::getUseCount() 
{
	if (_selectedReinforcementIndices.empty()) {
		return -2;
	}

	int previous = -1, current = -1;

	for (auto& item : _selectedReinforcementIndices) {
		current = std::get<1>(_reinforcementList.at(item));

		if (previous == -1 || current == previous) {
			previous = current;
			continue;
		} else {
			// no consistent Use Count in multiple items, return -1 so UI knows to display nothing.
			return -1;
		}
	}

	return current;
}

int ReinforcementsDialogModel::getBeforeArrivalDelay()
{
	if (_selectedReinforcementIndices.empty()) {
		return -2;
	}

	int previous = -1, current = 1;

	for (auto& item : _selectedReinforcementIndices) {
		current = std::get<2>(_reinforcementList.at(item));

		if (previous == -1 || current == previous) {
			previous = current;
		}
		else {
			// no consistent Arrival Delay in these multiple items, return -1 so UI knows to display nothing.
			return -1;
		}

	}

	return current;
}


void ReinforcementsDialogModel::selectReinforcement(const SCP_vector<SCP_string>& namesIn) 
{
	_selectedReinforcements = namesIn;

	updateSelectedIndices();

	Assertion(namesIn.size() == _selectedReinforcementIndices.size(), "%d vs %d", static_cast<int>(namesIn.size()), static_cast<int>(_selectedReinforcementIndices.size()));

	_numberLineEditUpdateRequired = true;
	modelChanged();
}

void ReinforcementsDialogModel::setUseCount(int count) 
{
	for (auto& reinforcement : _selectedReinforcementIndices) {
		std::get<1>(_reinforcementList[reinforcement]) = count;
	}
}

void ReinforcementsDialogModel::setBeforeArrivalDelay(int delay) 
{
	for (auto& reinforcement : _selectedReinforcementIndices) {
		std::get<2>(_reinforcementList[reinforcement]) = delay;
	}
}

void ReinforcementsDialogModel::moveReinforcementsUp()
{
	bool updatedRequired = false;

	for (auto& reinforcement : _selectedReinforcementIndices) {
		
		// these need to be greater than zero because they are moving up not down.
		if ((reinforcement > 0) && (reinforcement < static_cast<int>(_reinforcementList.size()))) {			
			
			std::swap(_reinforcementList[reinforcement], _reinforcementList[reinforcement - 1]);
			updatedRequired = true;
		}
	}

	if (updatedRequired) {
		updateSelectedIndices();
		_listUpdateRequired = true;
		modelChanged();
	}
}

void ReinforcementsDialogModel::moveReinforcementsDown()
{
	bool updatedRequired = false;

	for (auto reinforcement = _selectedReinforcementIndices.rbegin(); reinforcement != _selectedReinforcementIndices.rend(); reinforcement++) {

		if ((*reinforcement < static_cast<int>(_reinforcementList.size()) - 1)) {			
			std::swap(_reinforcementList[*reinforcement], _reinforcementList[*reinforcement + 1]);
			updatedRequired = true;
		}
	}

	if (updatedRequired) {
		updateSelectedIndices();
		_listUpdateRequired = true;
		modelChanged();
	}
}

void ReinforcementsDialogModel::updateSelectedIndices()
{
	_selectedReinforcementIndices.clear();

	for (auto& name : _selectedReinforcements) {
		int index = 0;

		for (auto& listItem : _reinforcementList) {
			if (name == std::get<0>(listItem)) {
				_selectedReinforcementIndices.push_back(index);
				break;
			}
			index++;
		}
	}

	std::sort(_selectedReinforcementIndices.begin(), _selectedReinforcementIndices.end());
}

}
}
}
