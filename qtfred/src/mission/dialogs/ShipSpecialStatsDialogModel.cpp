#include "ShipSpecialStatsDialogModel.h"
namespace fso {
	namespace fred {
		namespace dialogs {
			ShipSpecialStatsDialogModel::ShipSpecialStatsDialogModel(QObject* parent, EditorViewport* viewport)
				: AbstractDialogModel(parent, viewport)
			{
				initializeData();
			}

			void ShipSpecialStatsDialogModel::initializeData()
			{
				object* objp;
				num_selected_ships = 0;
				m_selected_ships.resize(MAX_SHIPS);

				objp = GET_FIRST(&obj_used_list);
				while (objp != END_OF_LIST(&obj_used_list)) {
					if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
						if (objp->flags[Object::Object_Flags::Marked]) {
							m_selected_ships[num_selected_ships] = objp->instance;
							num_selected_ships++;
						}
					}
					objp = GET_NEXT(objp);
				}

				m_ship = _editor->cur_ship;
				//Special Hits
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

				//Special Explosions
				if (!(Ships[m_ship].use_special_explosion)) {
					// get default_table_values
					ship_info* sip;
					sip = &Ship_info[Ships[m_ship].ship_info_index];

					m_inner_rad = (int)sip->shockwave.inner_rad;
					m_outer_rad = (int)sip->shockwave.outer_rad;
					m_damage = (int)sip->shockwave.damage;
					m_blast = (int)sip->shockwave.blast;
					m_shockwave = (int)sip->explosion_propagates;
					m_shock_speed = (int)sip->shockwave.speed;
					m_deathRoll = false;
					m_duration = 0;
					m_special_exp = false;

					if (m_inner_rad < 1) {
						m_inner_rad = 1;
					}
					if (m_outer_rad < 2) {
						m_outer_rad = 2;
					}
					if (m_shock_speed < 1) {
						m_shock_speed = 1;
					}
				}
				else {
					m_inner_rad = Ships[m_ship].special_exp_inner;
					m_outer_rad = Ships[m_ship].special_exp_outer;
					m_damage = Ships[m_ship].special_exp_damage;
					m_blast = Ships[m_ship].special_exp_blast;
					m_shockwave = Ships[m_ship].use_shockwave;
					m_shock_speed = Ships[m_ship].special_exp_shockwave_speed;
					m_deathRoll = (Ships[m_ship].special_exp_deathroll_time > 0);
					m_duration = Ships[m_ship].special_exp_deathroll_time;
					m_special_exp = true;
				}
			}

			bool ShipSpecialStatsDialogModel::apply()
			{
				float temp_max_hull_strength;
				int new_shield_strength, new_hull_strength;
				if (m_special_hitpoints_enabled) {

					// Don't update anything if the hull strength is invalid
					if (m_hull < 1) {
						return false;
					}

					// set to update

					new_hull_strength = m_hull;
					//Ships[m_ship_num].special_hitpoints = m_hull;

				}
				else {
					// set to update

					new_hull_strength = 0;
				}

				if (m_special_shield_enabled) {

					// Don't update anything if the hull strength is invalid
					if (m_shields < 0) {
						return false;
					}

					// set to update

					new_shield_strength = m_shields;
					//Ships[m_ship_num].special_shield = m_shields;

				}
				else {
					// set to update

					new_shield_strength = -1;
				}

				if (m_inner_rad > m_outer_rad) {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Invalid Entry", "Inner radius must be less than outer radius",
						{ DialogButton::Ok });
					if (button == DialogButton::Ok) {
						return false;
					}
				}

				for (auto& shipp : m_selected_ships) {
					// set the special hitpoints/shield
					Ships[shipp].special_hitpoints = new_hull_strength;
					Ships[shipp].special_shield = new_shield_strength;

					// calc kamikaze stuff
					if (Ships[shipp].special_hitpoints)
					{
						temp_max_hull_strength = (float)Ships[shipp].special_hitpoints;
					}
					else
					{
						temp_max_hull_strength = Ship_info[Ships[shipp].ship_info_index].max_hull_strength;
					}

					Ai_info[Ships[shipp].ai_index].kamikaze_damage = std::min(1000, 200 + (int)(temp_max_hull_strength / 4.0f));

					if (m_special_exp) {
						// set em
						Ships[shipp].use_special_explosion = true;
						Ships[shipp].special_exp_inner = m_inner_rad;
						Ships[shipp].special_exp_outer = m_outer_rad;
						Ships[shipp].special_exp_damage = m_damage;
						Ships[shipp].special_exp_blast = m_blast;
						Ships[shipp].use_shockwave = (m_shockwave ? 1 : 0);
						if (m_shock_speed) {
							if (m_shock_speed < 1) {
								m_shock_speed = 1;
								_viewport->dialogProvider->showButtonDialog(DialogType::Warning, "Invalid Entry", "Shockwave speed must be defined! Setting this to 1 now",
									{ DialogButton::Ok });
							}
							Ships[shipp].special_exp_shockwave_speed = m_shock_speed;
						}
						if (m_duration) {
							if (m_duration < 2) {
								m_duration = 2;
								_viewport->dialogProvider->showButtonDialog(DialogType::Warning, "Invalid Entry", "Death roll time must be at least 2 milliseconds Setting this to 2 now",
									{ DialogButton::Ok });
							}
							Ships[shipp].special_exp_deathroll_time = m_duration;
						}
					}
					else {
						Ships[shipp].use_special_explosion = false;
						Ships[shipp].special_exp_inner = -1;
						Ships[shipp].special_exp_outer = -1;
						Ships[shipp].special_exp_damage = -1;
						Ships[shipp].special_exp_blast = -1;
						Ships[shipp].use_shockwave = false;
						Ships[shipp].special_exp_shockwave_speed = -1;
						Ships[shipp].special_exp_deathroll_time = 0;
					}
				}
				_editor->missionChanged();
				return true;
			}
			void ShipSpecialStatsDialogModel::reject()
			{
			}
			void ShipSpecialStatsDialogModel::set_modified()
			{
				if (!_modified) {
					_modified = true;
				}
			}
			bool ShipSpecialStatsDialogModel::query_modified() {
				return _modified;
			}
			bool ShipSpecialStatsDialogModel::getSpecialExp() const
			{
				return m_special_exp;
			}
			void ShipSpecialStatsDialogModel::setSpecialExp(const bool value)
			{
				modify(m_special_exp, value);
			}
			bool ShipSpecialStatsDialogModel::getShockwave() const
			{
				return m_shockwave;
			}
			void ShipSpecialStatsDialogModel::setShockwave(const bool value)
			{
				modify(m_shockwave, value);
			}
			bool ShipSpecialStatsDialogModel::getDeathRoll() const
			{
				return m_deathRoll;
			}
			void ShipSpecialStatsDialogModel::setDeathRoll(const bool value)
			{
				modify(m_deathRoll, value);
			}
			int ShipSpecialStatsDialogModel::getDamage() const
			{
				return m_damage;
			}
			void ShipSpecialStatsDialogModel::setDamage(const int value)
			{
				modify(m_damage, value);
			}
			int ShipSpecialStatsDialogModel::getBlast() const
			{
				return m_blast;
			}
			void ShipSpecialStatsDialogModel::setBlast(const int value)
			{
				modify(m_blast, value);
			}
			int ShipSpecialStatsDialogModel::getInnerRadius() const
			{
				return m_inner_rad;
			}
			void ShipSpecialStatsDialogModel::setInnerRadius(const int value)
			{
				modify(m_inner_rad, value);
			}
			int ShipSpecialStatsDialogModel::getOuterRadius() const
			{
				return m_outer_rad;
			}
			void ShipSpecialStatsDialogModel::setOuterRadius(const int value)
			{
				modify(m_outer_rad, value);
			}
			int ShipSpecialStatsDialogModel::getShockwaveSpeed() const
			{
				return m_shock_speed;
			}
			void ShipSpecialStatsDialogModel::setShockwaveSpeed(const int value)
			{
				modify(m_shock_speed, value);
			}
			int ShipSpecialStatsDialogModel::getRollDuration() const
			{
				return m_duration;
			}
			void ShipSpecialStatsDialogModel::setRollDuration(const int value)
			{
				modify(m_duration, value);
			}
			bool ShipSpecialStatsDialogModel::getSpecialShield() const
			{
				return m_special_shield_enabled;
			}
			void ShipSpecialStatsDialogModel::setSpecialShield(const bool value)
			{
				modify(m_special_shield_enabled, value);
			}
			int ShipSpecialStatsDialogModel::getShield() const
			{
				return m_shields;
			}
			void ShipSpecialStatsDialogModel::setShield(const int value)
			{
				modify(m_shields, value);
			}
			bool ShipSpecialStatsDialogModel::getSpecialHull() const
			{
				return m_special_hitpoints_enabled;
			}
			void ShipSpecialStatsDialogModel::setSpecialHull(const bool value)
			{
				modify(m_special_hitpoints_enabled, value);
			}
			int ShipSpecialStatsDialogModel::getHull() const
			{
				return m_hull;
			}
			void ShipSpecialStatsDialogModel::setHull(const int value)
			{
				modify(m_hull, value);
			}
		}

	}
}