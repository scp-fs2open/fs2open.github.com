#include "ShipSpecialStatsDialogModel.h"
namespace fso {
	namespace fred {
		namespace dialogs {
			ShipSpecialStatsDialogModel::ShipSpecialStatsDialogModel(QObject* parent, EditorViewport* viewport, bool multi)
				: AbstractDialogModel(parent, viewport)
			{
				m_multi_edit = multi;
				initializeData();
			}

			void ShipSpecialStatsDialogModel::initializeData()
			{
				m_ship = _editor->cur_ship;
				if (Ships[m_ship].special_hitpoints) {
					m_hull = Ships[m_ship].special_hitpoints;
					m_special_hitpoints_enabled = true;
				}
				else {
					ship_info* sip;
					sip = &Ship_info[Ships[m_ship].ship_info_index];

					m_hull = (int)sip->max_hull_strength;
					m_special_hitpoints_enabled = false;

					if (m_hull < 1) { m_hull = 10; }
				}

				if (Ships[m_ship].special_shield > 0) {
					m_shields = Ships[m_ship].special_shield;
					m_special_shield_enabled = true;
				}
				else {
					// get default_table_values
					ship_info* sip;
					sip = &Ship_info[Ships[m_ship].ship_info_index];

					m_shields = (int)sip->max_shield_strength;
					m_special_shield_enabled = false;

					if (m_shields < 0) {
						m_shields = 0;
					}
				}
			}
		}
	}
}