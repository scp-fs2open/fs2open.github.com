#include "ShipPathsDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {
ShipPathsDialogModel::ShipPathsDialogModel(QObject* parent,
	EditorViewport* viewport,
	const int ship,
	int target_class,
	const bool departure)
	: AbstractDialogModel(parent, viewport)
{
	initalizeData(ship, target_class, departure);
}

void ShipPathsDialogModel::initalizeData(const int ship, int target_class, const bool departure)
{
	departureMode = departure;
	m_ship = ship;
	m_model = model_get(Ship_info[target_class].model_num);
	Assert(m_model->ship_bay);
	m_num_paths = m_model->ship_bay->num_paths;
	Assert(m_num_paths > 0);
	if (!departure) {
		m_path_mask = Ships[m_ship].arrival_path_mask;
	} else {
		m_path_mask = Ships[m_ship].departure_path_mask;
	}
	m_path_list.resize(m_num_paths, true);
	for (int i = 0; i < m_num_paths; i++) {
		bool allowed;
		if (m_path_mask == 0) {
			allowed = true;
		} else {
			allowed = (m_path_mask & (1 << i)) ? true : false;
		}
		m_path_list[i] = allowed;
	}
}

bool ShipPathsDialogModel::apply()
{
	if (!_modified) {
		return true;
	} else {
		int num_allowed = 0;
		m_path_mask = 0;
		for (int i = 0; i < m_num_paths; i++) {
			if (m_path_list[i] == true) {
				m_path_mask |= (1 << i);
				num_allowed++;
			}
		}
		if (num_allowed == m_num_paths) {
			m_path_mask = 0;
		}
		if (!departureMode) {
			Ships[m_ship].arrival_path_mask = m_path_mask;
		} else {
			Ships[m_ship].departure_path_mask = m_path_mask;
		}
		return true;
	}
}

void ShipPathsDialogModel::reject() {}

bool ShipPathsDialogModel::modify(const int index, const bool value)
{
	Assertion(index < m_num_paths, "Requsted index %d is larger than m_num_paths.\n", index);
	if (index < m_num_paths) {
		m_path_list[index] = value;
		_modified = true;
		return true;
	} else {
		return false;
	}
}
SCP_vector<bool> ShipPathsDialogModel::getPathList() const
{
	return m_path_list;
}
polymodel* ShipPathsDialogModel::getModel() const
{
	return m_model;
}
} // namespace dialogs
} // namespace fred
} // namespace fso