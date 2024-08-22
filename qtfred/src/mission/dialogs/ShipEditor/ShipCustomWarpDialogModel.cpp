#include "ShipCustomWarpDialogModel.h"

#include "ship/shipfx.h"
namespace fso {
namespace fred {
namespace dialogs {
ShipCustomWarpDialogModel::ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure)
	: AbstractDialogModel(parent, viewport), _m_departure(departure)
{
	initializeData();
}

bool ShipCustomWarpDialogModel::apply()
{
	return false;
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
	for (object* objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				if (!_m_departure) {
					params = &Warp_params[Ships[objp->instance].warpin_params_index];
				} else {
					params = &Warp_params[Ships[objp->instance].warpout_params_index];
				}
				if (objp->type = OBJ_START) {
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

		_m_supercap_warp_physics = params->supercap_warp_physics ? true : false;

		if (params->warpout_player_speed > 0.0f) {
			_m_player_warpout_speed = params->warpout_player_speed;
		}
	}
}

void ShipCustomWarpDialogModel::set_modified()
{
	if (!_modified) {
		_modified = true;
	}
}

bool ShipCustomWarpDialogModel::query_modified() const
{
	return _modified;
}
} // namespace dialogs
} // namespace fred
} // namespace fso