#include "ShipCustomWarpDialogModel.h"

#include "ship/shipfx.h"

namespace fso::fred::dialogs {

ShipCustomWarpDialogModel::ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure)
	: AbstractDialogModel(parent, viewport), _departure(departure), _target(Target::Selection)
{
	initializeData();
}

ShipCustomWarpDialogModel::ShipCustomWarpDialogModel(QObject* parent,
	EditorViewport* viewport,
	bool departure,
	Target target,
	int wingIndex)
	: AbstractDialogModel(parent, viewport), _departure(departure), _target(target), _wingIndex(wingIndex)
{
	initializeData();
}

bool ShipCustomWarpDialogModel::apply()
{
	WarpParams params;
	params.direction = _departure ? WarpDirection::WARP_OUT : WarpDirection::WARP_IN;

	if (_warpType < Num_warp_types) {
		params.warp_type = _warpType;
	} else {
		params.warp_type = (_warpType - Num_warp_types) | WT_DEFAULT_WITH_FIREBALL;
	}

	if (!_startSound.empty()) {
		gamesnd_id id = gamesnd_get_by_name(_startSound.c_str());
		if (id.value() == -1) {
			Warning(LOCATION, "Game Sound \"%s\" does not exist. Skipping", _startSound.c_str());
		} else {
			params.snd_start = id;
		}
	}

	if (!_endSound.empty()) {
		gamesnd_id id = gamesnd_get_by_name(_endSound.c_str());
		if (id.value() == -1) {
			Warning(LOCATION, "Game Sound \"%s\" does not exist. Skipping", _endSound.c_str());
		} else {
			params.snd_end = id;
		}
	}

	if (_departure && _warpoutEngageTime) {
		params.warpout_engage_time = fl2i(_warpoutEngageTime * 1000.0f);
	}
	if (_speed) {
		params.speed = _speed;
	}
	if (_time) {
		params.time = fl2i(_time * 1000.0f);
	}
	if (_accelExp) {
		params.accel_exp = _accelExp;
	}
	if (_radius) {
		params.radius = _radius;
	}
	if (!_anim.empty()) {
		strcpy_s(params.anim, _anim.c_str());
	}
	params.supercap_warp_physics = _supercapWarpPhysics;
	if (_departure && _playerWarpoutSpeed) {
		params.warpout_player_speed = _playerWarpoutSpeed;
	}
	int index = find_or_add_warp_params(params);

	if (_target == Target::Wing && _wingIndex >= 0) {
		for (object* objp : list_range(&obj_used_list)) {
			if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
				auto& sh = Ships[objp->instance];
				if (sh.wingnum != _wingIndex)
					continue;
				if (!_departure)
					sh.warpin_params_index = index;
				else
					sh.warpout_params_index = index;
			}
		}
	} else {
		for (object* objp : list_range(&obj_used_list)) {
			if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
				if (objp->flags[Object::Object_Flags::Marked]) {
					auto& sh = Ships[objp->instance];
					if (!_departure)
						sh.warpin_params_index = index;
					else
						sh.warpout_params_index = index;
				}
			}
		}
	}
	_editor->missionChanged();
	return true;
}

void ShipCustomWarpDialogModel::reject() {}

int ShipCustomWarpDialogModel::getType() const
{
	return _warpType;
}

SCP_string ShipCustomWarpDialogModel::getStartSound() const
{
	return _startSound;
}

SCP_string ShipCustomWarpDialogModel::getEndSound() const
{
	return _endSound;
}

float ShipCustomWarpDialogModel::getEngageTime() const
{
	return _warpoutEngageTime;
}

float ShipCustomWarpDialogModel::getSpeed() const
{
	return _speed;
}

float ShipCustomWarpDialogModel::getTime() const
{
	return _time;
}

float ShipCustomWarpDialogModel::getExponent() const
{
	return _accelExp;
}

float ShipCustomWarpDialogModel::getRadius() const
{
	return _radius;
}

SCP_string ShipCustomWarpDialogModel::getAnim() const
{
	return _anim;
}

bool ShipCustomWarpDialogModel::getSupercap() const
{
	return _supercapWarpPhysics;
}

float ShipCustomWarpDialogModel::getPlayerSpeed() const
{
	return _playerWarpoutSpeed;
}

bool ShipCustomWarpDialogModel::departMode() const
{
	return _departure;
}

bool ShipCustomWarpDialogModel::isPlayer() const
{
	return _player;
}

void ShipCustomWarpDialogModel::initializeData()
{
	// find the params of the first marked ship
	WarpParams* params = nullptr;
	if (_target == Target::Wing && _wingIndex >= 0) {
		// Use first ship in the wing for initial values; mark _player if the wing contains the player
		for (object* objp : list_range(&obj_used_list)) {
			if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
				const auto& sh = Ships[objp->instance];
				if (sh.wingnum == _wingIndex) {
					if (!_departure)
						params = &Warp_params[sh.warpin_params_index];
					else
						params = &Warp_params[sh.warpout_params_index];
					if (objp->type == OBJ_START)
						_player = true;
					break;
				}
			}
		}
	} else {
		for (object* objp : list_range(&obj_used_list)) {
			if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
				if (objp->flags[Object::Object_Flags::Marked]) {
					const auto& sh = Ships[objp->instance];
					if (!_departure)
						params = &Warp_params[sh.warpin_params_index];
					else
						params = &Warp_params[sh.warpout_params_index];
					if (objp->type == OBJ_START)
						_player = true;
					break;
				}
			}
		}
	}

	if (params != nullptr) {
		if (params->warp_type & WT_DEFAULT_WITH_FIREBALL) {
			_warpType = (params->warp_type & WT_FLAG_MASK) + Num_warp_types;
		} else if (params->warp_type >= 0 && params->warp_type < Num_warp_types) {
			_warpType = params->warp_type;
		}

		if (params->snd_start.isValid()) {
			_startSound = gamesnd_get_game_sound(params->snd_start)->name;
		}
		if (params->snd_end.isValid()) {
			_endSound = gamesnd_get_game_sound(params->snd_end)->name;
		}

		if (params->warpout_engage_time > 0) {
			_warpoutEngageTime = i2fl(params->warpout_engage_time) / 1000.0f;
		}

		if (params->speed > 0.0f) {
			_speed = params->speed;
		}

		if (params->time > 0.0f) {
			_time = i2fl(params->time) / 1000.0f;
		}
		if (params->accel_exp > 0.0f) {
			_accelExp = params->accel_exp;
		}

		if (params->radius > 0.0f) {
			_radius = params->radius;
		}

		if (strlen(params->anim) > 0) {
			_anim = params->anim;
		}

		_supercapWarpPhysics = params->supercap_warp_physics;

		if (params->warpout_player_speed > 0.0f) {
			_playerWarpoutSpeed = params->warpout_player_speed;
		}
	}
	_modified = false;
}

void ShipCustomWarpDialogModel::setType(int index)
{
	modify(_warpType, index);
}

void ShipCustomWarpDialogModel::setStartSound(const SCP_string& sound)
{
	if (!sound.empty()) {
		modify(_startSound, sound);
	} else {
		_startSound = "";
		set_modified();
	}
}

void ShipCustomWarpDialogModel::setEndSound(const SCP_string& sound)
{
	if (!sound.empty()) {
		modify(_endSound, sound);
	} else {
		_endSound = "";
		set_modified();
	}
}

void ShipCustomWarpDialogModel::setEngageTime(double engageTime)
{
	modify(_warpoutEngageTime, static_cast<float>(engageTime));
}

void ShipCustomWarpDialogModel::setSpeed(double speed)
{
	modify(_speed, static_cast<float>(speed));
}

void ShipCustomWarpDialogModel::setTime(double time)
{
	modify(_time, static_cast<float>(time));
}

void ShipCustomWarpDialogModel::setExponent(double exponent)
{
	modify(_accelExp, static_cast<float>(exponent));
}

void ShipCustomWarpDialogModel::setRadius(double radius)
{
	modify(_radius, static_cast<float>(radius));
}

void ShipCustomWarpDialogModel::setAnim(const SCP_string& anim)
{
	if (!anim.empty()) {
		modify(_anim, anim);
	} else {
		_anim = "";
		set_modified();
	}
}

void ShipCustomWarpDialogModel::setSupercap(bool supercap)
{
	modify(_supercapWarpPhysics, supercap);
}

void ShipCustomWarpDialogModel::setPlayerSpeed(double playerSpeed)
{
	modify(_playerWarpoutSpeed, static_cast<float>(playerSpeed));
}

} // namespace fso::fred::dialogs
