#include "ShipCustomWarpDialogModel.h"

#include "ship/shipfx.h"
namespace fso::fred::dialogs {
ShipCustomWarpDialogModel::ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure)
	: AbstractDialogModel(parent, viewport), _m_departure(departure)
{
	initializeData();
}

bool ShipCustomWarpDialogModel::apply()
{
	WarpParams params;
	params.direction = _m_departure ? WarpDirection::WARP_OUT : WarpDirection::WARP_IN;

	if (_m_warp_type < Num_warp_types) {
		params.warp_type = _m_warp_type;
	} else {
		params.warp_type = (_m_warp_type - Num_warp_types) | WT_DEFAULT_WITH_FIREBALL;
	}

	if (!_m_start_sound.empty()) {
		gamesnd_id id = gamesnd_get_by_name(_m_start_sound.c_str());
		if (id.value() == -1) {
			Warning(LOCATION, "Game Sound \"%s\" does not exist. Skipping", _m_start_sound.c_str());
		} else {
			params.snd_start = id;
		}
	}

	if (!_m_end_sound.empty()) {
		gamesnd_id id = gamesnd_get_by_name(_m_end_sound.c_str());
		if (id.value() == -1) {
			Warning(LOCATION, "Game Sound \"%s\" does not exist. Skipping", _m_end_sound.c_str());
		} else {
			params.snd_end = id;
		}
	}

	if (_m_departure && _m_warpout_engage_time) {
		params.warpout_engage_time = fl2i(_m_warpout_engage_time * 1000.0f);
	}
	if (_m_speed) {
		params.speed = _m_speed;
	}
	if (_m_time) {
		params.time = fl2i(_m_time * 1000.0f);
	}
	if (_m_accel_exp) {
		params.accel_exp = _m_accel_exp;
	}
	if (_m_radius) {
		params.accel_exp = _m_radius;
	}
	if (!_m_anim.empty()) {
		strcpy_s(params.anim, _m_anim.c_str());
	}
	params.supercap_warp_physics = _m_supercap_warp_physics;
	if (_m_departure && _m_player_warpout_speed) {
		params.warpout_player_speed = _m_player_warpout_speed;
	}
	int index = find_or_add_warp_params(params);

	for (object* objp : list_range(&obj_used_list)) {
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				if (!_m_departure)
					Ships[objp->instance].warpin_params_index = index;
				else
					Ships[objp->instance].warpout_params_index = index;
			}
		}
	}
	_editor->missionChanged();
	return true;
}

void ShipCustomWarpDialogModel::reject() {}

int ShipCustomWarpDialogModel::getType() const
{
	return _m_warp_type;
}

SCP_string ShipCustomWarpDialogModel::getStartSound() const
{
	return _m_start_sound;
}

SCP_string ShipCustomWarpDialogModel::getEndSound() const
{
	return _m_end_sound;
}

float ShipCustomWarpDialogModel::getEngageTime() const
{
	return _m_warpout_engage_time;
}

float ShipCustomWarpDialogModel::getSpeed() const
{
	return _m_speed;
}

float ShipCustomWarpDialogModel::getTime() const
{
	return _m_time;
}

float ShipCustomWarpDialogModel::getExponent() const
{
	return _m_accel_exp;
}

float ShipCustomWarpDialogModel::getRadius() const
{
	return _m_radius;
}

SCP_string ShipCustomWarpDialogModel::getAnim() const
{
	return _m_anim;
}

bool ShipCustomWarpDialogModel::getSupercap() const
{
	return _m_supercap_warp_physics;
}

float ShipCustomWarpDialogModel::getPlayerSpeed() const
{
	return _m_player_warpout_speed;
}

bool ShipCustomWarpDialogModel::departMode() const
{
	return _m_departure;
}

bool ShipCustomWarpDialogModel::isPlayer() const
{
	return _m_player;
}

void ShipCustomWarpDialogModel::initializeData()
{
	// find the params of the first marked ship
	WarpParams* params = nullptr;
	for (object* objp : list_range(&obj_used_list)) {
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				if (!_m_departure) {
					params = &Warp_params[Ships[objp->instance].warpin_params_index];
				} else {
					params = &Warp_params[Ships[objp->instance].warpout_params_index];
				}
				if (objp->type == OBJ_START) {
					_m_player = true;
				}
				break;
			}
		}
	}

	if (params != nullptr) {
		if (params->warp_type & WT_DEFAULT_WITH_FIREBALL) {
			_m_warp_type = (params->warp_type & WT_FLAG_MASK) + Num_warp_types;
		} else if (params->warp_type >= 0 && params->warp_type < Num_warp_types) {
			_m_warp_type = params->warp_type;
		}

		if (params->snd_start.isValid()) {
			_m_start_sound = gamesnd_get_game_sound(params->snd_start)->name;
		}
		if (params->snd_end.isValid()) {
			_m_end_sound = gamesnd_get_game_sound(params->snd_end)->name;
		}

		if (params->warpout_engage_time > 0) {
			_m_warpout_engage_time = i2fl(params->warpout_engage_time) / 1000.0f;
		}

		if (params->speed > 0.0f) {
			_m_speed = params->speed;
		}

		if (params->time > 0.0f) {
			_m_time = i2fl(params->time) / 1000.0f;
		}
		if (params->accel_exp > 0.0f) {
			_m_accel_exp = params->accel_exp;
		}

		if (params->radius > 0.0f) {
			_m_radius = params->radius;
		}

		if (strlen(params->anim) > 0) {
			_m_anim = params->anim;
		}

		_m_supercap_warp_physics = params->supercap_warp_physics;

		if (params->warpout_player_speed > 0.0f) {
			_m_player_warpout_speed = params->warpout_player_speed;
		}
	}
}

void ShipCustomWarpDialogModel::setType(const int index)
{
	modify(_m_warp_type, index);
}
void ShipCustomWarpDialogModel::setStartSound(const SCP_string& newSound)
{
	if (!newSound.empty()) {
		modify(_m_start_sound, newSound);
	} else {
		_m_start_sound = "";
		set_modified();
	}
}
void ShipCustomWarpDialogModel::setEndSound(const SCP_string& newSound)
{
	if (!newSound.empty()) {
		modify(_m_end_sound, newSound);
	} else {
		_m_end_sound = "";
		set_modified();
	}
}
void ShipCustomWarpDialogModel::setEngageTime(const double newValue)
{
	modify(_m_warpout_engage_time, static_cast<float>(newValue));
}
void ShipCustomWarpDialogModel::setSpeed(const double newValue)
{
	modify(_m_speed, static_cast<float>(newValue));
}
void ShipCustomWarpDialogModel::setTime(const double newValue)
{
	modify(_m_time, static_cast<float>(newValue));
}
void ShipCustomWarpDialogModel::setExponent(const double newValue)
{
	modify(_m_accel_exp, static_cast<float>(newValue));
}
void ShipCustomWarpDialogModel::setRadius(const double newValue)
{
	modify(_m_radius, static_cast<float>(newValue));
}
void ShipCustomWarpDialogModel::setAnim(const SCP_string& newAnim)
{
	if (!newAnim.empty()) {
		modify(_m_anim, newAnim);
	} else {
		_m_anim = "";
		set_modified();
	}
}
void ShipCustomWarpDialogModel::setSupercap(const bool checked)
{
	modify(_m_supercap_warp_physics, checked);
}
void ShipCustomWarpDialogModel::setPlayerSpeed(const double newValue)
{
	modify(_m_player_warpout_speed, static_cast<float>(newValue));
}
} // namespace dialogs