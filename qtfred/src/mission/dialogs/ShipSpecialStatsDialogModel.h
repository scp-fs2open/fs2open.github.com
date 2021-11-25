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

				int num_selected_ships;
				SCP_vector<int> m_selected_ships;
				//Special Explosion
				bool m_special_exp;
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

				void set_modified();

			public:
				ShipSpecialStatsDialogModel(QObject* parent, EditorViewport* viewport);
				bool apply() override;
				void reject() override;
				bool query_modified();

				bool getSpecialExp() const;
				void setSpecialExp(const bool);
				bool getShockwave() const;
				void setShockwave(const bool);
				bool getDeathRoll() const;
				void setDeathRoll(const bool);

				int getDamage() const;
				void setDamage(const int);
				int getBlast() const;
				void setBlast(const int);
				int getInnerRadius() const;
				void setInnerRadius(const int);
				int getOuterRadius() const;
				void setOuterRadius(const int);
				int getShockwaveSpeed() const;
				void setShockwaveSpeed(const int);
				int getRollDuration() const;
				void setRollDuration(const int);
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