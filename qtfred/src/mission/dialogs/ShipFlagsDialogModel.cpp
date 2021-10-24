//
//

#include "ShipFlagsDialogModel.h"

#include "ui/dialogs/ShipFlagsDialog.h"

#include <globalincs/linklist.h>

#include <QtWidgets>

namespace fso {
namespace fred {
namespace dialogs {
void ShipFlagsDialogModel::set_modified()
{
	if (!_modified) {
		_modified = true;
	}
}
int ShipFlagsDialogModel::tristate_set(int val, int cur_state)
{
	if (val) {
		if (!cur_state) {
			return Qt::PartiallyChecked;
		}
	} else {
		if (cur_state) {
			return Qt::PartiallyChecked;
		}
	}
	if (cur_state == 1) {

		return Qt::Checked;
	} else {
		return Qt::Unchecked;
	}
}
void ShipFlagsDialogModel::update_ship(int shipnum)
{
	ship* shipp = &Ships[shipnum];
	object* objp = &Objects[shipp->objnum];

	if (m_reinforcement != Qt::PartiallyChecked) {
		// Check if we're trying to add more and we've got too many.
		if ((Num_reinforcements >= MAX_REINFORCEMENTS) && (m_reinforcement == Qt::Checked)) {
			SCP_string error_message;
			sprintf(error_message,
				"Too many reinforcements; could not add ship '%s' to reinforcement list!",
				shipp->ship_name);
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Flag Error",
				error_message,
				{DialogButton::Ok});
		}
		// Otherwise, just update as normal.
		else {
			_editor->set_reinforcement(shipp->ship_name, m_reinforcement);
		}
	}

	switch (m_cargo_known) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Cargo_revealed])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Cargo_revealed);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Cargo_revealed]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Cargo_revealed);
		break;
	}

	// update the flags for IGNORE_COUNT and PROTECT_SHIP
	switch (m_protect_ship) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Protected])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Protected);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Protected]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Protected);
		break;
	}

	switch (m_beam_protect_ship) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Beam_protected])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Beam_protected);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Beam_protected]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Beam_protected);
		break;
	}

	switch (m_flak_protect_ship) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Flak_protected])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Flak_protected);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Flak_protected]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Flak_protected);
		break;
	}
	switch (m_laser_protect_ship) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Laser_protected])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Laser_protected);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Laser_protected]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Laser_protected);
		break;
	}
	switch (m_missile_protect_ship) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Missile_protected])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Missile_protected);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Missile_protected]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Missile_protected);
		break;
	}

	switch (m_invulnerable) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Invulnerable])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Invulnerable);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Invulnerable]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Invulnerable);
		break;
	}

	switch (m_targetable_as_bomb) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Targetable_as_bomb])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Targetable_as_bomb);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Targetable_as_bomb]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Targetable_as_bomb);
		break;
	}

	switch (m_immobile) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Immobile])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Immobile);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Immobile]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Immobile);
		break;
	}

	switch (m_hidden) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Hidden_from_sensors])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Hidden_from_sensors);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Hidden_from_sensors]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Hidden_from_sensors);
		break;
	}

	switch (m_primitive_sensors) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Primitive_sensors])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Primitive_sensors);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Primitive_sensors]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Primitive_sensors);
		break;
	}

	switch (m_no_subspace_drive) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::No_subspace_drive])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::No_subspace_drive);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::No_subspace_drive]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::No_subspace_drive);
		break;
	}

	switch (m_affected_by_gravity) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Affected_by_gravity])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Affected_by_gravity);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Affected_by_gravity]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Affected_by_gravity);
		break;
	}

	switch (m_toggle_subsystem_scanning) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Toggle_subsystem_scanning])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Toggle_subsystem_scanning);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Toggle_subsystem_scanning]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Toggle_subsystem_scanning);
		break;
	}
	switch (m_ignore_count) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Ignore_count])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Ignore_count);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Ignore_count]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Ignore_count);
		break;
	}

	switch (m_escort) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Escort])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Escort);
		shipp->escort_priority = m_escort_value;
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Escort]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Escort);
		break;
	}

	// deal with updating the "destroy before the mission" stuff
	switch (m_destroy) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Kill_before_mission])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Kill_before_mission);
		shipp->final_death_time = m_destroy_value;
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Kill_before_mission]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Kill_before_mission);
		break;
	}

	switch (m_no_arrival_music) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::No_arrival_music])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::No_arrival_music);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::No_arrival_music]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::No_arrival_music);
		break;
	}

	switch (m_scannable) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Scannable])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Scannable);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Scannable]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Scannable);
		break;
	}

	switch (m_red_alert_carry) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Red_alert_store_status])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Red_alert_store_status);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Red_alert_store_status]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Red_alert_store_status);
		break;
	}

	switch (m_special_warpin) {
	case Qt::Checked:
		if (!(objp->flags[Object::Object_Flags::Special_warpin])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Special_warpin);
		break;
	case Qt::Unchecked:
		if (objp->flags[Object::Object_Flags::Special_warpin]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Special_warpin);
		break;
	}

	switch (m_no_dynamic) {
	case Qt::Checked:
		if (!(Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::No_dynamic])) {
			set_modified();
		}
		Ai_info[shipp->ai_index].ai_flags.set(AI::AI_Flags::No_dynamic);
		break;
	case Qt::Unchecked:
		if (Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::No_dynamic]) {
			set_modified();
		}
		Ai_info[shipp->ai_index].ai_flags.remove(AI::AI_Flags::No_dynamic);
		break;
	}

	switch (m_kamikaze) {
	case Qt::Checked:
		if (!(Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::Kamikaze])) {
			set_modified();
		}
		Ai_info[shipp->ai_index].ai_flags.set(AI::AI_Flags::Kamikaze);
		Ai_info[shipp->ai_index].kamikaze_damage = 0;
		break;
	case Qt::Unchecked:
		if (Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::Kamikaze]) {
			set_modified();
		}
		Ai_info[shipp->ai_index].ai_flags.remove(AI::AI_Flags::Kamikaze);
		Ai_info[shipp->ai_index].kamikaze_damage = m_kdamage;
		break;
	}

	switch (m_disable_messages) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::No_builtin_messages])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::No_builtin_messages);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::No_builtin_messages]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::No_builtin_messages);
		break;
	}

	switch (m_set_class_dynamically) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Set_class_dynamically])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Set_class_dynamically);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Set_class_dynamically]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Set_class_dynamically);
		break;
	}

	switch (m_no_death_scream) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::No_death_scream])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::No_death_scream);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::No_death_scream]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::No_death_scream);
		break;
	}

	switch (m_always_death_scream) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Always_death_scream])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Always_death_scream);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Always_death_scream]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Always_death_scream);
		break;
	}

	switch (m_nav_carry) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Navpoint_needslink])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Navpoint_needslink);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Navpoint_needslink]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Navpoint_needslink);
		break;
	}

	switch (m_hide_ship_name) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Hide_ship_name])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Hide_ship_name);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Hide_ship_name]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Hide_ship_name);
		break;
	}

	switch (m_disable_ets) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::No_ets])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::No_ets);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::No_ets]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::No_ets);
		break;
	}

	switch (m_cloaked) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Cloaked])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Cloaked);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Cloaked]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Cloaked);
		break;
	}

	switch (m_guardian) {
	case Qt::Checked:
		if (!(shipp->ship_guardian_threshold)) {
			set_modified();
		}
		shipp->ship_guardian_threshold = SHIP_GUARDIAN_THRESHOLD_DEFAULT;
		break;
	case Qt::Unchecked:
		if (shipp->ship_guardian_threshold) {
			set_modified();
		}
		shipp->ship_guardian_threshold = 0;
		break;
	}

	switch (m_vaporize) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Vaporize])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Vaporize);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Vaporize]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Vaporize);
		break;
	}

	switch (m_stealth) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Stealth])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Stealth);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Stealth]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Stealth);
		break;
	}

	switch (m_friendly_stealth_invisible) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Friendly_stealth_invis])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Friendly_stealth_invis);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Friendly_stealth_invis]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Friendly_stealth_invis);
		break;
	}

	switch (m_scramble_messages) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::Scramble_messages])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::Scramble_messages);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::Scramble_messages]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::Scramble_messages);
		break;
	}

	switch (m_no_collide) {
	case Qt::Checked:
		if (objp->flags[Object::Object_Flags::Collides]) {
			set_modified();
		}
		objp->flags.remove(Object::Object_Flags::Collides);
		break;
	case Qt::Unchecked:
		if (!(objp->flags[Object::Object_Flags::Collides])) {
			set_modified();
		}
		objp->flags.set(Object::Object_Flags::Collides);
		break;
	}

	switch (m_no_disabled_self_destruct) {
	case Qt::Checked:
		if (!(shipp->flags[Ship::Ship_Flags::No_disabled_self_destruct])) {
			set_modified();
		}
		shipp->flags.set(Ship::Ship_Flags::No_disabled_self_destruct);
		break;
	case Qt::Unchecked:
		if (shipp->flags[Ship::Ship_Flags::No_disabled_self_destruct]) {
			set_modified();
		}
		shipp->flags.remove(Ship::Ship_Flags::No_disabled_self_destruct);
		break;
	}

	shipp->respawn_priority = 0;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		shipp->respawn_priority = m_respawn_priority;
	}
}
ShipFlagsDialogModel::ShipFlagsDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

bool ShipFlagsDialogModel::apply()
{
	object* objp;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				update_ship(objp->instance);
			}
		}
		objp = GET_NEXT(objp);
	}

	return true;
}

void ShipFlagsDialogModel::reject() {}

void ShipFlagsDialogModel::setDestroyed(int state)
{
	modify(m_destroy, state);
}

int ShipFlagsDialogModel::getDestroyed()
{
	return m_destroy;
}

void ShipFlagsDialogModel::setDestroyedSeconds(int value)
{
	modify(m_destroy_value, value);
}

int ShipFlagsDialogModel::getDestroyedSeconds()
{
	return m_destroy_value;
}

void ShipFlagsDialogModel::setScannable(int state)
{
	modify(m_scannable, state);
}

int ShipFlagsDialogModel::getScannable()
{
	return m_scannable;
}

void ShipFlagsDialogModel::setCargoKnown(int state)
{
	modify(m_cargo_known, state);
}

int ShipFlagsDialogModel::getCargoKnown()
{
	return m_cargo_known;
}

void ShipFlagsDialogModel::setSubsystemScanning(int state)
{
	modify(m_toggle_subsystem_scanning, state);
}

int ShipFlagsDialogModel::getSubsystemScanning()
{
	return m_toggle_subsystem_scanning;
}

void ShipFlagsDialogModel::setReinforcment(int state)
{
	modify(m_reinforcement, state);
}

int ShipFlagsDialogModel::getReinforcment()
{
	return m_reinforcement;
}

void ShipFlagsDialogModel::setProtectShip(int state)
{
	modify(m_protect_ship, state);
}

int ShipFlagsDialogModel::getProtectShip()
{
	return m_protect_ship;
}

void ShipFlagsDialogModel::setBeamProtect(int state)
{
	modify(m_beam_protect_ship, state);
}

int ShipFlagsDialogModel::getBeamProtect()
{
	return m_beam_protect_ship;
}

void ShipFlagsDialogModel::setFlakProtect(int state)
{
	modify(m_flak_protect_ship, state);
}

int ShipFlagsDialogModel::getFlakProtect()
{
	return m_flak_protect_ship;
}

void ShipFlagsDialogModel::setLaserProtect(int state)
{
	modify(m_laser_protect_ship, state);
}

int ShipFlagsDialogModel::getLaserProtect()
{
	return m_laser_protect_ship;
}

void ShipFlagsDialogModel::setMissileProtect(int state)
{
	modify(m_missile_protect_ship, state);
}

int ShipFlagsDialogModel::getMissileProtect()
{
	return m_missile_protect_ship;
}

void ShipFlagsDialogModel::setIgnoreForGoals(int state)
{
	modify(m_ignore_count, state);
}

int ShipFlagsDialogModel::getIgnoreForGoals()
{
	return m_ignore_count;
}

void ShipFlagsDialogModel::setEscort(int state)
{
	modify(m_escort, state);
}

int ShipFlagsDialogModel::getEscort()
{
	return m_escort;
}

void ShipFlagsDialogModel::setEscortValue(int value)
{
	modify(m_escort_value, value);
}

int ShipFlagsDialogModel::getEscortValue()
{
	return m_escort_value;
}

void ShipFlagsDialogModel::setNoArrivalMusic(int state)
{
	modify(m_no_arrival_music, state);
}

int ShipFlagsDialogModel::getNoArrivalMusic()
{
	return m_no_arrival_music;
}

void ShipFlagsDialogModel::setInvulnerable(int state)
{
	modify(m_invulnerable, state);
}

int ShipFlagsDialogModel::getInvulnerable()
{
	return m_invulnerable;
}

void ShipFlagsDialogModel::setGuardianed(int state)
{
	modify(m_guardian, state);
}

int ShipFlagsDialogModel::getGuardianed()
{
	return m_guardian;
}

void ShipFlagsDialogModel::setPrimitiveSensors(int state)
{
	modify(m_primitive_sensors, state);
}

int ShipFlagsDialogModel::getPrimitiveSensors()
{
	return m_primitive_sensors;
}

void ShipFlagsDialogModel::setNoSubspaceDrive(int state)
{
	modify(m_no_subspace_drive, state);
}

int ShipFlagsDialogModel::getNoSubspaceDrive()
{
	return m_no_subspace_drive;
}

void ShipFlagsDialogModel::setHidden(int state)
{
	modify(m_hidden, state);
}

int ShipFlagsDialogModel::getHidden()
{
	return m_hidden;
}

void ShipFlagsDialogModel::setStealth(int state)
{
	modify(m_stealth, state);
}

int ShipFlagsDialogModel::getStealth()
{
	return m_stealth;
}

void ShipFlagsDialogModel::setFriendlyStealth(int state)
{
	modify(m_friendly_stealth_invisible, state);
}

int ShipFlagsDialogModel::getFriendlyStealth()
{
	return m_friendly_stealth_invisible;
}

void ShipFlagsDialogModel::setKamikaze(int state)
{
	modify(m_kamikaze, state);
}

int ShipFlagsDialogModel::getKamikaze()
{
	return m_kamikaze;
}

void ShipFlagsDialogModel::setKamikazeDamage(int value)
{
	modify(m_kdamage, value);
}

int ShipFlagsDialogModel::getKamikazeDamage()
{
	return m_kdamage;
}

void ShipFlagsDialogModel::setImmobile(int state)
{
	modify(m_immobile, state);
}

int ShipFlagsDialogModel::getImmobile()
{
	return m_immobile;
}

void ShipFlagsDialogModel::setNoDynamicGoals(int state)
{
	modify(m_no_dynamic, state);
}

int ShipFlagsDialogModel::getNoDynamicGoals()
{
	return m_no_dynamic;
}

void ShipFlagsDialogModel::setRedAlert(int state)
{
	modify(m_red_alert_carry, state);
}

int ShipFlagsDialogModel::getRedAlert()
{
	return m_red_alert_carry;
}

void ShipFlagsDialogModel::setGravity(int state)
{
	modify(m_affected_by_gravity, state);
}

int ShipFlagsDialogModel::getGravity()
{
	return m_affected_by_gravity;
}

void ShipFlagsDialogModel::setWarpin(int state)
{
	modify(m_special_warpin, state);
}

int ShipFlagsDialogModel::getWarpin()
{
	return m_special_warpin;
}

void ShipFlagsDialogModel::setTargetableAsBomb(int state)
{
	modify(m_targetable_as_bomb, state);
}

int ShipFlagsDialogModel::getTargetableAsBomb()
{
	return m_targetable_as_bomb;
}

void ShipFlagsDialogModel::setDisableBuiltInMessages(int state)
{
	modify(m_disable_messages, state);
}

int ShipFlagsDialogModel::getDisableBuiltInMessages()
{
	return m_disable_messages;
}

void ShipFlagsDialogModel::setNeverScream(int state)
{
	modify(m_no_death_scream, state);
}

int ShipFlagsDialogModel::getNeverScream()
{
	return m_no_death_scream;
}

void ShipFlagsDialogModel::setAlwaysScream(int state) {
	modify(m_always_death_scream, state);
}

int ShipFlagsDialogModel::getAlwaysScream()
{
	return m_always_death_scream;
}

void ShipFlagsDialogModel::setVaporize(int state)
{
	modify(m_vaporize, state);
}

int ShipFlagsDialogModel::getVaporize()
{
	return m_vaporize;
}

void ShipFlagsDialogModel::setRespawnPriority(int value)
{
	modify(m_respawn_priority, value);
}

int ShipFlagsDialogModel::getRespawnPriority()
{
	return m_respawn_priority;
}

void ShipFlagsDialogModel::setAutoCarry(int state)
{
	modify(m_nav_carry, state);
}

int ShipFlagsDialogModel::getAutoCarry()
{
	return m_nav_carry;
}

void ShipFlagsDialogModel::setAutoLink(int state)
{
	modify(m_nav_needslink, state);
}

int ShipFlagsDialogModel::getAutoLink()
{
	return m_nav_needslink;
}

void ShipFlagsDialogModel::setHideShipName(int state)
{
	modify(m_hide_ship_name, state);
}

int ShipFlagsDialogModel::getHideShipName()
{
	return m_hide_ship_name;
}

void ShipFlagsDialogModel::setClassDynamic(int state)
{
	modify(m_set_class_dynamically, state);
}

int ShipFlagsDialogModel::getClassDynamic()
{
	return m_set_class_dynamically;
}

void ShipFlagsDialogModel::setDisableETS(int state)
{
	modify(m_disable_ets, state);
}

int ShipFlagsDialogModel::getDisableETS()
{
	return m_disable_ets;
}

void ShipFlagsDialogModel::setCloak(int state)
{
	modify(m_cloaked, state);
}

int ShipFlagsDialogModel::getCloak()
{
	return m_cloaked;
}

void ShipFlagsDialogModel::setScrambleMessages(int state)
{
	modify(m_scramble_messages, state);
}

int ShipFlagsDialogModel::getScrambleMessages()
{
	return m_scramble_messages;
}

void ShipFlagsDialogModel::setNoCollide(int state)
{
	modify(m_no_collide, state);
}

int ShipFlagsDialogModel::getNoCollide()
{
	return m_no_collide;
}

void ShipFlagsDialogModel::setNoSelfDestruct(int state)
{
	modify(m_no_disabled_self_destruct, state);
}

int ShipFlagsDialogModel::getNoSelfDestruct()
{
	return m_no_disabled_self_destruct;
}

bool ShipFlagsDialogModel::query_modified()
{
	return _modified;
}

void ShipFlagsDialogModel::initializeData()
{
	object* objp;
	ship* shipp;
	int j, first;

	first = 1;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				shipp = &Ships[objp->instance];

				if (first) {
					first = 0;
					m_scannable = (shipp->flags[Ship::Ship_Flags::Scannable]) ? 2 : 0;
					m_red_alert_carry = (shipp->flags[Ship::Ship_Flags::Red_alert_store_status]) ? 2 : 0;
					m_special_warpin = (objp->flags[Object::Object_Flags::Special_warpin]) ? 2 : 0;
					m_protect_ship = (objp->flags[Object::Object_Flags::Protected]) ? 2 : 0;
					m_beam_protect_ship = (objp->flags[Object::Object_Flags::Beam_protected]) ? 2 : 0;
					m_flak_protect_ship = (objp->flags[Object::Object_Flags::Flak_protected]) ? 2 : 0;
					m_laser_protect_ship = (objp->flags[Object::Object_Flags::Laser_protected]) ? 2 : 0;
					m_missile_protect_ship = (objp->flags[Object::Object_Flags::Missile_protected]) ? 2 : 0;
					m_invulnerable = (objp->flags[Object::Object_Flags::Invulnerable]) ? 2 : 0;
					m_targetable_as_bomb = (objp->flags[Object::Object_Flags::Targetable_as_bomb]) ? 2 : 0;
					m_immobile = (objp->flags[Object::Object_Flags::Immobile]) ? 2 : 0;
					m_hidden = (shipp->flags[Ship::Ship_Flags::Hidden_from_sensors]) ? 2 : 0;
					m_primitive_sensors = (shipp->flags[Ship::Ship_Flags::Primitive_sensors]) ? 2 : 0;
					m_no_subspace_drive = (shipp->flags[Ship::Ship_Flags::No_subspace_drive]) ? 2 : 0;
					m_affected_by_gravity = (shipp->flags[Ship::Ship_Flags::Affected_by_gravity]) ? 2 : 0;
					m_toggle_subsystem_scanning = (shipp->flags[Ship::Ship_Flags::Toggle_subsystem_scanning]) ? 2 : 0;
					m_ignore_count = (shipp->flags[Ship::Ship_Flags::Ignore_count]) ? 2 : 0;
					m_no_arrival_music = (shipp->flags[Ship::Ship_Flags::No_arrival_music]) ? 2 : 0;
					m_cargo_known = (shipp->flags[Ship::Ship_Flags::Cargo_revealed]) ? 2 : 0;
					m_no_dynamic = (Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::No_dynamic]) ? 2 : 0;
					m_disable_messages = (shipp->flags[Ship::Ship_Flags::No_builtin_messages]) ? 2 : 0;
					m_set_class_dynamically = (shipp->flags[Ship::Ship_Flags::Set_class_dynamically]) ? 2 : 0;
					m_no_death_scream = (shipp->flags[Ship::Ship_Flags::No_death_scream]) ? 2 : 0;
					m_always_death_scream = (shipp->flags[Ship::Ship_Flags::Always_death_scream]) ? 2 : 0;
					m_guardian = (shipp->ship_guardian_threshold) ? 2 : 0;
					m_vaporize = (shipp->flags[Ship::Ship_Flags::Vaporize]) ? 2 : 0;
					m_stealth = (shipp->flags[Ship::Ship_Flags::Stealth]) ? 2 : 0;
					m_friendly_stealth_invisible = (shipp->flags[Ship::Ship_Flags::Friendly_stealth_invis]) ? 2 : 0;
					m_nav_carry = (shipp->flags[Ship::Ship_Flags::Navpoint_carry]) ? 2 : 0;
					m_nav_needslink = (shipp->flags[Ship::Ship_Flags::Navpoint_needslink]) ? 2 : 0;
					m_hide_ship_name = (shipp->flags[Ship::Ship_Flags::Hide_ship_name]) ? 2 : 0;
					m_disable_ets = (shipp->flags[Ship::Ship_Flags::No_ets]) ? 2 : 0;
					m_cloaked = (shipp->flags[Ship::Ship_Flags::Cloaked]) ? 2 : 0;
					m_scramble_messages = (shipp->flags[Ship::Ship_Flags::Scramble_messages]) ? 2 : 0;
					m_no_collide = (objp->flags[Object::Object_Flags::Collides]) ? 0 : 2;
					m_no_disabled_self_destruct = (shipp->flags[Ship::Ship_Flags::No_disabled_self_destruct]) ? 2 : 0;

					m_destroy = (shipp->flags[Ship::Ship_Flags::Kill_before_mission]) ? 2 : 0;
					m_destroy_value = shipp->final_death_time;

					m_kamikaze = (Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::Kamikaze]) ? 2 : 0;
					m_kdamage = Ai_info[shipp->ai_index].kamikaze_damage;

					m_escort = (shipp->flags[Ship::Ship_Flags::Escort]) ? 2 : 0;
					m_escort_value = shipp->escort_priority;

					if (The_mission.game_type & MISSION_TYPE_MULTI) {
						m_respawn_priority = shipp->respawn_priority;
					}

					for (j = 0; j < Num_reinforcements; j++) {
						if (!stricmp(Reinforcements[j].name, shipp->ship_name)) {
							break;
						}
					}

					m_reinforcement = (j < Num_reinforcements) ? 1 : 0;

				} else {

					m_scannable = tristate_set(shipp->flags[Ship::Ship_Flags::Scannable], m_scannable);
					m_red_alert_carry =
						tristate_set(shipp->flags[Ship::Ship_Flags::Red_alert_store_status], m_red_alert_carry);
					m_special_warpin =
						tristate_set(objp->flags[Object::Object_Flags::Special_warpin], m_special_warpin);
					m_protect_ship = tristate_set(objp->flags[Object::Object_Flags::Protected], m_protect_ship);
					m_beam_protect_ship =
						tristate_set(objp->flags[Object::Object_Flags::Beam_protected], m_beam_protect_ship);
					m_flak_protect_ship =
						tristate_set(objp->flags[Object::Object_Flags::Flak_protected], m_flak_protect_ship);
					m_laser_protect_ship =
						tristate_set(objp->flags[Object::Object_Flags::Laser_protected], m_laser_protect_ship);
					m_missile_protect_ship =
						tristate_set(objp->flags[Object::Object_Flags::Missile_protected], m_missile_protect_ship);
					m_invulnerable = tristate_set(objp->flags[Object::Object_Flags::Invulnerable], m_invulnerable);
					m_targetable_as_bomb =
						tristate_set(objp->flags[Object::Object_Flags::Targetable_as_bomb], m_targetable_as_bomb);
					m_immobile = tristate_set(objp->flags[Object::Object_Flags::Immobile], m_immobile);
					m_hidden = tristate_set(shipp->flags[Ship::Ship_Flags::Hidden_from_sensors], m_hidden);
					m_primitive_sensors =
						tristate_set(shipp->flags[Ship::Ship_Flags::Primitive_sensors], m_primitive_sensors);
					m_no_subspace_drive =
						tristate_set(shipp->flags[Ship::Ship_Flags::No_subspace_drive], m_no_subspace_drive);
					m_affected_by_gravity =
						tristate_set(shipp->flags[Ship::Ship_Flags::Affected_by_gravity], m_affected_by_gravity);
					m_toggle_subsystem_scanning =
						tristate_set(shipp->flags[Ship::Ship_Flags::Toggle_subsystem_scanning],
							m_toggle_subsystem_scanning);
					m_ignore_count = tristate_set(shipp->flags[Ship::Ship_Flags::Ignore_count], m_ignore_count);
					m_no_arrival_music =
						tristate_set(shipp->flags[Ship::Ship_Flags::No_arrival_music], m_no_arrival_music);
					m_cargo_known = tristate_set(shipp->flags[Ship::Ship_Flags::Cargo_revealed], m_cargo_known);
					m_no_dynamic =
						tristate_set(Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::No_dynamic], m_no_dynamic);
					m_disable_messages =
						tristate_set(shipp->flags[Ship::Ship_Flags::No_builtin_messages], m_disable_messages);
					m_set_class_dynamically =
						tristate_set(shipp->flags[Ship::Ship_Flags::Set_class_dynamically], m_set_class_dynamically);
					m_no_death_scream =
						tristate_set(shipp->flags[Ship::Ship_Flags::No_death_scream], m_no_death_scream);
					m_always_death_scream =
						tristate_set(shipp->flags[Ship::Ship_Flags::Always_death_scream], m_always_death_scream);
					m_guardian = tristate_set(shipp->ship_guardian_threshold, m_guardian);
					m_vaporize = tristate_set(shipp->flags[Ship::Ship_Flags::Vaporize], m_vaporize);
					m_stealth = tristate_set(shipp->flags[Ship::Ship_Flags::Stealth], m_stealth);
					m_friendly_stealth_invisible = tristate_set(shipp->flags[Ship::Ship_Flags::Friendly_stealth_invis],
						m_friendly_stealth_invisible);
					m_nav_carry = tristate_set(shipp->flags[Ship::Ship_Flags::Navpoint_carry], m_nav_carry);
					m_nav_needslink = tristate_set(shipp->flags[Ship::Ship_Flags::Navpoint_needslink], m_nav_needslink);
					m_hide_ship_name = tristate_set(shipp->flags[Ship::Ship_Flags::Hide_ship_name], m_hide_ship_name);
					m_disable_ets = tristate_set(shipp->flags[Ship::Ship_Flags::No_ets], m_disable_ets);
					m_cloaked = tristate_set(shipp->flags[Ship::Ship_Flags::Cloaked], m_cloaked);
					m_scramble_messages =
						tristate_set(shipp->flags[Ship::Ship_Flags::Scramble_messages], m_scramble_messages);
					m_no_collide = tristate_set(!(objp->flags[Object::Object_Flags::Collides]), m_no_collide);
					m_no_disabled_self_destruct =
						tristate_set(shipp->flags[Ship::Ship_Flags::No_disabled_self_destruct],
							m_no_disabled_self_destruct);

					// check the final death time and set the internal variable according to whether or not
					// the final_death_time is set.  Also, the value in the edit box must be set if all the
					// values are the same, and cleared if the values are not the same.
					m_destroy = tristate_set(shipp->flags[Ship::Ship_Flags::Kill_before_mission], m_destroy);
					m_destroy_value = shipp->final_death_time;

					m_kamikaze = tristate_set(Ai_info[shipp->ai_index].ai_flags[AI::AI_Flags::Kamikaze], m_kamikaze);
					m_kdamage = Ai_info[shipp->ai_index].kamikaze_damage;

					m_escort = tristate_set(shipp->flags[Ship::Ship_Flags::Escort], m_escort);
					m_escort_value = shipp->escort_priority;

					if (The_mission.game_type & MISSION_TYPE_MULTI) {
						m_respawn_priority = shipp->escort_priority;
					}

					for (j = 0; j < Num_reinforcements; j++) {
						if (!stricmp(Reinforcements[j].name, shipp->ship_name)) {
							break;
						}
					}
					m_reinforcement = tristate_set(j < Num_reinforcements, m_reinforcement);

					;
				}
			}
		}

		objp = GET_NEXT(objp);
	}
}
} // namespace dialogs
} // namespace fred
} // namespace fso
