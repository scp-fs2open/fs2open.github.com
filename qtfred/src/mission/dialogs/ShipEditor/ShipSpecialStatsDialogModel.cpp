#include "ShipSpecialStatsDialogModel.h"

namespace fso::fred::dialogs {

ShipSpecialStatsDialogModel::ShipSpecialStatsDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

void ShipSpecialStatsDialogModel::initializeData()
{
	object* objp;
	_numSelectedShips = 0;
	_selectedShips.resize(MAX_SHIPS);

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				_selectedShips[_numSelectedShips] = objp->instance;
				_numSelectedShips++;
			}
		}
		objp = GET_NEXT(objp);
	}

	_ship = _editor->cur_ship;

	if (Ships[_ship].special_hitpoints) {
		_hull = Ships[_ship].special_hitpoints;
		_specialHitpointsEnabled = true;
	} else {
		ship_info* sip;
		sip = &Ship_info[Ships[_ship].ship_info_index];

		_hull = static_cast<int>(sip->max_hull_strength);
		_specialHitpointsEnabled = false;

		if (_hull < 1) { _hull = 10; }
	}

	if (Ships[_ship].special_shield > 0) {
		_shields = Ships[_ship].special_shield;
		_specialShieldEnabled = true;
	} else {
		ship_info* sip;
		sip = &Ship_info[Ships[_ship].ship_info_index];

		_shields = static_cast<int>(sip->max_shield_strength);
		_specialShieldEnabled = false;

		if (_shields < 0) {
			_shields = 0;
		}
	}

	if (!(Ships[_ship].use_special_explosion)) {
		ship_info* sip;
		sip = &Ship_info[Ships[_ship].ship_info_index];

		_innerRad = static_cast<int>(sip->shockwave.inner_rad);
		_outerRad = static_cast<int>(sip->shockwave.outer_rad);
		_damage = static_cast<int>(sip->shockwave.damage);
		_blast = static_cast<int>(sip->shockwave.blast);
		_shockwave = static_cast<int>(sip->explosion_propagates);
		_shockSpeed = static_cast<int>(sip->shockwave.speed);
		_deathRoll = false;
		_duration = 0;
		_specialExp = false;

		if (_innerRad < 1) {
			_innerRad = 1;
		}
		if (_outerRad < 2) {
			_outerRad = 2;
		}
		if (_shockSpeed < 1) {
			_shockSpeed = 1;
		}
	} else {
		_innerRad = Ships[_ship].special_exp_inner;
		_outerRad = Ships[_ship].special_exp_outer;
		_damage = Ships[_ship].special_exp_damage;
		_blast = Ships[_ship].special_exp_blast;
		_shockwave = Ships[_ship].use_shockwave;
		_shockSpeed = Ships[_ship].special_exp_shockwave_speed;
		_deathRoll = (Ships[_ship].special_exp_deathroll_time > 0);
		_duration = Ships[_ship].special_exp_deathroll_time;
		_specialExp = true;
	}
	modelChanged();
	_modified = false;
}

bool ShipSpecialStatsDialogModel::apply()
{
	float temp_max_hull_strength;
	int new_shield_strength, new_hull_strength;
	if (_specialHitpointsEnabled) {
		if (_hull < 1) {
			return false;
		}
		new_hull_strength = _hull;
	} else {
		new_hull_strength = 0;
	}

	if (_specialShieldEnabled) {
		if (_shields < 0) {
			return false;
		}
		new_shield_strength = _shields;
	} else {
		new_shield_strength = -1;
	}

	if (_innerRad > _outerRad) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Invalid Entry", "Inner radius must be less than outer radius",
			{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	for (auto& shipp : _selectedShips) {
		Ships[shipp].special_hitpoints = new_hull_strength;
		Ships[shipp].special_shield = new_shield_strength;

		if (Ships[shipp].special_hitpoints) {
			temp_max_hull_strength = (float)Ships[shipp].special_hitpoints;
		} else {
			temp_max_hull_strength = Ship_info[Ships[shipp].ship_info_index].max_hull_strength;
		}

		Ai_info[Ships[shipp].ai_index].kamikaze_damage = std::min(1000, 200 + static_cast<int>(temp_max_hull_strength / 4.0f));

		if (_specialExp) {
			Ships[shipp].use_special_explosion = true;
			Ships[shipp].special_exp_inner = _innerRad;
			Ships[shipp].special_exp_outer = _outerRad;
			Ships[shipp].special_exp_damage = _damage;
			Ships[shipp].special_exp_blast = _blast;
			Ships[shipp].use_shockwave = (_shockwave ? 1 : 0);
			if (_shockSpeed) {
				if (_shockSpeed < 1) {
					_shockSpeed = 1;
					_viewport->dialogProvider->showButtonDialog(DialogType::Warning, "Invalid Entry", "Shockwave speed must be defined! Setting this to 1 now",
						{ DialogButton::Ok });
				}
				Ships[shipp].special_exp_shockwave_speed = _shockSpeed;
			}
			if (_duration) {
				if (_duration < 2) {
					_duration = 2;
					_viewport->dialogProvider->showButtonDialog(DialogType::Warning, "Invalid Entry", "Death roll time must be at least 2 milliseconds Setting this to 2 now",
						{ DialogButton::Ok });
				}
				Ships[shipp].special_exp_deathroll_time = _duration;
			}
		} else {
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

void ShipSpecialStatsDialogModel::reject() {}

bool ShipSpecialStatsDialogModel::getSpecialExp() const
{
	return _specialExp;
}

void ShipSpecialStatsDialogModel::setSpecialExp(bool specialExp)
{
	modify(_specialExp, specialExp);
}

bool ShipSpecialStatsDialogModel::getShockwave() const
{
	return _shockwave;
}

void ShipSpecialStatsDialogModel::setShockwave(bool shockwave)
{
	modify(_shockwave, shockwave);
}

bool ShipSpecialStatsDialogModel::getDeathRoll() const
{
	return _deathRoll;
}

void ShipSpecialStatsDialogModel::setDeathRoll(bool deathRoll)
{
	modify(_deathRoll, deathRoll);
}

int ShipSpecialStatsDialogModel::getDamage() const
{
	return _damage;
}

void ShipSpecialStatsDialogModel::setDamage(int damage)
{
	modify(_damage, damage);
}

int ShipSpecialStatsDialogModel::getBlast() const
{
	return _blast;
}

void ShipSpecialStatsDialogModel::setBlast(int blast)
{
	modify(_blast, blast);
}

int ShipSpecialStatsDialogModel::getInnerRadius() const
{
	return _innerRad;
}

void ShipSpecialStatsDialogModel::setInnerRadius(int innerRadius)
{
	modify(_innerRad, innerRadius);
}

int ShipSpecialStatsDialogModel::getOuterRadius() const
{
	return _outerRad;
}

void ShipSpecialStatsDialogModel::setOuterRadius(int outerRadius)
{
	modify(_outerRad, outerRadius);
}

int ShipSpecialStatsDialogModel::getShockwaveSpeed() const
{
	return _shockSpeed;
}

void ShipSpecialStatsDialogModel::setShockwaveSpeed(int speed)
{
	modify(_shockSpeed, speed);
}

int ShipSpecialStatsDialogModel::getRollDuration() const
{
	return _duration;
}

void ShipSpecialStatsDialogModel::setRollDuration(int duration)
{
	modify(_duration, duration);
}

bool ShipSpecialStatsDialogModel::getSpecialShield() const
{
	return _specialShieldEnabled;
}

void ShipSpecialStatsDialogModel::setSpecialShield(bool specialShield)
{
	modify(_specialShieldEnabled, specialShield);
}

int ShipSpecialStatsDialogModel::getShield() const
{
	return _shields;
}

void ShipSpecialStatsDialogModel::setShield(int shield)
{
	modify(_shields, shield);
}

bool ShipSpecialStatsDialogModel::getSpecialHull() const
{
	return _specialHitpointsEnabled;
}

void ShipSpecialStatsDialogModel::setSpecialHull(bool specialHull)
{
	modify(_specialHitpointsEnabled, specialHull);
}

int ShipSpecialStatsDialogModel::getHull() const
{
	return _hull;
}

void ShipSpecialStatsDialogModel::setHull(int hull)
{
	modify(_hull, hull);
}

} // namespace fso::fred::dialogs
