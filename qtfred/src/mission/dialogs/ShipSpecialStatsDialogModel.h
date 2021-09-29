#pragma once

#include "AbstractDialogModel.h"

namespace fso {
	namespace fred {
		namespace dialogs {
			class ShipSpecialStatsDialogModel : public AbstractDialogModel {
			private:
				void initializeData();
				template <typename T>
				void modify(T& a, const T& b);
				bool _modified = false;
				bool m_multi_edit;
				int m_ship;
				//Special Explosion
				bool m_sepecial_exp;
				bool m_shockwave;
				bool m_deathRoll;
				int m_inner_rad;
				int m_outer_rad;
				int m_damage;
				int m_shock_speed;
				int m_duration;
				int m_blast;

				//Special Hits
				bool m_special_hitpoints_enabled;
				bool m_special_shield_enabled;
				int m_shields;
				int m_hull;

			public:
				ShipSpecialStatsDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
				bool apply() override;
				void reject() override;
			};
			template <typename T>
			inline void ShipSpecialStatsDialogModel::modify(T& a, const T& b)
			{
				if (a != b) {
					a = b;
					set_modified();
					modelChanged();
				}
			}
		}
	}
}